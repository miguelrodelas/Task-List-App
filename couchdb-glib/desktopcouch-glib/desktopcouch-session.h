/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2010 Canonical Services Ltd (www.canonical.com)
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

#ifndef __DESKTOPCOUCH_SESSION_H__
#define __DESKTOPCOUCH_SESSION_H__

#include <couchdb-glib.h>

G_BEGIN_DECLS

#define DESKTOPCOUCH_TYPE_SESSION                (desktopcouch_session_get_type ())
#define DESKTOPCOUCH_SESSION(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), DESKTOPCOUCH_TYPE_SESSION, DesktopcouchSession))
#define DESKTOPCOUCH_IS_SESSION(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DESKTOPCOUCH_TYPE_SESSION))
#define DESKTOPCOUCH_SESSION_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), DESKTOPCOUCH_TYPE_SESSION, DesktopcouchSessionClass))
#define DESKTOPCOUCH_IS_SESSION_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), DESKTOPCOUCH_TYPE_SESSION))
#define DESKTOPCOUCH_SESSION_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), DESKTOPCOUCH_TYPE_SESSION, DesktopouchSessionClass))

typedef struct {
	CouchdbSession parent;
} DesktopcouchSession;

typedef struct {
	CouchdbSessionClass parent_class;
} DesktopcouchSessionClass;

GType                desktopcouch_session_get_type (void);
DesktopcouchSession *desktopcouch_session_new (void);

G_END_DECLS

#endif
