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

#ifndef __COUCHDB_SESSION_H__
#define __COUCHDB_SESSION_H__

#include <glib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include "couchdb-types.h"
#include "couchdb-credentials.h"
#include "couchdb-database-info.h"

G_BEGIN_DECLS

#define COUCHDB_TYPE_SESSION                (couchdb_session_get_type ())
#define COUCHDB_SESSION(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), COUCHDB_TYPE_SESSION, CouchdbSession))
#define COUCHDB_IS_SESSION(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), COUCHDB_TYPE_SESSION))
#define COUCHDB_SESSION_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), COUCHDB_TYPE_SESSION, CouchdbSessionClass))
#define COUCHDB_IS_SESSION_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), COUCHDB_TYPE_SESSION))
#define COUCHDB_SESSION_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), COUCHDB_TYPE_SESSION, CouchdbSessionClass))

typedef struct _CouchdbSessionPrivate CouchdbSessionPrivate;

typedef struct {
	GObject parent;

	CouchdbSessionPrivate *priv;
} CouchdbSession;

typedef struct {
	GObjectClass parent_class;

	void (* authentication_failed) (CouchdbSession *couchdb);

	void (* database_created) (CouchdbSession *couchdb, const char *dbname);
	void (* database_deleted) (CouchdbSession *couchdb, const char *dbname);

	void (* document_created) (CouchdbSession *couchdb, const char *dbname, CouchdbDocument *document);
	void (* document_updated) (CouchdbSession *couchdb, const char *dbname, CouchdbDocument *document);
	void (* document_deleted) (CouchdbSession *couchdb, const char *dbname, const char *docid);
} CouchdbSessionClass;

GType                couchdb_session_get_type (void);
CouchdbSession      *couchdb_session_new (const char *uri);

const char          *couchdb_session_get_uri (CouchdbSession *couchdb);

GSList              *couchdb_session_list_databases (CouchdbSession *couchdb, GError **error);
void                 couchdb_session_free_database_list (GSList *dblist);

CouchdbDatabaseInfo *couchdb_session_get_database_info (CouchdbSession *couchdb, const char *dbname, GError **error);

gboolean             couchdb_session_create_database (CouchdbSession *couchdb, const char *dbname, GError **error);
gboolean             couchdb_session_delete_database (CouchdbSession *couchdb, const char *dbname, GError **error);
gboolean             couchdb_session_compact_database (CouchdbSession *couchdb, const char *dbname, GError **error);

void                 couchdb_session_listen_for_changes (CouchdbSession *couchdb, const char *dbname);

void                 couchdb_session_enable_authentication (CouchdbSession *couchdb, CouchdbCredentials *credentials);
void                 couchdb_session_disable_authentication (CouchdbSession *couchdb);
gboolean             couchdb_session_is_authentication_enabled (CouchdbSession *couchdb);

gboolean             couchdb_session_send_message (CouchdbSession *couchdb,
						   const char *method,
						   const char *url,
						   const char *body,
						   JsonParser *output,
						   GError **error);

GSList              *couchdb_session_list_documents (CouchdbSession *couchdb, const char *dbname, GError **error);
void                 couchdb_session_free_document_list (GSList *doclist);

gboolean             couchdb_session_replicate (CouchdbSession *couchdb,
						const gchar *source,
						const gchar *target,
						gboolean continous,
						GError **error);

G_END_DECLS

#endif /* __COUCHDB_H__ */
