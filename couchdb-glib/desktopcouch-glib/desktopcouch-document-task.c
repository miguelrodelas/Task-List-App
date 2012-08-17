/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2009 Canonical Services Ltd (www.canonical.com)
 *
 * Authors: Miguel Angel Rodelas Delgado <miguel.rodelas@gmail.com>
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

#include "desktopcouch-document-task.h"
#include "utils.h"

CouchdbDocument *
desktopcouch_document_task_new (CouchdbSession *couchdb)
{
	CouchdbDocument *document;

	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), NULL);

	document = couchdb_document_new (couchdb);
	if (document)
		desktopcouch_document_set_record_type (document, DESKTOPCOUCH_RECORD_TYPE_TASK);

	return document;
}

gboolean
desktopcouch_document_is_task (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), FALSE);

	return !g_ascii_strcasecmp (desktopcouch_document_get_record_type (document),
				    DESKTOPCOUCH_RECORD_TYPE_TASK);
}

const char *
desktopcouch_document_task_get_summary (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_task (document), NULL);

	return couchdb_document_get_string_field (document, "summary");
}

void
desktopcouch_document_task_set_summary (CouchdbDocument *document, const char *summary)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_task (document));
	g_return_if_fail (summary != NULL);

	couchdb_document_set_string_field (document, "summary", summary);
}

const char *
desktopcouch_document_task_get_description (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_task (document), NULL);

	return couchdb_document_get_string_field (document, "description");
}

void
desktopcouch_document_task_set_description (CouchdbDocument *document, const char *description)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_task (document));
	g_return_if_fail (description != NULL);

	couchdb_document_set_string_field (document, "description", description);
}
