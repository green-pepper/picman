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

#include "display-types.h"

#include "config/picmandisplayconfig.h"

#include "core/picman.h"
#include "core/picman-cairo.h"
#include "core/picmanboundary.h"
#include "core/picmanchannel.h"
#include "core/picmanimage.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-appearance.h"
#include "picmandisplayshell-draw.h"
#include "picmandisplayshell-expose.h"
#include "picmandisplayshell-selection.h"
#include "picmandisplayshell-transform.h"


struct _Selection
{
  PicmanDisplayShell *shell;            /*  shell that owns the selection     */

  PicmanSegment      *segs_in;          /*  gdk segments of area boundary     */
  gint              n_segs_in;        /*  number of segments in segs_in     */

  PicmanSegment      *segs_out;         /*  gdk segments of area boundary     */
  gint              n_segs_out;       /*  number of segments in segs_out    */

  guint             index;            /*  index of current stipple pattern  */
  gint              paused;           /*  count of pause requests           */
  gboolean          shell_visible;    /*  visility of the display shell     */
  gboolean          show_selection;   /*  is the selection visible?         */
  guint             timeout;          /*  timer for successive draws        */
  cairo_pattern_t  *segs_in_mask;     /*  cache for rendered segments       */
};


/*  local function prototypes  */

static void      selection_start          (Selection          *selection);
static void      selection_stop           (Selection          *selection);

static void      selection_draw           (Selection          *selection);
static void      selection_undraw         (Selection          *selection);

static void      selection_render_mask    (Selection          *selection);

static void      selection_zoom_segs      (Selection          *selection,
                                           const PicmanBoundSeg *src_segs,
                                           PicmanSegment        *dest_segs,
                                           gint                n_segs);
static void      selection_generate_segs  (Selection          *selection);
static void      selection_free_segs      (Selection          *selection);

static gboolean  selection_start_timeout  (Selection          *selection);
static gboolean  selection_timeout        (Selection          *selection);

static gboolean  selection_window_state_event      (GtkWidget           *shell,
                                                    GdkEventWindowState *event,
                                                    Selection           *selection);
static gboolean  selection_visibility_notify_event (GtkWidget           *shell,
                                                    GdkEventVisibility  *event,
                                                    Selection           *selection);


/*  public functions  */

void
picman_display_shell_selection_init (PicmanDisplayShell *shell)
{
  Selection *selection;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (shell->selection == NULL);

  selection = g_slice_new0 (Selection);

  selection->shell          = shell;
  selection->shell_visible  = TRUE;
  selection->show_selection = picman_display_shell_get_show_selection (shell);

  shell->selection = selection;

  g_signal_connect (shell, "window-state-event",
                    G_CALLBACK (selection_window_state_event),
                    selection);
  g_signal_connect (shell, "visibility-notify-event",
                    G_CALLBACK (selection_visibility_notify_event),
                    selection);
}

void
picman_display_shell_selection_free (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->selection)
    {
      Selection *selection = shell->selection;

      selection_stop (selection);

      g_signal_handlers_disconnect_by_func (shell,
                                            selection_window_state_event,
                                            selection);
      g_signal_handlers_disconnect_by_func (shell,
                                            selection_visibility_notify_event,
                                            selection);

      selection_free_segs (selection);

      g_slice_free (Selection, selection);

      shell->selection = NULL;
    }
}

void
picman_display_shell_selection_undraw (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->selection && picman_display_get_image (shell->display))
    {
      selection_undraw (shell->selection);
    }
  else
    {
      selection_stop (shell->selection);
      selection_free_segs (shell->selection);
    }
}

void
picman_display_shell_selection_restart (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->selection && picman_display_get_image (shell->display))
    {
      selection_start (shell->selection);
    }
}

void
picman_display_shell_selection_pause (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->selection && picman_display_get_image (shell->display))
    {
      if (shell->selection->paused == 0)
        selection_stop (shell->selection);

      shell->selection->paused++;
    }
}

void
picman_display_shell_selection_resume (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->selection && picman_display_get_image (shell->display))
    {
      shell->selection->paused--;

      if (shell->selection->paused == 0)
        selection_start (shell->selection);
    }
}

void
picman_display_shell_selection_set_show (PicmanDisplayShell *shell,
                                       gboolean          show)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->selection && picman_display_get_image (shell->display))
    {
      Selection *selection = shell->selection;

      if (show != selection->show_selection)
        {
          selection_undraw (selection);

          selection->show_selection = show;

          selection_start (selection);
        }
    }
}


/*  private functions  */

static void
selection_start (Selection *selection)
{
  selection_stop (selection);

  /*  If this selection is paused, do not start it  */
  if (selection->paused == 0)
    {
      selection->timeout = g_idle_add ((GSourceFunc) selection_start_timeout,
                                       selection);
    }
}

static void
selection_stop (Selection *selection)
{
  if (selection->timeout)
    {
      g_source_remove (selection->timeout);
      selection->timeout = 0;
    }
}

static void
selection_draw (Selection *selection)
{
  if (selection->segs_in)
    {
      cairo_t *cr;

      cr = gdk_cairo_create (gtk_widget_get_window (selection->shell->canvas));

      picman_display_shell_draw_selection_in (selection->shell, cr,
                                            selection->segs_in_mask,
                                            selection->index % 8);

      cairo_destroy (cr);
    }
}

static void
selection_undraw (Selection *selection)
{
  gint x1, y1, x2, y2;

  selection_stop (selection);

  if (picman_display_shell_mask_bounds (selection->shell, &x1, &y1, &x2, &y2))
    {
      /* expose will restart the selection */
      picman_display_shell_expose_area (selection->shell,
                                      x1, y1, (x2 - x1), (y2 - y1));
    }
  else
    {
      selection_start (selection);
    }
}

static void
selection_render_mask (Selection *selection)
{
  GdkWindow       *window;
  cairo_surface_t *surface;
  cairo_t         *cr;

  window = gtk_widget_get_window (selection->shell->canvas);
  surface = gdk_window_create_similar_surface (window, CAIRO_CONTENT_ALPHA,
                                               gdk_window_get_width  (window),
                                               gdk_window_get_height (window));
  cr = cairo_create (surface);

  cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
  cairo_set_line_width (cr, 1.0);

  if (selection->shell->rotate_transform)
    cairo_transform (cr, selection->shell->rotate_transform);

  picman_cairo_add_segments (cr,
                           selection->segs_in,
                           selection->n_segs_in);
  cairo_stroke (cr);

  selection->segs_in_mask = cairo_pattern_create_for_surface (surface);

  cairo_destroy (cr);
  cairo_surface_destroy (surface);
}

static void
selection_zoom_segs (Selection          *selection,
                     const PicmanBoundSeg *src_segs,
                     PicmanSegment        *dest_segs,
                     gint                n_segs)
{
  const gint xclamp = selection->shell->disp_width + 1;
  const gint yclamp = selection->shell->disp_height + 1;
  gint       i;

  picman_display_shell_zoom_segments (selection->shell,
                                    src_segs, dest_segs, n_segs,
                                    0.0, 0.0);

  for (i = 0; i < n_segs; i++)
    {
      dest_segs[i].x1 = CLAMP (dest_segs[i].x1, -1, xclamp);
      dest_segs[i].y1 = CLAMP (dest_segs[i].y1, -1, yclamp);

      dest_segs[i].x2 = CLAMP (dest_segs[i].x2, -1, xclamp);
      dest_segs[i].y2 = CLAMP (dest_segs[i].y2, -1, yclamp);

      /*  If this segment is a closing segment && the segments lie inside
       *  the region, OR if this is an opening segment and the segments
       *  lie outside the region...
       *  we need to transform it by one display pixel
       */
      if (! src_segs[i].open)
        {
          /*  If it is vertical  */
          if (dest_segs[i].x1 == dest_segs[i].x2)
            {
              dest_segs[i].x1 -= 1;
              dest_segs[i].x2 -= 1;
            }
          else
            {
              dest_segs[i].y1 -= 1;
              dest_segs[i].y2 -= 1;
            }
        }
    }
}

static void
selection_generate_segs (Selection *selection)
{
  PicmanImage          *image = picman_display_get_image (selection->shell->display);
  const PicmanBoundSeg *segs_in;
  const PicmanBoundSeg *segs_out;

  /*  Ask the image for the boundary of its selected region...
   *  Then transform that information into a new buffer of PicmanSegments
   */
  picman_channel_boundary (picman_image_get_mask (image),
                         &segs_in, &segs_out,
                         &selection->n_segs_in, &selection->n_segs_out,
                         0, 0, 0, 0);

  if (selection->n_segs_in)
    {
      selection->segs_in = g_new (PicmanSegment, selection->n_segs_in);
      selection_zoom_segs (selection, segs_in,
                           selection->segs_in, selection->n_segs_in);

      selection_render_mask (selection);
    }
  else
    {
      selection->segs_in = NULL;
    }

  /*  Possible secondary boundary representation  */
  if (selection->n_segs_out)
    {
      selection->segs_out = g_new (PicmanSegment, selection->n_segs_out);
      selection_zoom_segs (selection, segs_out,
                           selection->segs_out, selection->n_segs_out);
    }
  else
    {
      selection->segs_out = NULL;
    }
}

static void
selection_free_segs (Selection *selection)
{
  if (selection->segs_in)
    {
      g_free (selection->segs_in);
      selection->segs_in   = NULL;
      selection->n_segs_in = 0;
    }

  if (selection->segs_out)
    {
      g_free (selection->segs_out);
      selection->segs_out   = NULL;
      selection->n_segs_out = 0;
    }

  if (selection->segs_in_mask)
    {
      cairo_pattern_destroy (selection->segs_in_mask);
      selection->segs_in_mask = NULL;
    }
}

static gboolean
selection_start_timeout (Selection *selection)
{
  selection_free_segs (selection);
  selection->timeout = 0;

  if (! picman_display_get_image (selection->shell->display))
    return FALSE;

  selection_generate_segs (selection);

  selection->index = 0;

  /*  Draw the ants  */
  if (selection->show_selection)
    {
      PicmanDisplayConfig *config = selection->shell->display->config;

      selection_draw (selection);

      if (selection->segs_out)
        {
          cairo_t *cr;

          cr = gdk_cairo_create (gtk_widget_get_window (selection->shell->canvas));

          if (selection->shell->rotate_transform)
            cairo_transform (cr, selection->shell->rotate_transform);

          picman_display_shell_draw_selection_out (selection->shell, cr,
                                                 selection->segs_out,
                                                 selection->n_segs_out);

          cairo_destroy (cr);
        }

      if (selection->segs_in && selection->shell_visible)
        selection->timeout = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,
                                                 config->marching_ants_speed,
                                                 (GSourceFunc) selection_timeout,
                                                 selection, NULL);
    }

  return FALSE;
}

static gboolean
selection_timeout (Selection *selection)
{
  selection->index++;
  selection_draw (selection);

  return TRUE;
}

static void
selection_set_shell_visible (Selection *selection,
                             gboolean   shell_visible)
{
  if (selection->shell_visible != shell_visible)
    {
      selection->shell_visible = shell_visible;

      if (shell_visible)
        selection_start (selection);
      else
        selection_stop (selection);
    }
}

static gboolean
selection_window_state_event (GtkWidget           *shell,
                              GdkEventWindowState *event,
                              Selection           *selection)
{
  selection_set_shell_visible (selection,
                               (event->new_window_state & (GDK_WINDOW_STATE_WITHDRAWN |
                                                           GDK_WINDOW_STATE_ICONIFIED)) == 0);

  return FALSE;
}

static gboolean
selection_visibility_notify_event (GtkWidget          *shell,
                                   GdkEventVisibility *event,
                                   Selection          *selection)
{
  selection_set_shell_visible (selection,
                               event->state != GDK_VISIBILITY_FULLY_OBSCURED);

  return FALSE;
}
