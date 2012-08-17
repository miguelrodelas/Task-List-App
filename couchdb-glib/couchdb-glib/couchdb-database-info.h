/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2009 Canonical Services Ltd (www.canonical.com)
 *               2009 Mikkel Kamstrup Erlandsen
 *
 * Authors: Rodrigo Moya <rodrigo.moya@canonical.com>
 *          Mikkel Kamstrup Erlandsen <mikkel.kamstrup@gmail.com>
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
 
#ifndef __COUCHDB_DATABASE_INFO_H__
#define __COUCHDB_DATABASE_INFO_H__
 
#include <glib.h>
#include <glib-object.h>
#include "couchdb-types.h"

G_BEGIN_DECLS

#define COUCHDB_TYPE_DATABASE_INFO (couchdb_database_info_get_type ())

/*
 * CouchdbDatabaseInfo
 */

GType                couchdb_database_info_get_type (void);
CouchdbDatabaseInfo *couchdb_database_info_ref (CouchdbDatabaseInfo *dbinfo);
void                 couchdb_database_info_unref (CouchdbDatabaseInfo *dbinfo);

const char          *couchdb_database_info_get_dbname (CouchdbDatabaseInfo *dbinfo);
gint                 couchdb_database_info_get_documents_count (CouchdbDatabaseInfo *dbinfo);
gint                 couchdb_database_info_get_deleted_documents_count (CouchdbDatabaseInfo *dbinfo);
gint                 couchdb_database_info_get_update_sequence (CouchdbDatabaseInfo *dbinfo);
gboolean             couchdb_database_info_is_compact_running (CouchdbDatabaseInfo *dbinfo);
gint                 couchdb_database_info_get_disk_size (CouchdbDatabaseInfo *dbinfo);

CouchdbDatabaseInfo*	couchdb_database_info_new (const char *dbname,
						gint doc_count,
						gint doc_del_count,
						gint update_seq,
						gboolean compact_running,
						gint disk_size);

G_END_DECLS

#endif /* __COUCHDB_DATABASE_INFO_H__ */
