/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * dialogs.c
 * Copyright (C) 2010 Martin Nordholts <martinn@src.gnome.org>
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "config/picmanguiconfig.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanlist.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmandockwindow.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmenufactory.h"
#include "widgets/picmansessioninfo.h"
#include "widgets/picmansessioninfo-aux.h"
#include "widgets/picmansessionmanaged.h"
#include "widgets/picmantoolbox.h"

#include "dialogs.h"
#include "dialogs-constructors.h"

#include "picman-log.h"

#include "picman-intl.h"


PicmanContainer *global_recent_docks = NULL;


#define FOREIGN(id, singleton, remember_size) \
  { id                     /* identifier       */, \
    NULL                   /* name             */, \
    NULL                   /* blurb            */, \
    NULL                   /* stock_id         */, \
    NULL                   /* help_id          */, \
    NULL                   /* new_func         */, \
    dialogs_restore_dialog /* restore_func     */, \
    0                      /* view_size        */, \
    singleton              /* singleton        */, \
    TRUE                   /* session_managed  */, \
    remember_size          /* remember_size    */, \
    FALSE                  /* remember_if_open */, \
    TRUE                   /* hideable         */, \
    FALSE                  /* image_window     */, \
    FALSE                  /* dockable         */}

#define IMAGE_WINDOW(id, singleton, remember_size) \
  { id                     /* identifier       */, \
    NULL                   /* name             */, \
    NULL                   /* blurb            */, \
    NULL                   /* stock_id         */, \
    NULL                   /* help_id          */, \
    NULL                   /* new_func         */, \
    dialogs_restore_window /* restore_func     */, \
    0                      /* view_size        */, \
    singleton              /* singleton        */, \
    TRUE                   /* session_managed  */, \
    remember_size          /* remember_size    */, \
    TRUE                   /* remember_if_open */, \
    FALSE                  /* hideable         */, \
    TRUE                   /* image_window     */, \
    FALSE                  /* dockable         */}

#define TOPLEVEL(id, new_func, singleton, session_managed, remember_size) \
  { id                     /* identifier       */, \
    NULL                   /* name             */, \
    NULL                   /* blurb            */, \
    NULL                   /* stock_id         */, \
    NULL                   /* help_id          */, \
    new_func               /* new_func         */, \
    dialogs_restore_dialog /* restore_func     */, \
    0                      /* view_size        */, \
    singleton              /* singleton        */, \
    session_managed        /* session_managed  */, \
    remember_size          /* remember_size    */, \
    FALSE                  /* remember_if_open */, \
    TRUE                   /* hideable         */, \
    FALSE                  /* image_window     */, \
    FALSE                  /* dockable         */}

#define DOCKABLE(id, name, blurb, stock_id, help_id, new_func, view_size, singleton) \
  { id                     /* identifier       */, \
    name                   /* name             */, \
    blurb                  /* blurb            */, \
    stock_id               /* stock_id         */, \
    help_id                /* help_id          */, \
    new_func               /* new_func         */, \
    NULL                   /* restore_func     */, \
    view_size              /* view_size        */, \
    singleton              /* singleton        */, \
    FALSE                  /* session_managed  */, \
    FALSE                  /* remember_size    */, \
    TRUE                   /* remember_if_open */, \
    TRUE                   /* hideable         */, \
    FALSE                  /* image_window     */, \
    TRUE                   /* dockable         */}

#define DOCK(id, new_func) \
  { id                     /* identifier       */, \
    NULL                   /* name             */, \
    NULL                   /* blurb            */, \
    NULL                   /* stock_id         */, \
    NULL                   /* help_id          */, \
    new_func               /* new_func         */, \
    dialogs_restore_dialog /* restore_func     */, \
    0                      /* view_size        */, \
    FALSE                  /* singleton        */, \
    FALSE                  /* session_managed  */, \
    FALSE                  /* remember_size    */, \
    FALSE                  /* remember_if_open */, \
    TRUE                   /* hideable         */, \
    FALSE                  /* image_window     */, \
    FALSE                  /* dockable         */}

#define DOCK_WINDOW(id, new_func) \
  { id                     /* identifier       */, \
    NULL                   /* name             */, \
    NULL                   /* blurb            */, \
    NULL                   /* stock_id         */, \
    NULL                   /* help_id          */, \
    new_func               /* new_func         */, \
    dialogs_restore_dialog /* restore_func     */, \
    0                      /* view_size        */, \
    FALSE                  /* singleton        */, \
    TRUE                   /* session_managed  */, \
    TRUE                   /* remember_size    */, \
    TRUE                   /* remember_if_open */, \
    TRUE                   /* hideable         */, \
    FALSE                  /* image_window     */, \
    FALSE                  /* dockable         */}

#define LISTGRID(id, name, blurb, stock_id, help_id, view_size) \
  { "picman-"#id"-list"             /* identifier       */,  \
    name                          /* name             */,  \
    blurb                         /* blurb            */,  \
    stock_id                      /* stock_id         */,  \
    help_id                       /* help_id          */,  \
    dialogs_##id##_list_view_new  /* new_func         */,  \
    NULL                          /* restore_func     */,  \
    view_size                     /* view_size        */,  \
    FALSE                         /* singleton        */,  \
    FALSE                         /* session_managed  */,  \
    FALSE                         /* remember_size    */,  \
    TRUE                          /* remember_if_open */,  \
    TRUE                          /* hideable         */,  \
    FALSE                         /* image_window     */,  \
    TRUE                          /* dockable         */}, \
  { "picman-"#id"-grid"             /* identifier       */,  \
    name                          /* name             */,  \
    blurb                         /* blurb            */,  \
    stock_id                      /* stock_id         */,  \
    help_id                       /* help_id          */,  \
    dialogs_##id##_grid_view_new  /* new_func         */,  \
    NULL                          /* restore_func     */,  \
    view_size                     /* view_size        */,  \
    FALSE                         /* singleton        */,  \
    FALSE                         /* session_managed  */,  \
    FALSE                         /* remember_size    */,  \
    TRUE                          /* remember_if_open */,  \
    TRUE                          /* hideable         */,  \
    FALSE                         /* image_window     */,  \
    TRUE                          /* dockable         */}

#define LIST(id, new_func, name, blurb, stock_id, help_id, view_size) \
  { "picman-"#id"-list"                   /* identifier       */, \
    name                                /* name             */, \
    blurb                               /* blurb            */, \
    stock_id                            /* stock_id         */, \
    help_id                             /* help_id          */, \
    dialogs_##new_func##_list_view_new  /* new_func         */, \
    NULL                                /* restore_func     */, \
    view_size                           /* view_size        */, \
    FALSE                               /* singleton        */, \
    FALSE                               /* session_managed  */, \
    FALSE                               /* remember_size    */, \
    TRUE                                /* remember_if_open */, \
    TRUE                                /* hideable         */, \
    FALSE                               /* image_window     */, \
    TRUE                                /* dockable         */}


static GtkWidget * dialogs_restore_dialog (PicmanDialogFactory *factory,
                                           GdkScreen         *screen,
                                           PicmanSessionInfo   *info);
static GtkWidget * dialogs_restore_window (PicmanDialogFactory *factory,
                                           GdkScreen         *screen,
                                           PicmanSessionInfo   *info);


static const PicmanDialogFactoryEntry entries[] =
{
  /*  foreign toplevels without constructor  */
  FOREIGN ("picman-brightness-contrast-tool-dialog", TRUE,  FALSE),
  FOREIGN ("picman-color-balance-tool-dialog",       TRUE,  FALSE),
  FOREIGN ("picman-color-picker-tool-dialog",        TRUE,  TRUE),
  FOREIGN ("picman-colorize-tool-dialog",            TRUE,  FALSE),
  FOREIGN ("picman-crop-tool-dialog",                TRUE,  FALSE),
  FOREIGN ("picman-curves-tool-dialog",              TRUE,  TRUE),
  FOREIGN ("picman-desaturate-tool-dialog",          TRUE,  FALSE),
  FOREIGN ("picman-gegl-tool-dialog",                TRUE,  FALSE),
  FOREIGN ("picman-hue-saturation-tool-dialog",      TRUE,  FALSE),
  FOREIGN ("picman-levels-tool-dialog",              TRUE,  TRUE),
  FOREIGN ("picman-measure-tool-dialog",             TRUE,  FALSE),
  FOREIGN ("picman-operation-tool-dialog",           TRUE,  FALSE),
  FOREIGN ("picman-posterize-tool-dialog",           TRUE,  FALSE),
  FOREIGN ("picman-rotate-tool-dialog",              TRUE,  FALSE),
  FOREIGN ("picman-scale-tool-dialog",               TRUE,  FALSE),
  FOREIGN ("picman-shear-tool-dialog",               TRUE,  FALSE),
  FOREIGN ("picman-text-tool-dialog",                TRUE,  TRUE),
  FOREIGN ("picman-threshold-tool-dialog",           TRUE,  FALSE),
  FOREIGN ("picman-perspective-tool-dialog",         TRUE,  FALSE),
  FOREIGN ("picman-unified-transform-tool-dialog",   TRUE,  FALSE),

  FOREIGN ("picman-toolbox-color-dialog",            TRUE,  FALSE),
  FOREIGN ("picman-gradient-editor-color-dialog",    TRUE,  FALSE),
  FOREIGN ("picman-palette-editor-color-dialog",     TRUE,  FALSE),
  FOREIGN ("picman-colormap-editor-color-dialog",    TRUE,  FALSE),

  FOREIGN ("picman-controller-editor-dialog",        FALSE, TRUE),
  FOREIGN ("picman-controller-action-dialog",        FALSE, TRUE),

  /*  ordinary toplevels  */
  TOPLEVEL ("picman-image-new-dialog",
            dialogs_image_new_new,          FALSE, TRUE, FALSE),
  TOPLEVEL ("picman-file-open-dialog",
            dialogs_file_open_new,          TRUE,  TRUE, TRUE),
  TOPLEVEL ("picman-file-open-location-dialog",
            dialogs_file_open_location_new, FALSE, TRUE, FALSE),
  TOPLEVEL ("picman-file-save-dialog",
            dialogs_file_save_new,          FALSE, TRUE, TRUE),
  TOPLEVEL ("picman-file-export-dialog",
            dialogs_file_export_new,        FALSE, TRUE, TRUE),

  /*  singleton toplevels  */
  TOPLEVEL ("picman-preferences-dialog",
            dialogs_preferences_get,        TRUE, TRUE,  FALSE),
  TOPLEVEL ("picman-input-devices-dialog",
            dialogs_input_devices_get,      TRUE, TRUE,  FALSE),
  TOPLEVEL ("picman-keyboard-shortcuts-dialog",
            dialogs_keyboard_shortcuts_get, TRUE, TRUE,  TRUE),
  TOPLEVEL ("picman-module-dialog",
            dialogs_module_get,             TRUE, TRUE,  TRUE),
  TOPLEVEL ("picman-palette-import-dialog",
            dialogs_palette_import_get,     TRUE, TRUE,  TRUE),
  TOPLEVEL ("picman-tips-dialog",
            dialogs_tips_get,               TRUE, FALSE, FALSE),
  TOPLEVEL ("picman-about-dialog",
            dialogs_about_get,              TRUE, FALSE, FALSE),
  TOPLEVEL ("picman-error-dialog",
            dialogs_error_get,              TRUE, FALSE, FALSE),
  TOPLEVEL ("picman-close-all-dialog",
            dialogs_close_all_get,          TRUE, FALSE, FALSE),
  TOPLEVEL ("picman-quit-dialog",
            dialogs_quit_get,               TRUE, FALSE, FALSE),

  /*  docks  */
  DOCK ("picman-dock",
        dialogs_dock_new),
  DOCK ("picman-toolbox",
        dialogs_toolbox_new),

  /*  dock windows  */
  DOCK_WINDOW ("picman-dock-window",
               dialogs_dock_window_new),
  DOCK_WINDOW ("picman-toolbox-window",
               dialogs_toolbox_dock_window_new),

  /*  singleton dockables  */
  DOCKABLE ("picman-tool-options",
            N_("Tool Options"), NULL, PICMAN_STOCK_TOOL_OPTIONS,
            PICMAN_HELP_TOOL_OPTIONS_DIALOG,
            dialogs_tool_options_new, 0, TRUE),
  DOCKABLE ("picman-device-status",
            N_("Devices"), N_("Device Status"), PICMAN_STOCK_DEVICE_STATUS,
            PICMAN_HELP_DEVICE_STATUS_DIALOG,
            dialogs_device_status_new, 0, TRUE),
  DOCKABLE ("picman-error-console",
            N_("Errors"), N_("Error Console"), PICMAN_STOCK_WARNING,
            PICMAN_HELP_ERRORS_DIALOG,
            dialogs_error_console_new, 0, TRUE),
  DOCKABLE ("picman-cursor-view",
            N_("Pointer"), N_("Pointer Information"), PICMAN_STOCK_CURSOR,
            PICMAN_HELP_POINTER_INFO_DIALOG,
            dialogs_cursor_view_new, 0, TRUE),

  /*  list & grid views  */
  LISTGRID (image, N_("Images"), NULL, PICMAN_STOCK_IMAGES,
            PICMAN_HELP_IMAGE_DIALOG, PICMAN_VIEW_SIZE_MEDIUM),
  LISTGRID (brush, N_("Brushes"), NULL, PICMAN_STOCK_BRUSH,
            PICMAN_HELP_BRUSH_DIALOG, PICMAN_VIEW_SIZE_MEDIUM),
  LISTGRID (pattern, N_("Patterns"), NULL, PICMAN_STOCK_PATTERN,
            PICMAN_HELP_PATTERN_DIALOG, PICMAN_VIEW_SIZE_MEDIUM),
  LISTGRID (gradient, N_("Gradients"), NULL, PICMAN_STOCK_GRADIENT,
            PICMAN_HELP_GRADIENT_DIALOG, PICMAN_VIEW_SIZE_MEDIUM),
  LISTGRID (palette, N_("Palettes"), NULL, PICMAN_STOCK_PALETTE,
            PICMAN_HELP_PALETTE_DIALOG, PICMAN_VIEW_SIZE_MEDIUM),
  LISTGRID (font, N_("Fonts"), NULL, PICMAN_STOCK_FONT,
            PICMAN_HELP_FONT_DIALOG, PICMAN_VIEW_SIZE_MEDIUM),
  LISTGRID (buffer, N_("Buffers"), NULL, PICMAN_STOCK_BUFFER,
            PICMAN_HELP_BUFFER_DIALOG, PICMAN_VIEW_SIZE_MEDIUM),
  LISTGRID (document, N_("History"), N_("Document History"), "document-open-recent",
            PICMAN_HELP_DOCUMENT_DIALOG, PICMAN_VIEW_SIZE_LARGE),
  LISTGRID (template, N_("Templates"), N_("Image Templates"), PICMAN_STOCK_TEMPLATE,
            PICMAN_HELP_TEMPLATE_DIALOG, PICMAN_VIEW_SIZE_SMALL),

  /* Some things do not have grids, so just list */
  LIST (dynamics, dynamics, N_("Paint Dynamics"), NULL, PICMAN_STOCK_DYNAMICS,
        PICMAN_HELP_DYNAMICS_DIALOG, PICMAN_VIEW_SIZE_MEDIUM),
  LIST (tool-preset, tool_preset, N_("Tool Presets"), NULL, PICMAN_STOCK_TOOL_PRESET,
        PICMAN_HELP_TOOL_PRESET_DIALOG, PICMAN_VIEW_SIZE_MEDIUM),

  /*  image related  */
  DOCKABLE ("picman-layer-list",
            N_("Layers"), NULL, PICMAN_STOCK_LAYERS,
            PICMAN_HELP_LAYER_DIALOG,
            dialogs_layer_list_view_new, 0, FALSE),
  DOCKABLE ("picman-channel-list",
            N_("Channels"), NULL, PICMAN_STOCK_CHANNELS,
            PICMAN_HELP_CHANNEL_DIALOG,
            dialogs_channel_list_view_new, 0, FALSE),
  DOCKABLE ("picman-vectors-list",
            N_("Paths"), NULL, PICMAN_STOCK_PATHS,
            PICMAN_HELP_PATH_DIALOG,
            dialogs_vectors_list_view_new, 0, FALSE),
  DOCKABLE ("picman-indexed-palette",
            N_("Colormap"), NULL, PICMAN_STOCK_COLORMAP,
            PICMAN_HELP_INDEXED_PALETTE_DIALOG,
            dialogs_colormap_editor_new, 0, FALSE),
  DOCKABLE ("picman-histogram-editor",
            N_("Histogram"), NULL, PICMAN_STOCK_HISTOGRAM,
            PICMAN_HELP_HISTOGRAM_DIALOG,
            dialogs_histogram_editor_new, 0, FALSE),
  DOCKABLE ("picman-selection-editor",
            N_("Selection"), N_("Selection Editor"), PICMAN_STOCK_SELECTION,
            PICMAN_HELP_SELECTION_DIALOG,
            dialogs_selection_editor_new, 0, FALSE),
  DOCKABLE ("picman-undo-history",
            N_("Undo"), N_("Undo History"), PICMAN_STOCK_UNDO_HISTORY,
            PICMAN_HELP_UNDO_DIALOG,
            dialogs_undo_editor_new, 0, FALSE),
  DOCKABLE ("picman-sample-point-editor",
            N_("Sample Points"), N_("Sample Points"), PICMAN_STOCK_SAMPLE_POINT,
            PICMAN_HELP_SAMPLE_POINT_DIALOG,
            dialogs_sample_point_editor_new, 0, FALSE),

  /*  display related  */
  DOCKABLE ("picman-navigation-view",
            N_("Navigation"), N_("Display Navigation"), PICMAN_STOCK_NAVIGATION,
            PICMAN_HELP_NAVIGATION_DIALOG,
            dialogs_navigation_editor_new, 0, FALSE),

  /*  editors  */
  DOCKABLE ("picman-color-editor",
            N_("FG/BG"), N_("FG/BG Color"), PICMAN_STOCK_DEFAULT_COLORS,
            PICMAN_HELP_COLOR_DIALOG,
            dialogs_color_editor_new, 0, FALSE),

  /*  singleton editors  */
  DOCKABLE ("picman-brush-editor",
            N_("Brush Editor"), NULL, PICMAN_STOCK_BRUSH,
            PICMAN_HELP_BRUSH_EDITOR_DIALOG,
            dialogs_brush_editor_get, 0, TRUE),
  DOCKABLE ("picman-dynamics-editor",
            N_("Paint Dynamics Editor"), NULL, PICMAN_STOCK_DYNAMICS,
            PICMAN_HELP_DYNAMICS_EDITOR_DIALOG,
            dialogs_dynamics_editor_get, 0, TRUE),
  DOCKABLE ("picman-gradient-editor",
            N_("Gradient Editor"), NULL, PICMAN_STOCK_GRADIENT,
            PICMAN_HELP_GRADIENT_EDITOR_DIALOG,
            dialogs_gradient_editor_get, 0, TRUE),
  DOCKABLE ("picman-palette-editor",
            N_("Palette Editor"), NULL, PICMAN_STOCK_PALETTE,
            PICMAN_HELP_PALETTE_EDITOR_DIALOG,
            dialogs_palette_editor_get, 0, TRUE),
  DOCKABLE ("picman-tool-preset-editor",
            N_("Tool Preset Editor"), NULL, PICMAN_STOCK_TOOL_PRESET,
            PICMAN_HELP_TOOL_PRESET_EDITOR_DIALOG,
            dialogs_tool_preset_editor_get, 0, TRUE),

  /*  image windows  */
  IMAGE_WINDOW ("picman-empty-image-window",
                TRUE, TRUE),
  IMAGE_WINDOW ("picman-single-image-window",
                TRUE, TRUE)
};

/**
 * dialogs_restore_dialog:
 * @factory:
 * @screen:
 * @info:
 *
 * Creates a top level widget based on the given session info object
 * in which other widgets later can be be put, typically also restored
 * from the same session info object.
 *
 * Returns:
 **/
static GtkWidget *
dialogs_restore_dialog (PicmanDialogFactory *factory,
                        GdkScreen         *screen,
                        PicmanSessionInfo   *info)
{
  GtkWidget      *dialog;
  PicmanCoreConfig *config = picman_dialog_factory_get_context (factory)->picman->config;

  PICMAN_LOG (DIALOG_FACTORY, "restoring toplevel \"%s\" (info %p)",
            picman_session_info_get_factory_entry (info)->identifier,
            info);

  dialog =
    picman_dialog_factory_dialog_new (factory, screen,
                                    NULL /*ui_manager*/,
                                    picman_session_info_get_factory_entry (info)->identifier,
                                    picman_session_info_get_factory_entry (info)->view_size,
                                    ! PICMAN_GUI_CONFIG (config)->hide_docks);

  g_object_set_data (G_OBJECT (dialog), PICMAN_DIALOG_VISIBILITY_KEY,
                     GINT_TO_POINTER (PICMAN_GUI_CONFIG (config)->hide_docks ?
                                      PICMAN_DIALOG_VISIBILITY_HIDDEN :
                                      PICMAN_DIALOG_VISIBILITY_VISIBLE));

  return dialog;
}

/**
 * dialogs_restore_window:
 * @factory:
 * @screen:
 * @info:
 *
 * "restores" the image window. We don't really restore anything since
 * the image window is created earlier, so we just look for and return
 * the already-created image window.
 *
 * Returns: 
 **/
static GtkWidget *
dialogs_restore_window (PicmanDialogFactory *factory,
                        GdkScreen         *screen,
                        PicmanSessionInfo   *info)
{
  Picman             *picman    = picman_dialog_factory_get_context (factory)->picman;
  PicmanDisplay      *display = PICMAN_DISPLAY (picman_get_empty_display (picman));
  PicmanDisplayShell *shell   = picman_display_get_shell (display);
  GtkWidget        *dialog;

  dialog = GTK_WIDGET (picman_display_shell_get_window (shell));

  return dialog;
}


/*  public functions  */

void
dialogs_init (Picman            *picman,
              PicmanMenuFactory *menu_factory)
{
  PicmanDialogFactory *factory = NULL;
  gint               i       = 0;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory));

  factory = picman_dialog_factory_new ("toplevel",
                                     picman_get_user_context (picman),
                                     menu_factory);
  picman_dialog_factory_set_singleton (factory);

  for (i = 0; i < G_N_ELEMENTS (entries); i++)
    picman_dialog_factory_register_entry (picman_dialog_factory_get_singleton (),
                                        entries[i].identifier,
                                        gettext (entries[i].name),
                                        gettext (entries[i].blurb),
                                        entries[i].stock_id,
                                        entries[i].help_id,
                                        entries[i].new_func,
                                        entries[i].restore_func,
                                        entries[i].view_size,
                                        entries[i].singleton,
                                        entries[i].session_managed,
                                        entries[i].remember_size,
                                        entries[i].remember_if_open,
                                        entries[i].hideable,
                                        entries[i].image_window,
                                        entries[i].dockable);

  global_recent_docks = picman_list_new (PICMAN_TYPE_SESSION_INFO, FALSE);
}

void
dialogs_exit (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (picman_dialog_factory_get_singleton ())
    {
      /* run dispose manually so the factory destroys its dialogs, which
       * might in turn directly or indirectly ref the factory
       */
      g_object_run_dispose (G_OBJECT (picman_dialog_factory_get_singleton ()));

      g_object_unref (picman_dialog_factory_get_singleton ());
      picman_dialog_factory_set_singleton (NULL);
    }

  if (global_recent_docks)
    {
      g_object_unref (global_recent_docks);
      global_recent_docks = NULL;
    }
}

static void
dialogs_ensure_factory_entry_on_recent_dock (PicmanSessionInfo *info)
{
  if (! picman_session_info_get_factory_entry (info))
    {
      PicmanDialogFactoryEntry *entry = NULL;

      /* The recent docks container only contains session infos for
       * dock windows
       */
      entry = picman_dialog_factory_find_entry (picman_dialog_factory_get_singleton (),
                                              "picman-dock-window");

      picman_session_info_set_factory_entry (info, entry);
    }
}

static char *
dialogs_get_dockrc_filename (void)
{
  const gchar *basename;

  basename = g_getenv ("PICMAN_TESTING_DOCKRC_NAME");
  if (! basename)
    basename = "dockrc";

  return picman_personal_rc_file (basename);
}

void
dialogs_load_recent_docks (Picman *picman)
{
  gchar  *filename;
  GError *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  filename = dialogs_get_dockrc_filename ();

  if (picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  if (! picman_config_deserialize_file (PICMAN_CONFIG (global_recent_docks),
                                      filename,
                                      NULL, &error))
    {
      if (error->code != PICMAN_CONFIG_ERROR_OPEN_ENOENT)
        picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);

      g_clear_error (&error);
    }

  /* In PICMAN 2.6 dockrc did not contain the factory entries for the
   * session infos, so set that up manually if needed
   */
  picman_container_foreach (global_recent_docks,
                          (GFunc) dialogs_ensure_factory_entry_on_recent_dock,
                          NULL);

  picman_list_reverse (PICMAN_LIST (global_recent_docks));

  g_free (filename);
}

void
dialogs_save_recent_docks (Picman *picman)
{
  gchar  *filename;
  GError *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  filename = dialogs_get_dockrc_filename ();

  if (picman->be_verbose)
    g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

  if (! picman_config_serialize_to_file (PICMAN_CONFIG (global_recent_docks),
                                       filename,
                                       "recently closed docks",
                                       "end of recently closed docks",
                                       NULL, &error))
    {
      picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
      g_clear_error (&error);
    }

  g_free (filename);
}

GtkWidget *
dialogs_get_toolbox (void)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (picman_dialog_factory_get_singleton ()), NULL);

  for (list = picman_dialog_factory_get_open_dialogs (picman_dialog_factory_get_singleton ());
       list;
       list = g_list_next (list))
    {
      if (PICMAN_IS_DOCK_WINDOW (list->data) &&
          picman_dock_window_has_toolbox (list->data))
        return list->data;
    }

  return NULL;
}
