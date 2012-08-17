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

#include "couchdb-credentials.h"

struct _CouchdbCredentialsPrivate {
	CouchdbCredentialsType type;
	GHashTable *auth_data;
};

G_DEFINE_TYPE(CouchdbCredentials, couchdb_credentials, G_TYPE_OBJECT)

static void
couchdb_credentials_finalize (GObject *object)
{
	CouchdbCredentials *credentials = COUCHDB_CREDENTIALS (object);

	if (credentials->priv != NULL) {
		if (credentials->priv->auth_data != NULL) {
			g_hash_table_destroy (credentials->priv->auth_data);
			credentials->priv->auth_data = NULL;
		}

		g_free (credentials->priv);
		credentials->priv = NULL;
	}

	G_OBJECT_CLASS (couchdb_credentials_parent_class)->finalize (object);
}

static void
couchdb_credentials_class_init (CouchdbCredentialsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = couchdb_credentials_finalize;
}

static void
couchdb_credentials_init (CouchdbCredentials *credentials)
{
	credentials->priv = g_new0 (CouchdbCredentialsPrivate, 1);

	credentials->priv->type = COUCHDB_CREDENTIALS_TYPE_UNKNOWN;
	credentials->priv->auth_data = g_hash_table_new_full (g_str_hash, g_str_equal,
							      (GDestroyNotify) g_free,
							      (GDestroyNotify) g_free);
}

/**
 * couchdb_credentials_new_with_oauth:
 * @consumer_key: OAuth consumer key
 * @consumer_secret: OAuth consumer secret
 * @token_key: OAuth token key
 * @token_secret: OAuth token secret
 *
 * Create a new #CouchdbCredentials object to be used for OAuth authentication.
 *
 * Return value: A #CouchdbCredentials object.
 */
CouchdbCredentials *
couchdb_credentials_new_with_oauth (const gchar *consumer_key,
				    const gchar *consumer_secret,
				    const gchar *token_key,
				    const gchar *token_secret)
{
	CouchdbCredentials *credentials;

	credentials = COUCHDB_CREDENTIALS (g_object_new (COUCHDB_TYPE_CREDENTIALS, NULL));
	credentials->priv->type = COUCHDB_CREDENTIALS_TYPE_OAUTH;
	couchdb_credentials_set_item (credentials, COUCHDB_CREDENTIALS_ITEM_OAUTH_CONSUMER_KEY, consumer_key);
	couchdb_credentials_set_item (credentials, COUCHDB_CREDENTIALS_ITEM_OAUTH_CONSUMER_SECRET, consumer_secret);
	couchdb_credentials_set_item (credentials, COUCHDB_CREDENTIALS_ITEM_OAUTH_TOKEN_KEY, token_key);
	couchdb_credentials_set_item (credentials, COUCHDB_CREDENTIALS_ITEM_OAUTH_TOKEN_SECRET, token_secret);

	return credentials;
}

/**
 * couchdb_credentials_new_with_username_and_password:
 * @username: HTTP Authentication username
 * @password: HTTP Authentication password
 *
 * Create a new #CouchdbCredentials object to be used for username
 * and password based HTTP authentication scheme such as Basic or
 * Digest.
 *
 * Authentication is delegated to LibSoup.
 *
 * Return value: A #CouchdbCredentials object.
 */
CouchdbCredentials *
couchdb_credentials_new_with_username_and_password (const gchar *username,
						    const gchar *password)
{
	CouchdbCredentials *credentials;

	credentials = COUCHDB_CREDENTIALS (g_object_new (COUCHDB_TYPE_CREDENTIALS, NULL));
	credentials->priv->type = COUCHDB_CREDENTIALS_TYPE_USERNAME_AND_PASSWORD;
	couchdb_credentials_set_item (credentials,
				      COUCHDB_CREDENTIALS_ITEM_USERNAME,
				      username);
	couchdb_credentials_set_item (credentials,
				      COUCHDB_CREDENTIALS_ITEM_PASSWORD,
				      password);

	return credentials;
}

/**
 * couchdb_credentials_get_auth_type:
 * @credentials: A #CouchdbCredentials object
 *
 * Retrieve the type of authentication defined for the given #CouchdbCredentials object.
 *
 * Return value: A #CouchdbCredentialsType specifying the type of authentication.
 */
CouchdbCredentialsType
couchdb_credentials_get_auth_type (CouchdbCredentials *credentials)
{
	g_return_val_if_fail (COUCHDB_IS_CREDENTIALS (credentials), COUCHDB_CREDENTIALS_TYPE_UNKNOWN);

	return credentials->priv->type;
}

/**
 * couchdb_credentials_get_item:
 * @credentials: A #CouchdbCredentials object
 * @item: Name of the item to retrieve
 *
 * Get the value associated with one authentication item on the given #CouchdbCredentials object.
 * The valid values are the COUCHDB_CREDENTIALS_ITEM_* values defined in couchdb-credentials.h.
 *
 * Return value: The value associated with the given item, or NULL if not found.
 */
const gchar *
couchdb_credentials_get_item (CouchdbCredentials *credentials, const gchar *item)
{
	g_return_val_if_fail (COUCHDB_IS_CREDENTIALS (credentials), NULL);

	return (const gchar *) g_hash_table_lookup (credentials->priv->auth_data, item);
}

/**
 * couchdb_credentials_set_item:
 * @credentials: A #CouchdbCredentials object
 * @item: Name of the item to set the value of
 * #value: Value of the item
 *
 * Set the value associated with one authentication item on the given #CouchdbCredentials object.
 * The valid values are the COUCHDB_CREDENTIALS_ITEM_* values defined in couchdb-credentials.h.
 */
void
couchdb_credentials_set_item (CouchdbCredentials *credentials, const gchar *item, const gchar *value)
{
	g_return_if_fail (COUCHDB_IS_CREDENTIALS (credentials));

	g_hash_table_insert (credentials->priv->auth_data,
			     g_strdup (item),
			     g_strdup (value));
}
