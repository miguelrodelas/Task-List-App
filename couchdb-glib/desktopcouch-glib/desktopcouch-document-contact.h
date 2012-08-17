/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2010 Canonical Services Ltd (www.canonical.com)
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

#ifndef __DESKTOPCOUCH_DOCUMENT_CONTACT_H__
#define __DESKTOPCOUCH_DOCUMENT_CONTACT_H__

#include <couchdb-struct-field.h>
#include <desktopcouch-document.h>

G_BEGIN_DECLS

#define DESKTOPCOUCH_RECORD_TYPE_CONTACT "http://www.freedesktop.org/wiki/Specifications/desktopcouch/contact"

CouchdbDocument *desktopcouch_document_contact_new (CouchdbSession *couchdb);
gboolean         desktopcouch_document_is_contact (CouchdbDocument *document);

/*
 * Top level functions to manipulate documents representing a contact
 */

const char *desktopcouch_document_contact_get_title (CouchdbDocument *document);
void        desktopcouch_document_contact_set_title (CouchdbDocument *document, const char *title);
const char *desktopcouch_document_contact_get_first_name (CouchdbDocument *document);
void        desktopcouch_document_contact_set_first_name (CouchdbDocument *document, const char *first_name);
const char *desktopcouch_document_contact_get_middle_name (CouchdbDocument *document);
void        desktopcouch_document_contact_set_middle_name (CouchdbDocument *document, const char *middle_name);
const char *desktopcouch_document_contact_get_last_name (CouchdbDocument *document);
void        desktopcouch_document_contact_set_last_name (CouchdbDocument *document, const char *last_name);
const char *desktopcouch_document_contact_get_suffix (CouchdbDocument *document);
void        desktopcouch_document_contact_set_suffix (CouchdbDocument *document, const char *suffix);

const char *desktopcouch_document_contact_get_nick_name (CouchdbDocument *document);
void        desktopcouch_document_contact_set_nick_name (CouchdbDocument *document, const char *nick_name);
const char *desktopcouch_document_contact_get_spouse_name (CouchdbDocument *document);
void        desktopcouch_document_contact_set_spouse_name (CouchdbDocument *document, const char *spouse_name);
const char *desktopcouch_document_contact_get_birth_date (CouchdbDocument *document);
void        desktopcouch_document_contact_set_birth_date (CouchdbDocument *document, const char *birth_date);
const char *desktopcouch_document_contact_get_wedding_date (CouchdbDocument *document);
void        desktopcouch_document_contact_set_wedding_date (CouchdbDocument *document, const char *wedding_date);

const char *desktopcouch_document_contact_get_company (CouchdbDocument *document);
void        desktopcouch_document_contact_set_company (CouchdbDocument *document, const char *company);
const char *desktopcouch_document_contact_get_department (CouchdbDocument *document);
void        desktopcouch_document_contact_set_department (CouchdbDocument *document, const char *department);
const char *desktopcouch_document_contact_get_job_title (CouchdbDocument *document);
void        desktopcouch_document_contact_set_job_title (CouchdbDocument *document, const char *job_title);
const char *desktopcouch_document_contact_get_manager_name (CouchdbDocument *document);
void        desktopcouch_document_contact_set_manager_name (CouchdbDocument *document, const char *manager_name);
const char *desktopcouch_document_contact_get_assistant_name (CouchdbDocument *document);
void        desktopcouch_document_contact_set_assistant_name (CouchdbDocument *document, const char *assistant_name);
const char *desktopcouch_document_contact_get_office (CouchdbDocument *document);
void        desktopcouch_document_contact_set_office (CouchdbDocument *document, const char *office);

GSList     *desktopcouch_document_contact_get_email_addresses (CouchdbDocument *document);
void        desktopcouch_document_contact_set_email_addresses (CouchdbDocument *document, GSList *list);

GSList     *desktopcouch_document_contact_get_phone_numbers (CouchdbDocument *document);
void        desktopcouch_document_contact_set_phone_numbers (CouchdbDocument *document, GSList *list);

GSList     *desktopcouch_document_contact_get_addresses (CouchdbDocument *document);
void        desktopcouch_document_contact_set_addresses (CouchdbDocument *document, GSList *list);

GSList     *desktopcouch_document_contact_get_im_addresses (CouchdbDocument *document);
void        desktopcouch_document_contact_set_im_addresses (CouchdbDocument *document, GSList *list);

GSList     *desktopcouch_document_contact_get_urls (CouchdbDocument *document);
void        desktopcouch_document_contact_set_urls (CouchdbDocument *document, GSList *list);

const char *desktopcouch_document_contact_get_categories (CouchdbDocument *document);
void        desktopcouch_document_contact_set_categories (CouchdbDocument *document, const char *categories);

const char *desktopcouch_document_contact_get_notes (CouchdbDocument *document);
void        desktopcouch_document_contact_set_notes (CouchdbDocument *document, const char *notes);

/*
 * Utility functions to manipulate email addresses fields
 */

CouchdbStructField *desktopcouch_document_contact_email_new (const char *uuid, const char *address, const char *description);
const char         *desktopcouch_document_contact_email_get_address (CouchdbStructField *sf);
void                desktopcouch_document_contact_email_set_address (CouchdbStructField *sf, const char *email);

#define DESKTOPCOUCH_DOCUMENT_CONTACT_EMAIL_DESCRIPTION_HOME  "home"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_EMAIL_DESCRIPTION_OTHER "other"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_EMAIL_DESCRIPTION_WORK  "work"

const char         *desktopcouch_document_contact_email_get_description (CouchdbStructField *sf);
void                desktopcouch_document_contact_email_set_description (CouchdbStructField *sf, const char *description);

/*
 * Utility functions to manipulate phone numbers
 */

CouchdbStructField *desktopcouch_document_contact_phone_new (const char *uuid, const char *number, const char *description, gint priority);
gint                desktopcouch_document_contact_phone_get_priority (CouchdbStructField *sf);
void                desktopcouch_document_contact_phone_set_priority (CouchdbStructField *sf, gint priority);
const char         *desktopcouch_document_contact_phone_get_number (CouchdbStructField *sf);
void                desktopcouch_document_contact_phone_set_number (CouchdbStructField *sf, const char *number);

#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_ASSISTANT "assistant"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_CALLBACK  "callback"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_CAR       "car"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_COMPANY   "company"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_HOME      "home"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_HOME_FAX  "home fax"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_MOBILE    "mobile"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_OTHER     "other"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_OTHER_FAX "other fax"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_PAGER     "pager"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_PRIMARY   "primary"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_RADIO     "radio"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_TELEX     "telex"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_WORK      "work"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_PHONE_DESCRIPTION_WORK_FAX  "work fax"

const char         *desktopcouch_document_contact_phone_get_description (CouchdbStructField *sf);
void                desktopcouch_document_contact_phone_set_description (CouchdbStructField *sf, const char *description);

/*
 * Utility functions to manipulate addresses
 */
CouchdbStructField *desktopcouch_document_contact_address_new (const char *uuid,
							  const char *street,
							  const char *city,
							  const char *state,
							  const char *country,
							  const char *postalcode,
							  const char *pobox,
							  const char *description);
const char         *desktopcouch_document_contact_address_get_street (CouchdbStructField *sf);
void                desktopcouch_document_contact_address_set_street (CouchdbStructField *sf, const char *street);
const char         *desktopcouch_document_contact_address_get_city (CouchdbStructField *sf);
void                desktopcouch_document_contact_address_set_city (CouchdbStructField *sf, const char *city);
const char         *desktopcouch_document_contact_address_get_state (CouchdbStructField *sf);
void                desktopcouch_document_contact_address_set_state (CouchdbStructField *sf, const char *state);
const char         *desktopcouch_document_contact_address_get_country (CouchdbStructField *sf);
void                desktopcouch_document_contact_address_set_country (CouchdbStructField *sf, const char *country);
const char         *desktopcouch_document_contact_address_get_postalcode (CouchdbStructField *sf);
void                desktopcouch_document_contact_address_set_postalcode (CouchdbStructField *sf, const char *postalcode);
const char         *desktopcouch_document_contact_address_get_pobox (CouchdbStructField *sf);
void                desktopcouch_document_contact_address_set_pobox (CouchdbStructField *sf, const char *pobox);

#define DESKTOPCOUCH_DOCUMENT_CONTACT_ADDRESS_DESCRIPTION_HOME  "home"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_ADDRESS_DESCRIPTION_OTHER "other"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_ADDRESS_DESCRIPTION_WORK  "work"

const char         *desktopcouch_document_contact_address_get_description (CouchdbStructField *sf);
void                desktopcouch_document_contact_address_set_description (CouchdbStructField *sf, const char *description);

/*
 * Utility functions to manipulate IM addresses
 */
CouchdbStructField *desktopcouch_document_contact_im_new (const char *uuid,
						     const char *address,
						     const char *description,
						     const char *protocol);
const char         *desktopcouch_document_contact_im_get_address (CouchdbStructField *sf);
void                desktopcouch_document_contact_im_set_address (CouchdbStructField *sf, const char *address);
const char         *desktopcouch_document_contact_im_get_description (CouchdbStructField *sf);
void                desktopcouch_document_contact_im_set_description (CouchdbStructField *sf, const char *description);

#define DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_AIM       "aim"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_GADU_GADU "gadu-gadu"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_GROUPWISE "groupwise"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_ICQ       "icq"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_IRC       "irc"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_JABBER    "jabber"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_MSN       "msn"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_SKYPE     "skype"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_IM_PROTOCOL_YAHOO     "yahoo"

const char         *desktopcouch_document_contact_im_get_protocol (CouchdbStructField *sf);
void                desktopcouch_document_contact_im_set_protocol (CouchdbStructField *sf, const char *protocol);

/*
 * Utility functions to manipulate URLs
 */
CouchdbStructField *desktopcouch_document_contact_url_new (const char *uuid, const char *address, const char *description);
const char         *desktopcouch_document_contact_url_get_address  (CouchdbStructField *sf);
void                desktopcouch_document_contact_url_set_address (CouchdbStructField *sf, const char *address);

#define DESKTOPCOUCH_DOCUMENT_CONTACT_URL_DESCRIPTION_BLOG     "blog"
#define DESKTOPCOUCH_DOCUMENT_CONTACT_URL_DESCRIPTION_HOMEPAGE "home page"

const char         *desktopcouch_document_contact_url_get_description (CouchdbStructField *sf);
void                desktopcouch_document_contact_url_set_description (CouchdbStructField *sf, const char *description);

G_END_DECLS

#endif /* __DESKTOPCOUCH_DOCUMENT_CONTACT_H__ */
