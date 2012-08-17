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

#include <couchdb-glib.h>

int
main (int argc, char *argv[])
{
	CouchdbSession *couchdb;
	GSList *dblist, *sl;
	GError *error = NULL;


	g_type_init ();
	g_thread_init (NULL);

	/* Initialize Couchdb */
	couchdb =  argc > 1 ? couchdb_session_new (argv[1]) : couchdb_session_new (NULL);
	g_printf ("Connecting to Couchdb at %s\n", couchdb_session_get_uri (couchdb));
	
	if (!couchdb) {
		g_print ("Could not create Couchdb object\n");
		return -1;
	}

	dblist = couchdb_session_list_databases (couchdb, &error);
	if (error != NULL) {
		g_critical ("Error listing databases: %s", error->message);
		g_error_free (error);
	}
	
	for (sl = dblist; sl != NULL; sl = sl->next) {
		CouchdbDatabaseInfo *dbinfo;
		GSList *doclist;

		error = NULL;
		g_print ("Found database %s\n", (const char *) sl->data);
		dbinfo = couchdb_session_get_database_info (couchdb, (const char *) sl->data, &error);
		if (dbinfo) {
			g_print ("\tDatabase name: %s\n",
				 couchdb_database_info_get_dbname (dbinfo));
			g_print ("\t# of documents: %d\n",
				 couchdb_database_info_get_documents_count (dbinfo));
			g_print ("\t# of deleted documents: %d\n",
				 couchdb_database_info_get_deleted_documents_count (dbinfo));
			g_print ("\tUpdate sequence: %d\n",
				 couchdb_database_info_get_update_sequence (dbinfo));
			g_print ("\tCompact running?: %s\n",
				 couchdb_database_info_is_compact_running (dbinfo) ? "True" : "False");
			g_print ("\tDisk size: %d\n",
				 couchdb_database_info_get_disk_size (dbinfo));

			couchdb_database_info_unref (dbinfo);
		} else {
			g_print ("Could not retrieve info for database %s\n", (const char *) sl->data);
		}

		/* now, get list of documents */
		error = NULL;
		doclist = couchdb_session_list_documents (couchdb, (const char *) sl->data, &error);
		if (doclist) {
			GSList *sl_doc;

			g_print ("\tDocuments:\n");
			for (sl_doc = doclist; sl_doc != NULL; sl_doc = sl_doc->next) {
				CouchdbDocumentInfo *doc_info = sl_doc->data;
				CouchdbDocument *document;

				error = NULL;
				document = couchdb_document_get (couchdb,
								 (const char *) sl->data,
								 couchdb_document_info_get_docid (doc_info),
								 &error);
				if (document) {
					char *json;

					g_print ("\t\t%s\t(rev: %s)\n",
						 couchdb_document_get_id (document),
						 couchdb_document_get_revision (document));

					json = couchdb_document_to_string (document);
					g_print ("\t\t\t%s\n", json);
					g_free (json);

					g_object_unref (G_OBJECT (document));
				} else {
					g_print ("\t\t%s\t(rev: %s)\n",
						 couchdb_document_info_get_docid (doc_info),
						 couchdb_document_info_get_revision (doc_info));
				}
			}
		} else {
			g_print ("\tNo documents\n");
		}
	}

	couchdb_session_free_database_list (dblist);
	g_object_unref (G_OBJECT (couchdb));

	return 0;
}
