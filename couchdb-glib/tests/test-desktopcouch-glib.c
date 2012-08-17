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

#include <desktopcouch-glib.h>

static DesktopcouchSession *dc = NULL;

static void
test_connect_desktopcouch (void)
{
	/* Create desktopcouch session */
	dc = desktopcouch_session_new ();
	g_assert (DESKTOPCOUCH_IS_SESSION (dc));
}

static void
test_list_databases (void)
{
	GSList *dblist, *sl;
	GError *error = NULL;

	/* List databases */
	dblist = couchdb_session_list_databases (COUCHDB_SESSION (dc), &error);
	if (error != NULL) {
		g_warning ("Error listing databases: %s", error->message);
		g_error_free (error);
		g_assert (error == NULL);
	}

	for (sl = dblist; sl != NULL; sl = sl->next)
		g_print ("Found database %s\n", (const char *) sl->data);

	/* Free memory */
	couchdb_session_free_database_list (dblist);
}

int
main (int argc, char *argv[])
{
	g_type_init ();
	g_thread_init (NULL);
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/testdesktopcouchglib/Connect", test_connect_desktopcouch);
	g_test_add_func ("/testdesktopcouchglib/ListDatabases", test_list_databases);

	return g_test_run ();
}
