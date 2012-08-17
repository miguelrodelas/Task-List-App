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

#include "desktopcouch-document-contact.h"
#include "utils.h"

CouchdbDocument *
desktopcouch_document_contact_new (CouchdbSession *couchdb)
{
	CouchdbDocument *document;

	g_return_val_if_fail (COUCHDB_IS_SESSION (couchdb), NULL);

	document = couchdb_document_new (couchdb);
	if (document)
		desktopcouch_document_set_record_type (document, DESKTOPCOUCH_RECORD_TYPE_CONTACT);

	return document;
}

gboolean
desktopcouch_document_is_contact (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), FALSE);

	return !g_ascii_strcasecmp (desktopcouch_document_get_record_type (document),
				    DESKTOPCOUCH_RECORD_TYPE_CONTACT);
}

const char *
desktopcouch_document_contact_get_title (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "title");
}

void
desktopcouch_document_contact_set_title (CouchdbDocument *document, const char *title)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (title != NULL);

	couchdb_document_set_string_field (document, "title", title);
}

const char *
desktopcouch_document_contact_get_first_name (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "first_name");
}

void
desktopcouch_document_contact_set_first_name (CouchdbDocument *document, const char *first_name)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (first_name != NULL);

	couchdb_document_set_string_field (document, "first_name", first_name);
}

const char *
desktopcouch_document_contact_get_middle_name (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "middle_name");
}

void
desktopcouch_document_contact_set_middle_name (CouchdbDocument *document, const char *middle_name)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (middle_name != NULL);

	couchdb_document_set_string_field (document, "middle_name", middle_name);
}

const char *
desktopcouch_document_contact_get_last_name (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "last_name");
}

void
desktopcouch_document_contact_set_last_name (CouchdbDocument *document, const char *last_name)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (last_name != NULL);

	couchdb_document_set_string_field (document, "last_name", last_name);
}

const char *
desktopcouch_document_contact_get_suffix (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "suffix");
}

void
desktopcouch_document_contact_set_suffix (CouchdbDocument *document, const char *suffix)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (suffix != NULL);

	couchdb_document_set_string_field (document, "suffix", suffix);
}

const char *
desktopcouch_document_contact_get_nick_name (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "nick_name");
}

void
desktopcouch_document_contact_set_nick_name (CouchdbDocument *document, const char *nick_name)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (nick_name != NULL);

	couchdb_document_set_string_field (document, "nick_name", nick_name);
}

const char *
desktopcouch_document_contact_get_spouse_name (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "spouse_name");
}

void
desktopcouch_document_contact_set_spouse_name (CouchdbDocument *document, const char *spouse_name)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (spouse_name != NULL);

	couchdb_document_set_string_field (document, "spouse_name", spouse_name);
}

const char *
desktopcouch_document_contact_get_birth_date (CouchdbDocument *document)
{
	JsonObject *object;

	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "birth_date");
}

void
desktopcouch_document_contact_set_birth_date (CouchdbDocument *document, const char *birth_date)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (birth_date != NULL);

	couchdb_document_set_string_field (document, "birth_date", birth_date);
}

const char *
desktopcouch_document_contact_get_wedding_date (CouchdbDocument *document)
{
	JsonObject *object;

	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "wedding_date");
}

void
desktopcouch_document_contact_set_wedding_date (CouchdbDocument *document, const char *wedding_date)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (wedding_date != NULL);

	couchdb_document_set_string_field (document, "wedding_date", wedding_date);
}

const char *
desktopcouch_document_contact_get_company (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "company");
}

void
desktopcouch_document_contact_set_company (CouchdbDocument *document, const char *company)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (company != NULL);

	couchdb_document_set_string_field (document, "company", company);
}

const char *
desktopcouch_document_contact_get_department (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "department");
}

void
desktopcouch_document_contact_set_department (CouchdbDocument *document, const char *department)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (department != NULL);

	couchdb_document_set_string_field (document, "department", department);
}

const char *
desktopcouch_document_contact_get_job_title (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "job_title");
}

void
desktopcouch_document_contact_set_job_title (CouchdbDocument *document, const char *job_title)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (job_title != NULL);

	couchdb_document_set_string_field (document, "job_title", job_title);
}

const char *
desktopcouch_document_contact_get_manager_name (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "manager_name");
}

void
desktopcouch_document_contact_set_manager_name (CouchdbDocument *document, const char *manager_name)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (manager_name != NULL);

	couchdb_document_set_string_field (document, "manager_name", manager_name);
}

const char *
desktopcouch_document_contact_get_assistant_name (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "assistant_name");
}

void
desktopcouch_document_contact_set_assistant_name (CouchdbDocument *document, const char *assistant_name)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (assistant_name != NULL);

	couchdb_document_set_string_field (document, "assistant_name", assistant_name);
}

const char *
desktopcouch_document_contact_get_office (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "office");
}

void
desktopcouch_document_contact_set_office (CouchdbDocument *document, const char *office)
{
      	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (office != NULL);

	couchdb_document_set_string_field (document, "office", office);
}

static void
foreach_object_cb (JsonObject *object,
		  const char *member_name,
		  JsonNode *member_node,
		  gpointer user_data)
{
	GSList **list = (GSList **) user_data;

	if (json_node_get_node_type (member_node) == JSON_NODE_OBJECT) {
		CouchdbStructField *sf;

		sf = couchdb_struct_field_new_from_json_object (
			json_object_ref (json_node_get_object (member_node)));
		couchdb_struct_field_set_uuid (sf, member_name);
		*list = g_slist_prepend (*list, sf);
	}
}

/**
 * desktopcouch_document_contact_get_email_addresses:
 * @document: A #CouchdbDocument object representing a contact
 *
 * Retrieve a list of email addresses from the specified contact document.
 * Email addresses are returned in a GSList of #CouchdbStructField objects,
 * which can be manipulated with the desktopcouch_document_contact_email_* functions
 * and freed with:
 *     g_slist_foreach (list, (GFunc) couchdb_struct_field_unref, NULL);
 *     g_slist_free (list);
 *
 * Return value: a #GSList of #CouchdbStructField objects.
 */
GSList *
desktopcouch_document_contact_get_email_addresses (CouchdbDocument *document)
{
	GSList *list = NULL;
	JsonObject *addresses_json;

	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	addresses_json = json_object_get_object_member (
		couchdb_document_get_json_object (document), "email_addresses");;
	if (addresses_json) {
		json_object_foreach_member (addresses_json,
					    (JsonObjectForeach) foreach_object_cb,
					    &list);
	}

	return list;
}

void
desktopcouch_document_contact_set_email_addresses (CouchdbDocument *document, GSList *list)
{
	GSList *l;
	JsonObject *addresses_json;

	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));

	addresses_json = json_object_new ();
	for (l = list; l != NULL; l = l->next) {
		const gchar *address_str;
		CouchdbStructField *sf = (CouchdbStructField *) l->data;

		address_str = desktopcouch_document_contact_email_get_address (sf);
		if (address_str) {
			JsonObject *this_address;

			this_address = json_object_new ();
			json_object_set_string_member (this_address, "address", address_str);
			json_object_set_string_member (this_address, "description",
						       desktopcouch_document_contact_email_get_description (sf));

			json_object_set_object_member (addresses_json,
						       couchdb_struct_field_get_uuid (sf),
						       this_address);
		}
	}

	json_object_set_object_member (couchdb_document_get_json_object (document), "email_addresses", addresses_json);
}

GSList *
desktopcouch_document_contact_get_phone_numbers (CouchdbDocument *document)
{
	GSList *list = NULL;
	JsonObject *phone_numbers;

	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	phone_numbers = json_object_get_object_member (
		couchdb_document_get_json_object (document), "phone_numbers");
	if (phone_numbers) {
		json_object_foreach_member (phone_numbers,
					    (JsonObjectForeach) foreach_object_cb,
					    &list);
	}

	return list;
}

void
desktopcouch_document_contact_set_phone_numbers (CouchdbDocument *document, GSList *list)
{
	GSList *l;
	JsonObject *phone_numbers;

	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));

	phone_numbers = json_object_new ();
	for (l = list; l != NULL; l = l->next) {
		const gchar *number_str;
		CouchdbStructField *sf = (CouchdbStructField *) l->data;

		number_str = desktopcouch_document_contact_phone_get_number (sf);
		if (number_str) {
			JsonObject *this_phone;

			this_phone = json_object_new ();
			json_object_set_string_member (this_phone, "number", number_str);
			json_object_set_string_member (this_phone, "description",
						       desktopcouch_document_contact_phone_get_description (sf));
			json_object_set_int_member (this_phone, "priority",
						    desktopcouch_document_contact_phone_get_priority (sf));

			json_object_set_object_member (phone_numbers,
						       couchdb_struct_field_get_uuid (sf),
						       this_phone);
		}
	}

	json_object_set_object_member (couchdb_document_get_json_object (document), "phone_numbers", phone_numbers);
}

GSList *
desktopcouch_document_contact_get_addresses (CouchdbDocument *document)
{
	GSList *list = NULL;
	JsonObject *addresses;

	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	addresses = json_object_get_object_member (
		couchdb_document_get_json_object (document), "addresses");
	if (addresses) {
		json_object_foreach_member (addresses,
					    (JsonObjectForeach) foreach_object_cb,
					    &list);
	}

	return list;
}

void
desktopcouch_document_contact_set_addresses (CouchdbDocument *document, GSList *list)
{
	GSList *l;
	JsonObject *addresses;

	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));

	addresses = json_object_new ();
	for (l = list; l != NULL; l = l->next) {
		const gchar *street_str;
		CouchdbStructField *sf = (CouchdbStructField *) l->data;

		street_str = desktopcouch_document_contact_address_get_street (sf);
		if (street_str) {
			JsonObject *this_address;

			this_address = json_object_new ();
			json_object_set_string_member (this_address, "street", street_str);
			json_object_set_string_member (this_address, "city",
						       desktopcouch_document_contact_address_get_city (sf));
			json_object_set_string_member (this_address, "state",
						       desktopcouch_document_contact_address_get_state (sf));
			json_object_set_string_member (this_address, "country",
						       desktopcouch_document_contact_address_get_country (sf));
			json_object_set_string_member (this_address, "postalcode",
						       desktopcouch_document_contact_address_get_postalcode (sf));
			json_object_set_string_member (this_address, "pobox",
						       desktopcouch_document_contact_address_get_pobox (sf));
			json_object_set_string_member (this_address, "description",
						       desktopcouch_document_contact_address_get_description (sf));

			json_object_set_object_member (addresses,
						       couchdb_struct_field_get_uuid (sf),
						       this_address);
		}
	}

	json_object_set_object_member (couchdb_document_get_json_object (document), "addresses", addresses);
}

GSList *
desktopcouch_document_contact_get_im_addresses (CouchdbDocument *document)
{
	GSList *list = NULL;
	JsonObject *im_addresses;

	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	im_addresses = json_object_get_object_member (
		couchdb_document_get_json_object (document), "im_addresses");
	if (im_addresses != NULL) {
		json_object_foreach_member (im_addresses,
					    (JsonObjectForeach) foreach_object_cb,
					    &list);
	}

	return list;
}

void
desktopcouch_document_contact_set_im_addresses (CouchdbDocument *document, GSList *list)
{
	GSList *l;
	JsonObject *im_addresses_json;

	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));

	im_addresses_json = json_object_new ();
	for (l = list; l != NULL; l = l->next) {
		const gchar *address_str;
		CouchdbStructField *sf = (CouchdbStructField *) l->data;

		address_str = desktopcouch_document_contact_im_get_address (sf);
		if (address_str != NULL) {
			JsonObject *this_address;

			this_address = json_object_new ();
			json_object_set_string_member (this_address, "address", address_str);
			json_object_set_string_member (this_address, "description",
						       desktopcouch_document_contact_im_get_description (sf));
			json_object_set_string_member (this_address, "protocol",
						       desktopcouch_document_contact_im_get_protocol (sf));

			json_object_set_object_member (im_addresses_json,
						       couchdb_struct_field_get_uuid (sf),
						       this_address);
		}
	}

	json_object_set_object_member (couchdb_document_get_json_object (document), "im_addresses", im_addresses_json);
}

GSList *
desktopcouch_document_contact_get_urls (CouchdbDocument *document)
{
	GSList *list = NULL;
	JsonObject *urls;

	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	urls = json_object_get_object_member (
		couchdb_document_get_json_object (document), "urls");
	if (urls) {
		json_object_foreach_member (urls,
					    (JsonObjectForeach) foreach_object_cb,
					    &list);
	}

	return list;
}

void
desktopcouch_document_contact_set_urls (CouchdbDocument *document, GSList *list)
{
	GSList *l;
	JsonObject *urls_json;

	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));

	urls_json = json_object_new ();
	for (l = list; l != NULL; l = l->next) {
		const gchar *address_str;
		CouchdbStructField *sf = (CouchdbStructField *) l->data;

		address_str = desktopcouch_document_contact_url_get_address (sf);
		if (address_str) {
			JsonObject *this_url;

			this_url = json_object_new ();
			json_object_set_string_member (this_url, "address", address_str);
			json_object_set_string_member (this_url, "description",
						       desktopcouch_document_contact_url_get_description (sf));

			json_object_set_object_member (urls_json,
						       couchdb_struct_field_get_uuid (sf),
						       this_url);
		}
	}

	json_object_set_object_member (couchdb_document_get_json_object (document), "urls", urls_json);
}

/**
 * desktopcouch_document_contact_get_categories:
 * @document: A #CouchdbDocument object
 *
 * Get the list of categories (as a string) for this contact document.
 *
 * Return value: A comma separated list of categories as a string.
 */
const char *
desktopcouch_document_contact_get_categories (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "categories");
}

void
desktopcouch_document_contact_set_categories (CouchdbDocument *document, const char *categories)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (categories != NULL);

	couchdb_document_set_string_field (document, "categories", categories);
}

const char *
desktopcouch_document_contact_get_notes (CouchdbDocument *document)
{
	g_return_val_if_fail (COUCHDB_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (desktopcouch_document_is_contact (document), NULL);

	return couchdb_document_get_string_field (document, "notes");
}

void
desktopcouch_document_contact_set_notes (CouchdbDocument *document, const char *notes)
{
	g_return_if_fail (COUCHDB_IS_DOCUMENT (document));
	g_return_if_fail (desktopcouch_document_is_contact (document));
	g_return_if_fail (notes != NULL);

	couchdb_document_set_string_field (document, "notes", notes);
}

CouchdbStructField *
desktopcouch_document_contact_email_new (const char *uuid, const char *address, const char *description)
{
	CouchdbStructField *sf;

	sf = couchdb_struct_field_new_from_json_object (json_object_new ());
	if (uuid != NULL)
		couchdb_struct_field_set_uuid (sf, uuid);
	else {
		char *new_uuid = generate_uuid ();
		couchdb_struct_field_set_uuid (sf, new_uuid);
		g_free (new_uuid);
	}

	if (address)
		desktopcouch_document_contact_email_set_address (sf, address);
	if (description)
		desktopcouch_document_contact_email_set_description (sf, description);

	return sf;
}

const char *
desktopcouch_document_contact_email_get_address (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "address");
}

void
desktopcouch_document_contact_email_set_address (CouchdbStructField *sf, const char *email)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (email != NULL);

	couchdb_struct_field_set_string_field (sf, "address", email);
}

const char *
desktopcouch_document_contact_email_get_description (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "description");
}

void
desktopcouch_document_contact_email_set_description (CouchdbStructField *sf, const char *description)
{
	g_return_if_fail (sf != NULL);

	couchdb_struct_field_set_string_field (sf, "description", description);
}

CouchdbStructField *
desktopcouch_document_contact_phone_new (const char *uuid, const char *number, const char *description, gint priority)
{
	CouchdbStructField *sf;

	sf = couchdb_struct_field_new_from_json_object (json_object_new ());
	if (uuid != NULL)
		couchdb_struct_field_set_uuid (sf, uuid);
	else {
		char *new_uuid = generate_uuid ();
		couchdb_struct_field_set_uuid (sf, new_uuid);
		g_free (new_uuid);
	}

	if (number)
		desktopcouch_document_contact_phone_set_number (sf, number);
	if (description)
		desktopcouch_document_contact_phone_set_description (sf, description);
	desktopcouch_document_contact_phone_set_priority (sf, priority);

	return sf;
}

gint
desktopcouch_document_contact_phone_get_priority (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, 0);

	return couchdb_struct_field_get_int_field (sf, "priority");
}

void
desktopcouch_document_contact_phone_set_priority (CouchdbStructField *sf, gint priority)
{
	g_return_if_fail (sf != NULL);

	couchdb_struct_field_set_int_field (sf, "priority", priority);
}

const char *
desktopcouch_document_contact_phone_get_number (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "number");
}

void
desktopcouch_document_contact_phone_set_number (CouchdbStructField *sf, const char *number)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (number != NULL);

	couchdb_struct_field_set_string_field (sf, "number", number);
}

const char *
desktopcouch_document_contact_phone_get_description (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "description");
}

void
desktopcouch_document_contact_phone_set_description (CouchdbStructField *sf, const char *description)
{
	g_return_if_fail (sf != NULL);

	couchdb_struct_field_set_string_field (sf, "description", description);
}

CouchdbStructField *
desktopcouch_document_contact_address_new (const char *uuid,
				      const char *street,
				      const char *city,
				      const char *state,
				      const char *country,
				      const char *postalcode,
				      const char *pobox,
				      const char *description)
{
	CouchdbStructField *sf;

	sf = couchdb_struct_field_new_from_json_object (json_object_new ());
	if (uuid != NULL)
		couchdb_struct_field_set_uuid (sf, uuid);
	else {
		char *new_uuid = generate_uuid ();
		couchdb_struct_field_set_uuid (sf, new_uuid);
		g_free (new_uuid);
	}

	if (street)
		desktopcouch_document_contact_address_set_street (sf, street);
	if (city)
		desktopcouch_document_contact_address_set_city (sf, city);
	if (state)
		desktopcouch_document_contact_address_set_state (sf, state);
	if (country)
		desktopcouch_document_contact_address_set_country (sf, country);
	if (postalcode)
		desktopcouch_document_contact_address_set_postalcode (sf, postalcode);
	if (pobox)
		desktopcouch_document_contact_address_set_pobox (sf, pobox);
	if (description)
		desktopcouch_document_contact_address_set_description (sf, description);

	return sf;
}

const char *
desktopcouch_document_contact_address_get_street (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "street");
}

void
desktopcouch_document_contact_address_set_street (CouchdbStructField *sf, const char *street)
{
	g_return_if_fail (sf != NULL);

	couchdb_struct_field_set_string_field (sf, "street", street);
}

const char *
desktopcouch_document_contact_address_get_city (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "city");
}

void
desktopcouch_document_contact_address_set_city (CouchdbStructField *sf, const char *city)
{
	g_return_if_fail (sf != NULL);

	couchdb_struct_field_set_string_field (sf, "city", city);
}

const char *
desktopcouch_document_contact_address_get_state (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "state");
}

void
desktopcouch_document_contact_address_set_state (CouchdbStructField *sf, const char *state)
{
	g_return_if_fail (sf != NULL);

	couchdb_struct_field_set_string_field (sf, "state", state);
}

const char *
desktopcouch_document_contact_address_get_country (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "country");
}

void
desktopcouch_document_contact_address_set_country (CouchdbStructField *sf, const char *country)
{
	g_return_if_fail (sf != NULL);

	couchdb_struct_field_set_string_field (sf, "country", country);
}

const char *
desktopcouch_document_contact_address_get_postalcode (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "postalcode");
}

void
desktopcouch_document_contact_address_set_postalcode (CouchdbStructField *sf, const char *postalcode)
{
	g_return_if_fail (sf != NULL);

	couchdb_struct_field_set_string_field (sf, "postalcode", postalcode);
}

const char *
desktopcouch_document_contact_address_get_pobox (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "pobox");
}

void
desktopcouch_document_contact_address_set_pobox (CouchdbStructField *sf, const char *pobox)
{
	g_return_if_fail (sf != NULL);

	couchdb_struct_field_set_string_field (sf, "pobox", pobox);
}

const char *
desktopcouch_document_contact_address_get_description (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "description");
}

void
desktopcouch_document_contact_address_set_description (CouchdbStructField *sf, const char *description)
{
	g_return_if_fail (sf != NULL);

	couchdb_struct_field_set_string_field (sf, "description", description);
}

CouchdbStructField *
desktopcouch_document_contact_im_new (const char *uuid,
				 const char *address,
				 const char *description,
				 const char *protocol)
{
	CouchdbStructField *sf;

	sf = couchdb_struct_field_new_from_json_object (json_object_new ());
	if (uuid != NULL)
		couchdb_struct_field_set_uuid (sf, uuid);
	else {
		char *new_uuid = generate_uuid ();
		couchdb_struct_field_set_uuid (sf, new_uuid);
		g_free (new_uuid);
	}

	if (address != NULL)
		desktopcouch_document_contact_im_set_address (sf, address);
	if (description != NULL)
		desktopcouch_document_contact_im_set_description (sf, description);
	if (protocol != NULL)
		desktopcouch_document_contact_im_set_protocol (sf, protocol);

	return sf;
}

const char *
desktopcouch_document_contact_im_get_address (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "address");
}

void
desktopcouch_document_contact_im_set_address (CouchdbStructField *sf, const char *address)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (address != NULL);

	couchdb_struct_field_set_string_field (sf, "address", address);
}

const char *
desktopcouch_document_contact_im_get_description (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "description");
}

void
desktopcouch_document_contact_im_set_description (CouchdbStructField *sf, const char *description)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (description != NULL);

	couchdb_struct_field_set_string_field (sf, "description", description);
}

const char *
desktopcouch_document_contact_im_get_protocol (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "protocol");
}

void
desktopcouch_document_contact_im_set_protocol (CouchdbStructField *sf, const char *protocol)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (protocol != NULL);

	couchdb_struct_field_set_string_field (sf, "protocol", protocol);
}

CouchdbStructField *
desktopcouch_document_contact_url_new (const char *uuid, const char *address, const char *description)
{
	CouchdbStructField *sf;

	sf = couchdb_struct_field_new_from_json_object (json_object_new ());
	if (uuid != NULL)
		couchdb_struct_field_set_uuid (sf, uuid);
	else {
		char *new_uuid = generate_uuid ();
		couchdb_struct_field_set_uuid (sf, new_uuid);
		g_free (new_uuid);
	}

	if (address)
		desktopcouch_document_contact_url_set_address (sf, address);
	if (description)
		desktopcouch_document_contact_url_set_description (sf, description);

	return sf;
}

const char *
desktopcouch_document_contact_url_get_address (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "address");
}

void
desktopcouch_document_contact_url_set_address (CouchdbStructField *sf, const char *url)
{
	g_return_if_fail (sf != NULL);
	g_return_if_fail (url != NULL);

	couchdb_struct_field_set_string_field (sf, "address", url);
}

const char *
desktopcouch_document_contact_url_get_description (CouchdbStructField *sf)
{
	g_return_val_if_fail (sf != NULL, NULL);

	return couchdb_struct_field_get_string_field (sf, "description");
}

void
desktopcouch_document_contact_url_set_description (CouchdbStructField *sf, const char *description)
{
	g_return_if_fail (sf != NULL);

	couchdb_struct_field_set_string_field (sf, "description", description);
}
