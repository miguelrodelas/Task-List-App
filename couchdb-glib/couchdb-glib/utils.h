/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2009 Canonical Services Ltd (www.canonical.com)
 *
 * Authors: Rodrigo Moya <rodrigo.moya@canonical.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <glib.h>
#include <json-glib/json-glib.h>
#include "config.h"
#include "couchdb-session.h"

#ifndef DEBUG_MESSAGES
#undef g_debug
#define g_debug(...)
#endif

#define COUCHDB_ERROR couchdb_error_quark()
GQuark      couchdb_error_quark (void);

char* generate_uuid (void);

/* Private API */
JsonObject*	  couchdb_document_get_json_object	(CouchdbDocument *document);

CouchdbArrayField  *couchdb_array_field_new_from_json_array (JsonArray *json_array);
JsonArray          *couchdb_array_field_get_json_array (CouchdbArrayField *array);

CouchdbStructField *couchdb_struct_field_new_from_json_object (JsonObject *json_object);
JsonObject         *couchdb_struct_field_get_json_object (CouchdbStructField *sf);

#endif
