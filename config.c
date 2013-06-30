/*
 * Copyright 2013 Mo McRoberts.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "p_libsupport.h"

static void config_thread_init_(void);
static const char *config_get_unlocked_(const char *key, const char *defval);

static pthread_once_t config_control = PTHREAD_ONCE_INIT;
static pthread_rwlock_t config_lock;
	
static dictionary *defaults;
static dictionary *overrides;
static dictionary *config;

int
config_init(int (*defaults_cb)(void))
{
	pthread_once(&config_control, config_thread_init_);
	pthread_rwlock_wrlock(&config_lock);
	/* Defaults are the values used if no value is specified in the
	 * configuration file.
	 */
	defaults = dictionary_new(0);
	if(!defaults)
	{
		pthread_rwlock_unlock(&config_lock);
		return -1;
	}
	/* Overrides are the values used regardless of defaults or the
	 * the values present in the configuration file; the overrides
	 * dictionary only exists until the configuration is loaded: after
	 * that point, setting an override is simply a case of replacing
	 * a value in the config dictionary.
	 */
	overrides = dictionary_new(0);
	if(!overrides)
	{
		pthread_rwlock_unlock(&config_lock);
		return -1;
	}
	pthread_rwlock_unlock(&config_lock);
	/* If a callback was specified to populate defaults, invoke it */
	if(defaults_cb)
	{
		if(defaults_cb())
		{
			return -1;
		}
	}
	return 0;
}

int
config_load(const char *default_path)
{
	int n;
	const char *file;
	
	pthread_once(&config_control, config_thread_init_);
	pthread_rwlock_wrlock(&config_lock);
	file = config_get_unlocked_("global:configFile", default_path);
	config = iniparser_load(file);
	if(!config)
	{
		pthread_rwlock_unlock(&config_lock);
		return -1;
	}
	for(n = 0; n < overrides->n; n++)
	{
		iniparser_set(config, overrides->key[n], overrides->val[n]);
	}
	dictionary_del(overrides);
	overrides = NULL;
	pthread_rwlock_unlock(&config_lock);
	return 0;
}

int
config_set(const char *key, const char *value)
{
	pthread_once(&config_control, config_thread_init_);
	pthread_rwlock_wrlock(&config_lock);
	iniparser_set((overrides ? overrides : config), key, value);
	pthread_rwlock_unlock(&config_lock);
	return 0;
}

int
config_set_default(const char *key, const char *value)
{
	pthread_once(&config_control, config_thread_init_);
	pthread_rwlock_wrlock(&config_lock);
	if(defaults)
	{
		iniparser_set(defaults, key, value);
	}
	else if(!iniparser_getstring(config, key, NULL))
	{
		iniparser_set(config, key, value);
	}
	pthread_rwlock_unlock(&config_lock);
	return 0;
}

size_t
config_get(const char *key, const char *defval, char *buf, size_t bufsize)
{
	const char *ret;
	size_t r;
	
	pthread_once(&config_control, config_thread_init_);
	if(buf && bufsize)
	{
		*buf = 0;
	}
	pthread_rwlock_rdlock(&config_lock);
	ret = config_get_unlocked_(key, defval);
	if(ret)
	{
		r = strlen(ret) + 1;			
	}
	else
	{
		r = 1;
	}	
	if(!buf || !bufsize)
	{
		pthread_rwlock_unlock(&config_lock);
		return r;
	}
	strncpy(buf, ret, bufsize);
	buf[bufsize - 1] = 0;
	pthread_rwlock_unlock(&config_lock);
	return r;
}

int
config_get_int(const char *key, int defval)
{
	const char *s;
	int i;
	
	pthread_once(&config_control, config_thread_init_);
	pthread_rwlock_rdlock(&config_lock);
	s = config_get_unlocked_(key, NULL);
	if(s)
	{
		i = atoi(s);
	}
	else
	{
		i = defval;
	}
	pthread_rwlock_unlock(&config_lock);
	return i;
}

int
config_get_bool(const char *key, int defval)
{
	const char *s;
	int c, r;
	
	pthread_once(&config_control, config_thread_init_);
	pthread_rwlock_rdlock(&config_lock);
	s = config_get_unlocked_(key, NULL);
	r = defval;
	if(s)
	{
		c = toupper(s[0]);
		if(c == 'Y' || c == 'T' || c == '1')
		{
			r = -1;
		}
		else
		{
			r = atoi(s);
		}
	}
	pthread_rwlock_unlock(&config_lock);
	return r ? -1 : 0;
}

static void
config_thread_init_(void)
{
	pthread_rwlock_init(&config_lock, NULL);
}

static const char *
config_get_unlocked_(const char *key, const char *defval)
{
	if(config)
	{
		return iniparser_getstring(config, key, iniparser_getstring(defaults, key, (char *) defval));
	}
	return iniparser_getstring(overrides, key, iniparser_getstring(defaults, key, (char *) defval));
}