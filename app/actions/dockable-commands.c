/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "widgets/picmancontainerview.h"
#include "widgets/picmancontainerview-utils.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmandock.h"
#include "widgets/picmandockable.h"
#include "widgets/picmandockbook.h"
#include "widgets/picmandocked.h"
#include "widgets/picmansessioninfo.h"

#include "dockable-commands.h"


static PicmanDockable * dockable_get_current (PicmanDockbook *dockbook);


/*  public functions  */

void
dockable_add_tab_cmd_callback (GtkAction   *action,
                               const gchar *value,
                               gpointer     data)
{
  PicmanDockbook *dockbook = PICMAN_DOCKBOOK (data);

  picman_dockbook_add_from_dialog_factory (dockbook,
                                         value /*identifiers*/,
                                         -1);
}

void
dockable_close_tab_cmd_callback (GtkAction *action,
                                 gpointer   data)
{
  PicmanDockbook *dockbook = PICMAN_DOCKBOOK (data);
  PicmanDockable *dockable = dockable_get_current (dockbook);

  if (dockable)
    {
      g_object_ref (dockable);
      picman_dockbook_remove (dockbook, dockable);
      gtk_widget_destroy (GTK_WIDGET (dockable));
      g_object_unref (dockable);
    }
}

void
dockable_detach_tab_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanDockbook *dockbook = PICMAN_DOCKBOOK (data);
  PicmanDockable *dockable = dockable_get_current (dockbook);

  if (dockable)
    picman_dockable_detach (dockable);
}

void
dockable_lock_tab_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  PicmanDockbook *dockbook = PICMAN_DOCKBOOK (data);
  PicmanDockable *dockable = dockable_get_current (dockbook);

  if (dockable)
    {
      gboolean lock = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

      picman_dockable_set_locked (dockable, lock);
    }
}

void
dockable_toggle_view_cmd_callback (GtkAction *action,
                                   GtkAction *current,
                                   gpointer   data)
{
  PicmanDockbook *dockbook = PICMAN_DOCKBOOK (data);
  PicmanDockable *dockable;
  PicmanViewType  view_type;
  gint          page_num;

  view_type = (PicmanViewType)
    gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action));

  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (dockbook));

  dockable = (PicmanDockable *)
    gtk_notebook_get_nth_page (GTK_NOTEBOOK (dockbook), page_num);

  if (dockable)
    {
      PicmanDialogFactoryEntry *entry;

      picman_dialog_factory_from_widget (GTK_WIDGET (dockable), &entry);

      if (entry)
        {
          gchar *identifier;
          gchar *substring = NULL;

          identifier = g_strdup (entry->identifier);

          substring = strstr (identifier, "grid");

          if (substring && view_type == PICMAN_VIEW_TYPE_GRID)
            {
              g_free (identifier);
              return;
            }

          if (! substring)
            {
              substring = strstr (identifier, "list");

              if (substring && view_type == PICMAN_VIEW_TYPE_LIST)
                {
                  g_free (identifier);
                  return;
                }
            }

          if (substring)
            {
              PicmanContainerView *old_view;
              GtkWidget         *new_dockable;
              PicmanDock          *dock;
              gint               view_size = -1;

              if (view_type == PICMAN_VIEW_TYPE_LIST)
                memcpy (substring, "list", 4);
              else if (view_type == PICMAN_VIEW_TYPE_GRID)
                memcpy (substring, "grid", 4);

              old_view = picman_container_view_get_by_dockable (dockable);

              if (old_view)
                view_size = picman_container_view_get_view_size (old_view, NULL);

              dock         = picman_dockbook_get_dock (dockbook);
              new_dockable = picman_dialog_factory_dockable_new (picman_dock_get_dialog_factory (dock),
                                                               dock,
                                                               identifier,
                                                               view_size);

              if (new_dockable)
                {
                  PicmanDocked *old;
                  PicmanDocked *new;
                  gboolean    show;

                  picman_dockable_set_locked (PICMAN_DOCKABLE (new_dockable),
                                            picman_dockable_is_locked (dockable));

                  old = PICMAN_DOCKED (gtk_bin_get_child (GTK_BIN (dockable)));
                  new = PICMAN_DOCKED (gtk_bin_get_child (GTK_BIN (new_dockable)));

                  show = picman_docked_get_show_button_bar (old);
                  picman_docked_set_show_button_bar (new, show);

                  /*  Maybe picman_dialog_factory_dockable_new() returned
                   *  an already existing singleton dockable, so check
                   *  if it already is attached to a dockbook.
                   */
                  if (! picman_dockable_get_dockbook (PICMAN_DOCKABLE (new_dockable)))
                    {
                      picman_dockbook_add (dockbook, PICMAN_DOCKABLE (new_dockable),
                                         page_num);

                      g_object_ref (dockable);
                      picman_dockbook_remove (dockbook, dockable);
                      gtk_widget_destroy (GTK_WIDGET (dockable));
                      g_object_unref (dockable);

                      gtk_notebook_set_current_page (GTK_NOTEBOOK (dockbook),
                                                     page_num);
                    }
                }
            }

          g_free (identifier);
        }
    }
}

void
dockable_view_size_cmd_callback (GtkAction *action,
                                 GtkAction *current,
                                 gpointer   data)
{
  PicmanDockbook *dockbook = PICMAN_DOCKBOOK (data);
  PicmanDockable *dockable = dockable_get_current (dockbook);
  gint          view_size;

  view_size = gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action));

  if (dockable)
    {
      PicmanContainerView *view = picman_container_view_get_by_dockable (dockable);

      if (view)
        {
          gint old_size;
          gint border_width;

          old_size = picman_container_view_get_view_size (view, &border_width);

          if (old_size != view_size)
            picman_container_view_set_view_size (view, view_size, border_width);
        }
    }
}

void
dockable_tab_style_cmd_callback (GtkAction *action,
                                 GtkAction *current,
                                 gpointer   data)
{
  PicmanDockbook *dockbook = PICMAN_DOCKBOOK (data);
  PicmanDockable *dockable = dockable_get_current (dockbook);
  PicmanTabStyle  tab_style;

  tab_style = (PicmanTabStyle)
    gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action));

  if (dockable && picman_dockable_get_tab_style (dockable) != tab_style)
    {
      GtkWidget *tab_widget;

      picman_dockable_set_tab_style (dockable, tab_style);

      tab_widget = picman_dockbook_create_tab_widget (dockbook, dockable);

      gtk_notebook_set_tab_label (GTK_NOTEBOOK (dockbook),
                                  GTK_WIDGET (dockable),
                                  tab_widget);
    }
}

void
dockable_show_button_bar_cmd_callback (GtkAction *action,
                                       gpointer   data)
{
  PicmanDockbook *dockbook = PICMAN_DOCKBOOK (data);
  PicmanDockable *dockable = dockable_get_current (dockbook);

  if (dockable)
    {
      PicmanDocked *docked;
      gboolean    show;

      docked = PICMAN_DOCKED (gtk_bin_get_child (GTK_BIN (dockable)));
      show   = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

      picman_docked_set_show_button_bar (docked, show);
    }
}

static PicmanDockable *
dockable_get_current (PicmanDockbook *dockbook)
{
  gint page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (dockbook));

  return (PicmanDockable *) gtk_notebook_get_nth_page (GTK_NOTEBOOK (dockbook),
                                                     page_num);
}
