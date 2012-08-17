/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* e-book-backend-couchdb-factory.c - Couchdb contact backend factory.
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

#include <libebackend/e-data-server-module.h>
#include <libedata-book/e-book-backend-factory.h>
#include "e-book-backend-couchdb.h"

E_BOOK_BACKEND_FACTORY_SIMPLE (couchdb, CouchDB, e_book_backend_couchdb_new)

static GType couchdb_type;

void
eds_module_initialize (GTypeModule *module)
{
	couchdb_type = _couchdb_factory_get_type (module);
}

void
eds_module_shutdown (void)
{
}

void
eds_module_list_types (const GType **types, int *num_types)
{
	*types = &couchdb_type;
	*num_types = 1;
}
