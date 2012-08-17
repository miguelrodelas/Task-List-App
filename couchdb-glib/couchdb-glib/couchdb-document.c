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
#include <libsoup/soup-session-async.h>
#include <libsoup/soup-gnome.h>
#include <json-glib/json-glib.h>
#include "couchdb-document.h"
#include "utils.h"

struct _CouchdbDocument {
	GObject		parent;

	CouchdbSession  *couchdb;
	char		*dbname;
	JsonNode	*root_node;
};

G_DEFINE_TYPE(CouchdbDocument, couchdb_document, G_TYPE_OBJECT);

static void
couchdb_document_finalize (GObject *object)
{
	CouchdbDocument *document = COUCHDB_DOCUMENT (object);

	g_free (document->dbname);
	json_node_free (document->root_node);

	G_OBJECT_CLASS (couchdb_document_parent_class)->finalize (object);
}

static void
couchdb_document_class_init (CouchdbDocumentClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = couchdb_document_finalize;
}

static void
couchdb_document_init (CouchdbDocument *document)
{
	document->couchdb = NULL;
	document->root_node = NULL;
	document->dbname = NULL;
}

/**
 * couchdb_document_new:
 * @couchdb: A #CouchdbSession object
 *
 * Create an empty #CouchdbDocument object, which can then be populated with data
 * and added to a database on the specified CouchDB instance.
 *
 * Return value: A newly-created #CouchdbDocument object, with no data on it.
 */
CouchdbDocument *
couchdb_document_new (CouchdbSession *couchdb)
{
	CouchdbDocument *document;

	document = g_object_new (COUCHDB_TYPE_DOCUMENT, NULL);
	document->couchdb = couchdb;
	document->dbname = NULL;

	document->root_node = json_node_new (JSON_NODE_OBJECT);
	json_node_set_object (document->root_node, json_object_new ());

	return document;
}

/**
 * couchdb_document_get:
 * @couchdb: A #CouchdbSession object
 * @dbname: Name of the database to retrieve the document from
 * @docid: Unique ID of the document to be retrieved
 * @error: Placeholder for error information
 *
 * Retrieve the last revision of a document from the given database.
 *
 * Return value: A #CouchdbDocument object if successful, NULL otherwise, in
 * which case, the error argument will contain information about the error.
 */
CouchdbDocument *
couchdb_document_get (CouchdbSession *couchdb,
		      const char *dbname,
		      const char *docid,
		      GError **error)
{
	char *url, *encoded_docid;
	JsonParser *parser;
	CouchdbDocument *document = NULL;

	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), NULL);
	g_return_val_if_fail (dbname != NULL, NULL);
	g_return_val_if_fail (docid != NULL, NULL);

	encoded_docid = soup_uri_encode (docid, NULL);
	url = g_strdup_printf ("%s/%s/%s", couchdb_session_get_uri (couchdb), dbname, encoded_docid);
	parser = json_parser_new ();
	if (couchdb_session_send_message (couchdb, SOUP_METHOD_GET, url, NULL, parser, error)) {
		document = g_object_new (COUCHDB_TYPE_DOCUMENT, NULL);
		document->couchdb = couchdb;
		document->dbname = g_strdup (dbname);

		document->root_node = json_node_copy (json_parser_get_root (parser));		
	}
	g_object_unref (G_OBJECT (parser));
	g_free (encoded_docid);
	g_free (url);

	return document;
}

/**
 * couchdb_document_put:
 * @document: A #CouchdbDocument object
 * @dbname: Name of the database where the document will be stored
 * @error: Placeholder for error information
 *
 * Store a document on a CouchDB database.
 *
 * If it is a new document, and hence does not have a unique ID, a unique ID
 * will be generated and stored on the #CouchdbDocument object. Likewise,
 * whether the document is new or just an update to an existing one, the
 * #CouchdbDocument object passed to this function will be updated to contain
 * the latest revision of the document, as returned by CouchDB (revision that
 * can be retrieved by calling #couchdb_document_get_revision).
 *
 * Return value: TRUE if successful, FALSE otherwise, in which case the error
 * argument will contain information about the error.
 */
gboolean
couchdb_document_put (CouchdbDocument *document,
		      const char *dbname,
		      GError **error)
{
	char *url, *body;
	const char *id;
	JsonParser *parser;
	gboolean result = FALSE;
	gboolean send_ok;

	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), FALSE);
	g_return_val_if_fail (dbname != NULL, FALSE);

	id = couchdb_document_get_id (document);
	body = couchdb_document_to_string (document);
	parser = json_parser_new ();
	if (id) {
		char *encoded_docid;

		encoded_docid = soup_uri_encode (id, NULL);
		url = g_strdup_printf ("%s/%s/%s", couchdb_session_get_uri (document->couchdb), dbname, encoded_docid);		
		send_ok = couchdb_session_send_message (document->couchdb, SOUP_METHOD_PUT, url, body, parser, error);

		g_free (encoded_docid);
	} else {
		url = g_strdup_printf ("%s/%s/", couchdb_session_get_uri (document->couchdb), dbname);
		send_ok = couchdb_session_send_message (document->couchdb, SOUP_METHOD_POST, url, body, parser, error);
	}

	if (send_ok) {
		JsonObject *object;

		object = json_node_get_object (json_parser_get_root (parser));
		couchdb_document_set_id (document, json_object_get_string_member (object, "id"));
		couchdb_document_set_revision (document, json_object_get_string_member (object, "rev"));

		if (document->dbname) {
			g_free (document->dbname);
			document->dbname = g_strdup (dbname);
		}

		if (id)
			g_signal_emit_by_name (document->couchdb, "document_updated", dbname, document);
		else
			g_signal_emit_by_name (document->couchdb, "document_created", dbname, document);

		result = TRUE;
	}

	/* free memory */
	g_free (url);
	g_free (body);
	g_object_unref (G_OBJECT (parser));

	return result;
}

/**
 * couchdb_document_delete:
 * @document: A #CouchdbDocument object
 * @error: Placeholder for error information
 *
 * Delete an existing document from a CouchDB instance.
 *
 * Please take note that this operation can fail if there was an update to the
 * document and that last revision was not retrieved by the calling application.
 * This is due to the fact that, to remove a document from CouchDB, the latest
 * revision needs to be sent, so if the #CouchdbDocument object passed to this
 * function does not contain the last revision, the operation will fail. In that
 * case, retrieving the latest revision from CouchDB (with #couchdb_document_get)
 * and trying the delete operation again should fix the issue.
 *
 * Return value: TRUE if successful, FALSE otherwise, in which case the error
 * argument will contain information about the error.
 */
gboolean
couchdb_document_delete (CouchdbDocument *document, GError **error)
{
	const char *id, *revision;
	char *url;
	JsonParser *parser;
	gboolean result = FALSE;

	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), FALSE);

	id = couchdb_document_get_id (document);
	revision = couchdb_document_get_revision (document);
	if (!id || !revision) /* we can't remove a document without an ID and/or a REVISION */
		return FALSE;

	url = g_strdup_printf ("%s/%s/%s?rev=%s", couchdb_session_get_uri (document->couchdb), document->dbname, id, revision);
	
	/* We don't parse the http response, therefore the parser arg is NULL */
	if (couchdb_session_send_message (document->couchdb, SOUP_METHOD_DELETE, url, NULL, NULL, error)) {
		result = TRUE;		
		g_signal_emit_by_name (document->couchdb, "document_deleted", document->dbname, id);
	}

	g_free (url);

	return result;
}

/**
 * couchdb_document_get_id:
 * @document: A #CouchdbDocument object
 *
 * Retrieve the unique ID of the given document.
 *
 * Return value: The unique ID of the given document.
 */
const char *
couchdb_document_get_id (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);

	if (document->root_node &&
	    json_node_get_node_type (document->root_node) == JSON_NODE_OBJECT) {
		if (json_object_has_member (json_node_get_object (document->root_node),
					    "_id"))
			return json_object_get_string_member (
				json_node_get_object (document->root_node),
				"_id");
	}

	return NULL;
}

/**
 * couchdb_document_set_id:
 * @document: A #CouchdbDocument object
 * @id: New unique ID for the given document.
 *
 * Set the unique ID for a given document.
 *
 * This usually is not needed by calling applications, unless they want to
 * force IDs on the CouchDB documents created/updated for some reason, like
 * compatibility with 3rd party applications. In most cases, the autogenerated
 * IDs from CouchDB when new documents are created (see #couchdb_document_put)
 * should be ok for most applications.
 */
void
couchdb_document_set_id (CouchdbDocument *document, const char *id)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (id != NULL);

	json_object_set_string_member (json_node_get_object (document->root_node),
				       "_id",
				       id);
}

/**
 * couchdb_document_get_revision:
 * @document: A #CouchdbDocument object
 *
 * Retrieve the revision of a given document.
 *
 * CouchDB does not overwrite updated documents in place, instead it creates a
 * new document at the end of the database file, with the same ID but a new revision.
 *
 * Document revisions are used for optimistic concurrency control and applications
 * should not rely on document revisions for any other purpose than concurrency control.
 *
 * Return value: Revision of the given document.
 */
const char *
couchdb_document_get_revision (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);

	if (document->root_node &&
	    json_node_get_node_type (document->root_node) == JSON_NODE_OBJECT) {
		return json_object_get_string_member (
			json_node_get_object (document->root_node),
			"_rev");
	}

	return NULL;
}

/**
 * couchdb_document_set_revision:
 * @document: A #CouchdbDocument object
 * @revision: String specifying the revision to set the document to
 *
 * Set the revision of the given document. This should never be used by applications,
 * unless they really know what they are doing, since using a wrong revision string
 * will confuse CouchDB when doing updates to the document.
 */
void
couchdb_document_set_revision (CouchdbDocument *document, const char *revision)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (revision != NULL);

	json_object_set_string_member (json_node_get_object (document->root_node),
				       "_rev",
				       revision);
}

/**
 * couchdb_document_has_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to check existence for in the document
 *
 * Check whether the given document has a field with a specific name.
 *
 * Return value: TRUE if the field exists in the document, FALSE if not.
 */
gboolean
couchdb_document_has_field (CouchdbDocument *document, const char *field)
{
	JsonNode *node;

	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), FALSE);
	g_return_val_if_fail (field != NULL, FALSE);

	return json_object_has_member (json_node_get_object (document->root_node), field);
}

/**
 * couchdb_document_remove_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to remove from the document
 *
 * Remove a field from the given document.
 */
void
couchdb_document_remove_field (CouchdbDocument *document, const char *field)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (field != NULL);

	json_object_remove_member (json_node_get_object (document->root_node), field);
}

/**
 * couchdb_document_get_array_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to retrieve
 *
 * Retrieve the value of an array field from the given document.
 *
 * Return value: The value of the given array field.
 */
CouchdbArrayField *
couchdb_document_get_array_field (CouchdbDocument *document, const char *field)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (field != NULL, NULL);

	if (!json_object_has_member (json_node_get_object (document->root_node), field))
		return NULL;

	return couchdb_array_field_new_from_json_array (
		json_array_ref (json_object_get_array_member (json_node_get_object (document->root_node),
							       field)));
}

/**
 * couchdb_document_set_array_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to set
 * @value: Value to set the field to
 *
 * Set the value of an array field in the given document.
 */
void
couchdb_document_set_array_field (CouchdbDocument *document, const char *field, CouchdbArrayField *value)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (field != NULL);
	g_return_if_fail (value != NULL);

	json_object_set_array_member (json_node_get_object (document->root_node),
				      field,
				      json_array_ref (couchdb_array_field_get_json_array (value)));
}

/**
 * couchdb_document_get_boolean_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to retrieve
 *
 * Retrieve the value of a boolean field from the given document.
 *
 * Return value: The value of the given boolean field.
 */
gboolean
couchdb_document_get_boolean_field (CouchdbDocument *document, const char *field)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), FALSE);
	g_return_val_if_fail (field != NULL, FALSE);

	if (!json_object_has_member (json_node_get_object (document->root_node), field))
		return FALSE;

	return json_object_get_boolean_member (json_node_get_object (document->root_node),
					       field);
}

/**
 * couchdb_document_set_boolean_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to set
 * @value: Value to set the field to
 *
 * Set the value of a boolean field in the given document.
 */
void
couchdb_document_set_boolean_field (CouchdbDocument *document, const char *field, gboolean value)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (field != NULL);

	json_object_set_boolean_member (json_node_get_object (document->root_node),
					field,
					value);
}

/**
 * couchdb_document_get_int_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to retrieve
 *
 * Retrieve the value of an integer field from the given document.
 *
 * Return value: The value of the given integer field.
 */
gint
couchdb_document_get_int_field (CouchdbDocument *document, const char *field)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), -1);
	g_return_val_if_fail (field != NULL, -1);

	if (!json_object_has_member (json_node_get_object (document->root_node), field))
		return 0;

	return json_object_get_int_member (json_node_get_object (document->root_node),
					   field);
}

/**
 * couchdb_document_set_int_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to set
 * @value: Value to set the field to
 *
 * Set the value of an integer field in the given document.
 */
void
couchdb_document_set_int_field (CouchdbDocument *document, const char *field, gint value)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (field != NULL);

	json_object_set_int_member (json_node_get_object (document->root_node),
				    field,
				    value);
}

/**
 * couchdb_document_get_double_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to retrieve
 *
 * Retrieve the value of a decimal number field from the given document.
 *
 * Return value: The value of the given decimal number field.
 */
gdouble
couchdb_document_get_double_field (CouchdbDocument *document, const char *field)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), -1);
	g_return_val_if_fail (field != NULL, -1);

	if (!json_object_has_member (json_node_get_object (document->root_node), field))
		return 0.0;
	return json_object_get_double_member (json_node_get_object (document->root_node),
					      field);
}

/**
 * couchdb_document_set_double_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to set
 * @value: Value to set the field to
 *
 * Set the value of a decimal number field in the given document.
 */
void
couchdb_document_set_double_field (CouchdbDocument *document, const char *field, gdouble value)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (field != NULL);

	json_object_set_double_member (json_node_get_object (document->root_node),
				       field,
				       value);
}

/**
 * couchdb_document_get_string_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to retrieve
 *
 * Retrieve the value of a string field from the given document.
 *
 * Return value: The value of the given string field.
 */
const char *
couchdb_document_get_string_field (CouchdbDocument *document, const char *field)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (field != NULL, NULL);

	if (!json_object_has_member (json_node_get_object (document->root_node), field))
		return NULL;

	return json_object_get_string_member (json_node_get_object (document->root_node),
					      field);
}

/**
 * couchdb_document_set_string_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to set
 * @value: Value to set the field to
 *
 * Set the value of a string field in the given document.
 */
void
couchdb_document_set_string_field (CouchdbDocument *document, const char *field, const char *value)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (field != NULL);

	if (value) {
		json_object_set_string_member (json_node_get_object (document->root_node),
					       field,
					       value);
	} else {
		/* Remove field if it's a NULL value */
		couchdb_document_remove_field (document, field);
	}
}

/**
 * couchdb_document_get_struct_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to retrieve
 *
 * Retrieve the value of a struct field from the given document.
 *
 * Return value: The value of the given struct field.
 */
CouchdbStructField *
couchdb_document_get_struct_field (CouchdbDocument *document, const char *field)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (field != NULL, NULL);

	if (!json_object_has_member (json_node_get_object (document->root_node), field))
		return NULL;

	return couchdb_struct_field_new_from_json_object (
		json_object_ref (json_object_get_object_member (json_node_get_object (document->root_node),
								field)));
}

/**
 * couchdb_document_set_struct_field:
 * @document: A #CouchdbDocument object
 * @field: Name of the field to set
 * @value: Value to set the field to
 *
 * Set the value of a struct field in the given document.
 */
void
couchdb_document_set_struct_field (CouchdbDocument *document, const char *field, CouchdbStructField *value)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (field != NULL);
	g_return_if_fail (value != NULL);

	json_object_set_object_member (json_node_get_object (document->root_node),
				       field,
				       json_object_ref (couchdb_struct_field_get_json_object (value)));
}

/**
 * couchdb_document_to_string:
 * @document: A #CouchdbDocument object
 *
 * Convert the given #CouchdbDocument to a JSON string.
 *
 * Return value: A string containing the given document in JSON format.
 */
char *
couchdb_document_to_string (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);

	if (document->root_node) {
		JsonGenerator *generator;
		char *str;
		gsize size;

		generator = json_generator_new ();
		json_generator_set_root (generator, document->root_node);

		str = json_generator_to_data (generator, &size);
		g_object_unref (G_OBJECT (generator));

		return str;
	}

	return NULL;
}

JsonObject *
couchdb_document_get_json_object (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	
	return json_node_get_object (document->root_node);
}
