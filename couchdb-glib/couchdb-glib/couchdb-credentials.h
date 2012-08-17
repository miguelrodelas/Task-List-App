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

#ifndef __COUCHDB_CREDENTIALS_H__
#define __COUCHDB_CREDENTIALS_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define COUCHDB_TYPE_CREDENTIALS                (couchdb_credentials_get_type ())
#define COUCHDB_CREDENTIALS(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), COUCHDB_TYPE_CREDENTIALS, CouchdbCredentials))
#define COUCHDB_IS_CREDENTIALS(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), COUCHDB_TYPE_CREDENTIALS))
#define COUCHDB_CREDENTIALS_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), COUCHDB_TYPE_CREDENTIALS, CouchdbCredentialsClass))
#define COUCHDB_IS_CREDENTIALS_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), COUCHDB_TYPE_CREDENTIALS))
#define COUCHDB_CREDENTIALS_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), COUCHDB_TYPE_CREDENTIALS, CouchdbCredentialsClass))

typedef struct _CouchdbCredentialsPrivate CouchdbCredentialsPrivate;

typedef struct {
	GObject parent;
	CouchdbCredentialsPrivate *priv;
} CouchdbCredentials;

typedef struct {
	GObjectClass parent_class;
} CouchdbCredentialsClass;

typedef enum {
	COUCHDB_CREDENTIALS_TYPE_UNKNOWN = -1,
	COUCHDB_CREDENTIALS_TYPE_OAUTH,
	COUCHDB_CREDENTIALS_TYPE_USERNAME_AND_PASSWORD
} CouchdbCredentialsType;

GType                  couchdb_credentials_get_type (void);
CouchdbCredentials    *couchdb_credentials_new_with_oauth (const gchar *consumer_key,
							   const gchar *consumer_secret,
							   const gchar *token_key,
							   const gchar *token_secret);
CouchdbCredentials    *couchdb_credentials_new_with_username_and_password (const gchar *username,
									   const gchar *password);

CouchdbCredentialsType couchdb_credentials_get_auth_type (CouchdbCredentials *credentials);

#define COUCHDB_CREDENTIALS_ITEM_OAUTH_CONSUMER_KEY    "oauth_consumer_key"
#define COUCHDB_CREDENTIALS_ITEM_OAUTH_CONSUMER_SECRET "oauth_consumer_secret"
#define COUCHDB_CREDENTIALS_ITEM_OAUTH_TOKEN_KEY       "oauth_token_key"
#define COUCHDB_CREDENTIALS_ITEM_OAUTH_TOKEN_SECRET    "oauth_token_secret"
#define COUCHDB_CREDENTIALS_ITEM_USERNAME	       "username"
#define COUCHDB_CREDENTIALS_ITEM_PASSWORD	       "password"

const gchar           *couchdb_credentials_get_item (CouchdbCredentials *credentials, const gchar *item);
void                   couchdb_credentials_set_item (CouchdbCredentials *credentials,
						     const gchar *item,
						     const gchar *value);

G_END_DECLS

#endif /* __COUCHDB_CREDENTIALS_H__ */
