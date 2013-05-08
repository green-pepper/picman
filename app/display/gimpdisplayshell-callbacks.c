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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "core/picman.h"
#include "core/picmanimage.h"
#include "core/picmanimage-quick-mask.h"

#include "widgets/picmancairo-wilber.h"
#include "widgets/picmanuimanager.h"

#include "picmancanvasitem.h"
#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-appearance.h"
#include "picmandisplayshell-callbacks.h"
#include "picmandisplayshell-draw.h"
#include "picmandisplayshell-scale.h"
#include "picmandisplayshell-scroll.h"
#include "picmandisplayshell-selection.h"
#include "picmandisplayshell-title.h"
#include "picmandisplayxfer.h"
#include "picmanimagewindow.h"
#include "picmannavigationeditor.h"


/*  local function prototypes  */

static void       picman_display_shell_vadjustment_changed      (GtkAdjustment    *adjustment,
                                                               PicmanDisplayShell *shell);
static void       picman_display_shell_hadjustment_changed      (GtkAdjustment    *adjustment,
                                                               PicmanDisplayShell *shell);
static gboolean   picman_display_shell_vscrollbar_change_value  (GtkRange         *range,
                                                               GtkScrollType     scroll,
                                                               gdouble           value,
                                                               PicmanDisplayShell *shell);

static gboolean   picman_display_shell_hscrollbar_change_value  (GtkRange         *range,
                                                               GtkScrollType     scroll,
                                                               gdouble           value,
                                                               PicmanDisplayShell *shell);

static void       picman_display_shell_canvas_draw_image        (PicmanDisplayShell *shell,
                                                               cairo_t          *cr);
static void       picman_display_shell_canvas_draw_drop_zone    (PicmanDisplayShell *shell,
                                                               cairo_t          *cr);


/*  public functions  */

void
picman_display_shell_canvas_realize (GtkWidget        *canvas,
                                   PicmanDisplayShell *shell)
{
  PicmanCanvasPaddingMode padding_mode;
  PicmanRGB               padding_color;
  GtkAllocation         allocation;

  gtk_widget_grab_focus (canvas);

  picman_display_shell_get_padding (shell, &padding_mode, &padding_color);
  picman_display_shell_set_padding (shell, padding_mode, &padding_color);

  gtk_widget_get_allocation (canvas, &allocation);

  picman_display_shell_title_update (shell);

  shell->disp_width  = allocation.width;
  shell->disp_height = allocation.height;

  /*  set up the scrollbar observers  */
  g_signal_connect (shell->hsbdata, "value-changed",
                    G_CALLBACK (picman_display_shell_hadjustment_changed),
                    shell);
  g_signal_connect (shell->vsbdata, "value-changed",
                    G_CALLBACK (picman_display_shell_vadjustment_changed),
                    shell);

  g_signal_connect (shell->hsb, "change-value",
                    G_CALLBACK (picman_display_shell_hscrollbar_change_value),
                    shell);

  g_signal_connect (shell->vsb, "change-value",
                    G_CALLBACK (picman_display_shell_vscrollbar_change_value),
                    shell);

  /*  allow shrinking  */
  gtk_widget_set_size_request (GTK_WIDGET (shell), 0, 0);

  shell->xfer = picman_display_xfer_realize (GTK_WIDGET(shell));
}

void
picman_display_shell_canvas_size_allocate (GtkWidget        *widget,
                                         GtkAllocation    *allocation,
                                         PicmanDisplayShell *shell)
{
  /*  are we in destruction?  */
  if (! shell->display || ! picman_display_get_shell (shell->display))
    return;

  if ((shell->disp_width  != allocation->width) ||
      (shell->disp_height != allocation->height))
    {
      if (shell->zoom_on_resize   &&
          shell->disp_width  > 64 &&
          shell->disp_height > 64 &&
          allocation->width  > 64 &&
          allocation->height > 64)
        {
          gdouble scale = picman_zoom_model_get_factor (shell->zoom);
          gint    offset_x;
          gint    offset_y;

          /* FIXME: The code is a bit of a mess */

          /*  multiply the zoom_factor with the ratio of the new and
           *  old canvas diagonals
           */
          scale *= (sqrt (SQR (allocation->width) +
                          SQR (allocation->height)) /
                    sqrt (SQR (shell->disp_width) +
                          SQR (shell->disp_height)));

          offset_x = UNSCALEX (shell, shell->offset_x);
          offset_y = UNSCALEX (shell, shell->offset_y);

          picman_zoom_model_zoom (shell->zoom, PICMAN_ZOOM_TO, scale);

          shell->offset_x = SCALEX (shell, offset_x);
          shell->offset_y = SCALEY (shell, offset_y);
        }

      shell->disp_width  = allocation->width;
      shell->disp_height = allocation->height;

      /* When we size-allocate due to resize of the top level window,
       * we want some additional logic. Don't apply it on
       * zoom_on_resize though.
       */
      if (shell->size_allocate_from_configure_event &&
          ! shell->zoom_on_resize)
        {
          gboolean center_horizontally;
          gboolean center_vertically;
          gint     target_offset_x;
          gint     target_offset_y;
          gint     sw;
          gint     sh;

          picman_display_shell_scale_get_image_size (shell, &sw, &sh);

          center_horizontally = sw <= shell->disp_width;
          center_vertically   = sh <= shell->disp_height;

          picman_display_shell_scroll_center_image (shell,
                                                  center_horizontally,
                                                  center_vertically);

          /* This is basically the best we can do before we get an
           * API for storing the image offset at the start of an
           * image window resize using the mouse
           */
          target_offset_x = shell->offset_x;
          target_offset_y = shell->offset_y;

          if (! center_horizontally)
            {
              target_offset_x = MAX (shell->offset_x, 0);
            }

          if (! center_vertically)
            {
              target_offset_y = MAX (shell->offset_y, 0);
            }

          picman_display_shell_scroll_set_offset (shell,
                                                target_offset_x,
                                                target_offset_y);
        }

      picman_display_shell_scroll_clamp_and_update (shell);
      picman_display_shell_scaled (shell);

      /* Reset */
      shell->size_allocate_from_configure_event = FALSE;
    }
}

gboolean
picman_display_shell_canvas_expose (GtkWidget        *widget,
                                  GdkEventExpose   *eevent,
                                  PicmanDisplayShell *shell)
{
  /*  are we in destruction?  */
  if (! shell->display || ! picman_display_get_shell (shell->display))
    return TRUE;

  /*  ignore events on overlays  */
  if (eevent->window == gtk_widget_get_window (widget))
    {
      cairo_t *cr;

      cr = gdk_cairo_create (gtk_widget_get_window (shell->canvas));
      gdk_cairo_region (cr, eevent->region);
      cairo_clip (cr);

      if (picman_display_get_image (shell->display))
        {
          picman_display_shell_canvas_draw_image (shell, cr);
        }
      else
        {
          picman_display_shell_canvas_draw_drop_zone (shell, cr);
        }

      cairo_destroy (cr);
    }

  return FALSE;
}

gboolean
picman_display_shell_origin_button_press (GtkWidget        *widget,
                                        GdkEventButton   *event,
                                        PicmanDisplayShell *shell)
{
  if (! shell->display->picman->busy)
    {
      if (event->type == GDK_BUTTON_PRESS && event->button == 1)
        {
          gboolean unused;

          g_signal_emit_by_name (shell, "popup-menu", &unused);
        }
    }

  /* Return TRUE to stop signal emission so the button doesn't grab the
   * pointer away from us.
   */
  return TRUE;
}

gboolean
picman_display_shell_quick_mask_button_press (GtkWidget        *widget,
                                            GdkEventButton   *bevent,
                                            PicmanDisplayShell *shell)
{
  if (! picman_display_get_image (shell->display))
    return TRUE;

  if (gdk_event_triggers_context_menu ((GdkEvent *) bevent))
    {
      PicmanImageWindow *window = picman_display_shell_get_window (shell);

      if (window)
        {
          PicmanUIManager *manager = picman_image_window_get_ui_manager (window);

          picman_ui_manager_ui_popup (manager,
                                    "/quick-mask-popup",
                                    GTK_WIDGET (shell),
                                    NULL, NULL, NULL, NULL);
        }

      return TRUE;
    }

  return FALSE;
}

void
picman_display_shell_quick_mask_toggled (GtkWidget        *widget,
                                       PicmanDisplayShell *shell)
{
  PicmanImage *image  = picman_display_get_image (shell->display);
  gboolean   active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

  if (active != picman_image_get_quick_mask_state (image))
    {
      picman_image_set_quick_mask_state (image, active);

      picman_image_flush (image);
    }
}

gboolean
picman_display_shell_navigation_button_press (GtkWidget        *widget,
                                            GdkEventButton   *bevent,
                                            PicmanDisplayShell *shell)
{
  if (! picman_display_get_image (shell->display))
    return TRUE;

  if (bevent->type == GDK_BUTTON_PRESS && bevent->button == 1)
    {
      picman_navigation_editor_popup (shell, widget, bevent->x, bevent->y);
    }

  return TRUE;
}


/*  private functions  */

static void
picman_display_shell_vadjustment_changed (GtkAdjustment    *adjustment,
                                        PicmanDisplayShell *shell)
{
  /*  If we are panning with mouse, scrollbars are to be ignored or
   *  they will cause jitter in motion
   */
  if (! shell->scrolling)
    picman_display_shell_scroll (shell,
                               0,
                               gtk_adjustment_get_value (adjustment) -
                               shell->offset_y);
}

static void
picman_display_shell_hadjustment_changed (GtkAdjustment    *adjustment,
                                        PicmanDisplayShell *shell)
{
  /* If we are panning with mouse, scrollbars are to be ignored or
   * they will cause jitter in motion
   */
  if (! shell->scrolling)
    picman_display_shell_scroll (shell,
                               gtk_adjustment_get_value (adjustment) -
                               shell->offset_x,
                               0);
}

static gboolean
picman_display_shell_hscrollbar_change_value (GtkRange         *range,
                                            GtkScrollType     scroll,
                                            gdouble           value,
                                            PicmanDisplayShell *shell)
{
  if (! shell->display)
    return TRUE;

  if ((scroll == GTK_SCROLL_JUMP)          ||
      (scroll == GTK_SCROLL_PAGE_BACKWARD) ||
      (scroll == GTK_SCROLL_PAGE_FORWARD))
    return FALSE;

  g_object_freeze_notify (G_OBJECT (shell->hsbdata));

  picman_display_shell_scroll_setup_hscrollbar (shell, value);

  g_object_thaw_notify (G_OBJECT (shell->hsbdata)); /* emits "changed" */

  return FALSE;
}

static gboolean
picman_display_shell_vscrollbar_change_value (GtkRange         *range,
                                            GtkScrollType     scroll,
                                            gdouble           value,
                                            PicmanDisplayShell *shell)
{
  if (! shell->display)
    return TRUE;

  if ((scroll == GTK_SCROLL_JUMP)          ||
      (scroll == GTK_SCROLL_PAGE_BACKWARD) ||
      (scroll == GTK_SCROLL_PAGE_FORWARD))
    return FALSE;

  g_object_freeze_notify (G_OBJECT (shell->vsbdata));

  picman_display_shell_scroll_setup_vscrollbar (shell, value);

  g_object_thaw_notify (G_OBJECT (shell->vsbdata)); /* emits "changed" */

  return FALSE;
}

static void
picman_display_shell_canvas_draw_image (PicmanDisplayShell *shell,
                                      cairo_t          *cr)
{
  cairo_rectangle_list_t *clip_rectangles;
  cairo_rectangle_int_t   image_rect;

  image_rect.x = - shell->offset_x;
  image_rect.y = - shell->offset_y;
  picman_display_shell_scale_get_image_size (shell,
                                           &image_rect.width,
                                           &image_rect.height);


  /*  first, clear the exposed part of the region that is outside the
   *  image, which is the exposed region minus the image rectangle
   */

  cairo_save (cr);

  if (shell->rotate_transform)
    cairo_transform (cr, shell->rotate_transform);

  cairo_rectangle (cr,
                   image_rect.x,
                   image_rect.y,
                   image_rect.width,
                   image_rect.height);

  cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
  cairo_clip (cr);

  if (gdk_cairo_get_clip_rectangle (cr, NULL))
    picman_display_shell_draw_background (shell, cr);

  cairo_restore (cr);


  /*  then, draw the exposed part of the region that is inside the
   *  image
   */

  cairo_save (cr);
  clip_rectangles = cairo_copy_clip_rectangle_list (cr);

  if (shell->rotate_transform)
    cairo_transform (cr, shell->rotate_transform);

  cairo_rectangle (cr,
                   image_rect.x,
                   image_rect.y,
                   image_rect.width,
                   image_rect.height);
  cairo_clip (cr);

  if (gdk_cairo_get_clip_rectangle (cr, NULL))
    {
      gint i;

      cairo_save (cr);
      picman_display_shell_draw_checkerboard (shell, cr);
      cairo_restore (cr);

      for (i = 0; i < clip_rectangles->num_rectangles; i++)
        {
          cairo_rectangle_t rect = clip_rectangles->rectangles[i];

          picman_display_shell_draw_image (shell, cr,
                                         floor (rect.x),
                                         floor (rect.y),
                                         ceil (rect.width),
                                         ceil (rect.height));
        }
    }

  cairo_rectangle_list_destroy (clip_rectangles);
  cairo_restore (cr);


  /*  finally, draw all the remaining image window stuff on top
   */

  /* draw canvas items */
  cairo_save (cr);

  if (shell->rotate_transform)
    cairo_transform (cr, shell->rotate_transform);

  picman_canvas_item_draw (shell->canvas_item, cr);

  cairo_restore (cr);

  picman_canvas_item_draw (shell->unrotated_item, cr);

  /* restart (and recalculate) the selection boundaries */
  picman_display_shell_selection_restart (shell);
}

static void
picman_display_shell_canvas_draw_drop_zone (PicmanDisplayShell *shell,
                                          cairo_t          *cr)
{
  picman_display_shell_draw_background (shell, cr);

  picman_cairo_draw_drop_wilber (shell->canvas, cr);
}
