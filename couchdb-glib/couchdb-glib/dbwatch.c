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

#include <libsoup/soup-method.h>
#include "couchdb-document.h"
#include "dbwatch.h"
#include "utils.h"

#define LOCAL_TIMEOUT_SECONDS  60
#define REMOTE_TIMEOUT_SECONDS 300

static void
process_change (DBWatch *watch, JsonNode *node)
{
	JsonObject *this_change;
	const gchar *id;
	CouchdbDocument *document;
	GError *error = NULL;

	if (json_node_get_node_type (node) != JSON_NODE_OBJECT)
		return;

	this_change = json_node_get_object (node);
	if (!json_object_has_member (this_change, "id"))
		return;

	id = json_object_get_string_member (this_change, "id");

	/* We need to try retrieving the document, to check if it's removed or not */
	document = couchdb_document_get (watch->couchdb, watch->dbname, id, &error);
	if (document) {
		const gchar *revision;

		revision = couchdb_document_get_revision (document);
		if (revision != NULL) {
			if (revision[0] == '1')
				g_signal_emit_by_name (watch->couchdb, "document_created",
						       watch->dbname, document);
			else
				g_signal_emit_by_name (watch->couchdb, "document_updated",
						       watch->dbname, document);
		}

		g_object_unref (G_OBJECT (document));
	} else {
		if (error != NULL) {
			g_warning ("Error retrieving document '%s': %s", id, error->message);
			g_error_free (error);
		} else {
			/* The document is no longer in the DB, notify */
			g_signal_emit_by_name (watch->couchdb, "document_deleted", watch->dbname, id);
		}
	}
}

static gboolean
watch_timeout_cb (gpointer user_data)
{
	char *url;
	JsonParser *parser;
	GError *error = NULL;
	DBWatch *watch = (DBWatch *) user_data;

	url = g_strdup_printf ("%s/%s/_changes?since=%d",
			       couchdb_session_get_uri (watch->couchdb),
			       watch->dbname,
			       watch->last_update_seq);
	parser = json_parser_new ();
	if (couchdb_session_send_message (watch->couchdb, SOUP_METHOD_GET, url, NULL, parser, &error)) {
		JsonNode *root_node;

		root_node = json_parser_get_root (parser);
		if (json_node_get_node_type (root_node) == JSON_NODE_OBJECT) {
			JsonObject *root_object;
			JsonArray *results;

			root_object = json_node_get_object (root_node);
			results = json_object_get_array_member (root_object, "results");
			if (results) {
				GList *json_elements, *sl;

				json_elements = json_array_get_elements (results);
				for (sl = json_elements; sl != NULL; sl = sl->next)
					process_change (watch, (JsonNode *) sl->data);
			}

			if (json_object_has_member (root_object, "last_seq"))
				watch->last_update_seq = json_object_get_int_member (root_object, "last_seq");
		}		
	}

	/* Free memory */
	g_object_unref (G_OBJECT (parser));
	g_free (url);

	return TRUE;
}

DBWatch *
dbwatch_new (CouchdbSession *couchdb, const gchar *dbname, gint update_seq)
{
	DBWatch *watch;
	guint timeout;

	watch = g_new0 (DBWatch, 1);
	watch->couchdb = couchdb;
	watch->dbname = g_strdup (dbname);
	watch->last_update_seq = update_seq;

	/* Set timeout to check for changes every 5 minutes*/
	if (g_str_has_prefix (couchdb_session_get_uri (watch->couchdb), "http://127.0.0.1"))
		timeout = LOCAL_TIMEOUT_SECONDS;
	else
		timeout = REMOTE_TIMEOUT_SECONDS;

	watch->timeout_id = g_timeout_add (timeout * 1000, (GSourceFunc) watch_timeout_cb, watch);
	
	return watch;
}

void
dbwatch_free (DBWatch *watch)
{
	g_free (watch->dbname);
	g_source_remove (watch->timeout_id);

	g_free (watch);
}
