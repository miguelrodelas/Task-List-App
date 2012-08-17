/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2010 Canonical Services Ltd (www.canonical.com)
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

#include "desktopcouch-document.h"

/**
 * desktopcouch_document_get_record_type:
 * @document: A #CouchdbDocument object
 *
 * Retrieve the record type of the given document. Record types are special
 * fields in the CouchDB documents, used in DesktopCouch, to identify
 * standard records. All supported record types are listed at
 * http://www.freedesktop.org/wiki/Specifications/desktopcouch#Formats.
 *
 * Return value: The record type of the given document.
 */
const char *
desktopcouch_document_get_record_type (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);

	return couchdb_document_get_string_field (document, "record_type");
}

/**
 * desktopcouch_document_set_record_type:
 * @document: A #CouchdbDocument object
 * @record_type: Record type to set the document to
 *
 * Set the record type of the given document.
 */
void
desktopcouch_document_set_record_type (CouchdbDocument *document, const char *record_type)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (record_type != NULL);

	couchdb_document_set_string_field (document, "record_type", record_type);
}

/**
 * desktopcouch_document_get_application_annotations:
 * @document: A #CouchdbDocument object
 *
 * Retrieve the application annotations for the given document.
 *
 * Application annotations is a special field (named "application_annotations"), used
 * in desktopcouch to allow applications to set values on standard documents (as defined
 * at http://www.freedesktop.org/wiki/Specifications/desktopcouch#Formats) that are
 * not part of the standard, but still needed by the application.
 *
 * Return value: A #CouchdbStructField containing the value of the application
 * annotations for the given document.
 */
CouchdbStructField *
desktopcouch_document_get_application_annotations (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);

	return couchdb_document_get_struct_field (document, "application_annotations");
}

/**
 * desktopcouch_document_set_application_annotations:
 * @document: A #CouchdbDocument object
 * @annotations: A #CouchdbStructField with the contents of the application_annotations field.
 *
 * Set the application annotations for the given document.
 */
void
desktopcouch_document_set_application_annotations (CouchdbDocument *document, CouchdbStructField *annotations)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));

	couchdb_document_set_struct_field (document, "application_annotations", annotations);
}
