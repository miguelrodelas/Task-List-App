/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2009-2010 Canonical Services Ltd (www.canonical.com)
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

#ifndef __COUCHDB_ARRAY_FIELD_H__
#define __COUCHDB_ARRAY_FIELD_H__

#include <glib-object.h>
#include "couchdb-types.h"

G_BEGIN_DECLS

#define COUCHDB_TYPE_ARRAY_FIELD (couchdb_array_field_get_type ())

GType               couchdb_array_field_get_type (void);
CouchdbArrayField  *couchdb_array_field_new (void);
CouchdbArrayField  *couchdb_array_field_ref (CouchdbArrayField *array);
void                couchdb_array_field_unref (CouchdbArrayField *array);

guint               couchdb_array_field_get_length (CouchdbArrayField *array);
void                couchdb_array_field_add_array_element (CouchdbArrayField *array, const CouchdbArrayField *value);
void                couchdb_array_field_add_boolean_element (CouchdbArrayField *array, gboolean value);
void                couchdb_array_field_add_double_element (CouchdbArrayField *array, gdouble value);
void                couchdb_array_field_add_int_element (CouchdbArrayField *array, gint value);
void                couchdb_array_field_add_string_element (CouchdbArrayField *array, const gchar *value);
void                couchdb_array_field_add_struct_element (CouchdbArrayField *array, const CouchdbStructField *value);
void                couchdb_array_field_remove_element (CouchdbArrayField *array, guint index);

CouchdbArrayField  *couchdb_array_field_get_array_element (CouchdbArrayField *array, guint index);
gboolean            couchdb_array_field_get_boolean_element (CouchdbArrayField *array, guint index);
gdouble             couchdb_array_field_get_double_element (CouchdbArrayField *array, guint index);
gint                couchdb_array_field_get_int_element (CouchdbArrayField *array, guint index);
const gchar        *couchdb_array_field_get_string_element (CouchdbArrayField *array, guint index);
CouchdbStructField *couchdb_array_field_get_struct_element (CouchdbArrayField *array, guint index);

G_END_DECLS

#endif
