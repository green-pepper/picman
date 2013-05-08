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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"

#include "config/picmanguiconfig.h"

#include "widgets/picmanbrusheditor.h"
#include "widgets/picmanbrushfactoryview.h"
#include "widgets/picmanbufferview.h"
#include "widgets/picmanchanneltreeview.h"
#include "widgets/picmancoloreditor.h"
#include "widgets/picmancolormapeditor.h"
#include "widgets/picmandevicestatus.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmandockwindow.h"
#include "widgets/picmandocumentview.h"
#include "widgets/picmandynamicseditor.h"
#include "widgets/picmandynamicsfactoryview.h"
#include "widgets/picmanerrorconsole.h"
#include "widgets/picmanerrordialog.h"
#include "widgets/picmanfontview.h"
#include "widgets/picmangradienteditor.h"
#include "widgets/picmanhistogrameditor.h"
#include "widgets/picmanimageview.h"
#include "widgets/picmanlayertreeview.h"
#include "widgets/picmanmenudock.h"
#include "widgets/picmanpaletteeditor.h"
#include "widgets/picmanpatternfactoryview.h"
#include "widgets/picmansamplepointeditor.h"
#include "widgets/picmanselectioneditor.h"
#include "widgets/picmantemplateview.h"
#include "widgets/picmantoolbox.h"
#include "widgets/picmantooloptionseditor.h"
#include "widgets/picmantoolpresetfactoryview.h"
#include "widgets/picmantoolpreseteditor.h"
#include "widgets/picmanundoeditor.h"
#include "widgets/picmanvectorstreeview.h"

#include "display/picmancursorview.h"
#include "display/picmannavigationeditor.h"

#include "about-dialog.h"
#include "dialogs.h"
#include "dialogs-constructors.h"
#include "file-open-dialog.h"
#include "file-open-location-dialog.h"
#include "file-save-dialog.h"
#include "image-new-dialog.h"
#include "input-devices-dialog.h"
#include "keyboard-shortcuts-dialog.h"
#include "module-dialog.h"
#include "palette-import-dialog.h"
#include "preferences-dialog.h"
#include "quit-dialog.h"
#include "tips-dialog.h"

#include "picman-intl.h"


/**********************/
/*  toplevel dialogs  */
/**********************/

GtkWidget *
dialogs_image_new_new (PicmanDialogFactory *factory,
                       PicmanContext       *context,
                       PicmanUIManager     *ui_manager,
                       gint               view_size)
{
  return image_new_dialog_new (context);
}

GtkWidget *
dialogs_file_open_new (PicmanDialogFactory *factory,
                       PicmanContext       *context,
                       PicmanUIManager     *ui_manager,
                       gint               view_size)
{
  return file_open_dialog_new (context->picman);
}

GtkWidget *
dialogs_file_open_location_new (PicmanDialogFactory *factory,
                                PicmanContext       *context,
                                PicmanUIManager     *ui_manager,
                                gint               view_size)
{
  return file_open_location_dialog_new (context->picman);
}

GtkWidget *
dialogs_file_save_new (PicmanDialogFactory *factory,
                       PicmanContext       *context,
                       PicmanUIManager     *ui_manager,
                       gint               view_size)
{
  return file_save_dialog_new (context->picman, FALSE);
}

GtkWidget *
dialogs_file_export_new (PicmanDialogFactory *factory,
                         PicmanContext       *context,
                         PicmanUIManager     *ui_manager,
                         gint               view_size)
{
  return file_save_dialog_new (context->picman, TRUE);
}

GtkWidget *
dialogs_preferences_get (PicmanDialogFactory *factory,
                         PicmanContext       *context,
                         PicmanUIManager     *ui_manager,
                         gint               view_size)
{
  return preferences_dialog_create (context->picman);
}

GtkWidget *
dialogs_keyboard_shortcuts_get (PicmanDialogFactory *factory,
                                PicmanContext       *context,
                                PicmanUIManager     *ui_manager,
                                gint               view_size)
{
  return keyboard_shortcuts_dialog_new (context->picman);
}

GtkWidget *
dialogs_input_devices_get (PicmanDialogFactory *factory,
                           PicmanContext       *context,
                           PicmanUIManager     *ui_manager,
                           gint               view_size)
{
  return input_devices_dialog_new (context->picman);
}

GtkWidget *
dialogs_module_get (PicmanDialogFactory *factory,
                    PicmanContext       *context,
                    PicmanUIManager     *ui_manager,
                    gint               view_size)
{
  return module_dialog_new (context->picman);
}

GtkWidget *
dialogs_palette_import_get (PicmanDialogFactory *factory,
                            PicmanContext       *context,
                            PicmanUIManager     *ui_manager,
                            gint               view_size)
{
  return palette_import_dialog_new (context);
}

GtkWidget *
dialogs_tips_get (PicmanDialogFactory *factory,
                  PicmanContext       *context,
                  PicmanUIManager     *ui_manager,
                  gint               view_size)
{
  return tips_dialog_create (context->picman);
}

GtkWidget *
dialogs_about_get (PicmanDialogFactory *factory,
                   PicmanContext       *context,
                   PicmanUIManager     *ui_manager,
                   gint               view_size)
{
  return about_dialog_create (context);
}

GtkWidget *
dialogs_error_get (PicmanDialogFactory *factory,
                   PicmanContext       *context,
                   PicmanUIManager     *ui_manager,
                   gint               view_size)
{
  return picman_error_dialog_new (_("PICMAN Message"));
}

GtkWidget *
dialogs_close_all_get (PicmanDialogFactory *factory,
                       PicmanContext       *context,
                       PicmanUIManager     *ui_manager,
                       gint               view_size)
{
  return close_all_dialog_new (context->picman);
}

GtkWidget *
dialogs_quit_get (PicmanDialogFactory *factory,
                  PicmanContext       *context,
                  PicmanUIManager     *ui_manager,
                  gint               view_size)
{
  return quit_dialog_new (context->picman);
}


/***********/
/*  docks  */
/***********/

GtkWidget *
dialogs_toolbox_new (PicmanDialogFactory *factory,
                     PicmanContext       *context,
                     PicmanUIManager     *ui_manager,
                     gint               view_size)
{
  return picman_toolbox_new (factory,
                           context,
                           ui_manager);
}

GtkWidget *
dialogs_toolbox_dock_window_new (PicmanDialogFactory *factory,
                                 PicmanContext       *context,
                                 PicmanUIManager     *ui_manager,
                                 gint               view_size)
{
  static gint  role_serial = 1;
  GtkWidget   *dock;
  gchar       *role;

  role = g_strdup_printf ("picman-toolbox-%d", role_serial++);
  dock = picman_dock_window_new (role,
                               "<Toolbox>",
                               TRUE,
                               factory,
                               context);
  g_free (role);

  return dock;
}

GtkWidget *
dialogs_dock_new (PicmanDialogFactory *factory,
                  PicmanContext       *context,
                  PicmanUIManager     *ui_manager,
                  gint               view_size)
{
  return picman_menu_dock_new ();
}

GtkWidget *
dialogs_dock_window_new (PicmanDialogFactory *factory,
                         PicmanContext       *context,
                         PicmanUIManager     *ui_manager,
                         gint               view_size)
{
  static gint  role_serial = 1;
  GtkWidget   *dock;
  gchar       *role;

  role = g_strdup_printf ("picman-dock-%d", role_serial++);
  dock = picman_dock_window_new (role,
                               "<Dock>",
                               FALSE,
                               factory,
                               context);
  g_free (role);

  return dock;
}


/***************/
/*  dockables  */
/***************/

/*****  singleton dialogs  *****/

GtkWidget *
dialogs_tool_options_new (PicmanDialogFactory *factory,
                          PicmanContext       *context,
                          PicmanUIManager     *ui_manager,
                          gint               view_size)
{
  return picman_tool_options_editor_new (context->picman,
                                       picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_device_status_new (PicmanDialogFactory *factory,
                           PicmanContext       *context,
                           PicmanUIManager     *ui_manager,
                           gint               view_size)
{
  return picman_device_status_new (context->picman);
}

GtkWidget *
dialogs_error_console_new (PicmanDialogFactory *factory,
                           PicmanContext       *context,
                           PicmanUIManager     *ui_manager,
                           gint               view_size)
{
  return picman_error_console_new (context->picman,
                                 picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_cursor_view_new (PicmanDialogFactory *factory,
                         PicmanContext       *context,
                         PicmanUIManager     *ui_manager,
                         gint               view_size)
{
  return picman_cursor_view_new (picman_dialog_factory_get_menu_factory (factory));
}


/*****  list views  *****/

GtkWidget *
dialogs_image_list_view_new (PicmanDialogFactory *factory,
                             PicmanContext       *context,
                             PicmanUIManager     *ui_manager,
                             gint               view_size)
{
  return picman_image_view_new (PICMAN_VIEW_TYPE_LIST,
                              context->picman->images,
                              context,
                              view_size, 1,
                              picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_brush_list_view_new (PicmanDialogFactory *factory,
                             PicmanContext       *context,
                             PicmanUIManager     *ui_manager,
                             gint               view_size)
{
  return picman_brush_factory_view_new (PICMAN_VIEW_TYPE_LIST,
                                      context->picman->brush_factory,
                                      context,
                                      TRUE,
                                      view_size, 1,
                                      picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_dynamics_list_view_new (PicmanDialogFactory *factory,
                                PicmanContext       *context,
                                PicmanUIManager     *ui_manager,
                                gint               view_size)
{
  return picman_dynamics_factory_view_new (PICMAN_VIEW_TYPE_LIST,
                                         context->picman->dynamics_factory,
                                         context,
                                         view_size, 1,
                                         picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_pattern_list_view_new (PicmanDialogFactory *factory,
                               PicmanContext       *context,
                               PicmanUIManager     *ui_manager,
                               gint               view_size)
{
  return picman_pattern_factory_view_new (PICMAN_VIEW_TYPE_LIST,
                                        context->picman->pattern_factory,
                                        context,
                                        view_size, 1,
                                        picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_gradient_list_view_new (PicmanDialogFactory *factory,
                                PicmanContext       *context,
                                PicmanUIManager     *ui_manager,
                                gint               view_size)
{
  return picman_data_factory_view_new (PICMAN_VIEW_TYPE_LIST,
                                     context->picman->gradient_factory,
                                     context,
                                     view_size, 1,
                                     picman_dialog_factory_get_menu_factory (factory),
                                     "<Gradients>",
                                     "/gradients-popup",
                                     "gradients");
}

GtkWidget *
dialogs_palette_list_view_new (PicmanDialogFactory *factory,
                               PicmanContext       *context,
                               PicmanUIManager     *ui_manager,
                               gint               view_size)
{
  return picman_data_factory_view_new (PICMAN_VIEW_TYPE_LIST,
                                     context->picman->palette_factory,
                                     context,
                                     view_size, 1,
                                     picman_dialog_factory_get_menu_factory (factory),
                                     "<Palettes>",
                                     "/palettes-popup",
                                     "palettes");
}

GtkWidget *
dialogs_font_list_view_new (PicmanDialogFactory *factory,
                            PicmanContext       *context,
                            PicmanUIManager     *ui_manager,
                            gint               view_size)
{
  return picman_font_view_new (PICMAN_VIEW_TYPE_LIST,
                             context->picman->fonts,
                             context,
                             view_size, 1,
                             picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_buffer_list_view_new (PicmanDialogFactory *factory,
                              PicmanContext       *context,
                              PicmanUIManager     *ui_manager,
                              gint               view_size)
{
  return picman_buffer_view_new (PICMAN_VIEW_TYPE_LIST,
                               context->picman->named_buffers,
                               context,
                               view_size, 1,
                               picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_tool_preset_list_view_new (PicmanDialogFactory *factory,
                                   PicmanContext       *context,
                                   PicmanUIManager     *ui_manager,
                                   gint               view_size)
{
  return picman_tool_preset_factory_view_new (PICMAN_VIEW_TYPE_LIST,
                                            context->picman->tool_preset_factory,
                                            context,
                                            view_size, 1,
                                            picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_document_list_view_new (PicmanDialogFactory *factory,
                                PicmanContext       *context,
                                PicmanUIManager     *ui_manager,
                                gint               view_size)
{
  return picman_document_view_new (PICMAN_VIEW_TYPE_LIST,
                                 context->picman->documents,
                                 context,
                                 view_size, 0,
                                 picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_template_list_view_new (PicmanDialogFactory *factory,
                                PicmanContext       *context,
                                PicmanUIManager     *ui_manager,
                                gint               view_size)
{
  return picman_template_view_new (PICMAN_VIEW_TYPE_LIST,
                                 context->picman->templates,
                                 context,
                                 view_size, 0,
                                 picman_dialog_factory_get_menu_factory (factory));
}


/*****  grid views  *****/

GtkWidget *
dialogs_image_grid_view_new (PicmanDialogFactory *factory,
                             PicmanContext       *context,
                             PicmanUIManager     *ui_manager,
                             gint               view_size)
{
  return picman_image_view_new (PICMAN_VIEW_TYPE_GRID,
                              context->picman->images,
                              context,
                              view_size, 1,
                              picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_brush_grid_view_new (PicmanDialogFactory *factory,
                             PicmanContext       *context,
                             PicmanUIManager     *ui_manager,
                             gint               view_size)
{
  return picman_brush_factory_view_new (PICMAN_VIEW_TYPE_GRID,
                                      context->picman->brush_factory,
                                      context,
                                      TRUE,
                                      view_size, 1,
                                      picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_pattern_grid_view_new (PicmanDialogFactory *factory,
                               PicmanContext       *context,
                               PicmanUIManager     *ui_manager,
                               gint               view_size)
{
  return picman_pattern_factory_view_new (PICMAN_VIEW_TYPE_GRID,
                                        context->picman->pattern_factory,
                                        context,
                                        view_size, 1,
                                        picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_gradient_grid_view_new (PicmanDialogFactory *factory,
                                PicmanContext       *context,
                                PicmanUIManager     *ui_manager,
                                gint               view_size)
{
  return picman_data_factory_view_new (PICMAN_VIEW_TYPE_GRID,
                                     context->picman->gradient_factory,
                                     context,
                                     view_size, 1,
                                     picman_dialog_factory_get_menu_factory (factory),
                                     "<Gradients>",
                                     "/gradients-popup",
                                     "gradients");
}

GtkWidget *
dialogs_palette_grid_view_new (PicmanDialogFactory *factory,
                               PicmanContext       *context,
                               PicmanUIManager     *ui_manager,
                               gint               view_size)
{
  return picman_data_factory_view_new (PICMAN_VIEW_TYPE_GRID,
                                     context->picman->palette_factory,
                                     context,
                                     view_size, 1,
                                     picman_dialog_factory_get_menu_factory (factory),
                                     "<Palettes>",
                                     "/palettes-popup",
                                     "palettes");
}

GtkWidget *
dialogs_font_grid_view_new (PicmanDialogFactory *factory,
                            PicmanContext       *context,
                            PicmanUIManager     *ui_manager,
                            gint               view_size)
{
  return picman_font_view_new (PICMAN_VIEW_TYPE_GRID,
                             context->picman->fonts,
                             context,
                             view_size, 1,
                             picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_buffer_grid_view_new (PicmanDialogFactory *factory,
                              PicmanContext       *context,
                              PicmanUIManager     *ui_manager,
                              gint               view_size)
{
  return picman_buffer_view_new (PICMAN_VIEW_TYPE_GRID,
                               context->picman->named_buffers,
                               context,
                               view_size, 1,
                               picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_document_grid_view_new (PicmanDialogFactory *factory,
                                PicmanContext       *context,
                                PicmanUIManager     *ui_manager,
                                gint               view_size)
{
  return picman_document_view_new (PICMAN_VIEW_TYPE_GRID,
                                 context->picman->documents,
                                 context,
                                 view_size, 0,
                                 picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_template_grid_view_new (PicmanDialogFactory *factory,
                                PicmanContext       *context,
                                PicmanUIManager     *ui_manager,
                                gint               view_size)
{
  return picman_template_view_new (PICMAN_VIEW_TYPE_GRID,
                                 context->picman->templates,
                                 context,
                                 view_size, 0,
                                 picman_dialog_factory_get_menu_factory (factory));
}


/*****  image related dialogs  *****/

GtkWidget *
dialogs_layer_list_view_new (PicmanDialogFactory *factory,
                             PicmanContext       *context,
                             PicmanUIManager     *ui_manager,
                             gint               view_size)
{
  if (view_size < 1)
    view_size = context->picman->config->layer_preview_size;

  return picman_item_tree_view_new (PICMAN_TYPE_LAYER_TREE_VIEW,
                                  view_size, 2,
                                  picman_context_get_image (context),
                                  picman_dialog_factory_get_menu_factory (factory),
                                  "<Layers>",
                                  "/layers-popup");
}

GtkWidget *
dialogs_channel_list_view_new (PicmanDialogFactory *factory,
                               PicmanContext       *context,
                               PicmanUIManager     *ui_manager,
                               gint               view_size)
{
  if (view_size < 1)
    view_size = context->picman->config->layer_preview_size;

  return picman_item_tree_view_new (PICMAN_TYPE_CHANNEL_TREE_VIEW,
                                  view_size, 1,
                                  picman_context_get_image (context),
                                  picman_dialog_factory_get_menu_factory (factory),
                                  "<Channels>",
                                  "/channels-popup");
}

GtkWidget *
dialogs_vectors_list_view_new (PicmanDialogFactory *factory,
                               PicmanContext       *context,
                               PicmanUIManager     *ui_manager,
                               gint               view_size)
{
  if (view_size < 1)
    view_size = context->picman->config->layer_preview_size;

  return picman_item_tree_view_new (PICMAN_TYPE_VECTORS_TREE_VIEW,
                                  view_size, 1,
                                  picman_context_get_image (context),
                                  picman_dialog_factory_get_menu_factory (factory),
                                  "<Vectors>",
                                  "/vectors-popup");
}

GtkWidget *
dialogs_colormap_editor_new (PicmanDialogFactory *factory,
                             PicmanContext       *context,
                             PicmanUIManager     *ui_manager,
                             gint               view_size)
{
  return picman_colormap_editor_new (picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_histogram_editor_new (PicmanDialogFactory *factory,
                              PicmanContext       *context,
                              PicmanUIManager     *ui_manager,
                              gint               view_size)
{
  return picman_histogram_editor_new ();
}

GtkWidget *
dialogs_selection_editor_new (PicmanDialogFactory *factory,
                              PicmanContext       *context,
                              PicmanUIManager     *ui_manager,
                              gint               view_size)
{
  return picman_selection_editor_new (picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_undo_editor_new (PicmanDialogFactory *factory,
                         PicmanContext       *context,
                         PicmanUIManager     *ui_manager,
                         gint               view_size)
{
  return picman_undo_editor_new (context->picman->config,
                               picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_sample_point_editor_new (PicmanDialogFactory *factory,
                                 PicmanContext       *context,
                                 PicmanUIManager     *ui_manager,
                                 gint               view_size)
{
  return picman_sample_point_editor_new (picman_dialog_factory_get_menu_factory (factory));
}


/*****  display related dialogs  *****/

GtkWidget *
dialogs_navigation_editor_new (PicmanDialogFactory *factory,
                               PicmanContext       *context,
                               PicmanUIManager     *ui_manager,
                               gint               view_size)
{
  return picman_navigation_editor_new (picman_dialog_factory_get_menu_factory (factory));
}


/*****  misc dockables  *****/

GtkWidget *
dialogs_color_editor_new (PicmanDialogFactory *factory,
                          PicmanContext       *context,
                          PicmanUIManager     *ui_manager,
                          gint               view_size)
{
  return picman_color_editor_new (context);
}


/*************/
/*  editors  */
/*************/

GtkWidget *
dialogs_brush_editor_get (PicmanDialogFactory *factory,
                          PicmanContext       *context,
                          PicmanUIManager     *ui_manager,
                          gint               view_size)
{
  return picman_brush_editor_new (context,
                                picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_dynamics_editor_get (PicmanDialogFactory *factory,
                             PicmanContext       *context,
                             PicmanUIManager     *ui_manager,
                             gint               view_size)
{
  return picman_dynamics_editor_new (context,
                                   picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_gradient_editor_get (PicmanDialogFactory *factory,
                             PicmanContext       *context,
                             PicmanUIManager     *ui_manager,
                             gint               view_size)
{
  return picman_gradient_editor_new (context,
                                   picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_palette_editor_get (PicmanDialogFactory *factory,
                            PicmanContext       *context,
                            PicmanUIManager     *ui_manager,
                            gint               view_size)
{
  return picman_palette_editor_new (context,
                                  picman_dialog_factory_get_menu_factory (factory));
}

GtkWidget *
dialogs_tool_preset_editor_get (PicmanDialogFactory *factory,
                                PicmanContext       *context,
                                PicmanUIManager     *ui_manager,
                                gint               view_size)
{
  return picman_tool_preset_editor_new (context,
                                      picman_dialog_factory_get_menu_factory (factory));
}
