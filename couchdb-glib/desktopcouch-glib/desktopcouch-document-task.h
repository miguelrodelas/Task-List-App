/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2010 Canonical Services Ltd (www.canonical.com)
 *
 * Authors: Miguel Angel Rodelas Delgado <miguel.rodelas@gmail.com>
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

#ifndef __DESKTOPCOUCH_DOCUMENT_TASK_H__
#define __DESKTOPCOUCH_DOCUMENT_TASK_H__

#include <couchdb-struct-field.h>
#include <desktopcouch-document.h>

G_BEGIN_DECLS

#define DESKTOPCOUCH_RECORD_TYPE_TASK "http://www.freedesktop.org/wiki/Specifications/desktopcouch/task"

CouchdbDocument *desktopcouch_document_task_new (CouchdbSession *couchdb);
gboolean         desktopcouch_document_is_task (CouchdbDocument *document);

/*
 * Top level functions to manipulate documents representing a task
 */

const char *desktopcouch_document_task_get_summary (CouchdbDocument *document);
void        desktopcouch_document_task_set_summary (CouchdbDocument *document, const char *summary);

const char *desktopcouch_document_task_get_description (CouchdbDocument *document);
void        desktopcouch_document_task_set_description (CouchdbDocument *document, const char *description);

G_END_DECLS

#endif /* __DESKTOPCOUCH_DOCUMENT_TASK_H__ */
