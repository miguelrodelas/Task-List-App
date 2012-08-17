/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* e-cal-backend-couchdb-factory.h - Couchdb calendar backend factory.
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
 * Authors: Miguel Angel Rodelas Delgado <miguel.rodelas@gmail.com>
 */

#ifndef _E_CAL_BACKEND_COUCHDB_FACTORY_H
#define _E_CAL_BACKEND_COUCHDB_FACTORY_H

#include<glib.h>
#include "libedata-cal/e-cal-backend-factory.h"

G_BEGIN_DECLS

void eds_module_initialize (GTypeModule *module);
void eds_module_shutdown (void);
void eds_module_list_types (const GType **types, gint *num_types);

G_END_DECLS

#endif

