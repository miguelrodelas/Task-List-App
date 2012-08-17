/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2009-2010 Canonical Services Ltd (www.canonical.com)
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

#ifndef __COUCHDB_DOCUMENT_H__
#define __COUCHDB_DOCUMENT_H__

#include <glib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include "couchdb-types.h"
#include "couchdb-array-field.h"
#include "couchdb-struct-field.h"

G_BEGIN_DECLS

#define COUCHDB_TYPE_DOCUMENT                (couchdb_document_get_type ())
#define COUCHDB_DOCUMENT(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), COUCHDB_TYPE_DOCUMENT, CouchdbDocument))
#define COUCHDB_IS_DOCUMENT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), COUCHDB_TYPE_DOCUMENT))
#define COUCHDB_DOCUMENT_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), COUCHDB_TYPE_DOCUMENT, CouchdbDocumentClass))
#define COUCHDB_IS_DOCUMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), COUCHDB_TYPE_DOCUMENT))
#define COUCHDB_DOCUMENT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), COUCHDB_TYPE_DOCUMENT, CouchdbDocumentClass))

typedef struct {
	GObjectClass parent_class;
} CouchdbDocumentClass;


GType            couchdb_document_get_type (void);

CouchdbDocument *couchdb_document_new (CouchdbSession  *couchdb);
CouchdbDocument *couchdb_document_get (CouchdbSession  *couchdb,
				       const char      *dbname,
				       const char      *docid,
				       GError          **error);
gboolean         couchdb_document_put (CouchdbDocument *document,
				       const char      *dbname,
				       GError **error);
gboolean         couchdb_document_delete (CouchdbDocument *document, GError **error);

const char      *couchdb_document_get_id (CouchdbDocument *document);
void             couchdb_document_set_id (CouchdbDocument *document, const char *id);
const char      *couchdb_document_get_revision (CouchdbDocument *document);
void             couchdb_document_set_revision (CouchdbDocument *document, const char *revision);

gboolean         couchdb_document_has_field (CouchdbDocument *document, const char *field);
void             couchdb_document_remove_field (CouchdbDocument *document, const char *field);

CouchdbArrayField  *couchdb_document_get_array_field (CouchdbDocument *document, const char *field);
void                couchdb_document_set_array_field (CouchdbDocument *document, const char *field, CouchdbArrayField *value);
gboolean            couchdb_document_get_boolean_field (CouchdbDocument *document, const char *field);
void                couchdb_document_set_boolean_field (CouchdbDocument *document, const char *field, gboolean value);
gint                couchdb_document_get_int_field (CouchdbDocument *document, const char *field);
void                couchdb_document_set_int_field (CouchdbDocument *document, const char *field, gint value);
gdouble             couchdb_document_get_double_field (CouchdbDocument *document, const char *field);
void                couchdb_document_set_double_field (CouchdbDocument *document, const char *field, gdouble value);
const char         *couchdb_document_get_string_field (CouchdbDocument *document, const char *field);
void                couchdb_document_set_string_field (CouchdbDocument *document, const char *field, const char *value);
CouchdbStructField *couchdb_document_get_struct_field (CouchdbDocument *document, const char *field);
void                couchdb_document_set_struct_field (CouchdbDocument *document, const char *field, CouchdbStructField *value);
      
char             *couchdb_document_to_string (CouchdbDocument *document);

G_END_DECLS

#endif /* __COUCHDB_DOCUMENT_H__ */
