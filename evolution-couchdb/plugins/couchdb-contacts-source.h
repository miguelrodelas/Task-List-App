/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* couchdb-contacts-source.h - CouchDB contact backend
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

#ifndef __COUCHDB_CONTACTS_SOURCE_H__
#define __COUCHDB_CONTACTS_SOURCE_H__

GtkWidget *plugin_couchdb_contacts (EPlugin                    *epl,
				    EConfigHookItemFactoryData *data);

#endif
