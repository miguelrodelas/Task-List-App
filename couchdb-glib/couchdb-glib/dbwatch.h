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

#ifndef __DBWATCH_H__
#define __DBWATCH_H__

#include <glib.h>
#include "couchdb-session.h"
#include "utils.h"

typedef struct {
	CouchdbSession *couchdb;
	gchar *dbname;
	gint last_update_seq;
	guint timeout_id;
} DBWatch;

DBWatch *dbwatch_new (CouchdbSession *couchdb, const gchar *dbname, gint update_seq);
void     dbwatch_free (DBWatch *watch);

#endif /* __DBWATCH_H__ */
