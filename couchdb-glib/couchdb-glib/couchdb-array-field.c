/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2009-2010 Canonical Services Ltd (www.canonical.com)
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

#include <json-glib/json-glib.h>
#include "utils.h"
#include "couchdb-array-field.h"
#include "couchdb-struct-field.h"

struct _CouchdbArrayField {
	gint ref_count;
	JsonArray *json_array;
};

GType
couchdb_array_field_get_type (void)
{
	static GType object_type = 0;

	if (G_UNLIKELY (!object_type))
		object_type = g_boxed_type_register_static (g_intern_static_string ("CouchdbArrayField"),
							    (GBoxedCopyFunc) couchdb_array_field_ref,
							    (GBoxedFreeFunc) couchdb_array_field_unref);

	return object_type;
}

/**
 * couchdb_array_field_new:
 *
 * Create a new array field object.
 *
 * Return value: A newly-created #CouchdbArrayField object.
 */
CouchdbArrayField *
couchdb_array_field_new (void)
{
	CouchdbArrayField *array;

	array = g_slice_new (CouchdbArrayField);
	array->ref_count = 1;
	array->json_array = json_array_new ();

	return array;
}

CouchdbArrayField *
couchdb_array_field_new_from_json_array (JsonArray *json_array)
{
	CouchdbArrayField *array;

	array = g_slice_new (CouchdbArrayField);
	array->ref_count = 1;
	array->json_array = json_array_ref (json_array);

	return array;
}

/**
 * couchdb_array_field_ref:
 * @array: A #CouchdbArrayField object
 *
 * Increments reference count of a #CouchdbArrayField object.
 *
 * Return value: A pointer to the referenced object.
 */
CouchdbArrayField *
couchdb_array_field_ref (CouchdbArrayField *array)
{
	g_return_val_if_fail (array != NULL, NULL);
	g_return_val_if_fail (array->ref_count > 0, NULL);

	g_atomic_int_exchange_and_add (&array->ref_count, 1);

	return array;
}

/**
 * couchdb_array_field_unref:
 * @array: A #CouchdbArrayField object
 *
 * Decrements reference count of a #CouchdbArrayField object. When
 * the reference count is equal to 0, the object will be destroyed and
 * the memory it uses freed.
 */
void
couchdb_array_field_unref (CouchdbArrayField *array)
{
	gint old_ref;

	g_return_if_fail (array != NULL);
	g_return_if_fail (array->ref_count > 0);

	old_ref = g_atomic_int_get (&array->ref_count);
	if (old_ref > 1)
		g_atomic_int_compare_and_exchange (&array->ref_count, old_ref, old_ref - 1);
	else {
		json_array_unref (array->json_array);
		g_slice_free (CouchdbArrayField, array);
	}
}

/**
 * couchdb_array_field_get_length:
 * @array: A #CouchdbArrayField object
 *
 * Get the number of elements on the given #CouchdbArrayField object.
 *
 * Return value: Number of elements in the given array.
 */
guint
couchdb_array_field_get_length (CouchdbArrayField *array)
{
	g_return_val_if_fail (array != NULL, 0);

	return json_array_get_length (array->json_array);
}

/**
 * couchdb_array_field_add_array_element:
 * @array: A #CouchdbArrayField object
 * @value: Value to be added
 *
 * Add a new element of type array to the given array.
 */
void
couchdb_array_field_add_array_element (CouchdbArrayField *array, const CouchdbArrayField *value)
{
	g_return_if_fail (array != NULL);

	json_array_add_array_element (array->json_array, json_array_ref (value->json_array));
}

/**
 * couchdb_array_field_add_boolean_element:
 * @array: A #CouchdbArrayField object
 * @value: Value to be added
 *
 * Add a new element of type boolean to the given array.
 */
void
couchdb_array_field_add_boolean_element (CouchdbArrayField *array, gboolean value)
{
	g_return_if_fail (array != NULL);

	json_array_add_boolean_element (array->json_array, value);
}

/**
 * couchdb_array_field_add_int_element:
 * @array: A #CouchdbArrayField object
 * @value: Value to be added
 *
 * Add a new element of type integer to the given array.
 */
void
couchdb_array_field_add_int_element (CouchdbArrayField *array, gint value)
{
	g_return_if_fail (array != NULL);

	json_array_add_int_element (array->json_array, value);
}

/**
 * couchdb_array_field_add_double_element:
 * @array: A #CouchdbArrayField object
 * @value: Value to be added
 *
 * Add a new element of type double to the given array.
 */
void
couchdb_array_field_add_double_element (CouchdbArrayField *array, gdouble value)
{
	g_return_if_fail (array != NULL);

	json_array_add_double_element (array->json_array, value);
}

/**
 * couchdb_array_field_add_string_element:
 * @array: A #CouchdbArrayField object
 * @value: Value to be added
 *
 * Add a new element of type string to the given array.
 */
void
couchdb_array_field_add_string_element (CouchdbArrayField *array, const gchar *value)
{
	g_return_if_fail (array != NULL);

	json_array_add_string_element (array->json_array, value);
}

/**
 * couchdb_array_field_add_struct_element:
 * @array: A #CouchdbArrayField object
 * @value: Value to be added
 *
 * Add a new element of type struct to the given array.
 */
void
couchdb_array_field_add_struct_element (CouchdbArrayField *array, const CouchdbStructField *value)
{
	g_return_if_fail (array != NULL);

	json_array_add_object_element (
		array->json_array,
		json_object_ref (couchdb_struct_field_get_json_object ((CouchdbStructField *) value)));
}

/**
 * couchdb_array_field_remove_element:
 * @array: A #CouchdbArrayField object
 * @index: Position of the element to remove
 *
 * Remove an element from the given #CouchdbArrayField object.
 */
void
couchdb_array_field_remove_element (CouchdbArrayField *array, guint index)
{
	g_return_if_fail (array != NULL);

	json_array_remove_element (array->json_array, index);
}

/**
 * couchdb_array_field_get_array_element:
 * @array: A #CouchdbArrayField object
 * @index: Position of the element to retrieve
 *
 * Retrieve an array value on the given position of the array.
 *
 * Return value: Value of the element stored in the given position of the array.
 */
CouchdbArrayField *
couchdb_array_field_get_array_element (CouchdbArrayField *array, guint index)
{
	JsonArray *json_array;
	CouchdbArrayField *returned_array = NULL;
	g_return_val_if_fail (array != NULL, NULL);

	json_array = json_array_get_array_element (array->json_array, index);
	if (json_array) {
		guint i;

		returned_array = couchdb_array_field_new ();
		for (i = 0; i < json_array_get_length (json_array); i++) {
			json_array_add_element (returned_array->json_array,
						json_array_get_element (json_array, index));
		}
	}

	return returned_array;
}

/**
 * couchdb_array_field_get_boolean_element:
 * @array: A #CouchdbArrayField object
 * @index: Position of the element to retrieve
 *
 * Retrieve a boolean value on the given position of the array.
 *
 * Return value: Value of the element stored in the given position of the array.
 */
gboolean
couchdb_array_field_get_boolean_element (CouchdbArrayField *array, guint index)
{
	g_return_val_if_fail (array != NULL, FALSE);

	return json_array_get_boolean_element (array->json_array, index);
}

/**
 * couchdb_array_field_get_double_element:
 * @array: A #CouchdbArrayField object
 * @index: Position of the element to retrieve
 *
 * Retrieve a double value on the given position of the array.
 *
 * Return value: Value of the element stored in the given position of the array.
 */
gdouble
couchdb_array_field_get_double_element (CouchdbArrayField *array, guint index)
{
	g_return_val_if_fail (array != NULL, -1);

	return json_array_get_double_element (array->json_array, index);
}

/**
 * couchdb_array_field_get_int_element:
 * @array: A #CouchdbArrayField object
 * @index: Position of the element to retrieve
 *
 * Retrieve an integer value on the given position of the array.
 *
 * Return value: Value of the element stored in the given position of the array.
 */
gint
couchdb_array_field_get_int_element (CouchdbArrayField *array, guint index)
{
	g_return_val_if_fail (array != NULL, -1);

	return json_array_get_int_element (array->json_array, index);
}

/**
 * couchdb_array_field_get_string_element:
 * @array: A #CouchdbArrayField object
 * @index: Position of the element to retrieve
 *
 * Retrieve a string value on the given position of the array.
 *
 * Return value: Value of the element stored in the given position of the array.
 */
const gchar *
couchdb_array_field_get_string_element (CouchdbArrayField *array, guint index)
{
	g_return_val_if_fail (array != NULL, NULL);

	return json_array_get_string_element (array->json_array, index);
}

/**
 * couchdb_array_field_get_struct_element:
 * @array: A #CouchdbArrayField object
 * @index: Position of the element to retrieve
 *
 * Retrieve a struct value on the given position of the array.
 *
 * Return value: Value of the element stored in the given position of the array.
 */
CouchdbStructField *
couchdb_array_field_get_struct_element (CouchdbArrayField *array, guint index)
{
	JsonObject *json_object;

	g_return_val_if_fail (array != NULL, NULL);

	json_object = json_array_get_object_element (array->json_array, index);
	if (json_object != NULL)
		return couchdb_struct_field_new_from_json_object (json_object);

	return NULL;
}

JsonArray *
couchdb_array_field_get_json_array (CouchdbArrayField *array)
{
	g_return_val_if_fail (array != NULL, NULL);

	return array->json_array;
}
