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

#include "couchdb-database-info.h"

struct _CouchdbDatabaseInfo {
	gint ref_count;

	char *dbname;
	gint doc_count;
	gint doc_del_count;
	gint update_seq;
	gboolean compact_running;
	gint disk_size;
};

/*
 * CouchdbDatabaseInfo object
 */

GType
couchdb_database_info_get_type (void)
{
	static GType object_type = 0;

	if (G_UNLIKELY (!object_type))
		object_type = g_boxed_type_register_static (g_intern_static_string ("CouchdbDatabaseInfo"),
							    (GBoxedCopyFunc) couchdb_database_info_ref,
							    (GBoxedFreeFunc) couchdb_database_info_unref);

	return object_type;
}

/**
 * couchdb_database_info_new:
 * @dbname: Database name
 * @doc_count: Number of documents in the database
 * @doc_del_count: Number of deleted documents in the database
 * @update_seq: Last update sequence
 * @compact_running: Whether compacting is in progress
 * @disk_size: Size of database on disk
 *
 * Create a new @CouchdbDatabaseInfo object, which is used to store information
 * (name, number of documents, etc) of a database in CouchDB.
 *
 * Return value: A newly-created #CouchdbDatabaseInfo object.
 */
CouchdbDatabaseInfo *
couchdb_database_info_new (const char *dbname,
			   gint doc_count,
			   gint doc_del_count,
			   gint update_seq,
			   gboolean compact_running,
			   gint disk_size)
{
	CouchdbDatabaseInfo *dbinfo;

	dbinfo = g_slice_new (CouchdbDatabaseInfo);
	dbinfo->ref_count = 1;
	dbinfo->dbname = g_strdup (dbname);
	dbinfo->doc_count = doc_count;
	dbinfo->doc_del_count = doc_del_count;
	dbinfo->update_seq = update_seq;
	dbinfo->compact_running = compact_running;
	dbinfo->disk_size = disk_size;

	return dbinfo;
}

/**
 * couchdb_database_info_ref:
 * @doc_info: A #CouchdbDatabaseInfo object
 *
 * Increments reference counting of the given #CouchdbDatabaseInfo object.
 *
 * Return value: A pointer to the object being referenced.
 */
CouchdbDatabaseInfo *
couchdb_database_info_ref (CouchdbDatabaseInfo *dbinfo)
{
	g_return_val_if_fail (dbinfo != NULL, NULL);
	g_return_val_if_fail (dbinfo->ref_count > 0, NULL);

	g_atomic_int_exchange_and_add (&dbinfo->ref_count, 1);

	return dbinfo;
}

/**
 * couchdb_database_info_unref:
 * @doc_info: A #CouchdbDatabaseInfo object
 *
 * Decrements reference counting of the given #CouchdbDatabaseInfo object.
 * When the reference count is equal to 0, the object will be destroyed.
 */
void
couchdb_database_info_unref (CouchdbDatabaseInfo *dbinfo)
{
	gint old_ref;

	g_return_if_fail (dbinfo != NULL);
	g_return_if_fail (dbinfo->ref_count > 0);

	old_ref = g_atomic_int_get (&dbinfo->ref_count);
	if (old_ref > 1)
		g_atomic_int_compare_and_exchange (&dbinfo->ref_count, old_ref, old_ref - 1);
	else {
		g_free (dbinfo->dbname);
		g_slice_free (CouchdbDatabaseInfo, dbinfo);
	}
}

/**
 * couchdb_database_info_get_dbname:
 * @dbinfo: A #CouchdbDatabaseInfo object
 *
 * Get the database name stored in the #CouchdbDatabaseInfo object.
 *
 * Return value: Name of the database.
 */
const char *
couchdb_database_info_get_dbname (CouchdbDatabaseInfo *dbinfo)
{
	g_return_val_if_fail (dbinfo != NULL, NULL);

	return (const char *) dbinfo->dbname;
}

/**
 * couchdb_database_info_get_documents_count:
 * @dbinfo: A #CouchdbDatabaseInfo object
 *
 * Get the number of documents stored in the #CouchdbDatabaseInfo object.
 *
 * Return value: Number of documents in the database.
 */
gint
couchdb_database_info_get_documents_count (CouchdbDatabaseInfo *dbinfo)
{
	g_return_val_if_fail (dbinfo != NULL, 0);

	return dbinfo->doc_count;
}

/**
 * couchdb_database_info_get_deleted_documents_count:
 * @dbinfo: A #CouchdbDatabaseInfo object
 *
 * Get the number of deleted documents stored in the #CouchdbDatabaseInfo object.
 *
 * Return value: Number of deleted documents.
 */
gint
couchdb_database_info_get_deleted_documents_count (CouchdbDatabaseInfo *dbinfo)
{
	g_return_val_if_fail (dbinfo != NULL, 0);

	return dbinfo->doc_del_count;
}

/**
 * couchdb_database_info_get_update_sequence:
 * @dbinfo: A #CouchdbDatabaseInfo object
 *
 * Get the last update sequence stored in the #CouchdbDatabaseInfo object.
 * This sequence is incremented with each change done to the database.
 *
 * Return value: Last update sequence.
 */
gint
couchdb_database_info_get_update_sequence (CouchdbDatabaseInfo *dbinfo)
{
	g_return_val_if_fail (dbinfo != NULL, 0);

	return dbinfo->update_seq;
}

/**
 * couchdb_database_info_is_compact_running:
 * @dbinfo: A #CouchdbDatabaseInfo object
 *
 * Get whether compacting is running on the database at the time the information
 * was retrieved.
 *
 * Return value: Whether compacting is running or not.
 */
gboolean
couchdb_database_info_is_compact_running (CouchdbDatabaseInfo *dbinfo)
{
	g_return_val_if_fail (dbinfo != NULL, FALSE);

	return dbinfo->compact_running;
}

/**
 * couchdb_database_info_get_disk_size:
 * @dbinfo: A #CouchdbDatabaseInfo object
 *
 * Get the size of database on disk stored in the #CouchdbDatabaseInfo object.
 *
 * Return value: Size of the database on disk.
 */
gint
couchdb_database_info_get_disk_size (CouchdbDatabaseInfo *dbinfo)
{
	g_return_val_if_fail (dbinfo != NULL, 0);

	return dbinfo->disk_size;
}

