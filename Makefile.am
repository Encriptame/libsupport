## Copyright 2013 Mo McRoberts.
##
##  Licensed under the Apache License, Version 2.0 (the "License");
##  you may not use this file except in compliance with the License.
##  You may obtain a copy of the License at
##
##      http://www.apache.org/licenses/LICENSE-2.0
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##

EXTRA_DIST = PROVENANCE LICENSE-2.0

noinst_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES = \
	libsupport.h p_libsupport.h \
	config.c log.c \
    iniparser/src/dictionary.h \
    iniparser/src/dictionary.c \
    iniparser/src/iniparser.h \
    iniparser/src/iniparser.c

libsupport_la_CPPFLAGS = @AM_CPPFLAGS@ @CPPFLAGS@ -I$(srcdir)/iniparser/src
