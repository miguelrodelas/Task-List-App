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
 
#ifndef __COUCHDB_TYPES_H__
#define __COUCHDB_TYPES_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _CouchdbArrayField CouchdbArrayField;
typedef struct _CouchdbDocument CouchdbDocument;
typedef struct _CouchdbDatabaseInfo CouchdbDatabaseInfo;
typedef struct _CouchdbDocumentInfo CouchdbDocumentInfo;
typedef struct _CouchdbStructField CouchdbStructField;

G_END_DECLS

#endif /* __COUCHDB_TYPES_H__ */
