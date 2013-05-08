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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "config/picmandisplayconfig.h"
#include "config/picmandisplayoptions.h"

#include "core/picman.h"
#include "core/picmanguide.h"
#include "core/picmanimage.h"
#include "core/picmanimage-grid.h"
#include "core/picmanimage-guides.h"
#include "core/picmanimage-quick-mask.h"
#include "core/picmanimage-sample-points.h"
#include "core/picmanitem.h"
#include "core/picmanitemstack.h"
#include "core/picmansamplepoint.h"
#include "core/picmantreehandler.h"

#include "vectors/picmanvectors.h"

#include "file/file-utils.h"

#include "widgets/picmanwidgets-utils.h"

#include "picmancanvasguide.h"
#include "picmancanvaslayerboundary.h"
#include "picmancanvaspath.h"
#include "picmancanvasproxygroup.h"
#include "picmancanvassamplepoint.h"
#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-appearance.h"
#include "picmandisplayshell-callbacks.h"
#include "picmandisplayshell-expose.h"
#include "picmandisplayshell-handlers.h"
#include "picmandisplayshell-icon.h"
#include "picmandisplayshell-scale.h"
#include "picmandisplayshell-scroll.h"
#include "picmandisplayshell-selection.h"
#include "picmandisplayshell-title.h"
#include "picmanimagewindow.h"
#include "picmanstatusbar.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   picman_display_shell_clean_dirty_handler        (PicmanImage        *image,
                                                             PicmanDirtyMask     dirty_mask,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_undo_event_handler         (PicmanImage        *image,
                                                             PicmanUndoEvent     event,
                                                             PicmanUndo         *undo,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_grid_notify_handler        (PicmanGrid         *grid,
                                                             GParamSpec       *pspec,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_name_changed_handler       (PicmanImage        *image,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_selection_invalidate_handler
                                                            (PicmanImage        *image,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_size_changed_detailed_handler
                                                            (PicmanImage        *image,
                                                             gint              previous_origin_x,
                                                             gint              previous_origin_y,
                                                             gint              previous_width,
                                                             gint              previous_height,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_resolution_changed_handler (PicmanImage        *image,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_quick_mask_changed_handler (PicmanImage        *image,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_guide_add_handler          (PicmanImage        *image,
                                                             PicmanGuide        *guide,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_guide_remove_handler       (PicmanImage        *image,
                                                             PicmanGuide        *guide,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_guide_move_handler         (PicmanImage        *image,
                                                             PicmanGuide        *guide,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_sample_point_add_handler   (PicmanImage        *image,
                                                             PicmanSamplePoint  *sample_point,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_sample_point_remove_handler(PicmanImage        *image,
                                                             PicmanSamplePoint  *sample_point,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_sample_point_move_handler  (PicmanImage        *image,
                                                             PicmanSamplePoint  *sample_point,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_invalidate_preview_handler (PicmanImage        *image,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_profile_changed_handler    (PicmanColorManaged *image,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_saved_handler              (PicmanImage        *image,
                                                             const gchar      *uri,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_exported_handler           (PicmanImage        *image,
                                                             const gchar      *uri,
                                                             PicmanDisplayShell *shell);

static void   picman_display_shell_active_vectors_handler     (PicmanImage        *image,
                                                             PicmanDisplayShell *shell);

static void   picman_display_shell_vectors_freeze_handler     (PicmanVectors      *vectors,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_vectors_thaw_handler       (PicmanVectors      *vectors,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_vectors_visible_handler    (PicmanVectors      *vectors,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_vectors_add_handler        (PicmanContainer    *container,
                                                             PicmanVectors      *vectors,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_vectors_remove_handler     (PicmanContainer    *container,
                                                             PicmanVectors      *vectors,
                                                             PicmanDisplayShell *shell);

static void   picman_display_shell_check_notify_handler       (GObject          *config,
                                                             GParamSpec       *param_spec,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_title_notify_handler       (GObject          *config,
                                                             GParamSpec       *param_spec,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_nav_size_notify_handler    (GObject          *config,
                                                             GParamSpec       *param_spec,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_monitor_res_notify_handler (GObject          *config,
                                                             GParamSpec       *param_spec,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_padding_notify_handler     (GObject          *config,
                                                             GParamSpec       *param_spec,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_ants_speed_notify_handler  (GObject          *config,
                                                             GParamSpec       *param_spec,
                                                             PicmanDisplayShell *shell);
static void   picman_display_shell_quality_notify_handler     (GObject          *config,
                                                             GParamSpec       *param_spec,
                                                             PicmanDisplayShell *shell);


/*  public functions  */

void
picman_display_shell_connect (PicmanDisplayShell *shell)
{
  PicmanImage     *image;
  PicmanContainer *vectors;
  GList         *list;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_DISPLAY (shell->display));

  image   = picman_display_get_image (shell->display);
  vectors = picman_image_get_vectors (image);

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_connect (image, "clean",
                    G_CALLBACK (picman_display_shell_clean_dirty_handler),
                    shell);
  g_signal_connect (image, "dirty",
                    G_CALLBACK (picman_display_shell_clean_dirty_handler),
                    shell);
  g_signal_connect (image, "undo-event",
                    G_CALLBACK (picman_display_shell_undo_event_handler),
                    shell);

  g_signal_connect (picman_image_get_grid (image), "notify",
                    G_CALLBACK (picman_display_shell_grid_notify_handler),
                    shell);
  g_object_set (shell->grid, "grid", picman_image_get_grid (image), NULL);

  g_signal_connect (image, "name-changed",
                    G_CALLBACK (picman_display_shell_name_changed_handler),
                    shell);
  g_signal_connect (image, "selection-invalidate",
                    G_CALLBACK (picman_display_shell_selection_invalidate_handler),
                    shell);
  g_signal_connect (image, "size-changed-detailed",
                    G_CALLBACK (picman_display_shell_size_changed_detailed_handler),
                    shell);
  g_signal_connect (image, "resolution-changed",
                    G_CALLBACK (picman_display_shell_resolution_changed_handler),
                    shell);
  g_signal_connect (image, "quick-mask-changed",
                    G_CALLBACK (picman_display_shell_quick_mask_changed_handler),
                    shell);

  g_signal_connect (image, "guide-added",
                    G_CALLBACK (picman_display_shell_guide_add_handler),
                    shell);
  g_signal_connect (image, "guide-removed",
                    G_CALLBACK (picman_display_shell_guide_remove_handler),
                    shell);
  g_signal_connect (image, "guide-moved",
                    G_CALLBACK (picman_display_shell_guide_move_handler),
                    shell);
  for (list = picman_image_get_guides (image);
       list;
       list = g_list_next (list))
    {
      picman_display_shell_guide_add_handler (image, list->data, shell);
    }

  g_signal_connect (image, "sample-point-added",
                    G_CALLBACK (picman_display_shell_sample_point_add_handler),
                    shell);
  g_signal_connect (image, "sample-point-removed",
                    G_CALLBACK (picman_display_shell_sample_point_remove_handler),
                    shell);
  g_signal_connect (image, "sample-point-moved",
                    G_CALLBACK (picman_display_shell_sample_point_move_handler),
                    shell);
  for (list = picman_image_get_sample_points (image);
       list;
       list = g_list_next (list))
    {
      picman_display_shell_sample_point_add_handler (image, list->data, shell);
    }

  g_signal_connect (image, "invalidate-preview",
                    G_CALLBACK (picman_display_shell_invalidate_preview_handler),
                    shell);
  g_signal_connect (image, "profile-changed",
                    G_CALLBACK (picman_display_shell_profile_changed_handler),
                    shell);
  g_signal_connect (image, "saved",
                    G_CALLBACK (picman_display_shell_saved_handler),
                    shell);
  g_signal_connect (image, "exported",
                    G_CALLBACK (picman_display_shell_exported_handler),
                    shell);

  g_signal_connect (image, "active-vectors-changed",
                    G_CALLBACK (picman_display_shell_active_vectors_handler),
                    shell);

  shell->vectors_freeze_handler =
    picman_tree_handler_connect (vectors, "freeze",
                               G_CALLBACK (picman_display_shell_vectors_freeze_handler),
                               shell);
  shell->vectors_thaw_handler =
    picman_tree_handler_connect (vectors, "thaw",
                               G_CALLBACK (picman_display_shell_vectors_thaw_handler),
                               shell);
  shell->vectors_visible_handler =
    picman_tree_handler_connect (vectors, "visibility-changed",
                               G_CALLBACK (picman_display_shell_vectors_visible_handler),
                               shell);

  g_signal_connect (vectors, "add",
                    G_CALLBACK (picman_display_shell_vectors_add_handler),
                    shell);
  g_signal_connect (vectors, "remove",
                    G_CALLBACK (picman_display_shell_vectors_remove_handler),
                    shell);
  for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (vectors));
       list;
       list = g_list_next (list))
    {
      picman_display_shell_vectors_add_handler (vectors, list->data, shell);
    }

  g_signal_connect (shell->display->config,
                    "notify::transparency-size",
                    G_CALLBACK (picman_display_shell_check_notify_handler),
                    shell);
  g_signal_connect (shell->display->config,
                    "notify::transparency-type",
                    G_CALLBACK (picman_display_shell_check_notify_handler),
                    shell);

  g_signal_connect (shell->display->config,
                    "notify::image-title-format",
                    G_CALLBACK (picman_display_shell_title_notify_handler),
                    shell);
  g_signal_connect (shell->display->config,
                    "notify::image-status-format",
                    G_CALLBACK (picman_display_shell_title_notify_handler),
                    shell);
  g_signal_connect (shell->display->config,
                    "notify::navigation-preview-size",
                    G_CALLBACK (picman_display_shell_nav_size_notify_handler),
                    shell);
  g_signal_connect (shell->display->config,
                    "notify::monitor-resolution-from-windowing-system",
                    G_CALLBACK (picman_display_shell_monitor_res_notify_handler),
                    shell);
  g_signal_connect (shell->display->config,
                    "notify::monitor-xresolution",
                    G_CALLBACK (picman_display_shell_monitor_res_notify_handler),
                    shell);
  g_signal_connect (shell->display->config,
                    "notify::monitor-yresolution",
                    G_CALLBACK (picman_display_shell_monitor_res_notify_handler),
                    shell);

  g_signal_connect (shell->display->config->default_view,
                    "notify::padding-mode",
                    G_CALLBACK (picman_display_shell_padding_notify_handler),
                    shell);
  g_signal_connect (shell->display->config->default_view,
                    "notify::padding-color",
                    G_CALLBACK (picman_display_shell_padding_notify_handler),
                    shell);
  g_signal_connect (shell->display->config->default_fullscreen_view,
                    "notify::padding-mode",
                    G_CALLBACK (picman_display_shell_padding_notify_handler),
                    shell);
  g_signal_connect (shell->display->config->default_fullscreen_view,
                    "notify::padding-color",
                    G_CALLBACK (picman_display_shell_padding_notify_handler),
                    shell);

  g_signal_connect (shell->display->config,
                    "notify::marching-ants-speed",
                    G_CALLBACK (picman_display_shell_ants_speed_notify_handler),
                    shell);

  g_signal_connect (shell->display->config,
                    "notify::zoom-quality",
                    G_CALLBACK (picman_display_shell_quality_notify_handler),
                    shell);

  picman_display_shell_invalidate_preview_handler (image, shell);
  picman_display_shell_quick_mask_changed_handler (image, shell);

  picman_canvas_layer_boundary_set_layer (PICMAN_CANVAS_LAYER_BOUNDARY (shell->layer_boundary),
                                        picman_image_get_active_layer (image));
}

void
picman_display_shell_disconnect (PicmanDisplayShell *shell)
{
  PicmanImage     *image;
  PicmanContainer *vectors;
  GList         *list;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_DISPLAY (shell->display));

  image = picman_display_get_image (shell->display);

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  vectors = picman_image_get_vectors (image);

  picman_display_shell_icon_update_stop (shell);

  picman_canvas_layer_boundary_set_layer (PICMAN_CANVAS_LAYER_BOUNDARY (shell->layer_boundary),
                                        NULL);

  g_signal_handlers_disconnect_by_func (shell->display->config,
                                        picman_display_shell_quality_notify_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (shell->display->config,
                                        picman_display_shell_ants_speed_notify_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (shell->display->config->default_fullscreen_view,
                                        picman_display_shell_padding_notify_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (shell->display->config->default_view,
                                        picman_display_shell_padding_notify_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (shell->display->config,
                                        picman_display_shell_monitor_res_notify_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (shell->display->config,
                                        picman_display_shell_nav_size_notify_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (shell->display->config,
                                        picman_display_shell_title_notify_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (shell->display->config,
                                        picman_display_shell_check_notify_handler,
                                        shell);

  g_signal_handlers_disconnect_by_func (vectors,
                                        picman_display_shell_vectors_remove_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (vectors,
                                        picman_display_shell_vectors_add_handler,
                                        shell);

  picman_tree_handler_disconnect (shell->vectors_visible_handler);
  shell->vectors_visible_handler = NULL;

  picman_tree_handler_disconnect (shell->vectors_thaw_handler);
  shell->vectors_thaw_handler = NULL;

  picman_tree_handler_disconnect (shell->vectors_freeze_handler);
  shell->vectors_freeze_handler = NULL;

  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_active_vectors_handler,
                                        shell);

  for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (vectors));
       list;
       list = g_list_next (list))
    {
      picman_canvas_proxy_group_remove_item (PICMAN_CANVAS_PROXY_GROUP (shell->vectors),
                                           list->data);
    }

  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_exported_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_saved_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_profile_changed_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_invalidate_preview_handler,
                                        shell);

  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_guide_add_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_guide_remove_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_guide_move_handler,
                                        shell);
  for (list = picman_image_get_guides (image);
       list;
       list = g_list_next (list))
    {
      picman_canvas_proxy_group_remove_item (PICMAN_CANVAS_PROXY_GROUP (shell->guides),
                                           list->data);
    }

  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_sample_point_add_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_sample_point_remove_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_sample_point_move_handler,
                                        shell);
  for (list = picman_image_get_sample_points (image);
       list;
       list = g_list_next (list))
    {
      picman_canvas_proxy_group_remove_item (PICMAN_CANVAS_PROXY_GROUP (shell->sample_points),
                                           list->data);
    }

  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_quick_mask_changed_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_resolution_changed_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_size_changed_detailed_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_selection_invalidate_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_name_changed_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (picman_image_get_grid (image),
                                        picman_display_shell_grid_notify_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_undo_event_handler,
                                        shell);
  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_shell_clean_dirty_handler,
                                        shell);
}


/*  private functions  */

static void
picman_display_shell_clean_dirty_handler (PicmanImage        *image,
                                        PicmanDirtyMask     dirty_mask,
                                        PicmanDisplayShell *shell)
{
  picman_display_shell_title_update (shell);
}

static void
picman_display_shell_undo_event_handler (PicmanImage        *image,
                                       PicmanUndoEvent     event,
                                       PicmanUndo         *undo,
                                       PicmanDisplayShell *shell)
{
  picman_display_shell_title_update (shell);
}

static void
picman_display_shell_grid_notify_handler (PicmanGrid         *grid,
                                        GParamSpec       *pspec,
                                        PicmanDisplayShell *shell)
{
  g_object_set (shell->grid, "grid", grid, NULL);
}

static void
picman_display_shell_name_changed_handler (PicmanImage        *image,
                                         PicmanDisplayShell *shell)
{
  picman_display_shell_title_update (shell);
}

static void
picman_display_shell_selection_invalidate_handler (PicmanImage        *image,
                                                 PicmanDisplayShell *shell)
{
  picman_display_shell_selection_undraw (shell);
}

static void
picman_display_shell_resolution_changed_handler (PicmanImage        *image,
                                               PicmanDisplayShell *shell)
{
  picman_display_shell_scale_changed (shell);

  if (shell->dot_for_dot)
    {
      if (shell->unit != PICMAN_UNIT_PIXEL)
        {
          picman_display_shell_scale_update_rulers (shell);
        }

      picman_display_shell_scaled (shell);
    }
  else
    {
      /* A resolution change has the same effect as a size change from
       * a display shell point of view. Force a redraw of the display
       * so that we don't get any display garbage.
       */
      picman_display_shell_scale_resize (shell,
                                       shell->display->config->resize_windows_on_resize,
                                       FALSE);
    }
}

static void
picman_display_shell_quick_mask_changed_handler (PicmanImage        *image,
                                               PicmanDisplayShell *shell)
{
  GtkImage *gtk_image;
  gboolean  quick_mask_state;

  gtk_image = GTK_IMAGE (gtk_bin_get_child (GTK_BIN (shell->quick_mask_button)));

  g_signal_handlers_block_by_func (shell->quick_mask_button,
                                   picman_display_shell_quick_mask_toggled,
                                   shell);

  quick_mask_state = picman_image_get_quick_mask_state (image);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (shell->quick_mask_button),
                                quick_mask_state);

  if (quick_mask_state)
    gtk_image_set_from_stock (gtk_image, PICMAN_STOCK_QUICK_MASK_ON,
                              GTK_ICON_SIZE_MENU);
  else
    gtk_image_set_from_stock (gtk_image, PICMAN_STOCK_QUICK_MASK_OFF,
                              GTK_ICON_SIZE_MENU);

  g_signal_handlers_unblock_by_func (shell->quick_mask_button,
                                     picman_display_shell_quick_mask_toggled,
                                     shell);
}

static void
picman_display_shell_guide_add_handler (PicmanImage        *image,
                                      PicmanGuide        *guide,
                                      PicmanDisplayShell *shell)
{
  PicmanCanvasProxyGroup *group = PICMAN_CANVAS_PROXY_GROUP (shell->guides);
  PicmanCanvasItem       *item;

  item = picman_canvas_guide_new (shell,
                                picman_guide_get_orientation (guide),
                                picman_guide_get_position (guide),
                                TRUE);

  picman_canvas_proxy_group_add_item (group, guide, item);
  g_object_unref (item);
}

static void
picman_display_shell_guide_remove_handler (PicmanImage        *image,
                                         PicmanGuide        *guide,
                                         PicmanDisplayShell *shell)
{
  PicmanCanvasProxyGroup *group = PICMAN_CANVAS_PROXY_GROUP (shell->guides);

  picman_canvas_proxy_group_remove_item (group, guide);
}

static void
picman_display_shell_guide_move_handler (PicmanImage        *image,
                                       PicmanGuide        *guide,
                                       PicmanDisplayShell *shell)
{
  PicmanCanvasProxyGroup *group = PICMAN_CANVAS_PROXY_GROUP (shell->guides);
  PicmanCanvasItem       *item;

  item = picman_canvas_proxy_group_get_item (group, guide);

  picman_canvas_guide_set (item,
                         picman_guide_get_orientation (guide),
                         picman_guide_get_position (guide));
}

static void
picman_display_shell_sample_point_add_handler (PicmanImage        *image,
                                             PicmanSamplePoint  *sample_point,
                                             PicmanDisplayShell *shell)
{
  PicmanCanvasProxyGroup *group = PICMAN_CANVAS_PROXY_GROUP (shell->sample_points);
  PicmanCanvasItem       *item;
  GList                *list;
  gint                  i;

  item = picman_canvas_sample_point_new (shell,
                                       sample_point->x,
                                       sample_point->y,
                                       0, TRUE);

  picman_canvas_proxy_group_add_item (group, sample_point, item);
  g_object_unref (item);

  for (list = picman_image_get_sample_points (image), i = 1;
       list;
       list = g_list_next (list), i++)
    {
      PicmanSamplePoint *sample_point = list->data;

      item = picman_canvas_proxy_group_get_item (group, sample_point);

      if (item)
        g_object_set (item,
                      "index", i,
                      NULL);
    }
}

static void
picman_display_shell_sample_point_remove_handler (PicmanImage        *image,
                                                PicmanSamplePoint  *sample_point,
                                                PicmanDisplayShell *shell)
{
  PicmanCanvasProxyGroup *group = PICMAN_CANVAS_PROXY_GROUP (shell->sample_points);
  GList                *list;
  gint                  i;

  picman_canvas_proxy_group_remove_item (group, sample_point);

  for (list = picman_image_get_sample_points (image), i = 1;
       list;
       list = g_list_next (list), i++)
    {
      PicmanSamplePoint *sample_point = list->data;
      PicmanCanvasItem  *item;

      item = picman_canvas_proxy_group_get_item (group, sample_point);

      if (item)
        g_object_set (item,
                      "index", i,
                      NULL);
    }
}

static void
picman_display_shell_sample_point_move_handler (PicmanImage        *image,
                                              PicmanSamplePoint  *sample_point,
                                              PicmanDisplayShell *shell)
{
  PicmanCanvasProxyGroup *group = PICMAN_CANVAS_PROXY_GROUP (shell->sample_points);
  PicmanCanvasItem       *item;

  item = picman_canvas_proxy_group_get_item (group, sample_point);

  picman_canvas_sample_point_set (item, sample_point->x, sample_point->y);
}

static void
picman_display_shell_size_changed_detailed_handler (PicmanImage        *image,
                                                  gint              previous_origin_x,
                                                  gint              previous_origin_y,
                                                  gint              previous_width,
                                                  gint              previous_height,
                                                  PicmanDisplayShell *shell)
{
  if (shell->display->config->resize_windows_on_resize)
    {
      PicmanImageWindow *window = picman_display_shell_get_window (shell);

      if (window && picman_image_window_get_active_shell (window) == shell)
        {
          /* If the window is resized just center the image in it when it
           * has change size
           */
          picman_image_window_shrink_wrap (window, FALSE);
        }
    }
  else
    {
      PicmanImage *image                    = picman_display_get_image (shell->display);
      gint       new_width                = picman_image_get_width  (image);
      gint       new_height               = picman_image_get_height (image);
      gint       scaled_previous_origin_x = SCALEX (shell, previous_origin_x);
      gint       scaled_previous_origin_y = SCALEY (shell, previous_origin_y);
      gboolean   horizontally;
      gboolean   vertically;

      horizontally = (SCALEX (shell, previous_width)  >  shell->disp_width  &&
                      SCALEX (shell, new_width)       <= shell->disp_width);
      vertically   = (SCALEY (shell, previous_height) >  shell->disp_height &&
                      SCALEY (shell, new_height)      <= shell->disp_height);

      picman_display_shell_scroll_set_offset (shell,
                                            shell->offset_x + scaled_previous_origin_x,
                                            shell->offset_y + scaled_previous_origin_y);

      picman_display_shell_scroll_center_image (shell, horizontally, vertically);

      /* The above calls might not lead to a call to
       * picman_display_shell_scroll_clamp_and_update() in all cases we
       * need it to be called, so simply call it explicitly here at
       * the end
       */
      picman_display_shell_scroll_clamp_and_update (shell);

      picman_display_shell_expose_full (shell);
    }
}

static void
picman_display_shell_invalidate_preview_handler (PicmanImage        *image,
                                               PicmanDisplayShell *shell)
{
  picman_display_shell_icon_update (shell);
}

static void
picman_display_shell_profile_changed_handler (PicmanColorManaged *image,
                                            PicmanDisplayShell *shell)
{
  picman_color_managed_profile_changed (PICMAN_COLOR_MANAGED (shell));
}


static void
picman_display_shell_saved_handler (PicmanImage        *image,
                                  const gchar      *uri,
                                  PicmanDisplayShell *shell)
{
  PicmanStatusbar *statusbar = picman_display_shell_get_statusbar (shell);
  gchar         *filename  = file_utils_uri_display_name (uri);

  picman_statusbar_push_temp (statusbar, PICMAN_MESSAGE_INFO,
                            GTK_STOCK_SAVE, _("Image saved to '%s'"),
                            filename);
  g_free (filename);
}

static void
picman_display_shell_exported_handler (PicmanImage        *image,
                                     const gchar      *uri,
                                     PicmanDisplayShell *shell)
{
  PicmanStatusbar *statusbar = picman_display_shell_get_statusbar (shell);
  gchar         *filename  = file_utils_uri_display_name (uri);

  picman_statusbar_push_temp (statusbar, PICMAN_MESSAGE_INFO,
                            GTK_STOCK_SAVE, _("Image exported to '%s'"),
                            filename);
  g_free (filename);
}

static void
picman_display_shell_active_vectors_handler (PicmanImage        *image,
                                           PicmanDisplayShell *shell)
{
  PicmanCanvasProxyGroup *group  = PICMAN_CANVAS_PROXY_GROUP (shell->vectors);
  PicmanVectors          *active = picman_image_get_active_vectors (image);
  GList                *list;

  for (list = picman_image_get_vectors_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanVectors    *vectors = list->data;
      PicmanCanvasItem *item;

      item = picman_canvas_proxy_group_get_item (group, vectors);

      picman_canvas_item_set_highlight (item, vectors == active);
    }
}

static void
picman_display_shell_vectors_freeze_handler (PicmanVectors      *vectors,
                                           PicmanDisplayShell *shell)
{
  /* do nothing */
}

static void
picman_display_shell_vectors_thaw_handler (PicmanVectors      *vectors,
                                         PicmanDisplayShell *shell)
{
  PicmanCanvasProxyGroup *group = PICMAN_CANVAS_PROXY_GROUP (shell->vectors);
  PicmanCanvasItem       *item;

  item = picman_canvas_proxy_group_get_item (group, vectors);

  picman_canvas_path_set (item, picman_vectors_get_bezier (vectors));
}

static void
picman_display_shell_vectors_visible_handler (PicmanVectors      *vectors,
                                            PicmanDisplayShell *shell)
{
  PicmanCanvasProxyGroup *group = PICMAN_CANVAS_PROXY_GROUP (shell->vectors);
  PicmanCanvasItem       *item;

  item = picman_canvas_proxy_group_get_item (group, vectors);

  picman_canvas_item_set_visible (item,
                                picman_item_get_visible (PICMAN_ITEM (vectors)));
}

static void
picman_display_shell_vectors_add_handler (PicmanContainer    *container,
                                        PicmanVectors      *vectors,
                                        PicmanDisplayShell *shell)
{
  PicmanCanvasProxyGroup *group = PICMAN_CANVAS_PROXY_GROUP (shell->vectors);
  PicmanCanvasItem       *item;

  item = picman_canvas_path_new (shell,
                               picman_vectors_get_bezier (vectors),
                               0, 0,
                               FALSE,
                               PICMAN_PATH_STYLE_VECTORS);
  picman_canvas_item_set_visible (item,
                                picman_item_get_visible (PICMAN_ITEM (vectors)));

  picman_canvas_proxy_group_add_item (group, vectors, item);
  g_object_unref (item);
}

static void
picman_display_shell_vectors_remove_handler (PicmanContainer    *container,
                                           PicmanVectors      *vectors,
                                           PicmanDisplayShell *shell)
{
  PicmanCanvasProxyGroup *group = PICMAN_CANVAS_PROXY_GROUP (shell->vectors);

  picman_canvas_proxy_group_remove_item (group, vectors);
}

static void
picman_display_shell_check_notify_handler (GObject          *config,
                                         GParamSpec       *param_spec,
                                         PicmanDisplayShell *shell)
{
  PicmanCanvasPaddingMode padding_mode;
  PicmanRGB               padding_color;

  if (shell->checkerboard)
    {
      cairo_pattern_destroy (shell->checkerboard);
      shell->checkerboard = NULL;
    }

  picman_display_shell_get_padding (shell, &padding_mode, &padding_color);

  switch (padding_mode)
    {
    case PICMAN_CANVAS_PADDING_MODE_LIGHT_CHECK:
    case PICMAN_CANVAS_PADDING_MODE_DARK_CHECK:
      picman_display_shell_set_padding (shell, padding_mode, &padding_color);
      break;

    default:
      break;
    }

  picman_display_shell_expose_full (shell);
}

static void
picman_display_shell_title_notify_handler (GObject          *config,
                                         GParamSpec       *param_spec,
                                         PicmanDisplayShell *shell)
{
  picman_display_shell_title_update (shell);
}

static void
picman_display_shell_nav_size_notify_handler (GObject          *config,
                                            GParamSpec       *param_spec,
                                            PicmanDisplayShell *shell)
{
  if (shell->nav_popup)
    {
      gtk_widget_destroy (shell->nav_popup);
      shell->nav_popup = NULL;
    }
}

static void
picman_display_shell_monitor_res_notify_handler (GObject          *config,
                                               GParamSpec       *param_spec,
                                               PicmanDisplayShell *shell)
{
  if (PICMAN_DISPLAY_CONFIG (config)->monitor_res_from_gdk)
    {
      picman_get_screen_resolution (gtk_widget_get_screen (GTK_WIDGET (shell)),
                                  &shell->monitor_xres,
                                  &shell->monitor_yres);
    }
  else
    {
      shell->monitor_xres = PICMAN_DISPLAY_CONFIG (config)->monitor_xres;
      shell->monitor_yres = PICMAN_DISPLAY_CONFIG (config)->monitor_yres;
    }

  picman_display_shell_scale_changed (shell);

  if (! shell->dot_for_dot)
    {
      picman_display_shell_scroll_clamp_and_update (shell);

      picman_display_shell_scaled (shell);

      picman_display_shell_expose_full (shell);
    }
}

static void
picman_display_shell_padding_notify_handler (GObject          *config,
                                           GParamSpec       *param_spec,
                                           PicmanDisplayShell *shell)
{
  PicmanDisplayConfig     *display_config;
  PicmanImageWindow       *window;
  gboolean               fullscreen;
  PicmanCanvasPaddingMode  padding_mode;
  PicmanRGB                padding_color;

  display_config = shell->display->config;

  window = picman_display_shell_get_window (shell);

  if (window)
    fullscreen = picman_image_window_get_fullscreen (window);
  else
    fullscreen = FALSE;

  /*  if the user did not set the padding mode for this display explicitly  */
  if (! shell->fullscreen_options->padding_mode_set)
    {
      padding_mode  = display_config->default_fullscreen_view->padding_mode;
      padding_color = display_config->default_fullscreen_view->padding_color;

      if (fullscreen)
        {
          picman_display_shell_set_padding (shell, padding_mode, &padding_color);
        }
      else
        {
          shell->fullscreen_options->padding_mode  = padding_mode;
          shell->fullscreen_options->padding_color = padding_color;
        }
    }

  /*  if the user did not set the padding mode for this display explicitly  */
  if (! shell->options->padding_mode_set)
    {
      padding_mode  = display_config->default_view->padding_mode;
      padding_color = display_config->default_view->padding_color;

      if (fullscreen)
        {
          shell->options->padding_mode  = padding_mode;
          shell->options->padding_color = padding_color;
        }
      else
        {
          picman_display_shell_set_padding (shell, padding_mode, &padding_color);
        }
    }
}

static void
picman_display_shell_ants_speed_notify_handler (GObject          *config,
                                              GParamSpec       *param_spec,
                                              PicmanDisplayShell *shell)
{
  picman_display_shell_selection_pause (shell);
  picman_display_shell_selection_resume (shell);
}

static void
picman_display_shell_quality_notify_handler (GObject          *config,
                                           GParamSpec       *param_spec,
                                           PicmanDisplayShell *shell)
{
  picman_display_shell_expose_full (shell);
}
