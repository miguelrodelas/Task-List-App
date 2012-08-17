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

#include <string.h>
#include "utils.h"
#include "couchdb-struct-field.h"

struct _CouchdbStructField {
	gint ref_count;
	JsonObject *json_object;

	/* Extra data needed for some specific StructField's */
	char *uuid; /* the UUID of this item */
};

GType
couchdb_struct_field_get_type (void)
{
	static GType object_type = 0;

	if (G_UNLIKELY (!object_type))
		object_type = g_boxed_type_register_static (g_intern_static_string ("CouchdbStructField"),
							    (GBoxedCopyFunc) couchdb_struct_field_ref,
							    (GBoxedFreeFunc) couchdb_struct_field_unref);

	return object_type;
}

/**
 * couchdb_struct_field_new:
 *
 * Create a new struct field object, to be added to a #CouchdbDocument or to
 * another #CouchdbStructField.
 *
 * Return value: A newly-created #CouchdbStructField object.
 */
CouchdbStructField *
couchdb_struct_field_new (void)
{
	CouchdbStructField *sf;

	sf = g_slice_new (CouchdbStructField);
	sf->ref_count = 1;
	sf->json_object = json_object_new ();
	sf->uuid = NULL;

	return sf;
}

/**
 * couchdb_struct_field_new_from_string:
 * @str: A JSON string
 *
 * Create a new struct field object, filling it with values taken from a string
 * representing a JSON object.
 *
 * Return value: A newly-created #CouchdbStructField object.
 */
CouchdbStructField *
couchdb_struct_field_new_from_string (const char *str)
{
	JsonParser *parser;
	GError *error = NULL;
	CouchdbStructField *sf = NULL;

	g_return_val_if_fail (str != NULL, NULL);

	parser = json_parser_new ();
	if (json_parser_load_from_data (parser, str, strlen (str), &error)) {
		JsonNode *node = json_parser_get_root (parser);

		if (json_node_get_node_type (node) == JSON_NODE_OBJECT)
			sf = couchdb_struct_field_new_from_json_object (json_node_get_object (node));
	} else {
		g_warning ("Could not parse string: %s", error->message);
		g_error_free (error);
	}

	g_object_unref (G_OBJECT (parser));

	return sf;
}

CouchdbStructField *
couchdb_struct_field_new_from_json_object (JsonObject *json_object)
{
	CouchdbStructField *sf;

	sf = g_slice_new (CouchdbStructField);
	sf->ref_count = 1;
	sf->json_object = json_object_ref (json_object);
	sf->uuid = NULL;

	return sf;
}

/**
 * couchdb_struct_field_ref:
 * @sf: A #CouchdbStructField object
 *
 * Increments reference count of a #CouchdbStructField object.
 *
 * Return value: A pointer to the referenced object.
 */
CouchdbStructField *
couchdb_struct_field_ref (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);
	g_return_val_if_fail (sf->ref_count > 0, NULL);

	g_atomic_int_exchange_and_add (&sf->ref_count, 1);

	return sf;
}

/**
 * couchdb_struct_field_unref:
 * @sf: A #CouchdbStructField object
 *
 * Decrements reference count of a #CouchdbStructField object. When
 * the reference count is equal to 0, the object will be destroyed and
 * the memory it uses freed.
 */
void
couchdb_struct_field_unref (CouchdbStructField *sf)
{
	gint old_ref;

	g_return_if_fail (sf != NULL);
	g_return_if_fail (sf->ref_count > 0);

	old_ref = g_atomic_int_get (&sf->ref_count);
	if (old_ref > 1)
		g_atomic_int_compare_and_exchange (&sf->ref_count, old_ref, old_ref - 1);
	else {
		g_free (sf->uuid);
		json_object_unref (sf->json_object);
		g_slice_free (CouchdbStructField, sf);
	}
}

/**
 * couchdb_struct_field_has_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field to check
 *
 * Check whether a given field exists in the given #CouchdbStructField object.
 *
 * Return value: TRUE if the field exists, FALSE if not.
 */
gboolean
couchdb_struct_field_has_field (CouchdbStructField *sf, const char *field)
{
	g_return_val_if_fail (sf != NULL, FALSE);
	g_return_val_if_fail (field != NULL, FALSE);

	return json_object_has_member (sf->json_object, field);
}

/**
 * couchdb_struct_field_remove_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field to remove
 *
 * Remove a field from the given #CouchdbStructField object.
 */
void
couchdb_struct_field_remove_field (CouchdbStructField *sf, const char *field)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (field != NULL);

	json_object_remove_member (sf->json_object, field);
}

/**
 * couchdb_struct_field_get_array_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 *
 * Retrieve the value of an array field from the given struct field.
 *
 * Return value: The value of the given field.
 */
CouchdbArrayField *
couchdb_struct_field_get_array_field (CouchdbStructField *sf, const char *field)
{
	g_return_val_if_fail (sf != NULL, NULL);
	g_return_val_if_fail (field != NULL, NULL);

	if (!json_object_has_member (sf->json_object, field))
		return NULL;

	return couchdb_array_field_new_from_json_array (
		json_object_get_array_member (sf->json_object, field));
}

/**
 * couchdb_struct_field_set_array_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 * @calue: Value to set the field to
 *
 * Set the value of an array field in the given struct field.
 */
void
couchdb_struct_field_set_array_field (CouchdbStructField *sf, const char *field, CouchdbArrayField *value)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (field != NULL);
	g_return_if_fail (value != NULL);

	json_object_set_array_member (sf->json_object, field, json_array_ref (couchdb_array_field_get_json_array (value)));
}

/**
 * couchdb_struct_field_get_boolean_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 *
 * Retrieve the value of a boolean field from the given struct field.
 *
 * Return value: The value of the given field.
 */
gboolean
couchdb_struct_field_get_boolean_field (CouchdbStructField *sf, const char *field)
{
	g_return_val_if_fail (sf != NULL, FALSE);
	g_return_val_if_fail (field != NULL, FALSE);

	return json_object_get_boolean_member (sf->json_object, field);
}

/**
 * couchdb_struct_field_set_boolean_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 * @calue: Value to set the field to
 *
 * Set the value of a boolean field in the given struct field.
 */
void
couchdb_struct_field_set_boolean_field (CouchdbStructField *sf, const char *field, gboolean value)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (field != NULL);

	json_object_set_boolean_member (sf->json_object, field, value);
}

/**
 * couchdb_struct_field_get_double_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 *
 * Retrieve the value of a decimal number field from the given struct field.
 *
 * Return value: The value of the given field.
 */
gdouble
couchdb_struct_field_get_double_field (CouchdbStructField *sf, const char *field)
{
	g_return_val_if_fail (sf != NULL, 0);
	g_return_val_if_fail (field != NULL, 0);

	return json_object_get_double_member (sf->json_object, field);
}

/**
 * couchdb_struct_field_set_double_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 * @calue: Value to set the field to
 *
 * Set the value of a decimal number field in the given struct field.
 */
void
couchdb_struct_field_set_double_field (CouchdbStructField *sf, const char *field, gdouble value)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (field != NULL);

	json_object_set_double_member (sf->json_object, field, value);
}

/**
 * couchdb_struct_field_get_int_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 *
 * Retrieve the value of an integer field from the given struct field.
 *
 * Return value: The value of the given field.
 */
gint
couchdb_struct_field_get_int_field (CouchdbStructField *sf, const char *field)
{
	g_return_val_if_fail (sf != NULL, 0);
	g_return_val_if_fail (field != NULL, 0);

	return json_object_get_int_member (sf->json_object, field);
}

/**
 * couchdb_struct_field_set_int_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 * @calue: Value to set the field to
 *
 * Set the value of an integer field in the given struct field.
 */
void
couchdb_struct_field_set_int_field (CouchdbStructField *sf, const char *field, gint value)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (field != NULL);

	json_object_set_int_member (sf->json_object, field, value);
}

/**
 * couchdb_struct_field_get_string_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 *
 * Retrieve the value of a string field from the given struct field.
 *
 * Return value: The value of the given field.
 */
const char *
couchdb_struct_field_get_string_field (CouchdbStructField *sf, const char *field)
{
	g_return_val_if_fail (sf != NULL, NULL);
	g_return_val_if_fail (field != NULL, NULL);

	return json_object_get_string_member (sf->json_object, field);
}

/**
 * couchdb_struct_field_set_string_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 * @calue: Value to set the field to
 *
 * Set the value of a string field in the given struct field.
 */
void
couchdb_struct_field_set_string_field (CouchdbStructField *sf, const char *field, const char *value)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (field != NULL);

	if (value)
		json_object_set_string_member (sf->json_object, field, value);
	else {
		/* Remove the field if the value is NULL */
		couchdb_struct_field_remove_field (sf, field);
	}
}

/**
 * couchdb_struct_field_get_struct_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 *
 * Retrieve the value of a struct field from the given struct field.
 *
 * Return value: The value of the given field.
 */
CouchdbStructField *
couchdb_struct_field_get_struct_field (CouchdbStructField *sf, const char *field)
{
	g_return_val_if_fail (sf != NULL, NULL);
	g_return_val_if_fail (field != NULL, NULL);

	if (!json_object_has_member (sf->json_object, field))
		return NULL;

	return couchdb_struct_field_new_from_json_object (
		json_object_get_object_member (sf->json_object, field));
}

/**
 * couchdb_struct_field_set_struct_field:
 * @sf: A #CouchdbStructField object
 * @field: Name of the field
 * @calue: Value to set the field to
 *
 * Set the value of a string field in the given struct field.
 */
void
couchdb_struct_field_set_struct_field (CouchdbStructField *sf, const char *field, CouchdbStructField *value)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (field != NULL);
	g_return_if_fail (value != NULL);

	json_object_set_object_member (sf->json_object, field, json_object_ref (value->json_object));
}

/**
 * couchdb_struct_field_get_uuid:
 * @sf: A #CouchdbStructField object
 *
 * Retrieve the unique ID of the given struct field. Note that this is a convenience
 * function to allow documents with a format similar to:
 *
 * "list": {
 *        "unique-id-1": { "field": "value" },
 *        "unique-id-2": { "field": "value" }
 * }
 *
 * So, not all #CouchdbStructField objects would have a value for this, unless explicitly
 * used by the applications storing the documents on the CouchDB database.
 *
 * Return value: The unique ID of the given struct field.
 */
const char *
couchdb_struct_field_get_uuid (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return (const char *) sf->uuid;
}

/**
 * couchdb_struct_field_set_uuid:
 * @sf: A #CouchdbStructField object
 * @uuid: Unique ID
 *
 * Set the unique ID for the given struct field. See the explanation for #couchdb_struct_field_get_uuid
 * for knowing when to use this function.
 */
void
couchdb_struct_field_set_uuid (CouchdbStructField *sf, const char *uuid)
{
	g_return_if_fail (sf != NULL);

	if (sf->uuid)
		g_free (sf->uuid);

	sf->uuid = g_strdup (uuid);
}

/**
 * couchdb_struct_field_to_string:
 * @sf: A #CouchdbStructField object
 *
 * Convert a #CouchdbStructField to a JSON string.
 *
 * Return value: A string representing the contents of the given #CouchdbStructField
 * object in JSON format.
 */
char *
couchdb_struct_field_to_string (CouchdbStructField *sf)
{
	JsonNode *node;
	JsonGenerator *generator;
	gsize size;
	char *str = NULL;

	g_return_val_if_fail (sf != NULL, NULL);

	node = json_node_new (JSON_NODE_OBJECT);
	json_node_set_object (node, sf->json_object);

	generator = json_generator_new ();
	json_generator_set_root (generator, node);

	str = json_generator_to_data (generator, &size);
	g_object_unref (G_OBJECT (generator));

	json_node_free (node);

	return str;
}

JsonObject *
couchdb_struct_field_get_json_object (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);
	
	return sf->json_object;
}

