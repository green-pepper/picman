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

#include "widgets/picmanactiongroup.h"
#include "widgets/picmancontainerview.h"
#include "widgets/picmancontainerview-utils.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmandock.h"
#include "widgets/picmandockable.h"
#include "widgets/picmandockbook.h"
#include "widgets/picmandocked.h"
#include "widgets/picmanhelp-ids.h"

#include "dialogs-actions.h"
#include "dockable-actions.h"
#include "dockable-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry dockable_actions[] =
{
  { "dockable-popup", NULL,
    NC_("dockable-action", "Dialogs Menu"), NULL, NULL, NULL,
    PICMAN_HELP_DOCK },

  { "dockable-menu",              GTK_STOCK_MISSING_IMAGE, "" },
  { "dockable-add-tab-menu",      NULL, NC_("dockable-action",
                                            "_Add Tab")        },
  { "dockable-preview-size-menu", NULL, NC_("dockable-action",
                                            "_Preview Size")   },
  { "dockable-tab-style-menu",    NULL, NC_("dockable-action",
                                            "_Tab Style")      },

  { "dockable-close-tab", GTK_STOCK_CLOSE,
    NC_("dockable-action", "_Close Tab"), "", NULL,
    G_CALLBACK (dockable_close_tab_cmd_callback),
    PICMAN_HELP_DOCK_TAB_CLOSE },

  { "dockable-detach-tab", PICMAN_STOCK_DETACH,
    NC_("dockable-action", "_Detach Tab"), "", NULL,
    G_CALLBACK (dockable_detach_tab_cmd_callback),
    PICMAN_HELP_DOCK_TAB_DETACH }
};

#define VIEW_SIZE(action,label,size) \
  { "dockable-preview-size-" action, NULL, \
    (label), NULL, NULL, \
    (size), \
    PICMAN_HELP_DOCK_PREVIEW_SIZE }
#define TAB_STYLE(action,label,style) \
  { "dockable-tab-style-" action, NULL, \
    (label), NULL, NULL, \
    (style), \
    PICMAN_HELP_DOCK_TAB_STYLE }

static const PicmanRadioActionEntry dockable_view_size_actions[] =
{
  VIEW_SIZE ("tiny",
             NC_("preview-size", "_Tiny"),        PICMAN_VIEW_SIZE_TINY),
  VIEW_SIZE ("extra-small",
             NC_("preview-size", "E_xtra Small"), PICMAN_VIEW_SIZE_EXTRA_SMALL),
  VIEW_SIZE ("small",
             NC_("preview-size", "_Small"),       PICMAN_VIEW_SIZE_SMALL),
  VIEW_SIZE ("medium",
             NC_("preview-size", "_Medium"),      PICMAN_VIEW_SIZE_MEDIUM),
  VIEW_SIZE ("large",
             NC_("preview-size", "_Large"),       PICMAN_VIEW_SIZE_LARGE),
  VIEW_SIZE ("extra-large",
             NC_("preview-size", "Ex_tra Large"), PICMAN_VIEW_SIZE_EXTRA_LARGE),
  VIEW_SIZE ("huge",
             NC_("preview-size", "_Huge"),        PICMAN_VIEW_SIZE_HUGE),
  VIEW_SIZE ("enormous",
             NC_("preview-size", "_Enormous"),    PICMAN_VIEW_SIZE_ENORMOUS),
  VIEW_SIZE ("gigantic",
             NC_("preview-size", "_Gigantic"),    PICMAN_VIEW_SIZE_GIGANTIC)
};

static const PicmanRadioActionEntry dockable_tab_style_actions[] =
{
  TAB_STYLE ("icon",
             NC_("tab-style", "_Icon"),           PICMAN_TAB_STYLE_ICON),
  TAB_STYLE ("preview",
             NC_("tab-style", "Current _Status"), PICMAN_TAB_STYLE_PREVIEW),
  TAB_STYLE ("name",
             NC_("tab-style", "_Text"),           PICMAN_TAB_STYLE_NAME),
  TAB_STYLE ("icon-name",
             NC_("tab-style", "I_con & Text"),    PICMAN_TAB_STYLE_ICON_NAME),
  TAB_STYLE ("preview-name",
             NC_("tab-style", "St_atus & Text"),  PICMAN_TAB_STYLE_PREVIEW_NAME),
  TAB_STYLE ("automatic",
             NC_("tab-style", "Automatic"),       PICMAN_TAB_STYLE_AUTOMATIC)
};

#undef VIEW_SIZE
#undef TAB_STYLE


static const PicmanToggleActionEntry dockable_toggle_actions[] =
{
  { "dockable-lock-tab", NULL,
    NC_("dockable-action", "Loc_k Tab to Dock"), NULL,
    NC_("dockable-action",
        "Protect this tab from being dragged with the mouse pointer"),
    G_CALLBACK (dockable_lock_tab_cmd_callback),
    FALSE,
    PICMAN_HELP_DOCK_TAB_LOCK },

  { "dockable-show-button-bar", NULL,
    NC_("dockable-action", "Show _Button Bar"), NULL, NULL,
    G_CALLBACK (dockable_show_button_bar_cmd_callback),
    TRUE,
    PICMAN_HELP_DOCK_SHOW_BUTTON_BAR }
};

static const PicmanRadioActionEntry dockable_view_type_actions[] =
{
  { "dockable-view-type-list", NULL,
    NC_("dockable-action", "View as _List"), NULL, NULL,
    PICMAN_VIEW_TYPE_LIST,
    PICMAN_HELP_DOCK_VIEW_AS_LIST },

  { "dockable-view-type-grid", NULL,
    NC_("dockable-action", "View as _Grid"), NULL, NULL,
    PICMAN_VIEW_TYPE_GRID,
    PICMAN_HELP_DOCK_VIEW_AS_GRID }
};


void
dockable_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "dockable-action",
                                 dockable_actions,
                                 G_N_ELEMENTS (dockable_actions));

  picman_action_group_add_toggle_actions (group, "dockable-action",
                                        dockable_toggle_actions,
                                        G_N_ELEMENTS (dockable_toggle_actions));

  picman_action_group_add_string_actions (group, "dialogs-action",
                                        dialogs_dockable_actions,
                                        n_dialogs_dockable_actions,
                                        G_CALLBACK (dockable_add_tab_cmd_callback));

  picman_action_group_add_radio_actions (group, "preview-size",
                                       dockable_view_size_actions,
                                       G_N_ELEMENTS (dockable_view_size_actions),
                                       NULL,
                                       PICMAN_VIEW_SIZE_MEDIUM,
                                       G_CALLBACK (dockable_view_size_cmd_callback));

  picman_action_group_add_radio_actions (group, "tab-style",
                                       dockable_tab_style_actions,
                                       G_N_ELEMENTS (dockable_tab_style_actions),
                                       NULL,
                                       PICMAN_TAB_STYLE_AUTOMATIC,
                                       G_CALLBACK (dockable_tab_style_cmd_callback));

  picman_action_group_add_radio_actions (group, "dockable-action",
                                       dockable_view_type_actions,
                                       G_N_ELEMENTS (dockable_view_type_actions),
                                       NULL,
                                       PICMAN_VIEW_TYPE_LIST,
                                       G_CALLBACK (dockable_toggle_view_cmd_callback));
}

void
dockable_actions_update (PicmanActionGroup *group,
                         gpointer         data)
{
  PicmanDockable           *dockable;
  PicmanDockbook           *dockbook;
  PicmanDocked             *docked;
  PicmanDock               *dock;
  PicmanDialogFactoryEntry *entry;
  PicmanContainerView      *view;
  PicmanViewType            view_type           = -1;
  gboolean                list_view_available = FALSE;
  gboolean                grid_view_available = FALSE;
  gboolean                locked              = FALSE;
  PicmanViewSize            view_size           = -1;
  PicmanTabStyle            tab_style           = -1;
  gint                    n_pages             = 0;
  gint                    n_books             = 0;
  PicmanDockedInterface     *docked_iface       = NULL;

  if (PICMAN_IS_DOCKBOOK (data))
    {
      gint page_num;

      dockbook = PICMAN_DOCKBOOK (data);

      page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (dockbook));

      dockable = (PicmanDockable *)
        gtk_notebook_get_nth_page (GTK_NOTEBOOK (dockbook), page_num);
    }
  else if (PICMAN_IS_DOCKABLE (data))
    {
      dockable = PICMAN_DOCKABLE (data);
      dockbook = picman_dockable_get_dockbook (dockable);
    }
  else
    {
      return;
    }

  docked = PICMAN_DOCKED (gtk_bin_get_child (GTK_BIN (dockable)));
  dock   = picman_dockbook_get_dock (dockbook);


  picman_dialog_factory_from_widget (GTK_WIDGET (dockable), &entry);

  if (entry)
    {
      gchar *identifier;
      gchar *substring = NULL;

      identifier = g_strdup (entry->identifier);

      if ((substring = strstr (identifier, "grid")))
        view_type = PICMAN_VIEW_TYPE_GRID;
      else if ((substring = strstr (identifier, "list")))
        view_type = PICMAN_VIEW_TYPE_LIST;

      if (substring)
        {
          memcpy (substring, "list", 4);
          if (picman_dialog_factory_find_entry (picman_dock_get_dialog_factory (dock),
                                              identifier))
            list_view_available = TRUE;

          memcpy (substring, "grid", 4);
          if (picman_dialog_factory_find_entry (picman_dock_get_dialog_factory (dock),
                                              identifier))
            grid_view_available = TRUE;
        }

      g_free (identifier);
    }

  view = picman_container_view_get_by_dockable (dockable);

  if (view)
    view_size = picman_container_view_get_view_size (view, NULL);

  tab_style = picman_dockable_get_tab_style (dockable);

  n_pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (dockbook));
  n_books = g_list_length (picman_dock_get_dockbooks (dock));

#define SET_ACTIVE(action,active) \
        picman_action_group_set_action_active (group, action, (active) != 0)
#define SET_VISIBLE(action,active) \
        picman_action_group_set_action_visible (group, action, (active) != 0)
#define SET_SENSITIVE(action,sensitive) \
        picman_action_group_set_action_sensitive (group, action, (sensitive) != 0)


  locked = picman_dockable_is_locked (dockable);

  SET_SENSITIVE ("dockable-detach-tab", (! locked &&
                                         (n_pages > 1 || n_books > 1)));

  SET_ACTIVE ("dockable-lock-tab", locked);

  SET_VISIBLE ("dockable-preview-size-menu", view_size != -1);

  if (view_size != -1)
    {
      if (view_size >= PICMAN_VIEW_SIZE_GIGANTIC)
        {
          SET_ACTIVE ("dockable-preview-size-gigantic", TRUE);
        }
      else if (view_size >= PICMAN_VIEW_SIZE_ENORMOUS)
        {
          SET_ACTIVE ("dockable-preview-size-enormous", TRUE);
        }
      else if (view_size >= PICMAN_VIEW_SIZE_HUGE)
        {
          SET_ACTIVE ("dockable-preview-size-huge", TRUE);
        }
      else if (view_size >= PICMAN_VIEW_SIZE_EXTRA_LARGE)
        {
          SET_ACTIVE ("dockable-preview-size-extra-large", TRUE);
        }
      else if (view_size >= PICMAN_VIEW_SIZE_LARGE)
        {
          SET_ACTIVE ("dockable-preview-size-large", TRUE);
        }
      else if (view_size >= PICMAN_VIEW_SIZE_MEDIUM)
        {
          SET_ACTIVE ("dockable-preview-size-medium", TRUE);
        }
      else if (view_size >= PICMAN_VIEW_SIZE_SMALL)
        {
          SET_ACTIVE ("dockable-preview-size-small", TRUE);
        }
      else if (view_size >= PICMAN_VIEW_SIZE_EXTRA_SMALL)
        {
          SET_ACTIVE ("dockable-preview-size-extra-small", TRUE);
        }
      else if (view_size >= PICMAN_VIEW_SIZE_TINY)
        {
          SET_ACTIVE ("dockable-preview-size-tiny", TRUE);
        }
    }

  if (tab_style == PICMAN_TAB_STYLE_ICON)
    SET_ACTIVE ("dockable-tab-style-icon", TRUE);
  else if (tab_style == PICMAN_TAB_STYLE_PREVIEW)
    SET_ACTIVE ("dockable-tab-style-preview", TRUE);
  else if (tab_style == PICMAN_TAB_STYLE_NAME)
    SET_ACTIVE ("dockable-tab-style-name", TRUE);
  else if (tab_style == PICMAN_TAB_STYLE_ICON_NAME)
    SET_ACTIVE ("dockable-tab-style-icon-name", TRUE);
  else if (tab_style == PICMAN_TAB_STYLE_PREVIEW_NAME)
    SET_ACTIVE ("dockable-tab-style-preview-name", TRUE);
  else if (tab_style == PICMAN_TAB_STYLE_AUTOMATIC)
    SET_ACTIVE ("dockable-tab-style-automatic", TRUE);

  docked_iface = PICMAN_DOCKED_GET_INTERFACE (docked);
  SET_SENSITIVE ("dockable-tab-style-preview",
                 docked_iface->get_preview);
  SET_SENSITIVE ("dockable-tab-style-preview-name",
                 docked_iface->get_preview);

  SET_VISIBLE ("dockable-view-type-grid", view_type != -1);
  SET_VISIBLE ("dockable-view-type-list", view_type != -1);

  if (view_type != -1)
    {
      if (view_type == PICMAN_VIEW_TYPE_LIST)
        SET_ACTIVE ("dockable-view-type-list", TRUE);
      else
        SET_ACTIVE ("dockable-view-type-grid", TRUE);

      SET_SENSITIVE ("dockable-view-type-grid", grid_view_available);
      SET_SENSITIVE ("dockable-view-type-list", list_view_available);
    }

  SET_VISIBLE ("dockable-show-button-bar", picman_docked_has_button_bar (docked));
  SET_ACTIVE ("dockable-show-button-bar",
              picman_docked_get_show_button_bar (docked));

#undef SET_ACTIVE
#undef SET_VISIBLE
#undef SET_SENSITIVE
}
