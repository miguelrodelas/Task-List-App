/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* e-cal-backend-couchdb.c - CouchDB calendar backend
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

#include <string.h>
#include "e-cal-backend-couchdb.h"
#include <libedata-cal/e-cal-backend-sexp.h>
#include <libedata-cal/e-data-cal.h>
#include <libedata-cal/e-data-cal-view.h>
#include <dbus/dbus-glib.h>
#include <gnome-keyring.h>

#define COUCHDB_REVISION_PROP                "X-COUCHDB-REVISION"
#define COUCHDB_UUID_PROP                    "X-COUCHDB-UUID"
#define COUCHDB_APPLICATION_ANNOTATIONS_PROP "X-COUCHDB-APPLICATION-ANNOTATIONS"

G_DEFINE_TYPE (ECalBackendCouchDB, e_cal_backend_couchdb, E_TYPE_CAL_BACKEND);


/*********************** Auxiliar methods ********************************/


static ECalComponent *
task_from_couch_document (CouchdbDocument *document)
{
	ECalComponent *task;
	CouchdbStructField *app_annotations;
	
	const char *uid;
	ECalComponentText summary;
	ECalComponentText description;

	GSList *description_list;



	if (!desktopcouch_document_is_task (document))
		return NULL;

	/* Check if the task is marked for deletion */
	if ((app_annotations = desktopcouch_document_get_application_annotations (document))) {
		CouchdbStructField *u1_annotations;

		u1_annotations = couchdb_struct_field_get_struct_field (
			app_annotations, "Ubuntu One");
		if (u1_annotations != NULL) {
			CouchdbStructField *private_annotations;

			private_annotations = couchdb_struct_field_get_struct_field (
				u1_annotations, "private_application_annotations");
			if (private_annotations != NULL) {
				if (couchdb_struct_field_has_field (private_annotations, "deleted")
				    && couchdb_struct_field_get_boolean_field (private_annotations, "deleted"))
					couchdb_struct_field_unref (app_annotations);
					return NULL;
			}
		}
	}

	/* Fill in the ECalComponent with the data from the CouchDBDocument*/
	task = e_cal_component_new ();
	e_cal_component_set_new_vtype (task, E_CAL_COMPONENT_TODO);

	uid = couchdb_document_get_id (document);

	summary.value = (const gchar *)desktopcouch_document_task_get_summary (document);
	summary.altrep = NULL;

	description.value = (const gchar *)desktopcouch_document_task_get_description (document);
	description.altrep = NULL;
	description_list = g_slist_append (description_list, &description);


	if (uid == NULL)
		return NULL;
	else
		e_cal_component_set_uid (task, uid);
	
	if (summary.value == NULL)
		return NULL;
	else
		e_cal_component_set_summary (task, &summary);
	
	e_cal_component_set_description_list (task, description_list);
	
	return task;
}

static CouchdbDocument *
couch_document_from_task (ECalBackendCouchDB *couchdb_backend, ECalComponent *task)
{
	CouchdbDocument *document;

	const char *uid;
	ECalComponentText summary;
	ECalComponentText *description;

	GSList *description_list = NULL;


	/* create the CouchDBDocument to put on the database */
	document = desktopcouch_document_task_new (couchdb_backend->couchdb);

	e_cal_component_get_uid (task, &uid);
	e_cal_component_get_summary (task, &summary);
	e_cal_component_get_description_list (task, &description_list);

	if (uid)
		couchdb_document_set_id (document, uid);

	summary.altrep = NULL;
	description = (ECalComponentText *) description_list->data;

	desktopcouch_document_task_set_summary (document, (const char *)summary.value);
	desktopcouch_document_task_set_description (document, (const char *)description->value);
	
	return document;
}

static void
document_updated_cb (CouchdbSession *couchdb, const char *dbname, CouchdbDocument *document, gpointer user_data)
{
	ECalComponent *task;
	ECalBackendCouchDB *couchdb_backend = E_CAL_BACKEND_COUCHDB (user_data);
	const gchar *uid, *task_string;

	if (g_strcmp0 (dbname, couchdb_backend->dbname) != 0)
		return;

	task = task_from_couch_document (document);
	if (!task)
		return;

	e_cal_component_get_uid (task, &uid);
	e_cal_component_commit_sequence (task);
	task_string = e_cal_component_get_as_string(task);
	
	if (e_cal_backend_cache_get_component(couchdb_backend->cache, uid, NULL) != NULL) {
		e_cal_backend_notify_object_modified (E_CAL_BACKEND (couchdb_backend), task_string, task_string);
	} else {
		e_cal_backend_notify_object_created (E_CAL_BACKEND (couchdb_backend), task_string);

		/* Add the task to the cache */
		e_cal_backend_cache_put_component (couchdb_backend->cache, task);
	}

	g_object_unref (G_OBJECT (task));
}


static void
document_deleted_cb (CouchdbSession *couchdb, const char *dbname, const char *docid, gpointer user_data)
{
	ECalBackendCouchDB *couchdb_backend = E_CAL_BACKEND_COUCHDB (user_data);
	ECalComponentId id;

	id.uid = (gchar *)docid;

	if (g_strcmp0 (dbname, couchdb_backend->dbname) != 0)
		return;

	e_cal_backend_notify_object_removed (E_CAL_BACKEND (couchdb_backend), &id, NULL, NULL);

	/* Remove the task from the cache */
	e_cal_backend_cache_remove_component (couchdb_backend->cache, docid, NULL);
}

static ECalComponent *
put_document (ECalBackendCouchDB *couchdb_backend, CouchdbDocument *document)
{
	GError *error = NULL;

	if (couchdb_document_put (document, couchdb_backend->dbname, &error)) {
		ECalComponent *new_task;

		/* couchdb_document_put sets the ID for new documents, so need to send that back */
		new_task = task_from_couch_document (document);

		/* Add the new task to the cache */
		e_cal_backend_cache_put_component (couchdb_backend->cache, new_task);

		return new_task;

	} else {
		if (error != NULL) {
			g_warning ("Could not PUT document: %s\n", error->message);
			g_error_free (error);
		}
	}

	return NULL;
}


/* Virtual methods */


void 
e_cal_backend_couchdb_is_read_only (ECalBackend *backend, EDataCal *cal)
{
	e_data_cal_notify_read_only (cal, GNOME_Evolution_Calendar_Success, FALSE);
}


void 
e_cal_backend_couchdb_get_cal_address (ECalBackend *backend, EDataCal *cal)
{
	/* FIXME */
	const gchar *address;
	address = NULL;
	e_data_cal_notify_cal_address (cal, GNOME_Evolution_Calendar_Success, address);
}


void 
e_cal_backend_couchdb_get_alarm_email_address (ECalBackend *backend, EDataCal *cal)
{
	/* FIXME */
	const gchar *address;
	address = NULL;
	e_data_cal_notify_alarm_email_address (cal, GNOME_Evolution_Calendar_Success, address);
}

void 
e_cal_backend_couchdb_get_ldap_attribute (ECalBackend *backend, EDataCal *cal)
{
	/* FIXME */
	const gchar *attribute;
	attribute = NULL;
	e_data_cal_notify_ldap_attribute (cal, GNOME_Evolution_Calendar_Success, attribute);
}

void 
e_cal_backend_couchdb_get_static_capabilities (ECalBackend *backend, EDataCal *cal)
{
	/* FIXME */
	const gchar *capabilities;

	capabilities = g_strdup ("local,do-initial-query,bulk-removes");

	e_data_cal_notify_static_capabilities(cal, GNOME_Evolution_Calendar_Success, capabilities);
}

void 
e_cal_backend_couchdb_open (ECalBackend *backend, EDataCal *cal, gboolean only_if_exists, const gchar *username, const gchar *password)
{
	gchar *uri;
	const gchar *property;
	CouchdbDatabaseInfo *db_info;
	GError *error = NULL;
	GSList *doc_list, *sl;

	ECalBackendCouchDB *couchdb_backend = E_CAL_BACKEND_COUCHDB (backend);
	ESource *source;


	source = e_cal_backend_get_source (backend);
	if (!(E_IS_CAL_BACKEND_COUCHDB (couchdb_backend))) {
		e_data_cal_notify_open (cal, GNOME_Evolution_Calendar_OtherError);
	}

	if (couchdb_backend->couchdb != NULL)
		g_object_unref (G_OBJECT (couchdb_backend->couchdb));
	if (couchdb_backend->dbname != NULL)
		g_free (couchdb_backend->dbname);
	if (couchdb_backend->cache != NULL)
		g_object_unref (G_OBJECT (couchdb_backend->cache));

	/* create CouchDB main object */
	couchdb_backend->dbname = g_strdup ("tasks");
	couchdb_backend->using_desktopcouch = FALSE;

	property = e_source_get_property (source, "couchdb_instance");
	if (g_strcmp0 (property, "user") == 0) {
		if (! (couchdb_backend->couchdb = COUCHDB_SESSION (desktopcouch_session_new ()))) {
			g_warning ("Could not create DesktopcouchSession object");
			e_data_cal_notify_open (cal, GNOME_Evolution_Calendar_NoSuchCal);
		}

		couchdb_backend->using_desktopcouch = TRUE;
	} else {
		if (g_strcmp0 (property, "remote") == 0)
			uri = g_strdup_printf ("http://%s", e_source_get_property (source, "couchdb_remote_server"));
		else
			uri = g_strdup ("http://127.0.0.1:5984");

		if (! (couchdb_backend->couchdb = couchdb_session_new (uri))) {
			g_free (uri);
			e_data_cal_notify_open (cal, GNOME_Evolution_Calendar_NoSuchCal);
		}

		g_free (uri);
	}

	/* check if only_if_exists */
	error = NULL;
	db_info = couchdb_session_get_database_info (couchdb_backend->couchdb,
						     couchdb_backend->dbname,
						     &error);
	if (!db_info) {
		if (error) {
			g_warning ("Could not get CouchDB database info: %s", error->message);
			g_error_free (error);
		}

		if (only_if_exists)
			e_data_cal_notify_open (cal, GNOME_Evolution_Calendar_NoSuchCal);
		
		/* if it does not exist, create it */
		error = NULL;
		if (!couchdb_session_create_database (couchdb_backend->couchdb,
						      couchdb_backend->dbname,
						      &error)) {
			g_warning ("Could not create 'tasks' database: %s", error->message);
			g_error_free (error);

			e_data_cal_notify_open (cal, GNOME_Evolution_Calendar_PermissionDenied);
		}
	} else
		couchdb_database_info_unref (db_info);

	/* Create cache */
	couchdb_backend->cache = e_cal_backend_cache_new (e_cal_backend_get_uri (E_CAL_BACKEND (couchdb_backend)), E_CAL_SOURCE_TYPE_TODO);

	/* Populate the cache */
	e_file_cache_clean (E_FILE_CACHE (couchdb_backend->cache));
	error = NULL;
	doc_list = couchdb_session_list_documents (couchdb_backend->couchdb,
						   couchdb_backend->dbname,
						   &error);

	for (sl = doc_list; sl != NULL; sl = sl->next) {
		ECalComponent *task;
		CouchdbDocument *document;
		CouchdbDocumentInfo *doc_info = (CouchdbDocumentInfo *) sl->data;

		/* Retrieve this document */
		error = NULL;
		document = couchdb_document_get (couchdb_backend->couchdb,
						 couchdb_backend->dbname,
						 couchdb_document_info_get_docid (doc_info),
						 &error);
		if (!document)
			continue;

		task = task_from_couch_document (document);
		if (task != NULL) {
			e_cal_backend_cache_put_component (couchdb_backend->cache, task);
			g_object_unref (G_OBJECT (task));
		}

		g_object_unref (G_OBJECT (document));
	}

	couchdb_session_free_document_list (doc_list);

	/* Listen for changes on database */
	g_signal_connect (G_OBJECT (couchdb_backend->couchdb), "document_created",
			  G_CALLBACK (document_updated_cb), couchdb_backend);
	g_signal_connect (G_OBJECT (couchdb_backend->couchdb), "document_updated",
			  G_CALLBACK (document_updated_cb), couchdb_backend);
	g_signal_connect (G_OBJECT (couchdb_backend->couchdb), "document_deleted",
			  G_CALLBACK (document_deleted_cb), couchdb_backend);
	couchdb_session_listen_for_changes (couchdb_backend->couchdb, couchdb_backend->dbname);
	
	e_data_cal_notify_open (cal, GNOME_Evolution_Calendar_Success);
	
}

void 
e_cal_backend_couchdb_remove (ECalBackend *backend, EDataCal *cal)
{
	ECalBackendCouchDB *couchdb_backend = E_CAL_BACKEND_COUCHDB (backend);

	/* Remove the cache */
	if (couchdb_backend->cache != NULL) {

		e_file_cache_remove (E_FILE_CACHE (couchdb_backend->cache));

		g_object_unref (G_OBJECT (couchdb_backend->cache));
		couchdb_backend->cache = NULL;
	}

	/* We don't remove data from CouchDB, since it would affect other apps,
	   so just report success */
	e_data_cal_notify_remove (cal, GNOME_Evolution_Calendar_Success);
}

/* Object related virtual methods */
void 
e_cal_backend_couchdb_create_object (ECalBackend *backend, EDataCal *cal, const gchar *calobj)
{

	ECalComponent *task, *new_task;
	CouchdbDocument *document;
	ECalBackendCouchDB *couchdb_backend = E_CAL_BACKEND_COUCHDB (backend);
	const gchar *uid;

	task = e_cal_component_new_from_string (calobj);
	e_cal_component_get_uid (task, &uid);

 
	if (!task) {
		e_data_cal_notify_object_created (cal, GNOME_Evolution_Calendar_OtherError, uid, NULL);
		return;
	}

	document = couch_document_from_task (couchdb_backend, task);
	if (!document) {
		e_data_cal_notify_object_created (cal, GNOME_Evolution_Calendar_OtherError, uid, NULL);
		g_object_unref (G_OBJECT (task));
		return;
	}

	/* save the task into the DB */
	if ((new_task = put_document (couchdb_backend, document)) != NULL) {
		e_data_cal_notify_object_created (cal, GNOME_Evolution_Calendar_Success, uid, calobj);
		g_object_unref (new_task);
	} else
		e_data_cal_notify_object_created (cal, GNOME_Evolution_Calendar_OtherError, uid, NULL);

	/* free memory */
	g_object_unref (G_OBJECT (task));
	g_object_unref (G_OBJECT (document));
}

void 
e_cal_backend_couchdb_modify_object (ECalBackend *backend, EDataCal *cal, const gchar *calobj, CalObjModType mod)
{
	ECalComponent *task, *new_task;
	CouchdbDocument *document;
	ECalBackendCouchDB *couchdb_backend = E_CAL_BACKEND_COUCHDB (backend);

	task = e_cal_component_new_from_string (calobj);
	if (!task) {
		e_data_cal_notify_object_modified (cal, GNOME_Evolution_Calendar_OtherError, NULL, NULL);
		return;
	}

	document = couch_document_from_task (couchdb_backend, task);
	if (!document) {
		e_data_cal_notify_object_modified (cal, GNOME_Evolution_Calendar_OtherError, NULL, NULL);
		g_object_unref (G_OBJECT (document));
		return;
	}

	/* save the task into the DB */
	if ((new_task = put_document (couchdb_backend, document)) != NULL) {
		e_cal_component_commit_sequence (new_task);
		e_data_cal_notify_object_modified (cal, GNOME_Evolution_Calendar_Success, calobj,  e_cal_component_get_as_string (new_task));
		g_object_unref (new_task);
	} else
		e_data_cal_notify_object_modified (cal, GNOME_Evolution_Calendar_OtherError, NULL, NULL);

	/* free memory */
	g_object_unref (G_OBJECT (task));
	g_object_unref (G_OBJECT (document));
	e_data_cal_notify_object_modified (cal, GNOME_Evolution_Calendar_OtherError, NULL, NULL);
}

void 
e_cal_backend_couchdb_remove_object (ECalBackend *backend, EDataCal *cal, const gchar *uid, const gchar *rid, CalObjModType mod)
{

	ECalBackendCouchDB *couchdb_backend = E_CAL_BACKEND_COUCHDB (backend);
	CouchdbDocument *document;
	GError *error = NULL;
	ECalComponentId id;
	gboolean deleted;

	id.uid = (gchar *)uid;
	id.rid = (gchar *)rid;

	document = couchdb_document_get (couchdb_backend->couchdb, couchdb_backend->dbname, uid, &error);
	if (document) {
		if (couchdb_backend->using_desktopcouch) {
			CouchdbStructField *app_annotations, *u1_annotations, *private_annotations;

			/* For desktopcouch, we don't remove tasks, we just
			* mark them as deleted */
			app_annotations = desktopcouch_document_get_application_annotations (document);
			if (app_annotations == NULL)
				app_annotations = couchdb_struct_field_new ();

			u1_annotations = couchdb_struct_field_get_struct_field (app_annotations, "Ubuntu One");
			if (u1_annotations == NULL)
				u1_annotations = couchdb_struct_field_new ();

			private_annotations = couchdb_struct_field_get_struct_field (u1_annotations, "private_application_annotations");
			if (private_annotations == NULL)
				private_annotations = couchdb_struct_field_new ();

			couchdb_struct_field_set_boolean_field (private_annotations, "deleted", TRUE);
			couchdb_struct_field_set_struct_field (u1_annotations, "private_application_annotations", private_annotations);
			couchdb_struct_field_set_struct_field (app_annotations, "Ubuntu One", u1_annotations);
			desktopcouch_document_set_application_annotations (document, app_annotations);

			/* Now put the new revision of the document */
			if (couchdb_document_put (document, couchdb_backend->dbname, &error)) {
				deleted = TRUE;
				e_cal_backend_cache_remove_component (couchdb_backend->cache, uid, rid);
			} else {
				if (error != NULL) {
					g_warning ("Error deleting document: %s", error->message);
					g_error_free (error);
				} else
					g_warning ("Error deleting document");
			}

			/* Free memory */
			couchdb_struct_field_unref (app_annotations);
			couchdb_struct_field_unref (u1_annotations);
			couchdb_struct_field_unref (private_annotations);
		
		} else {
			
			if (couchdb_document_delete (document, &error)) {
				deleted = TRUE;
				e_cal_backend_cache_remove_component (couchdb_backend->cache, uid, rid);
			} else {
				if (error != NULL) {
					g_warning ("Error deleting document: %s", error->message);
					g_error_free (error);
				} else
					g_warning ("Error deleting document");
			}
		}
	} else {
		if (error != NULL) {
			g_warning ("Error getting document: %s", error->message);
			g_error_free (error);
		} else
			g_warning ("Error getting document");
	}

	if (deleted) {
		e_data_cal_notify_object_removed (cal, GNOME_Evolution_Calendar_Success, &id, NULL, NULL);
	} else {
		e_data_cal_notify_object_removed (cal, GNOME_Evolution_Calendar_OtherError, &id, NULL, NULL);
	}
}

/* Discard_alarm handler for the calendar backend */
void 
e_cal_backend_couchdb_discard_alarm (ECalBackend *backend, EDataCal *cal, const gchar *uid, const gchar *auid)
{
	e_data_cal_notify_alarm_discarded (cal, GNOME_Evolution_Calendar_Success);
}

void 
e_cal_backend_couchdb_receive_objects (ECalBackend *backend, EDataCal *cal, const gchar *calobj)
{
	e_data_cal_notify_objects_received (cal, GNOME_Evolution_Calendar_OtherError);
}

void 
e_cal_backend_couchdb_send_objects (ECalBackend *backend, EDataCal *cal, const gchar *calobj)
{
	e_data_cal_notify_objects_sent (cal, GNOME_Evolution_Calendar_OtherError, NULL, NULL);
}

void 
e_cal_backend_couchdb_get_default_object (ECalBackend *backend, EDataCal *cal)
{

	e_data_cal_notify_default_object (cal, GNOME_Evolution_Calendar_OtherError, NULL);
}

void 
e_cal_backend_couchdb_get_object (ECalBackend *backend, EDataCal *cal, const gchar *uid, const gchar *rid)
{
	GError *error = NULL;
	ECalComponent *task;
	ECalBackendCouchDB *couchdb_backend = E_CAL_BACKEND_COUCHDB (backend);

	task = e_cal_backend_cache_get_component (couchdb_backend->cache, uid, rid);
	if (task != NULL) {
		e_cal_component_commit_sequence (task);
		gchar *task_string = e_cal_component_get_as_string (task);

		g_object_unref (G_OBJECT (task));
		if (task_string != NULL) {
			e_data_cal_notify_object (cal, GNOME_Evolution_Calendar_Success, task_string);
			g_free (task_string);
			return;
		}
	}

	e_data_cal_notify_object (cal, GNOME_Evolution_Calendar_ObjectNotFound, "");
}

void 
e_cal_backend_couchdb_get_object_list (ECalBackend *backend, EDataCal *cal, const gchar *sexp)
{
	GList *doc_list, *tasks = NULL;
	GError *error = NULL;
	ECalBackendCouchDB *couchdb_backend = E_CAL_BACKEND_COUCHDB (backend);

	/* Get the list of documents from cache */
	doc_list = e_cal_backend_cache_get_components (couchdb_backend->cache);

	while (doc_list != NULL) {
		gchar *task_string;
		ECalComponent *task = E_CAL_COMPONENT (doc_list->data);

		 e_cal_component_commit_sequence (task);
		task_string = e_cal_component_get_as_string (task);
		if (!task_string)
			tasks = g_list_prepend (tasks, task_string);

		doc_list = g_list_remove (doc_list, task);
		g_object_unref (G_OBJECT (task));
	}

	e_data_cal_notify_object_list (cal, GNOME_Evolution_Calendar_Success, tasks);
}

void 
e_cal_backend_couchdb_get_attachment_list (ECalBackend *backend, EDataCal *cal, const gchar *uid, const gchar *rid)
{
	e_data_cal_notify_attachment_list (cal, GNOME_Evolution_Calendar_OtherError, NULL);
}

/* Timezone related virtual methods */
void 
e_cal_backend_couchdb_get_timezone (ECalBackend *backend, EDataCal *cal, const gchar *tzid)
{
	e_data_cal_notify_timezone_requested (cal, GNOME_Evolution_Calendar_OtherError, NULL);
}

void 
e_cal_backend_couchdb_add_timezone (ECalBackend *backend, EDataCal *cal, const gchar *object)
{
	e_data_cal_notify_timezone_added (cal, GNOME_Evolution_Calendar_OtherError, NULL);
}

void 
e_cal_backend_couchdb_set_default_zone (ECalBackend *backend, EDataCal *cal, const gchar *tzobj)
{
	icalcomponent *tz_comp;
	ECalBackendCouchDB *couchdb_backend;
	icaltimezone *zone;

	couchdb_backend = (ECalBackendCouchDB *) backend;

	if (!(E_IS_CAL_BACKEND_COUCHDB(couchdb_backend)))
		e_data_cal_notify_default_timezone_set (cal, GNOME_Evolution_Calendar_OtherError);
	if (!(tzobj != NULL))
		e_data_cal_notify_default_timezone_set (cal, GNOME_Evolution_Calendar_OtherError);

	tz_comp = icalparser_parse_string (tzobj);
	if (!tz_comp)
		e_data_cal_notify_default_timezone_set (cal, GNOME_Evolution_Calendar_InvalidObject);

	zone = icaltimezone_new ();
	icaltimezone_set_component (zone, tz_comp);

	if (couchdb_backend->default_zone)
		icaltimezone_free (couchdb_backend->default_zone, 1);

	couchdb_backend->default_zone = zone;
	e_data_cal_notify_default_timezone_set (cal, GNOME_Evolution_Calendar_Success);
}

/*void 
e_cal_backend_couchdb_set_default_timezone (ECalBackend *backend, EDataCal *cal, const gchar *tzid)
{
	e_cal_backend_notify_error (backend, "No implemented");
}*/

static icaltimezone *
e_cal_backend_couchdb_internal_get_default_timezone (ECalBackend *backend)
{	
	ECalBackendCouchDB *couchdb_backend = E_CAL_BACKEND_COUCHDB (backend);
	return couchdb_backend->default_zone;
}

void 
e_cal_backend_couchdb_start_query (ECalBackend *backend, EDataCalView *query)
{
	GList *doc_list;
	ECalBackendCouchDB *couchdb_backend = E_CAL_BACKEND_COUCHDB (backend);
	ECalBackendSExp *sexp;
	ECalComponentText summary;

	e_cal_backend_add_query (backend, query);
	sexp = e_data_cal_view_get_object_sexp (query);

	/* Get the list of documents from cache */
	doc_list = e_cal_backend_cache_get_components (couchdb_backend->cache);

	while (doc_list != NULL) {
		gchar *task_string;
		ECalComponent *task = E_CAL_COMPONENT (doc_list->data);

		e_cal_component_commit_sequence (task);
		task_string = e_cal_component_get_as_string (task);
		if (!task_string)
			continue;

		e_cal_component_get_summary (task, &summary);
		if (summary.value == NULL)
			continue;

		if (e_cal_backend_sexp_match_comp (sexp, task, backend))
			e_data_cal_view_notify_objects_added_1 (query, task_string);

		doc_list = g_list_remove (doc_list, task);
		g_object_unref (G_OBJECT (task));
	}


	e_data_cal_view_notify_done (query, GNOME_Evolution_Calendar_Success);
}

/* Mode relate virtual methods */
CalMode 
e_cal_backend_couchdb_get_mode (ECalBackend *backend)
{
	return CAL_MODE_LOCAL;
}

void 
e_cal_backend_couchdb_set_mode (ECalBackend *backend, CalMode mode)
{
	e_cal_backend_notify_mode (backend, GNOME_Evolution_Calendar_CalListener_MODE_SET, GNOME_Evolution_Calendar_MODE_REMOTE);
}

void 
e_cal_backend_couchdb_get_free_busy (ECalBackend *backend, EDataCal *cal, GList *users, time_t start, time_t end)
{
	e_data_cal_notify_free_busy (cal, GNOME_Evolution_Calendar_OtherError, NULL);
}

void 
e_cal_backend_couchdb_get_changes (ECalBackend *backend, EDataCal *cal, const gchar *change_id)
{
	e_data_cal_notify_changes (cal, GNOME_Evolution_Calendar_OtherError, NULL, NULL, NULL);
}

static void
e_cal_backend_couchdb_dispose (GObject *object)
{

	ECalBackendCouchDB *couchdb_backend;

	couchdb_backend = E_CAL_BACKEND_COUCHDB (object);

	/* Free all memory and resources */
	if (couchdb_backend->couchdb) {
		g_object_unref (G_OBJECT (couchdb_backend->couchdb));
		couchdb_backend->couchdb = NULL;
	}

	if (couchdb_backend->cache != NULL) {
		g_object_unref (G_OBJECT (couchdb_backend->cache));
		couchdb_backend->cache = NULL;
	}

	if (couchdb_backend->dbname) {
		g_free (couchdb_backend->dbname);
		couchdb_backend->dbname = NULL;
	}

}


static void
e_cal_backend_couchdb_class_init (ECalBackendCouchDBClass *klass)
{

	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	ECalBackendClass *parent_class;

	parent_class = E_CAL_BACKEND_CLASS (klass);

	/* Virtual methods */ 
	parent_class->is_read_only            = e_cal_backend_couchdb_is_read_only;
	parent_class->get_cal_address	      = e_cal_backend_couchdb_get_cal_address;
	parent_class->get_alarm_email_address = e_cal_backend_couchdb_get_alarm_email_address;
	parent_class->get_ldap_attribute      = e_cal_backend_couchdb_get_ldap_attribute;
	parent_class->get_static_capabilities = e_cal_backend_couchdb_get_static_capabilities;

	parent_class->open	              = e_cal_backend_couchdb_open;
	parent_class->remove		      = e_cal_backend_couchdb_remove;

	/* Object related virtual methods */
	parent_class->create_object           = e_cal_backend_couchdb_create_object;
	parent_class->modify_object           = e_cal_backend_couchdb_modify_object;
	parent_class->remove_object           = e_cal_backend_couchdb_remove_object;

	parent_class->discard_alarm           = e_cal_backend_couchdb_discard_alarm;

	parent_class->receive_objects         = e_cal_backend_couchdb_receive_objects;
	parent_class->send_objects    	      = e_cal_backend_couchdb_send_objects;

	parent_class->get_default_object      = e_cal_backend_couchdb_get_default_object;
	parent_class->get_object	      = e_cal_backend_couchdb_get_object;
	parent_class->get_object_list         = e_cal_backend_couchdb_get_object_list;

	parent_class->get_attachment_list     = e_cal_backend_couchdb_get_attachment_list;


	/* Timezone related virtual methods */
	parent_class->get_timezone            = e_cal_backend_couchdb_get_timezone;
	parent_class->add_timezone            = e_cal_backend_couchdb_add_timezone;
	parent_class->set_default_zone        = e_cal_backend_couchdb_set_default_zone;
	parent_class->internal_get_default_timezone = e_cal_backend_couchdb_internal_get_default_timezone;

	parent_class->start_query             = e_cal_backend_couchdb_start_query;

	/* Mode relate virtual methods */
	parent_class->get_mode                = e_cal_backend_couchdb_get_mode;
	parent_class->set_mode                = e_cal_backend_couchdb_set_mode;

	parent_class->get_free_busy           = e_cal_backend_couchdb_get_free_busy;
	parent_class->get_changes             = e_cal_backend_couchdb_get_changes;

	object_class->dispose                 = e_cal_backend_couchdb_dispose;
	

}

static void
e_cal_backend_couchdb_init (ECalBackendCouchDB *backend)
{
	backend->couchdb = NULL;
	backend->dbname = NULL;
	backend->cache = NULL;
}
