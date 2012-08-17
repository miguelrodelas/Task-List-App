/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* e-cal-backend-couchdb-factory.c - Couchdb calendar backend factory.
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

#ifdef CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "e-cal-backend-couchdb-factory.h"

#include <libebackend/e-data-server-module.h>
#include <libedata-cal/e-cal-backend-factory.h>
#include "e-cal-backend-couchdb.h"

typedef struct {
	ECalBackendFactory parent_object;
}ECalBackendCouchDBFactory;

typedef struct {
	ECalBackendFactoryClass parent_class;
}ECalBackendCouchDBFactoryClass;

static void
e_cal_backend_couchdb_factory_instance_init (ECalBackendCouchDBFactory *factory)
{

}

static const gchar *
get_protocol (ECalBackendFactory *factory)
{
	return "couchdb";
}

static ECalBackend *
todos_new_backend (ECalBackendFactory *factory, ESource *source)
{
	return g_object_new (e_cal_backend_couchdb_get_type (),
			     "source", source,
			     "kind", ICAL_VTODO_COMPONENT,
			     NULL);

}

static icalcomponent_kind
todos_get_kind (ECalBackendFactory *factory)
{
	return ICAL_VTODO_COMPONENT;
}

static ECalBackend*
events_new_backend (ECalBackendFactory *factory, ESource *source)
{
	return g_object_new (e_cal_backend_couchdb_get_type (),
			     "source", source,
			     "kind", ICAL_VEVENT_COMPONENT,
			     NULL);
}

static icalcomponent_kind
events_get_kind (ECalBackendFactory *factory)
{
	return ICAL_VEVENT_COMPONENT;
}

static void
todos_backend_factory_class_init (ECalBackendCouchDBFactoryClass *klass)
{
	E_CAL_BACKEND_FACTORY_CLASS (klass)->get_protocol = get_protocol;
	E_CAL_BACKEND_FACTORY_CLASS (klass)->get_kind     = todos_get_kind;
	E_CAL_BACKEND_FACTORY_CLASS (klass)->new_backend  = todos_new_backend;
}

static void
events_backend_factory_class_init (ECalBackendCouchDBFactoryClass *klass)
{
	E_CAL_BACKEND_FACTORY_CLASS (klass)->get_protocol = get_protocol;
	E_CAL_BACKEND_FACTORY_CLASS (klass)->get_kind     = events_get_kind;
	E_CAL_BACKEND_FACTORY_CLASS (klass)->new_backend  = events_new_backend;
}

static GType
events_backend_factory_get_type (GTypeModule *module)
{
	GType type;

	GTypeInfo info = {
		sizeof (ECalBackendCouchDBFactoryClass),
		NULL, /* base_class_init */
		NULL, /* base_class_finalize */
		(GClassInitFunc)  events_backend_factory_class_init,
		NULL, /* class_finalize */
		NULL, /* class_data */
		sizeof (ECalBackend),
		0,    /* n_preallocs */
		(GInstanceInitFunc) e_cal_backend_couchdb_factory_instance_init
	};

	type = g_type_module_register_type (module,
					    E_TYPE_CAL_BACKEND_FACTORY,
					    "ECalBackendCouchDBEventsFactory",
					    &info, 0);

	return type;
}

static GType
todos_backend_factory_get_type (GTypeModule *module)
{
	GType type;

	GTypeInfo info = {
		sizeof (ECalBackendCouchDBFactoryClass),
		NULL, /* base_class_init */
		NULL, /* base_class_finalize */
		(GClassInitFunc)  todos_backend_factory_class_init,
		NULL, /* class_finalize */
		NULL, /* class_data */
		sizeof (ECalBackend),
		0,    /* n_preallocs */
		(GInstanceInitFunc) e_cal_backend_couchdb_factory_instance_init
	};

	type = g_type_module_register_type (module,
					    E_TYPE_CAL_BACKEND_FACTORY,
					    "ECalBackendCouchDBTodosFactory",
					    &info, 0);

	return type;
}

static GType couchdb_types[2];

void
eds_module_initialize (GTypeModule *module)
{
	couchdb_types[0] = todos_backend_factory_get_type (module);
	couchdb_types[1] = events_backend_factory_get_type (module);
}

void
eds_module_shutdown (void)
{
}

void
eds_module_list_types (const GType **types, gint *num_types)
{
	*types = couchdb_types;
	*num_types = 2;
}
