/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2009 Canonical Services Ltd (www.canonical.com)
 *               2009 Mikkel Kamstrup Erlandsen
 *
 * Authors: Rodrigo Moya <rodrigo.moya@canonical.com>
 *          Mikkel Kamstrup Erlandsen <mikkel.kamstrup@gmail.com>
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

#ifndef __COUCHDB_STRUCT_FIELD_H__
#define __COUCHDB_STRUCT_FIELD_H__

#include <glib.h>
#include <glib-object.h>
#include "couchdb-array-field.h"
#include "couchdb-session.h"
#include "couchdb-types.h"

G_BEGIN_DECLS

#define COUCHDB_TYPE_STRUCT_FIELD  (couchdb_struct_field_get_type ())

GType               couchdb_struct_field_get_type (void);
CouchdbStructField *couchdb_struct_field_new (void);
CouchdbStructField *couchdb_struct_field_new_from_string (const char *str);
CouchdbStructField *couchdb_struct_field_ref (CouchdbStructField *sf);
void                couchdb_struct_field_unref (CouchdbStructField *sf);

gboolean            couchdb_struct_field_has_field (CouchdbStructField *sf, const char *field);
void                couchdb_struct_field_remove_field (CouchdbStructField *sf, const char *field);

CouchdbArrayField  *couchdb_struct_field_get_array_field (CouchdbStructField *sf, const char *field);
void                couchdb_struct_field_set_array_field (CouchdbStructField *sf, const char *field, CouchdbArrayField *value);
gboolean            couchdb_struct_field_get_boolean_field (CouchdbStructField *sf, const char *field);
void                couchdb_struct_field_set_boolean_field (CouchdbStructField *sf, const char *field, gboolean value);
gdouble             couchdb_struct_field_get_double_field (CouchdbStructField *sf, const char *field);
void                couchdb_struct_field_set_double_field (CouchdbStructField *sf, const char *field, gdouble value);
gint                couchdb_struct_field_get_int_field (CouchdbStructField *sf, const char *field);
void                couchdb_struct_field_set_int_field (CouchdbStructField *sf, const char *field, gint value);
const char         *couchdb_struct_field_get_string_field (CouchdbStructField *sf, const char *field);
void                couchdb_struct_field_set_string_field (CouchdbStructField *sf, const char *field, const char *value);
CouchdbStructField *couchdb_struct_field_get_struct_field (CouchdbStructField *sf, const char *field);
void                couchdb_struct_field_set_struct_field (CouchdbStructField *sf, const char *field, CouchdbStructField *value);

const char         *couchdb_struct_field_get_uuid (CouchdbStructField *sf);
void                couchdb_struct_field_set_uuid (CouchdbStructField *sf, const char *uuid);

char               *couchdb_struct_field_to_string (CouchdbStructField *sf);

G_END_DECLS

#endif /* __COUCHDB_STRUCT_FIELD__ */
