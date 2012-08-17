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

#include "couchdb-document-info.h"

struct _CouchdbDocumentInfo {
	gint ref_count;

	char *docid;
	char *revision;
};

/*
 * CouchdbDocumentInfo object
 */

GType
couchdb_document_info_get_type (void)
{
	static GType object_type = 0;

	if (G_UNLIKELY (!object_type))
		object_type = g_boxed_type_register_static (g_intern_static_string ("CouchdbDocumentInfo"),
							    (GBoxedCopyFunc) couchdb_document_info_ref,
							    (GBoxedFreeFunc) couchdb_document_info_unref);

	return object_type;
}

/**
 * couchdb_document_info_new:
 * @docid: Unique ID of the document
 * @revision: Current revision of the document
 *
 * Create a new #CouchdbDocumentInfo object, which is used to store information
 * about a document on a CouchDB database. It contains information like the
 * Unique ID and revision of the document.
 *
 * Return value: A newly-created #CouchDocumentInfo object.
 */
CouchdbDocumentInfo *
couchdb_document_info_new (const char *docid, const char *revision)
{
	CouchdbDocumentInfo *doc_info;

	doc_info = g_slice_new (CouchdbDocumentInfo);
	doc_info->ref_count = 1;
	doc_info->docid = g_strdup (docid);
	doc_info->revision = g_strdup (revision);

	return doc_info;
}

/**
 * couchdb_document_info_ref:
 * @doc_info: A #CouchdbDocumentInfo object
 *
 * Increments reference counting of the given #CouchdbDocumentInfo object.
 *
 * Return value: A pointer to the object being referenced.
 */
CouchdbDocumentInfo *
couchdb_document_info_ref (CouchdbDocumentInfo *doc_info)
{
	g_return_val_if_fail (doc_info != NULL, NULL);
	g_return_val_if_fail (doc_info->ref_count > 0, NULL);

	g_atomic_int_exchange_and_add (&doc_info->ref_count, 1);

	return doc_info;
}

/**
 * couchdb_document_info_unref:
 * @doc_info: A #CouchdbDocumentInfo object
 *
 * Decrements reference counting of the given #CouchdbDocumentInfo object.
 * When the reference count is equal to 0, the object will be destroyed.
 */
void
couchdb_document_info_unref (CouchdbDocumentInfo *doc_info)
{
	gint old_ref;

	g_return_if_fail (doc_info != NULL);
	g_return_if_fail (doc_info->ref_count > 0);

	old_ref = g_atomic_int_get (&doc_info->ref_count);
	if (old_ref > 1)
		g_atomic_int_compare_and_exchange (&doc_info->ref_count, old_ref, old_ref - 1);
	else {
		g_free (doc_info->docid);
		g_free (doc_info->revision);
		g_slice_free (CouchdbDocumentInfo, doc_info);
	}
}

/**
 * couchdb_document_info_get_docid:
 * @doc_info: A #CouchdbDocumentInfo object
 *
 * Get the unique ID stored in the #CouchdbDocumentInfo object.
 *
 * Return value: Unique ID.
 */
const char *
couchdb_document_info_get_docid (CouchdbDocumentInfo *doc_info)
{
	g_return_val_if_fail (doc_info != NULL, NULL);

	return (const char *) doc_info->docid;
}

/**
 * couchdb_document_info_get_revision:
 * @doc_info: A #CouchdbDocumentInfo object
 *
 * Get the revision stored in the #CouchdbDocumentInfo object.
 *
 * Return value: Revision number.
 */
const char *
couchdb_document_info_get_revision (CouchdbDocumentInfo *doc_info)
{
	g_return_val_if_fail (doc_info != NULL, NULL);

	return (const char *) doc_info->revision;
}

