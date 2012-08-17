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
#include <utils.h>

static CouchdbSession *couchdb;

static void
test_list_databases (void)
{
	GError *error = NULL;
	GSList *dblist;

	dblist = couchdb_session_list_databases (couchdb, &error);
	if (error != NULL) {
		/* A critical will abort the test case */
		g_critical ("Error listing databases: %s", error->message);
		g_error_free (error);
		error = NULL;		
	}

	while (dblist != NULL) {
		CouchdbDatabaseInfo *dbinfo;
		GSList *doclist;

		error = NULL;
		dbinfo = couchdb_session_get_database_info (couchdb, (const char *) dblist->data, &error);
		g_assert (error == NULL);
		g_assert (dbinfo != NULL);
		g_assert (couchdb_database_info_get_dbname (dbinfo) != NULL);

		/* Get list of documents to compare against couchdb_database_info_get_documents_count */
		error = NULL;
		doclist = couchdb_session_list_documents (couchdb, (const char *) dblist->data, &error);
		g_assert (error == NULL);
		g_assert (g_slist_length (doclist) == couchdb_database_info_get_documents_count (dbinfo));
		if (doclist)
			couchdb_session_free_document_list (doclist);

		dblist = g_slist_remove (dblist, dblist->data);
		couchdb_database_info_unref (dbinfo);
	}
}

static void
test_list_documents (void)
{
	GError *error = NULL;
	GSList *dblist;

	dblist = couchdb_session_list_databases (couchdb, &error);
	g_assert (error == NULL);

	while (dblist != NULL) {
		GSList *doclist;

		error = NULL;
		doclist = couchdb_session_list_documents (couchdb, (const char *) dblist->data, &error);
		g_assert (error == NULL);

		while (doclist) {
			CouchdbDocumentInfo *doc_info = doclist->data;
			CouchdbDocument *document;
			char *str;

			error = NULL;
			document = couchdb_document_get (couchdb, (const char *) dblist->data,
							 couchdb_document_info_get_docid (doc_info),
							 &error);
			g_assert (error == NULL);
			g_assert (document != NULL);
			g_assert (g_strcmp0 (couchdb_document_info_get_docid (doc_info), couchdb_document_get_id (document)) == 0);
			g_assert (g_strcmp0 (couchdb_document_info_get_revision (doc_info), couchdb_document_get_revision (document)) == 0);

			str = couchdb_document_to_string (document);
			g_assert (str != NULL);
			g_free (str);

			g_object_unref (G_OBJECT (document));

			doclist = g_slist_remove (doclist, doc_info);
			couchdb_document_info_unref (doc_info);
		}

		dblist = g_slist_remove (dblist, dblist->data);
	}
}

static void
test_change_databases (void)
{
	char *dbname;
	gint i;
	GError *error = NULL;

	dbname = generate_uuid ();
	g_assert (dbname != NULL);

	/* Database name can not start with a digit
	 * we will overwrite the first character with 'a' */
	 dbname[0] = 'a';

	/* Create database */
	couchdb_session_create_database (couchdb, dbname, &error);
	if (error) {
		g_critical ("Error creating database '%s': %s", dbname, error->message);
		g_error_free (error);
	}

	couchdb_session_listen_for_changes (couchdb, dbname);

	/* Create some documents */
	for (i = 0; i < 10; i++) {
		CouchdbDocument *document;
		char *str;

		document = couchdb_document_new (couchdb);
		g_assert (document != NULL);

		couchdb_document_set_boolean_field (document, "boolean", TRUE);
		couchdb_document_set_int_field (document, "int", i);
		couchdb_document_set_double_field (document, "double", (gdouble) i);

		str = g_strdup_printf ("value%d", i);
		couchdb_document_set_string_field (document, "string", str);
		g_free (str);

		g_assert (couchdb_document_put (document, dbname, &error));
		g_assert (error == NULL);
	}
	
	/* Delete database */
	g_assert (couchdb_session_delete_database (couchdb, dbname, &error));
	g_assert (error == NULL);

	/* Free memory */
	g_free (dbname);
}

static void
db_created_cb (CouchdbSession *couchdb, const char *dbname, gpointer user_data)
{
	g_print ("Database %s has been created\n", dbname);
}

static void
db_deleted_cb (CouchdbSession *couchdb, const char *dbname, gpointer user_data)
{
	g_print ("Database %s has been deleted\n", dbname);
}

static void
doc_changed_cb (CouchdbSession *couchdb, const char *dbname, CouchdbDocument *document, gpointer user_data)
{
	char *doc_str;

	doc_str = couchdb_document_to_string (document);
	g_print ("Document %s has been %s: %s\n",
		 couchdb_document_get_id (document),
		 (const gchar *) user_data,
		 doc_str);

	g_free (doc_str);
}

static void
doc_deleted_cb (CouchdbSession *couchdb, const char *dbname, const char *docid, gpointer user_data)
{
	g_print ("Document %s in database %s has been deleted\n", docid, dbname);
}

int
main (int argc, char *argv[])
{
	g_type_init ();
	g_thread_init (NULL);
	g_test_init (&argc, &argv, NULL);

	/* Initialize data needed for all tests */
	couchdb =  argc > 1 ? couchdb_session_new (argv[1]) : couchdb_session_new ("http://test:test@127.0.0.1:5985");
	g_printf ("Connecting to Couchdb at %s\n", couchdb_session_get_uri (couchdb));
	
	if (!couchdb) {
		g_print ("Could not create Couchdb object\n");
		return -1;
	}

	g_signal_connect (G_OBJECT (couchdb), "database_created", G_CALLBACK (db_created_cb), NULL);
	g_signal_connect (G_OBJECT (couchdb), "database_deleted", G_CALLBACK (db_deleted_cb), NULL);
	g_signal_connect (G_OBJECT (couchdb), "document_created", G_CALLBACK (doc_changed_cb), "created");
	g_signal_connect (G_OBJECT (couchdb), "document_updated", G_CALLBACK (doc_changed_cb), "updated");
	g_signal_connect (G_OBJECT (couchdb), "document_deleted", G_CALLBACK (doc_deleted_cb), NULL);

	/* Setup test functions */
	g_test_add_func ("/testcouchdbglib/ListDatabases", test_list_databases);
	g_test_add_func ("/testcouchdbglib/ListDocuments", test_list_documents);
	g_test_add_func ("/testcouchdbglib/ChangeDatabases", test_change_databases);

	return g_test_run ();
}
