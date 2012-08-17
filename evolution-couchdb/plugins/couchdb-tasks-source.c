/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* couchdb-tasks-source.c - CouchDB task backend
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <e-util/e-config.h>
#include <e-util/e-plugin.h>
#include <calendar/gui/e-cal-config.h>
#include <libedataserver/eds-version.h>
#include <libedataserver/e-source.h>
#include <libedataserver/e-source-list.h>
#include <libedataserver/e-url.h>
#include <libedataserver/e-account-list.h>

#define COUCHDB_BASE_URI "couchdb://"

#if EDS_MINOR_VERSION < 27
/* Copied from e-d-s 2.27.x branch, since it doesn't exist in 2.26.x */

ESourceGroup *
e_source_list_peek_group_by_base_uri (ESourceList *list, const gchar *base_uri)
{
        GSList *p;
        gint len;

        g_return_val_if_fail (E_IS_SOURCE_LIST (list), NULL);
        g_return_val_if_fail (base_uri != NULL, NULL);

        len = strlen (base_uri);

        for (p = e_source_list_peek_groups (list); p != NULL; p = p->next) {
                ESourceGroup *group = E_SOURCE_GROUP (p->data);
                const gchar *buri = e_source_group_peek_base_uri (group);

                if (buri && g_ascii_strncasecmp (buri, base_uri, len) == 0)
                        return group;
        }

        return NULL;
}

ESourceGroup *
e_source_list_ensure_group (ESourceList *list, const gchar *name, const gchar *base_uri, gboolean ret_it)
{
        ESourceGroup *group;

        g_return_val_if_fail (E_IS_SOURCE_LIST (list), NULL);
        g_return_val_if_fail (name != NULL, NULL);
        g_return_val_if_fail (base_uri != NULL, NULL);

        group = e_source_list_peek_group_by_base_uri (list, base_uri);
        if (group) {
                e_source_group_set_name (group, name);
                if (ret_it)
                        g_object_ref (group);
                else
                        group = NULL;
        } else {
                group = e_source_group_new (name, base_uri);

                if (!e_source_list_add_group (list, group, -1)) {
                        g_warning ("Could not add source group %s with base uri %s to a source list", name, base_uri);
                        g_object_unref (group);
                        group = NULL;
                } else {
                        /* save it now */
                        e_source_list_sync (list, NULL);

                        if (!ret_it) {
                                g_object_unref (group);
                                group = NULL;
                        }
                }
        }

        return group;
}
#endif

void
ensure_couchdb_tasks_source_group (void)
{
	ESourceList *source_list;

	source_list = e_source_list_new_for_gconf_default("/apps/evolution/tasks/sources");
	if (source_list) {
		e_source_list_ensure_group (source_list, _("CouchDB"), COUCHDB_BASE_URI, FALSE);

		g_object_unref (G_OBJECT (source_list));
	}
}

void
remove_couchdb_tasks_source_group (void)
{
	ESourceList *source_list;

	source_list = e_source_list_new_for_gconf_default("/apps/evolution/tasks/sources");
	if (source_list) {
		ESourceGroup *group;

		group = e_source_list_peek_group_by_base_uri (source_list, COUCHDB_BASE_URI);
		if (group) {
			GSList *sources;

			sources = e_source_group_peek_sources (group);
			if (sources == NULL) {
				e_source_list_remove_group (source_list, group);
				e_source_list_sync (source_list, NULL);
			}
		}

		g_object_unref (G_OBJECT (source_list));
	}
}

typedef struct {
	ESource *source;
	GtkWidget *vbox;
	GtkWidget *user_db_button;
	GtkWidget *system_db_button;
	GtkWidget *remote_db_button;
	GtkWidget *remote_db_entry;
} UIData;

static void
destroy_ui_data (gpointer data)
{
	UIData *ui = data;

	if (ui && ui->vbox)
		gtk_widget_destroy (ui->vbox);

	g_free (ui);
}

static void
on_user_db_toggled (GtkToggleButton *button, gpointer *user_data)
{
	UIData *ui = (UIData *) user_data;

	if (gtk_toggle_button_get_active (button)) {
		e_source_set_property (ui->source, "couchdb_instance", "user");
		gtk_widget_set_sensitive (ui->remote_db_entry, FALSE);
	}
}

static void
on_system_db_toggled (GtkToggleButton *button, gpointer *user_data)
{
	UIData *ui = (UIData *) user_data;

	if (gtk_toggle_button_get_active (button)) {
		e_source_set_property (ui->source, "couchdb_instance", "system");
		gtk_widget_set_sensitive (ui->remote_db_entry, FALSE);
	}
}

static void
on_remote_db_toggled (GtkToggleButton *button, gpointer *user_data)
{
	UIData *ui = (UIData *) user_data;

	if (gtk_toggle_button_get_active (button)) {
		e_source_set_property (ui->source, "couchdb_instance", "remote");
		gtk_widget_set_sensitive (ui->remote_db_entry, TRUE);
	}
}

static void
on_remote_db_changed (GtkEditable *entry, gpointer user_data)
{
	UIData *ui = (UIData *) user_data;

	e_source_set_property (ui->source, "couchdb_remote_server", gtk_entry_get_text (GTK_ENTRY (entry)));
}

GtkWidget *
plugin_couchdb_tasks (EPlugin *epl, EConfigHookItemFactoryData *data)
{
	ESource *source;
        ESourceGroup *group;
	const gchar *base_uri;
	GtkWidget *parent;
	GtkWidget *table, *label, *parent_vbox;
	UIData *ui;
	const gchar *property;
	ECalConfigTargetSource *t = (ECalConfigTargetSource *) data->target;

	source = t->source;
        group  = e_source_peek_group (source);

        base_uri = e_source_group_peek_base_uri (group);

        g_object_set_data (G_OBJECT (epl), "cwidget", NULL);

	if (strcmp (base_uri, COUCHDB_BASE_URI) != 0)
                return NULL;

	/* Build up the UI */
	ui = g_new0 (UIData, 1);
	ui->source = t->source;

	parent = data->parent;
	parent_vbox = gtk_widget_get_ancestor (gtk_widget_get_parent (parent), GTK_TYPE_VBOX);

	ui->vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (parent_vbox), ui->vbox, FALSE, FALSE, 0);

	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), _("<b>Server</b>"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
	gtk_box_pack_start (GTK_BOX (ui->vbox), label, FALSE, FALSE, 0);

	table = gtk_table_new (3, 3, FALSE);
	gtk_box_pack_start (GTK_BOX (ui->vbox), table, TRUE, TRUE, 0);

	label = gtk_label_new ("   ");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	ui->user_db_button = gtk_radio_button_new_with_label (NULL, _("Desktop CouchDB"));
	gtk_table_attach (GTK_TABLE (table), ui->user_db_button, 1, 3, 0, 1,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 3, 3);

	ui->system_db_button = gtk_radio_button_new_with_label (
		gtk_radio_button_get_group (GTK_RADIO_BUTTON (ui->user_db_button)),
		_("System-wide CouchDB"));
	gtk_table_attach (GTK_TABLE (table), ui->system_db_button, 1, 3, 1, 2,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 3, 3);

	ui->remote_db_button = gtk_radio_button_new_with_label (
		gtk_radio_button_get_group (GTK_RADIO_BUTTON (ui->user_db_button)),
		_("Remote CouchDB server"));
	gtk_table_attach (GTK_TABLE (table), ui->remote_db_button, 1, 2, 2, 3,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 3, 3);
	ui->remote_db_entry = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), ui->remote_db_entry, 2, 3, 2, 3,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 3, 3);

	gtk_widget_show_all (ui->vbox);

	/* Set values from the source */
	property = e_source_get_property (ui->source, "couchdb_instance");
	if (g_strcmp0 (property, "system") == 0) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ui->system_db_button), TRUE);
		gtk_widget_set_sensitive (ui->remote_db_entry, FALSE);
	} else if (g_strcmp0 (property, "remote") == 0) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ui->remote_db_button), TRUE);
		gtk_widget_set_sensitive (ui->remote_db_entry, TRUE);
		gtk_entry_set_text (GTK_ENTRY (ui->remote_db_entry),
				    e_source_get_property (ui->source, "couchdb_remote_server"));
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ui->user_db_button), TRUE);
		if (!property)
			e_source_set_property (ui->source, "couchdb_instance", "user");
		gtk_widget_set_sensitive (ui->remote_db_entry, FALSE);
	}

	g_object_set_data_full (G_OBJECT (epl), "cwidget", ui, destroy_ui_data);
	g_signal_connect (ui->vbox, "destroy", G_CALLBACK (gtk_widget_destroyed), &ui->vbox);

	/* Signals */
	g_signal_connect (G_OBJECT (ui->user_db_button), "toggled", G_CALLBACK (on_user_db_toggled), ui);
	g_signal_connect (G_OBJECT (ui->system_db_button), "toggled", G_CALLBACK (on_system_db_toggled), ui);
	g_signal_connect (G_OBJECT (ui->remote_db_button), "toggled", G_CALLBACK (on_remote_db_toggled), ui);
	g_signal_connect (G_OBJECT (ui->remote_db_entry), "changed", G_CALLBACK (on_remote_db_changed), ui);

	return NULL;
}
