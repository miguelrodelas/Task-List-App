/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* e-book-backend-couchdb.h - CouchDB contact backend factory.
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

#ifndef __E_BOOK_BACKEND_COUCHDB_H__
#define __E_BOOK_BACKEND_COUCHDB_H__

#include <couchdb-glib.h>
#include <desktopcouch-glib.h>
#include <libedata-book/e-book-backend.h>
#include <libedata-book/e-book-backend-cache.h>

#define E_TYPE_BOOK_BACKEND_COUCHDB        (e_book_backend_couchdb_get_type ())
#define E_BOOK_BACKEND_COUCHDB(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), E_TYPE_BOOK_BACKEND_COUCHDB, EBookBackendCouchDB))
#define E_BOOK_BACKEND_COUCHDB_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), E_TYPE_BOOK_BACKEND_COUCHDB, EBookBackendCouchDBClass))
#define E_IS_BOOK_BACKEND_COUCHDB(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), E_TYPE_BOOK_BACKEND_COUCHDB))
#define E_IS_BOOK_BACKEND_COUCHDB_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), E_TYPE_BOOK_BACKEND_COUCHDB))
#define E_BOOK_BACKEND_COUCHDB_GET_CLASS(k) (G_TYPE_INSTANCE_GET_CLASS ((obj), E_TYPE_BOOK_BACKEND_COUCHDB, EBookBackenCouchDBClass))

typedef struct {
	EBookBackend parent_object;

	CouchdbSession *couchdb;
	EBookBackendCache *cache;
	char *dbname;
	gboolean using_desktopcouch;
} EBookBackendCouchDB;

typedef struct {
	EBookBackendClass parent_class;
} EBookBackendCouchDBClass;

GType         e_book_backend_couchdb_get_type (void);
EBookBackend *e_book_backend_couchdb_new (void);

#endif
