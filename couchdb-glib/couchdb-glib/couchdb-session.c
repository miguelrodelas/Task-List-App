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

#include <libsoup/soup-logger.h>
#include <libsoup/soup-gnome.h>
#include <libsoup/soup-message.h>
#include "couchdb-session.h"
#include "couchdb-document.h"
#include "couchdb-document-info.h"
#include "couchdb-marshal.h"
#include "dbwatch.h"
#include "utils.h"
#include <string.h>
#ifdef HAVE_OAUTH
#include <time.h>
#include <stdlib.h>
#include "oauth.h"
#endif

#define COUCHDB_SIGNAL_AUTHENTICATION_FAILED "authentication-failed"

struct _CouchdbSessionPrivate {
	char *uri;
	SoupSession *http_session;
	GHashTable *db_watchlist;
	CouchdbCredentials *credentials;
};

G_DEFINE_TYPE(CouchdbSession, couchdb_session, G_TYPE_OBJECT)

enum {
	AUTHENTICATION_FAILED,
	DATABASE_CREATED,
	DATABASE_DELETED,
	DOCUMENT_CREATED,
	DOCUMENT_UPDATED,
	DOCUMENT_DELETED,
	LAST_SIGNAL
};
static guint couchdb_session_signals[LAST_SIGNAL];

enum {
    PROP_0,
    PROP_URI
};

#ifdef DEBUG_MESSAGES
#define COUCHDB_ENV_DEBUG_MESSAGES "COUCHDB_DEBUG_MESSAGES"

static void debug_print_headers (const char *name, const char *value,
				 gpointer user_data);
static void debug_message (const gchar *log_domain,
			   GLogLevelFlags log_level,
			   const gchar *message, gpointer user_data);
#endif

static gboolean _session_authenticate(SoupSession *session,
				      SoupMessage *msg,
				      SoupAuth *auth,
				      gboolean retrying,
				      gpointer couchdb);


static void
couchdb_session_finalize (GObject *object)
{
	CouchdbSession *couchdb = COUCHDB_SESSION (object);

	g_hash_table_destroy (couchdb->priv->db_watchlist);

	g_free (couchdb->priv->uri);
	g_object_unref (couchdb->priv->http_session);

	if (couchdb->priv->credentials)
		g_object_unref (G_OBJECT (couchdb->priv->credentials));

	g_free (couchdb->priv);

	G_OBJECT_CLASS (couchdb_session_parent_class)->finalize (object);
}

static void
couchdb_session_set_property (GObject         *object,
			      guint            prop_id,
			      const GValue    *value,
			      GParamSpec      *pspec)

{
	CouchdbSession *couchdb = COUCHDB_SESSION (object);

	switch (prop_id) {
	case PROP_URI:
		g_free(couchdb->priv->uri);
		couchdb->priv->uri = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
couchdb_session_get_property (GObject         *object,
			      guint            prop_id,
			      GValue          *value,
			      GParamSpec      *pspec)
{
	CouchdbSession *couchdb = COUCHDB_SESSION (object);

	switch (prop_id) {
	case PROP_URI:
		g_value_set_string (value, couchdb->priv->uri);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
couchdb_session_class_init (CouchdbSessionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = couchdb_session_finalize;
	object_class->set_property = couchdb_session_set_property;
	object_class->get_property = couchdb_session_get_property;

	g_object_class_install_property (object_class,
        	                         PROP_URI,
                	                 g_param_spec_string ("uri",
                        	                              "Uri",
							      "Uri pointing to the host to connect to",
							      NULL,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	/* Signals */
	couchdb_session_signals[AUTHENTICATION_FAILED] =
		g_signal_new (COUCHDB_SIGNAL_AUTHENTICATION_FAILED,
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (CouchdbSessionClass, authentication_failed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0,
			      NULL);
	couchdb_session_signals[DATABASE_CREATED] =
		g_signal_new ("database_created",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (CouchdbSessionClass, database_created),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE, 1,
			      G_TYPE_STRING);
	couchdb_session_signals[DATABASE_DELETED] =
		g_signal_new ("database_deleted",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (CouchdbSessionClass, database_deleted),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE, 1,
			      G_TYPE_STRING);
	couchdb_session_signals[DOCUMENT_CREATED] =
		g_signal_new ("document_created",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (CouchdbSessionClass, document_created),
			      NULL, NULL,
			      _couchdb_marshal_VOID__STRING_OBJECT,
			      G_TYPE_NONE, 2,
			      G_TYPE_STRING,
			      G_TYPE_OBJECT);
	couchdb_session_signals[DOCUMENT_UPDATED] =
		g_signal_new ("document_updated",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (CouchdbSessionClass, document_updated),
			      NULL, NULL,
			      _couchdb_marshal_VOID__STRING_OBJECT,
			      G_TYPE_NONE, 2,
			      G_TYPE_STRING,
			      G_TYPE_OBJECT);
	couchdb_session_signals[DOCUMENT_DELETED] =
		g_signal_new ("document_deleted",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (CouchdbSessionClass, document_deleted),
			      NULL, NULL,
			      _couchdb_marshal_VOID__STRING_STRING,
			      G_TYPE_NONE, 2,
			      G_TYPE_STRING,
			      G_TYPE_STRING);
}

static void
couchdb_session_init (CouchdbSession *couchdb)
{
	couchdb->priv = g_new0 (CouchdbSessionPrivate, 1);

	couchdb->priv->db_watchlist = g_hash_table_new_full (g_str_hash, g_str_equal,
							     (GDestroyNotify) g_free,
							     (GDestroyNotify) dbwatch_free);
	if (couchdb->priv->uri == NULL)
		couchdb->priv->uri = g_strdup("http://127.0.0.1:5984");

	couchdb->priv->http_session = soup_session_sync_new_with_options (
		SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_GNOME_FEATURES_2_26,
                NULL);

	couchdb->priv->credentials = NULL;

#ifdef DEBUG_MESSAGES
	g_log_set_handler (NULL, G_LOG_LEVEL_DEBUG, debug_message, NULL);
#endif
}

/**
 * couchdb_session_new:
 * @uri: URI of the CouchDB instance to connect to
 *
 * Create a new #CouchdbSession object, which is the entry point for operations on a
 * CouchDB instance.
 *
 * Return value: A newly-created #CouchdbSession object.
 */
CouchdbSession *
couchdb_session_new (const char *uri)
{
	if (!uri)
		uri = "http://127.0.0.1:5984";

	return g_object_new (COUCHDB_TYPE_SESSION, "uri", uri, NULL);
}

/**
 * couchdb_session_get_uri:
 * @couchdb: A #CouchdbSession object
 *
 * Retrieve the URI of the CouchDB instance a #CouchdbSession object is bound to.
 *
 * Return value: the URI of the CouchDB instance used by this #CouchdbSession object.
 */
const char *
couchdb_session_get_uri (CouchdbSession *couchdb)
{
	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), NULL);

	return (const char *) couchdb->priv->uri;
}

/**
 * couchdb_session_list_databases:
 * @couchdb: A #CouchdbSession object
 * @error: Placeholder for error information
 *
 * Retrieve the list of databases that exist in the CouchDB instance being used.
 *
 * Return value: A list of strings containing the names of all the databases
 * that exist in the CouchDB instance connected to. Once no longer needed, this
 * list can be freed by calling #couchdb_session_free_database_list.
 */
GSList *
couchdb_session_list_databases (CouchdbSession *couchdb, GError **error)
{
	char *url;
	GSList *dblist = NULL;
	JsonParser *parser;

	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), NULL);

	/* Prepare request */
	url = g_strdup_printf ("%s/_all_dbs", couchdb->priv->uri);
	parser = json_parser_new ();
	if (couchdb_session_send_message (couchdb, SOUP_METHOD_GET, url, NULL, parser, error)) {
		JsonNode *root_node;

		root_node = json_parser_get_root (parser);
		if (json_node_get_node_type (root_node) == JSON_NODE_ARRAY) {
			GList *json_elements, *sl;

			json_elements = json_array_get_elements (json_node_get_array (root_node));
			for (sl = json_elements; sl != NULL; sl = sl->next) {
				dblist = g_slist_append (
					dblist,
					g_strdup (json_node_get_string ((JsonNode *) sl->data)));
			}
		}		
	}

	/* Free memory */
	g_object_unref (G_OBJECT (parser));
	g_free (url);

	return dblist;
}

/**
 * couchdb_session_get_database_info:
 * @couchdb: A #CouchdbSession object
 * @dbname: Name of the database for which to retrieve the information
 * @error: Placeholder for error information
 *
 * Retrieve information about a given database.
 *
 * Return value: A #CouchdbDatabaseInfo object, whose API can be used to retrieve
 * all the information returned by CouchDB about this database.
 */
CouchdbDatabaseInfo *
couchdb_session_get_database_info (CouchdbSession *couchdb, const char *dbname, GError **error)
{
	char *url;
	JsonParser *parser;
	CouchdbDatabaseInfo *result = NULL;

	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), NULL);
	g_return_val_if_fail (dbname != NULL, NULL);

	url = g_strdup_printf ("%s/%s/", couchdb->priv->uri, dbname);
	parser = json_parser_new ();
	if (couchdb_session_send_message (couchdb, SOUP_METHOD_GET, url, NULL, parser, error)) {
		JsonNode *root_node;

		root_node = json_parser_get_root (parser);
		if (json_node_get_node_type (root_node) == JSON_NODE_OBJECT) {
			JsonObject *object = json_node_get_object (root_node);

			result = couchdb_database_info_new (json_object_get_string_member (object, "db_name"),
							    json_object_get_int_member (object, "doc_count"),
							    json_object_get_int_member (object, "doc_del_count"),
							    json_object_get_int_member (object, "update_seq"),
							    json_object_get_boolean_member (object, "compact_running"),
							    json_object_get_int_member (object, "disk_size"));
		}
	}
	g_object_unref (G_OBJECT (parser));

	return result;
}

/**
 * couchdb_session_create_database
 * @couchdb: A #CouchdbSession object
 * @dbname: Name of the database to be created
 * @error: Placeholder for error information
 *
 * Create a new database on a CouchDB instance.
 *
 * Return value: TRUE if successful, FALSE otherwise.
 */
gboolean
couchdb_session_create_database (CouchdbSession *couchdb, const char *dbname, GError **error)
{
	char *url;
	JsonParser *parser;
	gboolean result = FALSE;

	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), FALSE);
	g_return_val_if_fail (dbname != NULL, FALSE);

	url = g_strdup_printf ("%s/%s/", couchdb->priv->uri, dbname);
	parser = json_parser_new ();
	if (couchdb_session_send_message (couchdb, SOUP_METHOD_PUT, url, NULL, parser, error)) {
		JsonNode *root_node;

		root_node = json_parser_get_root (parser);
		if (json_node_get_node_type (root_node) == JSON_NODE_OBJECT)
			result = json_object_get_boolean_member (
				json_node_get_object (root_node), "ok");		
	}
	
	g_object_unref (G_OBJECT (parser));
	g_free (url);

	if (result)
		g_signal_emit_by_name (couchdb, "database_created", dbname);

	return result;
}

/**
 * couchdb_session_delete_database
 * @couchdb: A #CouchdbSession object
 * @dbname: Name of the database to be deleted
 * @error: Placeholder for error information
 *
 * Delete an existing database on a CouchDB instance.
 *
 * Return value: TRUE if successful, FALSE otherwise.
 */
gboolean
couchdb_session_delete_database (CouchdbSession *couchdb, const char *dbname, GError **error)
{
	char *url;
	JsonParser *parser;
	gboolean result = FALSE;

	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), FALSE);
	g_return_val_if_fail (dbname != NULL, FALSE);

	url = g_strdup_printf ("%s/%s/", couchdb->priv->uri, dbname);
	parser = json_parser_new ();
	if (couchdb_session_send_message (couchdb, SOUP_METHOD_DELETE, url, NULL, parser, error)) {
		JsonNode *root_node;

		root_node = json_parser_get_root (parser);
		if (json_node_get_node_type (root_node) == JSON_NODE_OBJECT)
			result = json_object_get_boolean_member (
				json_node_get_object (root_node), "ok");		
	}

	g_object_unref (G_OBJECT (parser));
	g_free (url);

	if (result) {
		/* If we're listening for changes on this database, stop doing so */
		if (g_hash_table_lookup (couchdb->priv->db_watchlist, dbname))
			g_hash_table_remove (couchdb->priv->db_watchlist, dbname);

		g_signal_emit_by_name (couchdb, "database_deleted", dbname);
	}

	return result;
}

/**
 * couchdb_session_compact_database:
 * @couchdb: A #CouchdbSession object
 * @dbname: Name of the database to be compacted
 * @error: Placeholder for error information
 *
 * Compact the given database, which means removing outdated document
 * revisions and deleted documents.
 *
 * Return value: TRUE if successful, FALSE otherwise.
 */
gboolean
couchdb_session_compact_database (CouchdbSession *couchdb, const char *dbname, GError **error)
{
	char *url;
	JsonParser *output;
	gboolean result = FALSE;

	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), FALSE);
	g_return_val_if_fail (dbname != NULL, FALSE);

	url = g_strdup_printf ("%s/%s/_compact", couchdb_session_get_uri (couchdb), dbname);
	output = json_parser_new ();

	if (couchdb_session_send_message (couchdb, SOUP_METHOD_POST, url, NULL, output, error)) {
		JsonNode *root_node;

		root_node = json_parser_get_root (output);
		if (json_node_get_node_type (root_node) == JSON_NODE_OBJECT)
			result = json_object_get_boolean_member (
				json_node_get_object (root_node), "ok");
	}

	g_object_unref (G_OBJECT (output));
	g_free (url);

	return result;
}

/**
 * couchdb_session_free_database_list:
 * @dblist: A list of databases, as returned by #couchdb_session_list_databases
 *
 * Free the list of databases returned by #couchdb_session_list_databases.
 */
void
couchdb_session_free_database_list (GSList *dblist)
{
	g_return_if_fail (dblist != NULL);

	g_slist_foreach (dblist, (GFunc) couchdb_database_info_unref, NULL);
	g_slist_free (dblist);
}

/**
 * couchdb_session_list_documents:
 * @couchdb: A #CouchdbSession object
 * @dbname: Name of the databases to retrieve documents from
 * @error: Placeholder for error information
 *
 * Retrieve the list of all documents from a database on a running CouchDB instance.
 * For each document, a #CouchdbDocumentInfo object is returned on the list, which
 * can then be used for retrieving specific information for each document.
 *
 * Return Value: a list of #CouchdbDocumentInfo objects, or NULL if there are none
 * or there was an error (in which case the error argument will contain information
 * about the error). Once no longer needed, the list should be freed by calling
 * #couchdb_session_free_document_list.
 */
GSList *
couchdb_session_list_documents (CouchdbSession *couchdb, const char *dbname, GError **error)
{
	char *url;
	JsonParser *parser;
	GSList *doclist = NULL;

	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), NULL);
	g_return_val_if_fail (dbname != NULL, NULL);

	url = g_strdup_printf ("%s/%s/_all_docs", couchdb->priv->uri, dbname);
	parser = json_parser_new ();
	if (couchdb_session_send_message (couchdb, SOUP_METHOD_GET, url, NULL, parser, error)) {
		JsonNode *root_node;

		root_node = json_parser_get_root (parser);
		if (json_node_get_node_type (root_node) == JSON_NODE_OBJECT) {
			JsonArray *rows;
			gint i;

			rows = json_object_get_array_member (
				json_node_get_object (root_node), "rows");
			for (i = 0; i < json_array_get_length (rows); i++) {
				JsonObject *doc;
				CouchdbDocumentInfo *doc_info;

				doc = json_array_get_object_element (rows, i);
				if (!doc)
					continue;

				doc_info = couchdb_document_info_new (
					json_object_get_string_member (doc, "id"),
					json_object_get_string_member (
						json_object_get_object_member (doc, "value"),
						"rev"));
				doclist = g_slist_append (doclist, doc_info);
			}
		}
	}

	g_object_unref (G_OBJECT (parser));
	g_free (url);

	return doclist;
}

/**
 * couchdb_session_free_document_list:
 * @doclist: A list of #CouchdbDocumentInfo objects, as returned by
 * #couchdb_session_list_documents
 *
 * Free the list of documents returned by #couchdb_session_list_documents.
 */
void
couchdb_session_free_document_list (GSList *doclist)
{
	g_return_if_fail (doclist != NULL);

	g_slist_foreach (doclist, (GFunc) couchdb_document_info_unref, NULL);
	g_slist_free (doclist);
}

/**
 * couchdb_session_listen_for_changes:
 * @couchdb: A #CouchdbSession object
 * @dbname: Name of the database to poll changes for
 *
 * Setup a listener to get information about changes done to a specific database. Please
 * note that changes done in the application using couchdb-glib will be notified
 * without the need of calling this function. But if the application wants to receive
 * notifications of changes done externally (by another application, or by any other
 * means, like replication with a remote database), it needs to call this function.
 *
 * For each change, one of the signals on the #CouchdbSession object will be emitted,
 * so applications just have to connect to those signals before calling this function.
 */
void
couchdb_session_listen_for_changes (CouchdbSession *couchdb, const char *dbname)
{
	DBWatch *watch;
	CouchdbDatabaseInfo *db_info;
	GError *error = NULL;

	g_return_if_fail (COUCHDB_IS_SESSION (couchdb));
	g_return_if_fail (dbname != NULL);

	watch = g_hash_table_lookup (couchdb->priv->db_watchlist, dbname);
	if (watch) {
		g_warning ("Already listening for changes in '%s' database", dbname);
		return;
	}

	/* Retrieve information for database, to know the last_update_sequence */
	db_info = couchdb_session_get_database_info (couchdb, dbname, &error);
	if (!db_info) {
		g_warning ("Could not retrieve information for '%s' database: %s",
			   dbname, error->message);
		g_error_free (error);

		return;
	}

	watch = dbwatch_new (couchdb,
			     dbname,
			     couchdb_database_info_get_update_sequence (db_info));
	if (watch)
		g_hash_table_insert (couchdb->priv->db_watchlist, g_strdup (dbname), watch);

	/* Free memory */
	couchdb_database_info_unref (db_info);
}

/**
 * couchdb_session_enable_authentication:
 * @couchdb: A #CouchdbSession object
 * @credentials: A #CouchdbCredentials object
 *
 * Enables authentication for the given #CouchdbSession object. The authentication
 * mechanism should be specificied when creating the #CouchdbCredentials object.
 */
void
couchdb_session_enable_authentication (CouchdbSession *couchdb,
				       CouchdbCredentials *credentials)
{
	g_return_if_fail (COUCHDB_IS_SESSION (couchdb));

	if (couchdb->priv->credentials)
		g_object_unref (G_OBJECT (couchdb->priv->credentials));

	couchdb->priv->credentials = COUCHDB_CREDENTIALS (g_object_ref (G_OBJECT (credentials)));
	if (couchdb_credentials_get_auth_type (couchdb->priv->credentials) == COUCHDB_CREDENTIALS_TYPE_USERNAME_AND_PASSWORD) {
		g_signal_connect (couchdb->priv->http_session,
				  "authenticate",
				  G_CALLBACK (_session_authenticate),
				  couchdb);
	}
}

/**
 * couchdb_session_disable_authentication:
 * @couchdb: A #CouchdbSession object
 *
 * Disables authentication for the given #CouchdbSession object.
 */
void
couchdb_session_disable_authentication (CouchdbSession *couchdb)
{
	g_return_if_fail (COUCHDB_IS_SESSION (couchdb));

	if (couchdb_credentials_get_auth_type (couchdb->priv->credentials) == COUCHDB_CREDENTIALS_TYPE_USERNAME_AND_PASSWORD) {
		g_signal_handlers_disconnect_by_func (couchdb->priv->http_session,
						      G_CALLBACK (_session_authenticate),
						      couchdb);
	}

	if (couchdb->priv->credentials) {
		g_object_unref (G_OBJECT (couchdb->priv->credentials));
		couchdb->priv->credentials = NULL;
	}
}

/**
 * couchdb_session_is_authentication_enabled:
 * @couchdb: A #CouchdbSession object
 *
 * Gets whether the given #CouchdbSession object has authentication enabled.
 *
 * Return value: TRUE if authentication is enabled, FALSE otherwise.
 */
gboolean
couchdb_session_is_authentication_enabled (CouchdbSession *couchdb)
{
	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), FALSE);
	
	return couchdb->priv->credentials != NULL;
}

static gboolean
_session_authenticate(SoupSession *session, SoupMessage *msg,
		      SoupAuth *auth, gboolean retrying,
		      gpointer callback_data)
{
	CouchdbSession *couchdb;

	g_return_val_if_fail (COUCHDB_IS_SESSION (callback_data), FALSE);

	couchdb = COUCHDB_SESSION (callback_data);

	if (retrying) {
		g_signal_emit_by_name (couchdb, COUCHDB_SIGNAL_AUTHENTICATION_FAILED, NULL);
		g_debug ("Authentication failed!");
		return FALSE;
	}

	if (couchdb_credentials_get_auth_type (couchdb->priv->credentials) == COUCHDB_CREDENTIALS_TYPE_USERNAME_AND_PASSWORD) {
		const char *username = couchdb_credentials_get_item (couchdb->priv->credentials,
								     COUCHDB_CREDENTIALS_ITEM_USERNAME);
		const char *password = couchdb_credentials_get_item (couchdb->priv->credentials,
								     COUCHDB_CREDENTIALS_ITEM_PASSWORD);

		soup_auth_authenticate (auth, username, password);
	}

	return TRUE;
}

static void
add_oauth_signature (CouchdbSession *couchdb, SoupMessage *http_message, const char *method, const char *url)
{
#ifdef HAVE_OAUTH
	/* This method is a no-op if we are configured without OAUTH */
	char *signed_url;

	signed_url = oauth_sign_url2 (
		url, NULL, OA_HMAC, method,
		couchdb_credentials_get_item (couchdb->priv->credentials, COUCHDB_CREDENTIALS_ITEM_OAUTH_CONSUMER_KEY),
		couchdb_credentials_get_item (couchdb->priv->credentials, COUCHDB_CREDENTIALS_ITEM_OAUTH_CONSUMER_SECRET),
		couchdb_credentials_get_item (couchdb->priv->credentials, COUCHDB_CREDENTIALS_ITEM_OAUTH_TOKEN_KEY),
		couchdb_credentials_get_item (couchdb->priv->credentials, COUCHDB_CREDENTIALS_ITEM_OAUTH_TOKEN_SECRET));
	if (signed_url != NULL) {
		char **parsed_url;
		GString *header = NULL;

		/* Get the OAuth signature from the signed URL */
		parsed_url = g_strsplit (signed_url, "?", 2);
		if (parsed_url != NULL) {
			gchar **params;
			int i;

			params = g_strsplit (parsed_url[1], "&", 0);
#ifdef DEBUG_MESSAGES
			g_debug ("Parsing %s", parsed_url[1]);
#endif
			for (i = 0; params[i] != NULL; i++) {
				gchar **url_param;
				
				/* Don't include non-OAuth URL parameters in OAuth header */
				if (!g_str_has_prefix (params[i], "oauth_"))
					continue;
				
				url_param = g_strsplit (params[i], "=", 2);
				if (url_param == NULL)
					continue;

				if (header != NULL)
					header = g_string_append (header, ", ");
				else
					header = g_string_new ("OAuth ");

				header = g_string_append (header, url_param[0]);
				header = g_string_append (header, "=\"");
				header = g_string_append (header, url_param[1]);
				header = g_string_append (header, "\"");

				g_strfreev (url_param);
			}

			if (params)
				g_strfreev (params);

			g_strfreev (parsed_url);
		}

		if (header != NULL) {
			soup_message_headers_append (http_message->request_headers, "Authorization", header->str);

			g_string_free (header, TRUE);
		}

		free (signed_url);
	}
#endif /* HAVE_OAUTH */
}

static gboolean
parse_json_response (CouchdbSession *couchdb, JsonParser *json_parser, SoupMessage *http_message, GError **error)
{
	SoupBuffer *buffer;
        GString *str = NULL;
        goffset offset = 0;
        gboolean success = TRUE;
	
	while ((buffer = soup_message_body_get_chunk (http_message->response_body, offset))) {
		if (!str)
                        str = g_string_new ("");
                g_string_append_len (str, buffer->data, buffer->length);

                offset += buffer->length;
                soup_buffer_free (buffer);
	}

	if (str && str->len > 0) {
		g_debug ("Response body: %s", str->str);
		if (!json_parser_load_from_data (json_parser,
						 (const gchar *) str->str,
						 str->len,
						 error)) {
			g_object_unref (G_OBJECT (json_parser));
			g_set_error (error, COUCHDB_ERROR, -1, "Invalid JSON response");
			success = FALSE;
		}

		g_string_free (str, TRUE);
	}

	return success;
}

/**
 * couchdb_session_send_message:
 * @couchdb: A #CouchdbSession object
 * @method: HTTP method to use
 * @url: URL to send the message to
 * @body: Body of the HTTP request
 * @output: Placeholder for output information
 * @error: Placeholder for error information
 *
 * This function is used to communicate with CouchDB over HTTP, and should not be used
 * by applications unless they really have a need (like missing API in couchdb-glib which
 * the application needs).
 *
 * Return value: TRUE if successful, FALSE otherwise.
 */
gboolean
couchdb_session_send_message (CouchdbSession *couchdb, const char *method, const char *url, const char *body, JsonParser *output, GError **error)
{
	SoupMessage *http_message;
	guint status;
	GError **real_error;
	
	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), FALSE);
	g_return_val_if_fail (method != NULL, FALSE);

	if (error != NULL)
		real_error = error;
	else
		*real_error = NULL;

	http_message = soup_message_new (method, url);
	if (body != NULL) {
		soup_message_set_request (http_message, "application/json", SOUP_MEMORY_COPY,
					  body, strlen (body));
	}

	if (couchdb_session_is_authentication_enabled (couchdb)) {
		switch (couchdb_credentials_get_auth_type (couchdb->priv->credentials)) {
		case COUCHDB_CREDENTIALS_TYPE_OAUTH:
			add_oauth_signature (couchdb, http_message, method, url);
			break;
		default:
			g_warning ("Got unknown credentials object, not authenticating message");
		}
	}

	g_debug ("Sending %s to %s... with headers\n: ", method, url);
#ifdef DEBUG_MESSAGES
	soup_message_headers_foreach (http_message->request_headers,
				      (SoupMessageHeadersForeachFunc) debug_print_headers,
				      NULL);
#endif
	status = soup_session_send_message (couchdb->priv->http_session, http_message);
	if (SOUP_STATUS_IS_SUCCESSFUL (status)) {
		if (output != NULL)
		       	parse_json_response (couchdb, output, http_message, real_error);
	       	return TRUE;
	} else {
		if (error != NULL)
			g_set_error (error, COUCHDB_ERROR, status, "%s", http_message->reason_phrase);
		return FALSE;
	}
}

#ifdef DEBUG_MESSAGES
static void
debug_print_headers (const char *name, const char *value, gpointer user_data)
{
	g_debug ("\t%s: %s\n", name, value);
}

static void
debug_message (const gchar *log_domain, GLogLevelFlags log_level,
	       const gchar *message, gpointer user_data)
{
	const gchar* couchdb_env_debug_messages;

	couchdb_env_debug_messages = g_getenv(COUCHDB_ENV_DEBUG_MESSAGES);
	if (couchdb_env_debug_messages != NULL) {
		g_printf ("couchdb-glib:DEBUG: %s\n", message);
	}
}
#endif

/**
 * couchdb_session_replicate:
 * @couchdb: A #CouchdbSession object
 * @source: Source database
 * @target: Target database
 * @continous: Whether to replicate once or keep replicating
 * @error: Placeholder for error information
 *
 * Replicates a source database to another database, on the same CouchDB instance
 * or on a remote instance.
 *
 * If @continous is FALSE, the replication will happen once, but if set to TRUE,
 * CouchDB will listen to all changes made to the source database, and automatically
 * replicate over any new docs as the come into the source to the target.
 *
 * Return value: TRUE if successful, FALSE otherwise, in which case the @error
 * parameter will be set to contain information about the error.
 */
gboolean
couchdb_session_replicate (CouchdbSession *couchdb,
			   const gchar *source,
			   const gchar *target,
			   gboolean continous,
			   GError **error)
{
	char *url, *body;
	CouchdbDocument *input;
	JsonParser *output;
	gboolean send_ok;

	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), FALSE);
	g_return_val_if_fail (source != NULL, FALSE);
	g_return_val_if_fail (target != NULL, FALSE);

	/* Build the input document */
	input = couchdb_document_new (couchdb);
	couchdb_document_set_string_field (input, "source", source);
	couchdb_document_set_string_field (input, "target", target);
	if (continous)
		couchdb_document_set_boolean_field (input, "continous", TRUE);

	/* Send message */
	url = g_strdup_printf ("%s/_replicate", couchdb_session_get_uri (couchdb));
	body = couchdb_document_to_string (input);
	output = json_parser_new ();

	send_ok = couchdb_session_send_message (couchdb, SOUP_METHOD_POST, url, body, output, error);
	/* FIXME: what to do with the information returned? -> http://books.couchdb.org/relax/reference/replication */

	/* Free memory */
	g_object_unref (G_OBJECT (output));
	g_object_unref (G_OBJECT (input));
	g_free (body);
	g_free (url);

	return send_ok;
}
