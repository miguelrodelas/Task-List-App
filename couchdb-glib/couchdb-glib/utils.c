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

#include <uuid/uuid.h>
#include <string.h>
#include <libsoup/soup-session-async.h>
#include "couchdb-glib.h"
#include "utils.h"

GQuark
couchdb_error_quark (void)
{
	static GQuark error;

	if (!error)
		error = g_quark_from_static_string ("couchdb_glib");

	return error;
}

char *
generate_uuid (void)
{
	uuid_t uuid;
	char uuid_string[37];

	uuid_generate_random (uuid);
	uuid_unparse (uuid, uuid_string);

	return g_strdup (uuid_string);
}
