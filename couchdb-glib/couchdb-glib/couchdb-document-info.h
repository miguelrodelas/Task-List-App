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

#ifndef __COUCHDB_DOCUMENT_INFO_H__
#define __COUCHDB_DOCUMENT_INFO_H__

#include <glib.h>
#include <glib-object.h>
#include "couchdb-types.h"

G_BEGIN_DECLS

#define COUCHDB_TYPE_DOCUMENT_INFO (couchdb_document_info_get_type ())

GType                couchdb_document_info_get_type (void);
CouchdbDocumentInfo *couchdb_document_info_new (const char *docid, const char *revision);
CouchdbDocumentInfo *couchdb_document_info_ref (CouchdbDocumentInfo *dbinfo);
void                 couchdb_document_info_unref (CouchdbDocumentInfo *dbinfo);

const char          *couchdb_document_info_get_docid (CouchdbDocumentInfo *doc_info);
const char          *couchdb_document_info_get_revision (CouchdbDocumentInfo *doc_info);

G_END_DECLS

#endif /* __COUCHDB_DOCUMENT_INFO_H__ */
