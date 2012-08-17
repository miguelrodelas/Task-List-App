/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* e-book-backend-couchdb.c - CouchDB contact backend
 *
 * Copyright (C) 2009 Canonical, Ltd. (www.canonical.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Rodrigo Moya <rodrigo.moya@canonical.com>
 */

#include <string.h>
#include "e-book-backend-couchdb.h"
#include <libedata-book/e-book-backend-sexp.h>
#include <libedata-book/e-data-book.h>
#include <libedata-book/e-data-book-view.h>
#include <dbus/dbus-glib.h>
#include <gnome-keyring.h>

#define COUCHDB_REVISION_PROP                "X-COUCHDB-REVISION"
#define COUCHDB_UUID_PROP                    "X-COUCHDB-UUID"
#define COUCHDB_APPLICATION_ANNOTATIONS_PROP "X-COUCHDB-APPLICATION-ANNOTATIONS"

G_DEFINE_TYPE (EBookBackendCouchDB, e_book_backend_couchdb, E_TYPE_BOOK_BACKEND);

static void
get_current_time (gchar time_string[100])
{
	const struct tm *tm = NULL;
	time_t t;

	t = time (NULL);
	tm = gmtime (&t);
	if (tm)
		strftime (time_string, 100, "%Y-%m-%dT%H:%M:%SZ", tm);
}

static EContact *
contact_from_couch_document (CouchdbDocument *document)
{
	EContact *contact;
	char *str;
	GSList *list, *sl;
	GList *attr_list;
	CouchdbStructField *app_annotations;
	EContactName contact_name;

	if (!desktopcouch_document_is_contact (document))
		return NULL;

	/* Check if the contact is marked for deletion */
	if ((app_annotations = desktopcouch_document_get_application_annotations (document))) {
		CouchdbStructField *u1_annotations;

		u1_annotations = couchdb_struct_field_get_struct_field (
			app_annotations, "Ubuntu One");
		if (u1_annotations != NULL) {
			CouchdbStructField *private_annotations;

			private_annotations = couchdb_struct_field_get_struct_field (
				u1_annotations, "private_application_annotations");
			if (private_annotations != NULL) {
				if (couchdb_struct_field_has_field (private_annotations, "deleted")
				    && couchdb_struct_field_get_boolean_field (private_annotations, "deleted"))
					couchdb_struct_field_unref (app_annotations);
					return NULL;
			}
		}
	}

	/* Fill in the EContact with the data from the CouchDBDocument */
	contact = e_contact_new ();
	e_vcard_add_attribute_with_value (E_VCARD (contact),
					  e_vcard_attribute_new (NULL, COUCHDB_REVISION_PROP),
					  couchdb_document_get_revision (document));

	e_contact_set (contact, E_CONTACT_UID, (const gpointer) couchdb_document_get_id (document));

	contact_name.family = desktopcouch_document_contact_get_last_name (document);
	contact_name.given = desktopcouch_document_contact_get_first_name (document);
	contact_name.additional = desktopcouch_document_contact_get_middle_name (document);
	contact_name.prefixes = desktopcouch_document_contact_get_title (document);
	contact_name.suffixes = desktopcouch_document_contact_get_suffix (document);
	e_contact_set (contact, E_CONTACT_NAME, (const gpointer) &contact_name);

	str = e_contact_name_to_string (&contact_name);
	e_contact_set (contact, E_CONTACT_FULL_NAME, (const gpointer) str);
	g_free (str);

	e_contact_set (contact, E_CONTACT_NICKNAME,
		       (const gpointer) desktopcouch_document_contact_get_nick_name (document));
	e_contact_set (contact, E_CONTACT_SPOUSE,
		       (const gpointer) desktopcouch_document_contact_get_spouse_name (document));

	e_contact_set (contact, E_CONTACT_ORG,
		       (const gpointer) desktopcouch_document_contact_get_company (document));
	e_contact_set (contact, E_CONTACT_ORG_UNIT,
		       (const gpointer) desktopcouch_document_contact_get_department (document));
	e_contact_set (contact, E_CONTACT_TITLE,
		       (const gpointer) desktopcouch_document_contact_get_job_title (document));
	e_contact_set (contact, E_CONTACT_MANAGER,
		       (const gpointer) desktopcouch_document_contact_get_manager_name (document));
	e_contact_set (contact, E_CONTACT_ASSISTANT,
		       (const gpointer) desktopcouch_document_contact_get_assistant_name (document));
	e_contact_set (contact, E_CONTACT_OFFICE,
		       (const gpointer) desktopcouch_document_contact_get_office (document));
	e_contact_set (contact, E_CONTACT_CATEGORIES,
		       (const gpointer) desktopcouch_document_contact_get_categories (document));
	e_contact_set (contact, E_CONTACT_NOTE,
		       (const gpointer) desktopcouch_document_contact_get_notes (document));

	/* parse email addresses */
	attr_list = NULL;

	list = desktopcouch_document_contact_get_email_addresses (document);
	while (list != NULL) {
		const char *email_str, *description_str, *uuid_str;
		EVCardAttribute *attr;
		CouchdbStructField *email_address = (CouchdbStructField *) list->data;

		email_str = desktopcouch_document_contact_email_get_address (email_address);
		description_str = desktopcouch_document_contact_email_get_description (email_address);
		uuid_str = couchdb_struct_field_get_uuid (email_address);

		attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_EMAIL));

		if (uuid_str != NULL) {
			EVCardAttributeParam *param;

			param = e_vcard_attribute_param_new (COUCHDB_UUID_PROP);
			e_vcard_attribute_add_param_with_value (attr, param, uuid_str);
		}

		if (description_str != NULL) {
			EVCardAttributeParam *param;

			param = e_vcard_attribute_param_new (EVC_TYPE);
			if (!g_ascii_strcasecmp (description_str, "home"))
				e_vcard_attribute_add_param_with_value (attr, param, "HOME");
			else if (!g_ascii_strcasecmp (description_str, "work"))
				e_vcard_attribute_add_param_with_value (attr, param, "WORK");
			else
				e_vcard_attribute_param_free (param);
		}

		e_vcard_attribute_add_value (attr, email_str);
		attr_list = g_list_append (attr_list, attr);

		/* remove address from list */
		list = g_slist_remove (list, email_address);
		couchdb_struct_field_unref (email_address);
	}

	if (attr_list) {
		e_contact_set_attributes (contact, E_CONTACT_EMAIL, attr_list);
		g_list_foreach (attr_list, (GFunc) e_vcard_attribute_free, NULL);
		g_list_free (attr_list);
	}

	/* parse phone numbers */
	list = desktopcouch_document_contact_get_phone_numbers (document);
	while (list != NULL) {
		const char *phone_str, *description_str, *uuid_str;
		EVCardAttribute *attr;
		CouchdbStructField *phone_number = (CouchdbStructField *) list->data;

		phone_str = desktopcouch_document_contact_phone_get_number (phone_number);
		description_str = desktopcouch_document_contact_phone_get_description (phone_number);
		uuid_str = couchdb_struct_field_get_uuid (phone_number);

		if (description_str != NULL) {
			if (!g_ascii_strcasecmp (description_str, "home"))
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_PHONE_HOME));
			else if (!g_ascii_strcasecmp (description_str, "work"))
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_PHONE_BUSINESS));
			else
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_PHONE_OTHER));
		} else
			attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_PHONE_OTHER));

		if (uuid_str != NULL) {
			EVCardAttributeParam *param;

			param = e_vcard_attribute_param_new (COUCHDB_UUID_PROP);
			e_vcard_attribute_add_param_with_value (attr, param, uuid_str);
		}

		if (description_str) {
			if (!g_ascii_strcasecmp (description_str, "home")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"HOME");
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"VOICE");
			} else if (!g_ascii_strcasecmp (description_str, "work")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"WORK");
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"VOICE");
			} else if (!g_ascii_strcasecmp (description_str, "home fax")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"HOME");
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"FAX");
			} else if (!g_ascii_strcasecmp (description_str, "work fax")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"WORK");
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"FAX");
		        } else if (!g_ascii_strcasecmp (description_str, "other fax")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"FAX");
		        } else if (!g_ascii_strcasecmp (description_str, "pager")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"PAGER");
			} else if (!g_ascii_strcasecmp (description_str, "mobile")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"CELL");
                        } else if (!g_ascii_strcasecmp (description_str, "assistant")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									EVC_X_ASSISTANT);
                        } else if (!g_ascii_strcasecmp (description_str, "callback")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									EVC_X_CALLBACK);
                        } else if (!g_ascii_strcasecmp (description_str, "car")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"CAR");
                        } else if (!g_ascii_strcasecmp (description_str, "primary")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"PREF");
			} else if (!g_ascii_strcasecmp (description_str, "radio")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									EVC_X_RADIO);
                        } else if (!g_ascii_strcasecmp (description_str, "telex")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									EVC_X_TELEX);
                        } else if (!g_ascii_strcasecmp (description_str, "company")) {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									EVC_X_COMPANY);
			} else {
				e_vcard_attribute_add_param_with_value (attr,
									e_vcard_attribute_param_new (EVC_TYPE),
									"VOICE");
			}
		}

		e_vcard_attribute_add_value (attr, phone_str);
		e_vcard_add_attribute (E_VCARD (contact), attr);

		/* remove phones from list */
		list = g_slist_remove (list, phone_number);
		couchdb_struct_field_unref (phone_number);
	}

	/* parse postal addresses */
	list = desktopcouch_document_contact_get_addresses (document);
	while (list != NULL) {
		char **street_lines;
		const char *description_str;
		EContactAddress *contact_address;
		CouchdbStructField *address = (CouchdbStructField *) list->data;

		contact_address = g_new0 (EContactAddress, 1);
		contact_address->address_format = g_strdup ("");

		street_lines = g_strsplit (desktopcouch_document_contact_address_get_street (address), "\n", 2);
		if (street_lines != NULL) {
			contact_address->street = g_strdup (street_lines[0]);
			if (street_lines[1] != NULL)
				contact_address->ext = g_strdup (street_lines[1]);
			g_strfreev (street_lines);
		} else
			contact_address->street = g_strdup (desktopcouch_document_contact_address_get_street (address));

		contact_address->locality = g_strdup (desktopcouch_document_contact_address_get_city (address));
		contact_address->region = g_strdup (desktopcouch_document_contact_address_get_state (address));
		contact_address->country = g_strdup (desktopcouch_document_contact_address_get_country (address));
		contact_address->code = g_strdup (desktopcouch_document_contact_address_get_postalcode (address));
		contact_address->po = g_strdup (desktopcouch_document_contact_address_get_pobox (address));
		
		description_str = desktopcouch_document_contact_address_get_description (address);
		if (!g_ascii_strcasecmp (description_str, "home"))
			e_contact_set (contact, E_CONTACT_ADDRESS_HOME, (const gpointer) contact_address);
		else if (!g_ascii_strcasecmp (description_str, "work"))
			e_contact_set (contact, E_CONTACT_ADDRESS_WORK, (const gpointer) contact_address);
		else
			e_contact_set (contact, E_CONTACT_ADDRESS_OTHER, (const gpointer) contact_address);

		/* remove addresses from list */
		list = g_slist_remove (list, address);
		couchdb_struct_field_unref (address);

		e_contact_address_free (contact_address);
	}

	/* parse URLs */
	list = desktopcouch_document_contact_get_urls (document);
	while (list != NULL) {
		const char *description_str, *address_str, *uuid_str;
		EVCardAttribute *attr;
		CouchdbStructField *url = (CouchdbStructField *) list->data;

		address_str = desktopcouch_document_contact_url_get_address (url);
		description_str = desktopcouch_document_contact_url_get_description (url);
		uuid_str = couchdb_struct_field_get_uuid (url);

		if (description_str != NULL) {
			if (g_ascii_strcasecmp (description_str, "blog") == 0)
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_BLOG_URL));
			else
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_HOMEPAGE_URL));
		} else
			attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_HOMEPAGE_URL));

		if (uuid_str != NULL) {
			EVCardAttributeParam *param;

			param = e_vcard_attribute_param_new (COUCHDB_UUID_PROP);
			e_vcard_attribute_add_param_with_value (attr, param, uuid_str);
		}

		e_vcard_attribute_add_value (attr, address_str);
		e_vcard_add_attribute (E_VCARD (contact), attr);

		/* remove urls from list */
		list = g_slist_remove (list, url);
		couchdb_struct_field_unref (url);
	}

	/* parse IM addresses */
	list = desktopcouch_document_contact_get_im_addresses (document);
	while (list != NULL) {
		const char *address_str, *description_str, *protocol_str, *uuid_str;
		EVCardAttribute *attr = NULL;
		CouchdbStructField *im = (CouchdbStructField *) list->data;

		address_str = desktopcouch_document_contact_im_get_address (im);
		description_str = desktopcouch_document_contact_im_get_description (im);
		protocol_str = desktopcouch_document_contact_im_get_protocol (im);
		/* Some records don't have the 'protocol' field, and use the
		   'description' field to specify the kind of IM address this
		   refers to */
		if (protocol_str == NULL)
			protocol_str = description_str;
		uuid_str = couchdb_struct_field_get_uuid (im);

		if (protocol_str != NULL) {
			if (g_strcmp0 (protocol_str, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_AIM) == 0)
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_IM_AIM));
			else if (g_strcmp0 (protocol_str, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_GADU_GADU) == 0)
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_IM_GADUGADU));
			else if (g_strcmp0 (protocol_str, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_GROUPWISE) == 0)
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_IM_GROUPWISE));
			else if (g_strcmp0 (protocol_str, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_ICQ) == 0)
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_IM_ICQ));
			else if (g_strcmp0 (protocol_str, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_JABBER) == 0)
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_IM_JABBER));
			else if (g_strcmp0 (protocol_str, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_MSN) == 0)
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_IM_MSN));
			else if (g_strcmp0 (protocol_str, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_SKYPE) == 0)
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_IM_SKYPE));
			else if (g_strcmp0 (protocol_str, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_YAHOO) == 0)
				attr = e_vcard_attribute_new (NULL, e_contact_vcard_attribute (E_CONTACT_IM_YAHOO));

			if (attr != NULL) {
				if (description_str) {
					if (!g_ascii_strcasecmp (description_str, "home")) {
						e_vcard_attribute_add_param_with_value (attr,
											e_vcard_attribute_param_new (EVC_TYPE),
											"HOME");
					} else if (!g_ascii_strcasecmp (description_str, "work")) {
						e_vcard_attribute_add_param_with_value (attr,
											e_vcard_attribute_param_new (EVC_TYPE),
											"WORK");
					}
				}

				if (uuid_str != NULL) {
					EVCardAttributeParam *param;

					param = e_vcard_attribute_param_new (COUCHDB_UUID_PROP);
					e_vcard_attribute_add_param_with_value (attr, param, uuid_str);
				}

				e_vcard_attribute_add_value (attr, address_str);
				e_vcard_add_attribute (E_VCARD (contact), attr);
			}
		}

		/* remove addresses from list */
		list = g_slist_remove (list, im);
		couchdb_struct_field_unref (im);
	}

	/* birth date */
	str = (char *) desktopcouch_document_contact_get_birth_date (document);
	if (str && strlen (str) > 0) {
		EContactDate *dt;

		dt = e_contact_date_from_string (str);
		if (dt) {
			e_contact_set (contact, E_CONTACT_BIRTH_DATE, (const gpointer) dt);
			e_contact_date_free (dt);
		}
	}

	/* wedding date */
	str = (char *) desktopcouch_document_contact_get_wedding_date (document);
	if (str && strlen (str)) {
		EContactDate *dt;

		dt = e_contact_date_from_string (str);
		if (dt) {
			e_contact_set (contact, E_CONTACT_ANNIVERSARY, (const gpointer) dt);
			e_contact_date_free (dt);
		}
	}

	/* application annotations */
	if (app_annotations != NULL) {
		/* Always have a REV field on the VCARD, for SyncEvolution (bug LP:#479110) */
		CouchdbStructField *evo_annotations;
		gchar time_string[100] = {0};

		evo_annotations = couchdb_struct_field_get_struct_field (app_annotations, "Evolution");
		if (evo_annotations != NULL) {
			if (couchdb_struct_field_has_field (evo_annotations, "revision")) {
				e_contact_set (contact, E_CONTACT_REV,
					       couchdb_struct_field_get_string_field (evo_annotations, "revision"));
			} else {
				get_current_time (time_string);
				e_contact_set (contact, E_CONTACT_REV, time_string);
			}
		} else {
			get_current_time (time_string);
			e_contact_set (contact, E_CONTACT_REV, time_string);
		}

		/* Save the entire app_annotations field as a string on the VCARD */
		str = couchdb_struct_field_to_string (app_annotations);
		e_vcard_add_attribute_with_value (E_VCARD (contact),
						  e_vcard_attribute_new (NULL, COUCHDB_APPLICATION_ANNOTATIONS_PROP),
						  str);

		g_free (str);
		couchdb_struct_field_unref (app_annotations);
	} else {
		/* Always have a REV field on the VCARD, for SyncEvolution (bug LP:#479110) */
		gchar time_string[100] = {0};

		get_current_time (time_string);
		e_contact_set (contact, E_CONTACT_REV, time_string);
	}

	return contact;
}

static CouchdbStructField *
contact_email_to_struct_field (EVCardAttribute *attr)
{
	const gchar *email;
	GList *params, *pl;
	const gchar *description = "other", *uuid = NULL;

	email = e_vcard_attribute_get_value (attr);
	if (email == NULL)
		return NULL;

	params = e_vcard_attribute_get_params (attr);
	if (!params)
		return desktopcouch_document_contact_email_new (NULL, email, NULL);

	for (pl = params; pl != NULL; pl = pl->next) {
		GList *v;
		EVCardAttributeParam *p = pl->data;

		if (!g_strcmp0 (EVC_TYPE, e_vcard_attribute_param_get_name (p)) != 0) {
			v = e_vcard_attribute_param_get_values (p);
			while (v && v->data) {
				if (g_ascii_strcasecmp ((const gchar *) v->data, "HOME") == 0) {
					description = "home";
					break;
				} else if (g_ascii_strcasecmp ((const gchar *) v->data, "WORK") == 0) {
					description = "work";
					break;
				}

				v = v->next;
			}
		} else if (!g_strcmp0 (COUCHDB_UUID_PROP, e_vcard_attribute_param_get_name (p)) != 0) {
			v = e_vcard_attribute_param_get_values (p);
			uuid = (const gchar *) v->data;
		}
	}

	return desktopcouch_document_contact_email_new (uuid, email, description);
}

static CouchdbStructField *
contact_phone_to_struct_field (EVCardAttribute *attr)
{
	const gchar *phone;
	GList *params, *pl;
	CouchdbStructField *sf;
	gchar *final_description = NULL;
	const gchar *description = NULL, *uuid = NULL, *kind = NULL;

	phone = e_vcard_attribute_get_value (attr);
	if (!phone)
		return NULL;

	params = e_vcard_attribute_get_params (attr);
	if (!params)
		return desktopcouch_document_contact_phone_new (NULL, phone, NULL, /* FIXME */ 0);

	for (pl = params; pl != NULL; pl = pl->next) {
		GList *v;
		EVCardAttributeParam *p = pl->data;

		if (g_strcmp0 (EVC_TYPE, e_vcard_attribute_param_get_name (p)) == 0) {
			v = e_vcard_attribute_param_get_values (p);
			while (v && v->data) {
				if (g_ascii_strcasecmp ((const gchar *) v->data, "HOME") == 0)
					description = "home";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, "WORK") == 0)
					description = "work";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, "OTHER") == 0)
					description = "other";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, "PAGER") == 0)
					description = "pager";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, "CELL") == 0)
					description = "mobile";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, EVC_X_ASSISTANT) == 0)
					description = "assistant";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, EVC_X_CALLBACK) == 0)
					description = "callback";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, "CAR") == 0)
					description = "car";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, "PREF") == 0)
					description = "primary";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, EVC_X_RADIO) == 0)
					description = "radio";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, EVC_X_TELEX) == 0)
					description = "telex";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, EVC_X_COMPANY) == 0)
					description = "company";
				else if (g_ascii_strcasecmp ((const gchar *) v->data, "FAX") == 0)
					kind = "fax";

				v = v->next;
			}
		} else if (!g_strcmp0 (COUCHDB_UUID_PROP, e_vcard_attribute_param_get_name (p)) != 0) {
			v = e_vcard_attribute_param_get_values (p);
			uuid = (const gchar *) v->data;
		}
	}

	if (kind == NULL)
		final_description = g_strdup (description);
	else
		final_description = g_strdup_printf ("%s %s", description, kind);

	sf = desktopcouch_document_contact_phone_new (uuid, phone, final_description, /* FIXME */ 0);
	g_free (final_description);

	return sf;
}

static CouchdbStructField *
contact_postal_address_to_struct_field (EContact *contact, EContactField field, const char *description)
{
	CouchdbStructField *sf = NULL;
	EContactAddress *contact_address;

	contact_address = e_contact_get (contact, field);
	if (contact_address) {
		char *street = NULL;

		if (contact_address->ext && *contact_address->ext)
			street = g_strdup_printf ("%s\n%s", contact_address->street, contact_address->ext);
		else if (contact_address->street && *contact_address->street)
			street = g_strdup (contact_address->street);

		sf = desktopcouch_document_contact_address_new (/* FIXME */ NULL,
							   street,
							   contact_address->locality,
							   contact_address->region,
							   contact_address->country,
							   contact_address->code,
							   contact_address->po,
							   description);
		
		e_contact_address_free (contact_address);
		g_free (street);
	}

	return sf;
}

static CouchdbStructField *
contact_url_to_struct_field (EVCardAttribute *attr, const gchar *description)
{
	const gchar *address, *uuid;
	GList *params, *pl;

	address = e_vcard_attribute_get_value (attr);
	if (address == NULL || strlen (address) <= 0)
		return NULL;

	params = e_vcard_attribute_get_params (attr);
	if (!params)
		return desktopcouch_document_contact_url_new (NULL, address, description);

	for (pl = params; pl != NULL; pl = pl->next) {
		GList *v;
		EVCardAttributeParam *p = pl->data;

		if (g_strcmp0 (COUCHDB_UUID_PROP, e_vcard_attribute_param_get_name (p)) == 0) {
			v = e_vcard_attribute_param_get_values (p);
			if (v && v->data)
				uuid = (const gchar *) v->data;
		}
	}

	return desktopcouch_document_contact_url_new (uuid, address, description);
}

static CouchdbStructField *
contact_im_to_struct_field (EVCardAttribute *attr, const gchar *protocol)
{
	const gchar *address, *description = NULL, *uuid = NULL;
	GList *params, *pl;

	address = e_vcard_attribute_get_value (attr);
	if (!address)
		return NULL;

	params = e_vcard_attribute_get_params (attr);
	if (!params)
		return desktopcouch_document_contact_im_new (NULL, address, "other", protocol);

	for (pl = params; pl != NULL; pl = pl->next) {
		GList *v;
		EVCardAttributeParam *p = pl->data;

		if (g_strcmp0 (COUCHDB_UUID_PROP, e_vcard_attribute_param_get_name (p)) == 0) {
			v = e_vcard_attribute_param_get_values (p);
			if (v && v->data)
				uuid = (const gchar *) v->data;
		} else if (g_strcmp0 (EVC_TYPE, e_vcard_attribute_param_get_name (p)) == 0) {
			v = e_vcard_attribute_param_get_values (p);
			if (v && v->data) {
			        if (g_strcmp0 ("HOME", (const gchar *) v->data) == 0)
					description = "home";
				else if (g_strcmp0 ("WORK", (const gchar *) v->data) == 0)
					description = "work";
				else
					description = "other";
			}
		}
	}

	return desktopcouch_document_contact_im_new (uuid, address, description, protocol);
}

static void
set_vcard_revision (CouchdbStructField *app_annotations, EContact *contact)
{
	CouchdbStructField *evo_annotations;
	const gchar *rev;

	if (couchdb_struct_field_has_field (app_annotations, "Evolution"))
		evo_annotations = couchdb_struct_field_get_struct_field (app_annotations, "Evolution");
	else
		evo_annotations = couchdb_struct_field_new ();

	rev = e_contact_get_const (contact,  E_CONTACT_REV);
	if (rev && *rev) {
		couchdb_struct_field_set_string_field (evo_annotations, "revision", rev);
	} else { 
		gchar time_string[100] = {0};

		get_current_time (time_string);
		couchdb_struct_field_set_string_field (evo_annotations, "revision", time_string);
	}

	couchdb_struct_field_set_struct_field (app_annotations, "Evolution", evo_annotations);
	couchdb_struct_field_unref (evo_annotations);
}

static CouchdbDocument *
couch_document_from_contact (EBookBackendCouchDB *couchdb_backend, EContact *contact)
{
	EContactDate *dt;
	GSList *list;
	GList *attr_list, *al;
	const char *str;
	CouchdbDocument *document;
	gint i;
	CouchdbStructField *postal_address, *app_annotations;
	EContactName *contact_name;

	/* create the CouchDBDocument to put on the database */
	document = desktopcouch_document_contact_new (couchdb_backend->couchdb);

	str = e_contact_get_const (contact, E_CONTACT_UID);
	if (str)
		couchdb_document_set_id (document, str);

	str = e_vcard_attribute_get_value (e_vcard_get_attribute (E_VCARD (contact), COUCHDB_REVISION_PROP));
	if (str)
		couchdb_document_set_revision (document, str);

	contact_name = (EContactName *) e_contact_get (contact, E_CONTACT_NAME);
	if (contact_name != NULL) {
		if (contact_name->prefixes != NULL)
			desktopcouch_document_contact_set_title (document, (const char *) contact_name->prefixes);
		if (contact_name->given != NULL)
			desktopcouch_document_contact_set_first_name (document, (const char *) contact_name->given);
		if (contact_name->additional != NULL)
			desktopcouch_document_contact_set_middle_name (document, (const gchar *) contact_name->additional);
		if (contact_name->family != NULL)
			desktopcouch_document_contact_set_last_name (document, (const char *) contact_name->family);
		if (contact_name->suffixes != NULL)
			desktopcouch_document_contact_set_suffix (document, (const char *) contact_name->suffixes);
	}

	e_contact_name_free (contact_name);

	desktopcouch_document_contact_set_nick_name (document, (const char *) e_contact_get_const (contact, E_CONTACT_NICKNAME));
	desktopcouch_document_contact_set_spouse_name (document, (const char *) e_contact_get_const (contact, E_CONTACT_SPOUSE));

	desktopcouch_document_contact_set_company (document, (const char *) e_contact_get_const (contact, E_CONTACT_ORG));
	desktopcouch_document_contact_set_department (document, (const char *) e_contact_get_const (contact, E_CONTACT_ORG_UNIT));
	desktopcouch_document_contact_set_job_title (document, (const char *) e_contact_get_const (contact, E_CONTACT_TITLE));
	desktopcouch_document_contact_set_manager_name (document, (const char *) e_contact_get_const (contact, E_CONTACT_MANAGER));
	desktopcouch_document_contact_set_assistant_name (document, (const char *) e_contact_get_const (contact, E_CONTACT_ASSISTANT));
	desktopcouch_document_contact_set_office (document, (const char *) e_contact_get_const (contact, E_CONTACT_OFFICE));
	desktopcouch_document_contact_set_categories (document, (const char *) e_contact_get_const (contact, E_CONTACT_CATEGORIES));
	desktopcouch_document_contact_set_notes (document, (const char *) e_contact_get_const (contact, E_CONTACT_NOTE));

	/* email addresses */
	list = NULL;
	attr_list = e_contact_get_attributes (contact, E_CONTACT_EMAIL);
	if (attr_list != NULL) {
		for (al = attr_list; al != NULL; al = al->next) {
			CouchdbStructField *sf;
			EVCardAttribute *attr = (EVCardAttribute *) al->data;

			sf = contact_email_to_struct_field (attr);
			if (sf)
				list = g_slist_append (list, sf);
		}

		if (list) {
			desktopcouch_document_contact_set_email_addresses (document, list);

			g_slist_foreach (list, (GFunc) couchdb_struct_field_unref, NULL);
			g_slist_free (list);
		}

		g_list_foreach (attr_list, (GFunc) e_vcard_attribute_free, NULL);
		g_list_free (attr_list);
	}

	/* phone numbers */
	list = NULL;
	attr_list = e_vcard_get_attributes (E_VCARD (contact));
	for (al = attr_list; al != NULL; al = al->next) {
		CouchdbStructField *sf;
		EVCardAttribute *attr = (EVCardAttribute *) al->data;

		if (g_strcmp0 (e_vcard_attribute_get_name (attr), EVC_TEL) == 0) {
			sf = contact_phone_to_struct_field (attr);
			if (sf)
				list = g_slist_append (list, sf);
		}
	}

	if (list) {
		desktopcouch_document_contact_set_phone_numbers (document, list);

		g_slist_foreach (list, (GFunc) couchdb_struct_field_unref, NULL);
		g_slist_free (list);
	}

	/* postal addresses */
	list = NULL;
	postal_address = contact_postal_address_to_struct_field (contact, E_CONTACT_ADDRESS_HOME, "home");
	if (postal_address)
		list = g_slist_append (list, postal_address);

	postal_address = contact_postal_address_to_struct_field (contact, E_CONTACT_ADDRESS_WORK, "work");
	if (postal_address)
		list = g_slist_append (list, postal_address);

	postal_address = contact_postal_address_to_struct_field (contact, E_CONTACT_ADDRESS_OTHER, "other");
	if (postal_address)
		list = g_slist_append (list, postal_address);

	if (list) {
		desktopcouch_document_contact_set_addresses (document, list);

		g_slist_foreach (list, (GFunc) couchdb_struct_field_unref, NULL);
		g_slist_free (list);
	}

	/* URLS */
	list = NULL;
	attr_list = e_vcard_get_attributes (E_VCARD (contact));
	for (al = attr_list; al != NULL; al = al->next) {
		CouchdbStructField *sf = NULL;
		EVCardAttribute *attr = (EVCardAttribute *) al->data;

		if (g_strcmp0 (e_vcard_attribute_get_name (attr),
			       e_contact_vcard_attribute (E_CONTACT_HOMEPAGE_URL)) == 0)
			sf = contact_url_to_struct_field (attr, "home page");
		else if (g_strcmp0 (e_vcard_attribute_get_name (attr),
				    e_contact_vcard_attribute (E_CONTACT_BLOG_URL)) == 0)
			sf = contact_url_to_struct_field (attr, "blog");

		if (sf != NULL)
			list = g_slist_append (list, sf);
	}

	if (list != NULL) {
		desktopcouch_document_contact_set_urls (document, list);

		g_slist_foreach (list, (GFunc) couchdb_struct_field_unref, NULL);
		g_slist_free (list);
	}

	/* IM addresses */
	list = NULL;
	attr_list = e_vcard_get_attributes (E_VCARD (contact));
	for (al = attr_list; al != NULL; al = al->next) {
		CouchdbStructField *sf = NULL;
		EVCardAttribute *attr = (EVCardAttribute *) al->data;

		if (g_strcmp0 (e_vcard_attribute_get_name (attr),
			       e_contact_vcard_attribute (E_CONTACT_IM_AIM)) == 0)
			sf = contact_im_to_struct_field (attr, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_AIM);
		else if (g_strcmp0 (e_vcard_attribute_get_name (attr),
			       e_contact_vcard_attribute (E_CONTACT_IM_GADUGADU)) == 0)
			sf = contact_im_to_struct_field (attr, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_GADU_GADU);
		else if (g_strcmp0 (e_vcard_attribute_get_name (attr),
			       e_contact_vcard_attribute (E_CONTACT_IM_GROUPWISE)) == 0)
			sf = contact_im_to_struct_field (attr, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_GROUPWISE);
		else if (g_strcmp0 (e_vcard_attribute_get_name (attr),
			       e_contact_vcard_attribute (E_CONTACT_IM_ICQ)) == 0)
			sf = contact_im_to_struct_field (attr, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_ICQ);
		else if (g_strcmp0 (e_vcard_attribute_get_name (attr),
			       e_contact_vcard_attribute (E_CONTACT_IM_JABBER)) == 0)
			sf = contact_im_to_struct_field (attr, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_JABBER);
		else if (g_strcmp0 (e_vcard_attribute_get_name (attr),
			       e_contact_vcard_attribute (E_CONTACT_IM_MSN)) == 0)
			sf = contact_im_to_struct_field (attr, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_MSN);
		else if (g_strcmp0 (e_vcard_attribute_get_name (attr),
			       e_contact_vcard_attribute (E_CONTACT_IM_SKYPE)) == 0)
			sf = contact_im_to_struct_field (attr, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_SKYPE);
		else if (g_strcmp0 (e_vcard_attribute_get_name (attr),
			       e_contact_vcard_attribute (E_CONTACT_IM_YAHOO)) == 0)
			sf = contact_im_to_struct_field (attr, DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_YAHOO);

		if (sf != NULL)
			list = g_slist_append (list, sf);
	}

	if (list != NULL) {
		desktopcouch_document_contact_set_im_addresses (document, list);

		g_slist_foreach (list, (GFunc) couchdb_struct_field_unref, NULL);
		g_slist_free (list);
	}

	/* birth date */
	dt = (EContactDate *) e_contact_get (contact, E_CONTACT_BIRTH_DATE);
	if (dt) {
		char *dt_str = e_contact_date_to_string (dt);
		if (dt_str != NULL) {
			desktopcouch_document_contact_set_birth_date (document, (const char *) dt_str);

			g_free (dt_str);
		}
	}

	/* wedding date */
	dt = (EContactDate *) e_contact_get (contact, E_CONTACT_ANNIVERSARY);
	if (dt) {
		char *dt_str = e_contact_date_to_string (dt);
		if (dt_str != NULL) {
			desktopcouch_document_contact_set_wedding_date (document, (const char *) dt_str);

			g_free (dt_str);
		}
	}

	/* application annotations */
	str = e_vcard_attribute_get_value (e_vcard_get_attribute (E_VCARD (contact), COUCHDB_APPLICATION_ANNOTATIONS_PROP));
	if (str)
		app_annotations = couchdb_struct_field_new_from_string (str);
	else
		app_annotations = couchdb_struct_field_new ();

	set_vcard_revision (app_annotations, contact);
	desktopcouch_document_set_application_annotations (document, app_annotations);
	couchdb_struct_field_unref (app_annotations);

	return document;
}

static void
document_updated_cb (CouchdbSession *couchdb, const char *dbname, CouchdbDocument *document, gpointer user_data)
{
	EContact *contact;
	EBookBackendCouchDB *couchdb_backend = E_BOOK_BACKEND_COUCHDB (user_data);

	if (g_strcmp0 (dbname, couchdb_backend->dbname) != 0)
		return;

	contact = contact_from_couch_document (document);
	if (!contact)
		return;

	e_book_backend_notify_update (E_BOOK_BACKEND (couchdb_backend), contact);

	/* Add the contact to the cache */
	e_book_backend_cache_add_contact (couchdb_backend->cache, contact);

	g_object_unref (G_OBJECT (contact));
}

static void
document_deleted_cb (CouchdbSession *couchdb, const char *dbname, const char *docid, gpointer user_data)
{
	EBookBackendCouchDB *couchdb_backend = E_BOOK_BACKEND_COUCHDB (user_data);

	if (g_strcmp0 (dbname, couchdb_backend->dbname) != 0)
		return;

	e_book_backend_notify_remove (E_BOOK_BACKEND (couchdb_backend), docid);

	/* Remove the contact from the cache */
	e_book_backend_cache_remove_contact (couchdb_backend->cache, docid);
}

static GNOME_Evolution_Addressbook_CallStatus
e_book_backend_couchdb_load_source (EBookBackend *backend,
				    ESource *source,
				    gboolean only_if_exists)
{
	gchar *uri;
	const gchar *property;
	CouchdbDatabaseInfo *db_info;
	GError *error = NULL;
	GSList *doc_list, *sl;
	EBookBackendCouchDB *couchdb_backend = E_BOOK_BACKEND_COUCHDB (backend);

	g_return_val_if_fail (E_IS_BOOK_BACKEND_COUCHDB (couchdb_backend), GNOME_Evolution_Addressbook_OtherError);

	if (couchdb_backend->couchdb != NULL)
		g_object_unref (G_OBJECT (couchdb_backend->couchdb));
	if (couchdb_backend->dbname != NULL)
		g_free (couchdb_backend->dbname);
	if (couchdb_backend->cache != NULL)
		g_object_unref (G_OBJECT (couchdb_backend->cache));

	/* create CouchDB main object */
	couchdb_backend->dbname = g_strdup ("contacts");
	couchdb_backend->using_desktopcouch = FALSE;

	property = e_source_get_property (source, "couchdb_instance");
	if (g_strcmp0 (property, "user") == 0) {
		if (! (couchdb_backend->couchdb = COUCHDB_SESSION (desktopcouch_session_new ()))) {
			g_warning ("Could not create DesktopcouchSession object");
			return GNOME_Evolution_Addressbook_NoSuchBook;
		}

		couchdb_backend->using_desktopcouch = TRUE;
	} else {
		if (g_strcmp0 (property, "remote") == 0)
			uri = g_strdup_printf ("http://%s", e_source_get_property (source, "couchdb_remote_server"));
		else
			uri = g_strdup ("http://127.0.0.1:5984");

		if (! (couchdb_backend->couchdb = couchdb_session_new (uri))) {
			g_free (uri);
			return GNOME_Evolution_Addressbook_NoSuchBook;
		}

		g_free (uri);
	}

	/* check if only_if_exists */
	error = NULL;
	db_info = couchdb_session_get_database_info (couchdb_backend->couchdb,
						     couchdb_backend->dbname,
						     &error);
	if (!db_info) {
		if (error) {
			g_warning ("Could not get CouchDB database info: %s", error->message);
			g_error_free (error);
		}

		if (only_if_exists)
			return GNOME_Evolution_Addressbook_NoSuchBook;
		
		/* if it does not exist, create it */
		error = NULL;
		if (!couchdb_session_create_database (couchdb_backend->couchdb,
						      couchdb_backend->dbname,
						      &error)) {
			g_warning ("Could not create 'contacts' database: %s", error->message);
			g_error_free (error);

			return GNOME_Evolution_Addressbook_PermissionDenied;
		}
	} else
		couchdb_database_info_unref (db_info);

	/* Create cache */
	uri = e_source_get_uri (source);
	couchdb_backend->cache = e_book_backend_cache_new ((const gchar *) uri);
	g_free (uri);

	/* Populate the cache */
	e_file_cache_clean (E_FILE_CACHE (couchdb_backend->cache));
	error = NULL;
	doc_list = couchdb_session_list_documents (couchdb_backend->couchdb,
						   couchdb_backend->dbname,
						   &error);
	for (sl = doc_list; sl != NULL; sl = sl->next) {
		EContact *contact;
		CouchdbDocument *document;
		CouchdbDocumentInfo *doc_info = (CouchdbDocumentInfo *) sl->data;

		/* Retrieve this document */
		error = NULL;
		document = couchdb_document_get (couchdb_backend->couchdb,
						 couchdb_backend->dbname,
						 couchdb_document_info_get_docid (doc_info),
						 &error);
		if (!document)
			continue;

		contact = contact_from_couch_document (document);
		if (contact != NULL) {
			e_book_backend_cache_add_contact (couchdb_backend->cache, contact);
			g_object_unref (G_OBJECT (contact));
		}

		g_object_unref (G_OBJECT (document));
	}

	couchdb_session_free_document_list (doc_list);

	/* Listen for changes on database */
	g_signal_connect (G_OBJECT (couchdb_backend->couchdb), "document_created",
			  G_CALLBACK (document_updated_cb), couchdb_backend);
	g_signal_connect (G_OBJECT (couchdb_backend->couchdb), "document_updated",
			  G_CALLBACK (document_updated_cb), couchdb_backend);
	g_signal_connect (G_OBJECT (couchdb_backend->couchdb), "document_deleted",
			  G_CALLBACK (document_deleted_cb), couchdb_backend);
	couchdb_session_listen_for_changes (couchdb_backend->couchdb, couchdb_backend->dbname);

	e_book_backend_set_is_loaded (backend, TRUE);
	e_book_backend_set_is_writable (backend, TRUE);

	return GNOME_Evolution_Addressbook_Success;
}

static void
e_book_backend_couchdb_remove (EBookBackend *backend, EDataBook *book, guint32 opid)
{
	EBookBackendCouchDB *couchdb_backend = E_BOOK_BACKEND_COUCHDB (backend);

	/* Remove the cache */
	if (couchdb_backend->cache != NULL) {
		g_object_unref (G_OBJECT (couchdb_backend->cache));
		couchdb_backend->cache = NULL;
	}

	/* We don't remove data from CouchDB, since it would affect other apps,
	   so just report success */
	e_data_book_respond_remove (book, opid, GNOME_Evolution_Addressbook_Success);
}

static char *
e_book_backend_couchdb_get_static_capabilities (EBookBackend *backend)
{
	return g_strdup ("local,do-initial-query,bulk-removes");
}

static EContact *
put_document (EBookBackendCouchDB *couchdb_backend, CouchdbDocument *document)
{
	GError *error = NULL;

	if (couchdb_document_put (document, couchdb_backend->dbname, &error)) {
		EContact *new_contact;

		/* couchdb_document_put sets the ID for new documents, so need to send that back */
		new_contact = contact_from_couch_document (document);

		/* Add the new contact to the cache */
		e_book_backend_cache_add_contact (couchdb_backend->cache, new_contact);

		return new_contact;
	} else {
		if (error != NULL) {
			g_warning ("Could not PUT document: %s\n", error->message);
			g_error_free (error);
		}
	}

	return NULL;
}

static void
e_book_backend_couchdb_create_contact (EBookBackend *backend,
				       EDataBook *book,
				       guint32 opid,
				       const char *vcard)
{
	EContact *contact, *new_contact;
	CouchdbDocument *document;
	EBookBackendCouchDB *couchdb_backend = E_BOOK_BACKEND_COUCHDB (backend);

	contact = e_contact_new_from_vcard (vcard);
	if (!contact) {
		e_data_book_respond_create (book, opid, GNOME_Evolution_Addressbook_OtherError, NULL);
		return;
	}

	document = couch_document_from_contact (couchdb_backend, contact);
	if (!document) {
		e_data_book_respond_create (book, opid, GNOME_Evolution_Addressbook_OtherError, NULL);
		g_object_unref (G_OBJECT (contact));
		return;
	}

	/* save the contact into the DB */
	if ((new_contact = put_document (couchdb_backend, document)) != NULL) {
		e_data_book_respond_create (book, opid, GNOME_Evolution_Addressbook_Success, new_contact);
		g_object_unref (new_contact);
	} else
		e_data_book_respond_create (book, opid, GNOME_Evolution_Addressbook_OtherError, NULL);

	/* free memory */
	g_object_unref (G_OBJECT (contact));
	g_object_unref (G_OBJECT (document));
}

static void
e_book_backend_couchdb_remove_contacts (EBookBackend *backend,
					EDataBook *book,
					guint32 opid,
					GList *id_list)
{
	GList *l, *deleted_ids = NULL;
	EBookBackendCouchDB *couchdb_backend = E_BOOK_BACKEND_COUCHDB (backend);

	for (l = id_list; l != NULL; l = l->next) {
		CouchdbDocument *document;
		GError *error = NULL;
		const gchar *uid = (const gchar *) l->data;

		document = couchdb_document_get (couchdb_backend->couchdb, couchdb_backend->dbname, uid, &error);
		if (document) {
			if (couchdb_backend->using_desktopcouch) {
				CouchdbStructField *app_annotations, *u1_annotations, *private_annotations;

				/* For desktopcouch, we don't remove contacts, we just
				 * mark them as deleted */
				app_annotations = desktopcouch_document_get_application_annotations (document);
				if (app_annotations == NULL)
					app_annotations = couchdb_struct_field_new ();

				u1_annotations = couchdb_struct_field_get_struct_field (app_annotations, "Ubuntu One");
				if (u1_annotations == NULL)
					u1_annotations = couchdb_struct_field_new ();

				private_annotations = couchdb_struct_field_get_struct_field (u1_annotations, "private_application_annotations");
				if (private_annotations == NULL)
					private_annotations = couchdb_struct_field_new ();

				couchdb_struct_field_set_boolean_field (private_annotations, "deleted", TRUE);
				couchdb_struct_field_set_struct_field (u1_annotations, "private_application_annotations", private_annotations);
				couchdb_struct_field_set_struct_field (app_annotations, "Ubuntu One", u1_annotations);
				desktopcouch_document_set_application_annotations (document, app_annotations);

				/* Now put the new revision of the document */
				if (couchdb_document_put (document, couchdb_backend->dbname, &error)) {
					deleted_ids = g_list_append (deleted_ids, (gpointer) uid);
					e_book_backend_cache_remove_contact (couchdb_backend->cache, uid);
				} else {
					if (error != NULL) {
						g_debug ("Error deleting document: %s", error->message);
						g_error_free (error);
					} else
						g_debug ("Error deleting document");
				}

				/* Free memory */
				couchdb_struct_field_unref (app_annotations);
				couchdb_struct_field_unref (u1_annotations);
				couchdb_struct_field_unref (private_annotations);
			} else {
				if (couchdb_document_delete (document, &error)) {
					deleted_ids = g_list_append (deleted_ids, (gpointer) uid);
					e_book_backend_cache_remove_contact (couchdb_backend->cache, uid);
				} else {
					if (error != NULL) {
						g_debug ("Error deleting document: %s", error->message);
						g_error_free (error);
					} else
						g_debug ("Error deleting document");
				}
			}
		} else {
			if (error != NULL) {
				g_debug ("Error getting document: %s", error->message);
				g_error_free (error);
			} else
				g_debug ("Error getting document");
		}
	}

	if (deleted_ids) {
		e_data_book_respond_remove_contacts (book, opid,
						     GNOME_Evolution_Addressbook_Success, deleted_ids);
		g_list_free (deleted_ids);
	} else
		e_data_book_respond_remove_contacts (book, opid,
						     GNOME_Evolution_Addressbook_OtherError, NULL);
}

static void
e_book_backend_couchdb_modify_contact (EBookBackend *backend,
				       EDataBook *book,
				       guint32 opid,
				       const char *vcard)
{
	EContact *contact, *new_contact;
	CouchdbDocument *document;
	EBookBackendCouchDB *couchdb_backend = E_BOOK_BACKEND_COUCHDB (backend);

	contact = e_contact_new_from_vcard (vcard);
	if (!contact) {
		e_data_book_respond_modify (book, opid, GNOME_Evolution_Addressbook_OtherError, NULL);
		return;
	}

	document = couch_document_from_contact (couchdb_backend, contact);
	if (!document) {
		e_data_book_respond_modify (book, opid, GNOME_Evolution_Addressbook_OtherError, NULL);
		g_object_unref (G_OBJECT (document));
		return;
	}

	/* save the contact into the DB */
	if ((new_contact = put_document (couchdb_backend, document)) != NULL) {
		e_data_book_respond_modify (book, opid, GNOME_Evolution_Addressbook_Success, new_contact);
		g_object_unref (new_contact);
	} else
		e_data_book_respond_modify (book, opid, GNOME_Evolution_Addressbook_OtherError, NULL);

	/* free memory */
	g_object_unref (G_OBJECT (contact));
	g_object_unref (G_OBJECT (document));
}

static void
e_book_backend_couchdb_get_contact (EBookBackend *backend,
				    EDataBook *book,
				    guint32 opid,
				    const char *id)
{
	GError *error = NULL;
	EContact *contact;
	EBookBackendCouchDB *couchdb_backend = E_BOOK_BACKEND_COUCHDB (backend);

	contact = e_book_backend_cache_get_contact (couchdb_backend->cache, id);
	if (contact != NULL) {
		char *vcard = e_vcard_to_string (E_VCARD (contact), EVC_FORMAT_VCARD_30);

		g_object_unref (G_OBJECT (contact));
		if (vcard != NULL) {
			e_data_book_respond_get_contact (book,
							 opid,
							 GNOME_Evolution_Addressbook_Success,
							 vcard);
			g_free (vcard);
			return;
		}
	}

	e_data_book_respond_get_contact (book, opid, GNOME_Evolution_Addressbook_ContactNotFound, "");
}

static void
e_book_backend_couchdb_get_contact_list (EBookBackend *backend,
					 EDataBook *book,
					 guint32 opid, const char *query)
{
	GList *doc_list, *contacts = NULL;
	GError *error = NULL;
	EBookBackendCouchDB *couchdb_backend = E_BOOK_BACKEND_COUCHDB (backend);

	/* Get the list of documents from cache */
	doc_list = e_book_backend_cache_get_contacts (couchdb_backend->cache, query);
	while (doc_list != NULL) {
		char *vcard;
		EContact *contact = E_CONTACT (doc_list->data);

		vcard = e_vcard_to_string (E_VCARD (contact), EVC_FORMAT_VCARD_30);
		if (vcard != NULL)
			contacts = g_list_prepend (contacts, vcard);

		doc_list = g_list_remove (doc_list, contact);
		g_object_unref (G_OBJECT (contact));
	}

	e_data_book_respond_get_contact_list (book, opid, GNOME_Evolution_Addressbook_Success, contacts);
}

static void
e_book_backend_couchdb_start_book_view (EBookBackend *backend,
					EDataBookView *book_view)
{
	GList *doc_list;
	EBookBackendCouchDB *couchdb_backend = E_BOOK_BACKEND_COUCHDB (backend);

	e_book_backend_add_book_view (backend, book_view);

	/* Get the list of documents from cache */
	doc_list = e_book_backend_cache_get_contacts (couchdb_backend->cache,
						      e_data_book_view_get_card_query (book_view));
	while (doc_list != NULL) {
		char *vcard;
		EContact *contact = E_CONTACT (doc_list->data);

		vcard = e_vcard_to_string (E_VCARD (contact), EVC_FORMAT_VCARD_30);
		if (!vcard)
			continue;

		e_data_book_view_notify_update_vcard (book_view, vcard);

		doc_list = g_list_remove (doc_list, contact);
		g_object_unref (G_OBJECT (contact));
	}

	e_data_book_view_notify_complete (book_view, GNOME_Evolution_Addressbook_Success);
}

static void
e_book_backend_couchdb_stop_book_view (EBookBackend *backend,
				       EDataBookView *book_view)
{
	e_book_backend_remove_book_view (backend, book_view);
}

static void
e_book_backend_couchdb_get_changes (EBookBackend *backend,
				    EDataBook *book,
				    guint32 opid,
				    const char *change_id)
{
}

static void
e_book_backend_couchdb_authenticate_user (EBookBackend *backend,
					  EDataBook *book,
					  guint32 opid,
					  const char *user,
					  const char *passwd,
					  const char *auth_method)
{
}

static void
e_book_backend_couchdb_get_required_fields (EBookBackend *backend,
					    EDataBook *book,
					    guint32 opid)
{
	GList *fields = NULL;
	const gchar *field_name;

	field_name = e_contact_field_name (E_CONTACT_GIVEN_NAME);
	fields = g_list_append (fields, g_strdup (field_name));

	e_data_book_respond_get_required_fields(book, opid,
						GNOME_Evolution_Addressbook_Success, fields);

	g_list_foreach (fields, (GFunc) g_free, NULL);
	g_list_free (fields);
}

static void
e_book_backend_couchdb_get_supported_fields (EBookBackend *backend,
					     EDataBook *book,
					     guint32 opid)
{
	GList *fields = NULL;
	gint i;

	/* Basic fields */
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_UID)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_GIVEN_NAME)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_FAMILY_NAME)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_FULL_NAME)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_NAME)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_NICKNAME)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_SPOUSE)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_BIRTH_DATE)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_ANNIVERSARY)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_NOTE)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_CATEGORIES)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_REV)));

	/* URLS */
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_HOMEPAGE_URL)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_BLOG_URL)));

	/* Company fields */
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_ORG)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_ORG_UNIT)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_TITLE)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_MANAGER)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_ASSISTANT)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_OFFICE)));

	/* Email addresses */
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_EMAIL_1)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_EMAIL_2)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_EMAIL_3)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_EMAIL_4)));

	/* Phone numbers */
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_HOME)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_HOME_FAX)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_BUSINESS)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_BUSINESS_FAX)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_OTHER)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_OTHER_FAX)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_PAGER)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_MOBILE)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_ASSISTANT)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_CALLBACK)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_CAR)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_PRIMARY)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_RADIO)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_TELEX)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_PHONE_COMPANY)));

	/* Postal addresses */
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_ADDRESS_HOME)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_ADDRESS_WORK)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_ADDRESS_OTHER)));

	/* IM addresses */
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_IM_AIM)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_IM_GADUGADU)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_IM_GROUPWISE)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_IM_ICQ)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_IM_JABBER)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_IM_MSN)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_IM_SKYPE)));
	fields = g_list_append (fields, g_strdup (e_contact_field_name (E_CONTACT_IM_YAHOO)));

	e_data_book_respond_get_supported_fields (book, opid,
						  GNOME_Evolution_Addressbook_Success, fields);

	g_list_foreach (fields, (GFunc) g_free, NULL);
	g_list_free (fields);
}

static void
e_book_backend_couchdb_get_supported_auth_methods (EBookBackend *backend, EDataBook *book, guint32 opid)
{
	GList *auth_methods = NULL;

	auth_methods = g_list_append (auth_methods, g_strdup ("plain/password"));

	e_data_book_respond_get_supported_auth_methods (book, opid,
							GNOME_Evolution_Addressbook_Success, auth_methods);

	g_list_foreach (auth_methods, (GFunc) g_free, NULL);
	g_list_free (auth_methods);
}

static GNOME_Evolution_Addressbook_CallStatus
e_book_backend_couchdb_cancel_operation (EBookBackend *backend, EDataBook *book)
{
	return GNOME_Evolution_Addressbook_CouldNotCancel;
}

static void
e_book_backend_couchdb_set_mode (EBookBackend *backend, int mode)
{
}

/**
 * e_book_backend_couchdb_new:
 */
EBookBackend *
e_book_backend_couchdb_new (void)
{
	return E_BOOK_BACKEND (g_object_new (E_TYPE_BOOK_BACKEND_COUCHDB, NULL));
}

static void
e_book_backend_couchdb_dispose (GObject *object)
{
	EBookBackendCouchDB *couchdb_backend;

	couchdb_backend = E_BOOK_BACKEND_COUCHDB (object);

	/* Free all memory and resources */
	if (couchdb_backend->couchdb) {
		g_object_unref (G_OBJECT (couchdb_backend->couchdb));
		couchdb_backend->couchdb = NULL;
	}

	if (couchdb_backend->cache != NULL) {
		g_object_unref (G_OBJECT (couchdb_backend->cache));
		couchdb_backend->cache = NULL;
	}

	if (couchdb_backend->dbname) {
		g_free (couchdb_backend->dbname);
		couchdb_backend->dbname = NULL;
	}
}

static void
e_book_backend_couchdb_class_init (EBookBackendCouchDBClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	EBookBackendClass *parent_class;

	parent_class = E_BOOK_BACKEND_CLASS (klass);

	parent_class->load_source             = e_book_backend_couchdb_load_source;
	parent_class->get_static_capabilities = e_book_backend_couchdb_get_static_capabilities;

	parent_class->create_contact          = e_book_backend_couchdb_create_contact;
	parent_class->remove_contacts         = e_book_backend_couchdb_remove_contacts;
	parent_class->modify_contact          = e_book_backend_couchdb_modify_contact;
	parent_class->get_contact             = e_book_backend_couchdb_get_contact;
	parent_class->get_contact_list        = e_book_backend_couchdb_get_contact_list;
	parent_class->start_book_view         = e_book_backend_couchdb_start_book_view;
	parent_class->stop_book_view          = e_book_backend_couchdb_stop_book_view;
	parent_class->get_changes             = e_book_backend_couchdb_get_changes;
	parent_class->authenticate_user       = e_book_backend_couchdb_authenticate_user;
	parent_class->get_required_fields     = e_book_backend_couchdb_get_required_fields;
	parent_class->get_supported_fields    = e_book_backend_couchdb_get_supported_fields;
	parent_class->get_supported_auth_methods = e_book_backend_couchdb_get_supported_auth_methods;
	parent_class->cancel_operation        = e_book_backend_couchdb_cancel_operation;
	parent_class->remove                  = e_book_backend_couchdb_remove;
	parent_class->set_mode                = e_book_backend_couchdb_set_mode;

	object_class->dispose                 = e_book_backend_couchdb_dispose;
}

static void
e_book_backend_couchdb_init (EBookBackendCouchDB *backend)
{
	backend->couchdb = NULL;
	backend->dbname = NULL;
	backend->cache = NULL;
}
