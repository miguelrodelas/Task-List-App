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

#include <dbus/dbus-glib.h>
#include <gnome-keyring.h>
#include "desktopcouch-session.h"

G_DEFINE_TYPE(DesktopcouchSession, desktopcouch_session, COUCHDB_TYPE_SESSION)

static void
desktopcouch_session_finalize (GObject *object)
{
	G_OBJECT_CLASS (desktopcouch_session_parent_class)->finalize (object);
}

static void
desktopcouch_session_class_init (DesktopcouchSessionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = desktopcouch_session_finalize;
}

static void
desktopcouch_session_init (DesktopcouchSession *dc)
{
}

/**
 * desktopcouch_session_new:
 *
 * Create a #CouchdbSession instance prepared to connect to desktopcouch. Once
 * created, applications can use the #CouchdbSession API to communicate with
 * the underlying CouchDB instance.
 *
 * Return value: A #CouchdbSession instance.
 */
DesktopcouchSession *
desktopcouch_session_new (void)
{
	char *uri = NULL;
	DBusGConnection *bus;
	DBusGProxy *proxy;
	gint port;
	GError *error;
	gboolean success;

	/* Get the desktopcouch port via the org.desktopcouch.CouchDB interface */
	error = NULL;
	bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (error) {
		g_warning ("Couldn't get session bus: %s", error->message);
		g_error_free (error);

		return NULL;
	}

	proxy = dbus_g_proxy_new_for_name (bus,
					   "org.desktopcouch.CouchDB",
					   "/",
					   "org.desktopcouch.CouchDB");

	error = NULL;
	success = dbus_g_proxy_call (proxy, "getPort", &error, G_TYPE_INVALID,
				     G_TYPE_INT, &port, G_TYPE_INVALID);

	/* Free memory */
	g_object_unref (G_OBJECT (proxy));
	dbus_g_connection_unref (bus);

	if (success) {
		GnomeKeyringAttributeList *attrs;
		GnomeKeyringResult result;
		GList *items_found;

		uri = g_strdup_printf ("http://127.0.0.1:%d", port);

		/* Get OAuth tokens from GnomeKeyring */
		attrs = gnome_keyring_attribute_list_new ();
		gnome_keyring_attribute_list_append_string (attrs, "desktopcouch", "oauth");

		result = gnome_keyring_find_items_sync (GNOME_KEYRING_ITEM_GENERIC_SECRET,
							attrs, &items_found);
		if (result == GNOME_KEYRING_RESULT_OK && items_found != NULL) {
			gchar **items;
			char *oauth_c_key = NULL, *oauth_c_secret = NULL, *oauth_t_key = NULL, *oauth_t_secret = NULL;
			DesktopcouchSession *dc;
			CouchdbCredentials *credentials;
			GnomeKeyringFound *first_item = (GnomeKeyringFound *) items_found->data;

			items = g_strsplit (first_item->secret, ":", 4);
			if (items) {
				oauth_c_key = g_strdup (items[0]);
				oauth_c_secret = g_strdup (items[1]);
				oauth_t_key = g_strdup (items[2]);
				oauth_t_secret = g_strdup (items[3]);
				g_strfreev (items);
			}

			gnome_keyring_found_list_free (items_found);

			/* Enable OAuth on this connection */
			dc = DESKTOPCOUCH_SESSION (g_object_new (DESKTOPCOUCH_TYPE_SESSION, "uri", uri, NULL));

			credentials = couchdb_credentials_new_with_oauth (oauth_c_key,
									  oauth_c_secret,
									  oauth_t_key,
									  oauth_t_secret);
			couchdb_session_enable_authentication (COUCHDB_SESSION (dc), credentials);

			/* Free memory */
			g_free (oauth_c_key);
			g_free (oauth_c_secret);
			g_free (oauth_t_key);
			g_free (oauth_t_secret);
			g_free (uri);
			g_object_unref (G_OBJECT (credentials));

			return dc;
		} else {
			g_warning ("Could not get OAuth tokens from keyring: %s",
				   gnome_keyring_result_to_message (result));
		}

		/* Free memory */
		gnome_keyring_attribute_list_free (attrs);
	} else {
		g_warning ("Couldn't get port for desktopcouch: %s", error->message);
		g_error_free (error);
	}

	return NULL;
}
