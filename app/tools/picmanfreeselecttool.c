/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Major improvement to support polygonal segments
 * Copyright (C) 2008 Martin Nordholts
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

#include <gegl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman-utils.h"
#include "core/picmanchannel.h"
#include "core/picmanchannel-select.h"
#include "core/picmanimage.h"
#include "core/picmanlayer-floating-sel.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvasgroup.h"
#include "display/picmandisplay.h"

#include "picmanfreeselecttool.h"
#include "picmanselectionoptions.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


#define POINT_GRAB_THRESHOLD_SQ SQR (PICMAN_TOOL_HANDLE_SIZE_CIRCLE / 2)
#define POINT_SHOW_THRESHOLD_SQ SQR (PICMAN_TOOL_HANDLE_SIZE_CIRCLE * 7)
#define N_ITEMS_PER_ALLOC       1024
#define INVALID_INDEX           (-1)
#define NO_CLICK_TIME_AVAILABLE 0

#define GET_PRIVATE(fst)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((fst), \
    PICMAN_TYPE_FREE_SELECT_TOOL, PicmanFreeSelectToolPrivate))


typedef struct
{
  /* Index of grabbed segment index. */
  gint               grabbed_segment_index;

  /* We need to keep track of a number of points when we move a
   * segment vertex
   */
  PicmanVector2       *saved_points_lower_segment;
  PicmanVector2       *saved_points_higher_segment;
  gint               max_n_saved_points_lower_segment;
  gint               max_n_saved_points_higher_segment;

  /* Keeps track whether or not a modification of the polygon has been
   * made between _button_press and _button_release
   */
  gboolean           polygon_modified;

  /* Point which is used to draw the polygon but which is not part of
   * it yet
   */
  PicmanVector2        pending_point;
  gboolean           show_pending_point;

  /* The points of the polygon */
  PicmanVector2       *points;
  gint               max_n_points;

  /* The number of points actually in use */
  gint               n_points;


  /* Any int array containing the indices for the points in the
   * polygon that connects different segments together
   */
  gint              *segment_indices;
  gint               max_n_segment_indices;

  /* The number of segment indices actually in use */
  gint               n_segment_indices;

  /* The selection operation active when the tool was started */
  PicmanChannelOps     operation_at_start;

  /* Whether or not to constrain the angle for newly created polygonal
   * segments.
   */
  gboolean           constrain_angle;

  /* Whether or not to suppress handles (so that new segments can be
   * created immediately after an existing segment vertex.
   */
  gboolean           supress_handles;

  /* Last _oper_update or _motion coords */
  PicmanVector2        last_coords;

  /* A double-click commits the selection, keep track of last
   * click-time and click-position.
   */
  guint32            last_click_time;
  PicmanCoords         last_click_coord;

} PicmanFreeSelectToolPrivate;


static void     picman_free_select_tool_finalize            (GObject               *object);
static void     picman_free_select_tool_control             (PicmanTool              *tool,
                                                           PicmanToolAction         action,
                                                           PicmanDisplay           *display);
static void     picman_free_select_tool_oper_update         (PicmanTool              *tool,
                                                           const PicmanCoords      *coords,
                                                           GdkModifierType        state,
                                                           gboolean               proximity,
                                                           PicmanDisplay           *display);
static void     picman_free_select_tool_cursor_update       (PicmanTool              *tool,
                                                           const PicmanCoords      *coords,
                                                           GdkModifierType        state,
                                                           PicmanDisplay           *display);
static void     picman_free_select_tool_button_press        (PicmanTool              *tool,
                                                           const PicmanCoords      *coords,
                                                           guint32                time,
                                                           GdkModifierType        state,
                                                           PicmanButtonPressType    press_type,
                                                           PicmanDisplay           *display);
static void     picman_free_select_tool_button_release      (PicmanTool              *tool,
                                                           const PicmanCoords      *coords,
                                                           guint32                time,
                                                           GdkModifierType        state,
                                                           PicmanButtonReleaseType  release_type,
                                                           PicmanDisplay           *display);
static void     picman_free_select_tool_motion              (PicmanTool              *tool,
                                                           const PicmanCoords      *coords,
                                                           guint32                time,
                                                           GdkModifierType        state,
                                                           PicmanDisplay           *display);
static gboolean picman_free_select_tool_key_press           (PicmanTool              *tool,
                                                           GdkEventKey           *kevent,
                                                           PicmanDisplay           *display);
static void     picman_free_select_tool_modifier_key        (PicmanTool              *tool,
                                                           GdkModifierType        key,
                                                           gboolean               press,
                                                           GdkModifierType        state,
                                                           PicmanDisplay           *display);
static void     picman_free_select_tool_active_modifier_key (PicmanTool              *tool,
                                                           GdkModifierType        key,
                                                           gboolean               press,
                                                           GdkModifierType        state,
                                                           PicmanDisplay           *display);
static void     picman_free_select_tool_draw                (PicmanDrawTool          *draw_tool);
static void     picman_free_select_tool_real_select         (PicmanFreeSelectTool    *fst,
                                                           PicmanDisplay           *display);


G_DEFINE_TYPE (PicmanFreeSelectTool, picman_free_select_tool,
               PICMAN_TYPE_SELECTION_TOOL);

#define parent_class picman_free_select_tool_parent_class


static const PicmanVector2 vector2_zero = { 0.0, 0.0 };


void
picman_free_select_tool_register (PicmanToolRegisterCallback  callback,
                                gpointer                  data)
{
  (* callback) (PICMAN_TYPE_FREE_SELECT_TOOL,
                PICMAN_TYPE_SELECTION_OPTIONS,
                picman_selection_options_gui,
                0,
                "picman-free-select-tool",
                _("Free Select"),
                _("Free Select Tool: Select a hand-drawn region with free and polygonal segments"),
                N_("_Free Select"), "F",
                NULL, PICMAN_HELP_TOOL_FREE_SELECT,
                PICMAN_STOCK_TOOL_FREE_SELECT,
                data);
}

static void
picman_free_select_tool_class_init (PicmanFreeSelectToolClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  object_class->finalize          = picman_free_select_tool_finalize;

  tool_class->control             = picman_free_select_tool_control;
  tool_class->oper_update         = picman_free_select_tool_oper_update;
  tool_class->cursor_update       = picman_free_select_tool_cursor_update;
  tool_class->button_press        = picman_free_select_tool_button_press;
  tool_class->button_release      = picman_free_select_tool_button_release;
  tool_class->motion              = picman_free_select_tool_motion;
  tool_class->key_press           = picman_free_select_tool_key_press;
  tool_class->modifier_key        = picman_free_select_tool_modifier_key;
  tool_class->active_modifier_key = picman_free_select_tool_active_modifier_key;

  draw_tool_class->draw           = picman_free_select_tool_draw;

  klass->select                   = picman_free_select_tool_real_select;

  g_type_class_add_private (klass, sizeof (PicmanFreeSelectToolPrivate));
}

static void
picman_free_select_tool_init (PicmanFreeSelectTool *fst)
{
  PicmanTool                  *tool = PICMAN_TOOL (fst);
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  picman_tool_control_set_wants_click (tool->control, TRUE);
  picman_tool_control_set_precision   (tool->control,
                                     PICMAN_CURSOR_PRECISION_SUBPIXEL);
  picman_tool_control_set_tool_cursor (tool->control,
                                     PICMAN_TOOL_CURSOR_FREE_SELECT);

  priv->grabbed_segment_index             = INVALID_INDEX;

  priv->saved_points_lower_segment        = NULL;
  priv->saved_points_higher_segment       = NULL;
  priv->max_n_saved_points_lower_segment  = 0;
  priv->max_n_saved_points_higher_segment = 0;

  priv->polygon_modified                  = FALSE;

  priv->show_pending_point                = FALSE;

  priv->points                            = NULL;
  priv->n_points                          = 0;
  priv->max_n_points                      = 0;

  priv->segment_indices                   = NULL;
  priv->n_segment_indices                 = 0;
  priv->max_n_segment_indices             = 0;

  priv->constrain_angle                   = FALSE;
  priv->supress_handles                   = FALSE;

  priv->last_click_time                   = NO_CLICK_TIME_AVAILABLE;
}

static void
picman_free_select_tool_finalize (GObject *object)
{
  PicmanFreeSelectTool        *fst  = PICMAN_FREE_SELECT_TOOL (object);
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  g_free (priv->points);
  g_free (priv->segment_indices);
  g_free (priv->saved_points_lower_segment);
  g_free (priv->saved_points_higher_segment);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_free_select_tool_get_segment (PicmanFreeSelectTool  *fst,
                                   PicmanVector2        **points,
                                   gint                *n_points,
                                   gint                 segment_start,
                                   gint                 segment_end)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  *points   = &priv->points[priv->segment_indices[segment_start]];
  *n_points = priv->segment_indices[segment_end] -
              priv->segment_indices[segment_start] +
              1;
}

static void
picman_free_select_tool_get_segment_point (PicmanFreeSelectTool *fst,
                                         gdouble            *start_point_x,
                                         gdouble            *start_point_y,
                                         gint                segment_index)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  *start_point_x = priv->points[priv->segment_indices[segment_index]].x;
  *start_point_y = priv->points[priv->segment_indices[segment_index]].y;
}

static void
picman_free_select_tool_get_last_point (PicmanFreeSelectTool *fst,
                                      gdouble            *start_point_x,
                                      gdouble            *start_point_y)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  picman_free_select_tool_get_segment_point (fst,
                                           start_point_x,
                                           start_point_y,
                                           priv->n_segment_indices - 1);
}

static void
picman_free_select_tool_get_double_click_settings (gint *double_click_time,
                                                 gint *double_click_distance)
{
  GdkScreen *screen = gdk_screen_get_default ();

  if (screen != NULL)
    {
      GtkSettings *settings = gtk_settings_get_for_screen (screen);

      g_object_get (settings,
                    "gtk-double-click-time",     double_click_time,
                    "gtk-double-click-distance", double_click_distance,
                    NULL);
    }
}

static gboolean
picman_free_select_tool_should_close (PicmanFreeSelectTool *fst,
                                    PicmanDisplay        *display,
                                    guint32             time,
                                    const PicmanCoords   *coords)
{
  PicmanFreeSelectToolPrivate *priv         = GET_PRIVATE (fst);
  gboolean                   double_click = FALSE;
  gdouble                    dist         = G_MAXDOUBLE;

  if (priv->polygon_modified       ||
      priv->n_segment_indices <= 0 ||
      PICMAN_TOOL (fst)->display == NULL)
    return FALSE;

  dist = picman_draw_tool_calc_distance_square (PICMAN_DRAW_TOOL (fst),
                                              display,
                                              coords->x,
                                              coords->y,
                                              priv->points[0].x,
                                              priv->points[0].y);

  /* Handle double-click. It must be within GTK+ global double-click
   * time since last click, and it must be within GTK+ global
   * double-click distance away from the last point
   */
  if (time != NO_CLICK_TIME_AVAILABLE)
    {
      gint    double_click_time;
      gint    double_click_distance;
      gint    click_time_passed;
      gdouble dist_from_last_point;

      click_time_passed = time - priv->last_click_time;

      dist_from_last_point =
        picman_draw_tool_calc_distance_square (PICMAN_DRAW_TOOL (fst),
                                             display,
                                             coords->x,
                                             coords->y,
                                             priv->last_click_coord.x,
                                             priv->last_click_coord.y);

      picman_free_select_tool_get_double_click_settings (&double_click_time,
                                                       &double_click_distance);

      double_click = click_time_passed    < double_click_time &&
                     dist_from_last_point < double_click_distance;
    }

  return ((! priv->supress_handles && dist < POINT_GRAB_THRESHOLD_SQ) ||
          double_click);
}

static void
picman_free_select_tool_handle_segment_selection (PicmanFreeSelectTool *fst,
                                                PicmanDisplay        *display,
                                                const PicmanCoords   *coords)
{
  PicmanFreeSelectToolPrivate *priv                  = GET_PRIVATE (fst);
  PicmanDrawTool              *draw_tool             = PICMAN_DRAW_TOOL (fst);
  gdouble                    shortest_dist         = POINT_GRAB_THRESHOLD_SQ;
  gint                       grabbed_segment_index = INVALID_INDEX;
  gint                       i;

  if (PICMAN_TOOL (fst)->display != NULL &&
      ! priv->supress_handles)
    {
      for (i = 0; i < priv->n_segment_indices; i++)
        {
          gdouble      dist;
          PicmanVector2 *point;

          point = &priv->points[priv->segment_indices[i]];

          dist = picman_draw_tool_calc_distance_square (draw_tool,
                                                      display,
                                                      coords->x,
                                                      coords->y,
                                                      point->x,
                                                      point->y);

          if (dist < shortest_dist)
            {
              grabbed_segment_index = i;
            }
        }
    }

  if (grabbed_segment_index != priv->grabbed_segment_index)
    {
      picman_draw_tool_pause (draw_tool);

      priv->grabbed_segment_index = grabbed_segment_index;

      picman_draw_tool_resume (draw_tool);
    }
}

static void
picman_free_select_tool_revert_to_last_segment (PicmanFreeSelectTool *fst)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  priv->n_points = priv->segment_indices[priv->n_segment_indices - 1] + 1;
}

static void
picman_free_select_tool_remove_last_segment (PicmanFreeSelectTool *fst)
{
  PicmanFreeSelectToolPrivate *priv      = GET_PRIVATE (fst);
  PicmanDrawTool              *draw_tool = PICMAN_DRAW_TOOL (fst);

  picman_draw_tool_pause (draw_tool);

  if (priv->n_segment_indices > 0)
    priv->n_segment_indices--;

  if (priv->n_segment_indices <= 0)
    {
      picman_tool_control (PICMAN_TOOL (fst), PICMAN_TOOL_ACTION_HALT,
                         PICMAN_TOOL (fst)->display);
    }
  else
    {
      picman_free_select_tool_revert_to_last_segment (fst);
    }

  picman_draw_tool_resume (draw_tool);
}

static void
picman_free_select_tool_add_point (PicmanFreeSelectTool *fst,
                                 gdouble             x,
                                 gdouble             y)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  if (priv->n_points >= priv->max_n_points)
    {
      priv->max_n_points += N_ITEMS_PER_ALLOC;

      priv->points = g_realloc (priv->points,
                                    sizeof (PicmanVector2) * priv->max_n_points);
    }

  priv->points[priv->n_points].x = x;
  priv->points[priv->n_points].y = y;

  priv->n_points++;
}

static void
picman_free_select_tool_add_segment_index (PicmanFreeSelectTool *fst,
                                         gint                index)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  if (priv->n_segment_indices >= priv->max_n_segment_indices)
    {
      priv->max_n_segment_indices += N_ITEMS_PER_ALLOC;

      priv->segment_indices = g_realloc (priv->segment_indices,
                                         sizeof (PicmanVector2) *
                                         priv->max_n_segment_indices);
    }

  priv->segment_indices[priv->n_segment_indices] = index;

  priv->n_segment_indices++;
}

static gboolean
picman_free_select_tool_is_point_grabbed (PicmanFreeSelectTool *fst)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  return priv->grabbed_segment_index != INVALID_INDEX;
}

static void
picman_free_select_tool_fit_segment (PicmanFreeSelectTool *fst,
                                   PicmanVector2        *dest_points,
                                   PicmanVector2         dest_start_target,
                                   PicmanVector2         dest_end_target,
                                   const PicmanVector2  *source_points,
                                   gint                n_points)
{
  PicmanVector2 origo_translation_offset;
  PicmanVector2 untranslation_offset;
  gdouble     rotation;
  gdouble     scale;

  /* Handle some quick special cases */
  if (n_points <= 0)
    {
      return;
    }
  else if (n_points == 1)
    {
      dest_points[0] = dest_end_target;
      return;
    }
  else if (n_points == 2)
    {
      dest_points[0] = dest_start_target;
      dest_points[1] = dest_end_target;
      return;
    }

  /* Copy from source to dest; we work on the dest data */
  memcpy (dest_points, source_points, sizeof (PicmanVector2) * n_points);

  /* Transform the destination end point */
  {
    PicmanVector2 *dest_end;
    PicmanVector2  origo_translated_end_target;
    gdouble      target_rotation;
    gdouble      current_rotation;
    gdouble      target_length;
    gdouble      current_length;

    dest_end = &dest_points[n_points - 1];

    /* Transate to origin */
    picman_vector2_sub (&origo_translation_offset,
                      &vector2_zero,
                      &dest_points[0]);
    picman_vector2_add (dest_end,
                      dest_end,
                      &origo_translation_offset);

    /* Calculate origo_translated_end_target */
    picman_vector2_sub (&origo_translated_end_target,
                      &dest_end_target,
                      &dest_start_target);

    /* Rotate */
    target_rotation  = atan2 (vector2_zero.y - origo_translated_end_target.y,
                              vector2_zero.x - origo_translated_end_target.x);
    current_rotation = atan2 (vector2_zero.y - dest_end->y,
                              vector2_zero.x - dest_end->x);
    rotation         = current_rotation - target_rotation;

    picman_vector2_rotate (dest_end, rotation);


    /* Scale */
    target_length  = picman_vector2_length (&origo_translated_end_target);
    current_length = picman_vector2_length (dest_end);
    scale          = target_length / current_length;

    picman_vector2_mul (dest_end, scale);


    /* Untranslate */
    picman_vector2_sub (&untranslation_offset,
                      &dest_end_target,
                      dest_end);
    picman_vector2_add (dest_end,
                      dest_end,
                      &untranslation_offset);
  }

  /* Do the same transformation for the rest of the points */
  {
    gint i;

    for (i = 0; i < n_points - 1; i++)
      {
        /* Translate */
        picman_vector2_add (&dest_points[i],
                          &origo_translation_offset,
                          &dest_points[i]);

        /* Rotate */
        picman_vector2_rotate (&dest_points[i],
                             rotation);

        /* Scale */
        picman_vector2_mul (&dest_points[i],
                          scale);

        /* Untranslate */
        picman_vector2_add (&dest_points[i],
                          &dest_points[i],
                          &untranslation_offset);
      }
  }
}

static void
picman_free_select_tool_move_segment_vertex_to (PicmanFreeSelectTool *fst,
                                              gint                segment_index,
                                              gdouble             new_x,
                                              gdouble             new_y)
{
  PicmanFreeSelectToolPrivate *priv         = GET_PRIVATE (fst);
  PicmanVector2                cursor_point = { new_x, new_y };
  PicmanVector2               *dest;
  PicmanVector2               *dest_start_target;
  PicmanVector2               *dest_end_target;
  gint                       n_points;

  /* Handle the segment before the grabbed point */
  if (segment_index > 0)
    {
      picman_free_select_tool_get_segment (fst,
                                         &dest,
                                         &n_points,
                                         priv->grabbed_segment_index - 1,
                                         priv->grabbed_segment_index);

      dest_start_target = &dest[0];
      dest_end_target   = &cursor_point;

      picman_free_select_tool_fit_segment (fst,
                                         dest,
                                         *dest_start_target,
                                         *dest_end_target,
                                         priv->saved_points_lower_segment,
                                         n_points);
    }

  /* Handle the segment after the grabbed point */
  if (segment_index < priv->n_segment_indices - 1)
    {
      picman_free_select_tool_get_segment (fst,
                                         &dest,
                                         &n_points,
                                         priv->grabbed_segment_index,
                                         priv->grabbed_segment_index + 1);

      dest_start_target = &cursor_point;
      dest_end_target   = &dest[n_points - 1];

      picman_free_select_tool_fit_segment (fst,
                                         dest,
                                         *dest_start_target,
                                         *dest_end_target,
                                         priv->saved_points_higher_segment,
                                         n_points);
    }

  /* Handle when there only is one point */
  if (segment_index == 0 &&
      priv->n_segment_indices == 1)
    {
      priv->points[0].x = new_x;
      priv->points[0].y = new_y;
    }
}

/**
 * picman_free_select_tool_finish_line_segment:
 * @free_ploy_sel_tool:
 * @end_x:
 * @end_y:
 *
 * Adds a line segment. Also cancels a pending free segment if any.
 **/
static void
picman_free_select_tool_finish_line_segment (PicmanFreeSelectTool *fst)
{
  /* Revert any free segment points that might have been added */
  picman_free_select_tool_revert_to_last_segment (fst);
}

/**
 * picman_free_select_tool_finish_free_segment:
 * @fst:
 *
 * Finnishes off the creation of a free segment.
 **/
static void
picman_free_select_tool_finish_free_segment (PicmanFreeSelectTool *fst)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  /* The points are all setup, just make a segment */
  picman_free_select_tool_add_segment_index (fst,
                                           priv->n_points - 1);
}

static void
picman_free_select_tool_commit (PicmanFreeSelectTool *fst,
                              PicmanDisplay        *display)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  if (priv->n_points >= 3)
    {
      picman_free_select_tool_select (fst, display);
    }

  picman_image_flush (picman_display_get_image (display));
}

static void
picman_free_select_tool_revert_to_saved_state (PicmanFreeSelectTool *fst)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);
  PicmanVector2               *dest;
  gint                       n_points;

  /* Without a point grab we have no sensible information to fall back
   * on, bail out
   */
  if (! picman_free_select_tool_is_point_grabbed (fst))
    {
      return;
    }

  if (priv->grabbed_segment_index > 0)
    {
      picman_free_select_tool_get_segment (fst,
                                         &dest,
                                         &n_points,
                                         priv->grabbed_segment_index - 1,
                                         priv->grabbed_segment_index);

      memcpy (dest,
              priv->saved_points_lower_segment,
              sizeof (PicmanVector2) * n_points);
    }

  if (priv->grabbed_segment_index < priv->n_segment_indices - 1)
    {
      picman_free_select_tool_get_segment (fst,
                                         &dest,
                                         &n_points,
                                         priv->grabbed_segment_index,
                                         priv->grabbed_segment_index + 1);

      memcpy (dest,
              priv->saved_points_higher_segment,
              sizeof (PicmanVector2) * n_points);
    }

  if (priv->grabbed_segment_index == 0 &&
      priv->n_segment_indices     == 1)
    {
      priv->points[0] = *priv->saved_points_lower_segment;
    }
}

static void
picman_free_select_tool_handle_click (PicmanFreeSelectTool *fst,
                                    const PicmanCoords   *coords,
                                    guint32             time,
                                    PicmanDisplay        *display)
{
  PicmanFreeSelectToolPrivate *priv  = GET_PRIVATE (fst);
  PicmanImage                 *image = picman_display_get_image (display);

  /*  If there is a floating selection, anchor it  */
  if (picman_image_get_floating_selection (image))
    {
      floating_sel_anchor (picman_image_get_floating_selection (image));

      picman_tool_control (PICMAN_TOOL (fst), PICMAN_TOOL_ACTION_HALT, display);
    }
  else
    {
      /* First finish of the line segment if no point was grabbed */
      if (! picman_free_select_tool_is_point_grabbed (fst))
        {
          picman_free_select_tool_finish_line_segment (fst);
        }

      /* After the segments are up to date and we have handled
       * double-click, see if it's committing time
       */
      if (picman_free_select_tool_should_close (fst,
                                              display,
                                              time,
                                              coords))
        {
          /* We can get a click notification even though the end point
           * has been moved a few pixels. Since a move will change the
           * free selection, revert it before doing the commit.
           */
          picman_free_select_tool_revert_to_saved_state (fst);

          picman_free_select_tool_commit (fst, display);
        }

      priv->last_click_time  = time;
      priv->last_click_coord = *coords;
    }
}

static void
picman_free_select_tool_handle_normal_release (PicmanFreeSelectTool *fst,
                                             const PicmanCoords   *coords,
                                             PicmanDisplay        *display)
{
  /* First finish of the free segment if no point was grabbed */
  if (! picman_free_select_tool_is_point_grabbed (fst))
    {
      picman_free_select_tool_finish_free_segment (fst);
    }

  /* After the segments are up to date, see if it's committing time */
  if (picman_free_select_tool_should_close (fst,
                                          display,
                                          NO_CLICK_TIME_AVAILABLE,
                                          coords))
    {
      picman_free_select_tool_commit (fst, display);
    }
}

static void
picman_free_select_tool_handle_cancel (PicmanFreeSelectTool *fst)
{
  if (picman_free_select_tool_is_point_grabbed (fst))
    {
      picman_free_select_tool_revert_to_saved_state (fst);
    }
  else
    {
      picman_free_select_tool_remove_last_segment (fst);
    }
}

void
picman_free_select_tool_select (PicmanFreeSelectTool *fst,
                              PicmanDisplay        *display)
{
  g_return_if_fail (PICMAN_IS_FREE_SELECT_TOOL (fst));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  PICMAN_FREE_SELECT_TOOL_GET_CLASS (fst)->select (fst,
                                                 display);
}

static void
picman_free_select_tool_prepare_for_move (PicmanFreeSelectTool *fst)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);
  PicmanVector2               *source;
  gint                       n_points;

  if (priv->grabbed_segment_index > 0)
    {
      picman_free_select_tool_get_segment (fst,
                                         &source,
                                         &n_points,
                                         priv->grabbed_segment_index - 1,
                                         priv->grabbed_segment_index);

      if (n_points > priv->max_n_saved_points_lower_segment)
        {
          priv->max_n_saved_points_lower_segment = n_points;

          priv->saved_points_lower_segment = g_realloc (priv->saved_points_lower_segment,
                                                        sizeof (PicmanVector2) *
                                                        n_points);
        }

      memcpy (priv->saved_points_lower_segment,
              source,
              sizeof (PicmanVector2) * n_points);
    }

  if (priv->grabbed_segment_index < priv->n_segment_indices - 1)
    {
      picman_free_select_tool_get_segment (fst,
                                         &source,
                                         &n_points,
                                         priv->grabbed_segment_index,
                                         priv->grabbed_segment_index + 1);

      if (n_points > priv->max_n_saved_points_higher_segment)
        {
          priv->max_n_saved_points_higher_segment = n_points;

          priv->saved_points_higher_segment = g_realloc (priv->saved_points_higher_segment,
                                                         sizeof (PicmanVector2) * n_points);
        }

      memcpy (priv->saved_points_higher_segment,
              source,
              sizeof (PicmanVector2) * n_points);
    }

  /* A special-case when there only is one point */
  if (priv->grabbed_segment_index == 0 &&
      priv->n_segment_indices     == 1)
    {
      if (priv->max_n_saved_points_lower_segment == 0)
        {
          priv->max_n_saved_points_lower_segment = 1;

          priv->saved_points_lower_segment = g_new0 (PicmanVector2, 1);
        }

      *priv->saved_points_lower_segment = priv->points[0];
    }
}

static void
picman_free_select_tool_update_motion (PicmanFreeSelectTool *fst,
                                     gdouble             new_x,
                                     gdouble             new_y)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  if (picman_free_select_tool_is_point_grabbed (fst))
    {
      priv->polygon_modified = TRUE;

      if (priv->constrain_angle &&
          priv->n_segment_indices > 1 )
        {
          gdouble start_point_x;
          gdouble start_point_y;
          gint    segment_index;

          /* Base constraints on the last segment vertex if we move
           * the first one, otherwise base on the previous segment
           * vertex
           */
          if (priv->grabbed_segment_index == 0)
            {
              segment_index = priv->n_segment_indices - 1;
            }
          else
            {
              segment_index = priv->grabbed_segment_index - 1;
            }

          picman_free_select_tool_get_segment_point (fst,
                                                   &start_point_x,
                                                   &start_point_y,
                                                   segment_index);

          picman_constrain_line (start_point_x,
                               start_point_y,
                               &new_x,
                               &new_y,
                               PICMAN_CONSTRAIN_LINE_15_DEGREES);
        }

      picman_free_select_tool_move_segment_vertex_to (fst,
                                                    priv->grabbed_segment_index,
                                                    new_x,
                                                    new_y);

      /* We also must update the pending point if we are moving the
       * first point
       */
      if (priv->grabbed_segment_index == 0)
        {
          priv->pending_point.x = new_x;
          priv->pending_point.y = new_y;
        }
    }
  else
    {
      /* Don't show the pending point while we are adding points */
      priv->show_pending_point = FALSE;

      picman_free_select_tool_add_point (fst,
                                       new_x,
                                       new_y);
    }
}

static void
picman_free_select_tool_status_update (PicmanFreeSelectTool *fst,
                                     PicmanDisplay        *display,
                                     const PicmanCoords   *coords,
                                     gboolean            proximity)
{
  PicmanTool                  *tool = PICMAN_TOOL (fst);
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  picman_tool_pop_status (tool, display);

  if (proximity)
    {
      const gchar *status_text = NULL;

      if (picman_free_select_tool_is_point_grabbed (fst))
        {
          if (picman_free_select_tool_should_close (fst,
                                                  display,
                                                  NO_CLICK_TIME_AVAILABLE,
                                                  coords))
            {
              status_text = _("Click to complete selection");
            }
          else
            {
              status_text = _("Click-Drag to move segment vertex");
            }
        }
      else if (priv->n_segment_indices >= 3)
        {
          status_text = _("Return commits, Escape cancels, Backspace removes last segment");
        }
      else
        {
          status_text = _("Click-Drag adds a free segment, Click adds a polygonal segment");
        }

      if (status_text)
        {
          picman_tool_push_status (tool, display, "%s", status_text);
        }
    }
}

static void
picman_free_select_tool_control (PicmanTool       *tool,
                               PicmanToolAction  action,
                               PicmanDisplay    *display)
{
  PicmanFreeSelectTool        *fst  = PICMAN_FREE_SELECT_TOOL (tool);
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      priv->grabbed_segment_index = INVALID_INDEX;
      priv->show_pending_point    = FALSE;
      priv->n_points              = 0;
      priv->n_segment_indices     = 0;
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_free_select_tool_oper_update (PicmanTool         *tool,
                                   const PicmanCoords *coords,
                                   GdkModifierType   state,
                                   gboolean          proximity,
                                   PicmanDisplay      *display)
{
  PicmanFreeSelectTool        *fst  = PICMAN_FREE_SELECT_TOOL (tool);
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);
  gboolean                   hovering_first_point;

  picman_free_select_tool_handle_segment_selection (fst,
                                                  display,
                                                  coords);
  hovering_first_point =
    picman_free_select_tool_should_close (fst,
                                        display,
                                        NO_CLICK_TIME_AVAILABLE,
                                        coords);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  priv->last_coords.x = coords->x;
  priv->last_coords.y = coords->y;

  if (priv->n_points == 0 ||
      (picman_free_select_tool_is_point_grabbed (fst) &&
       ! hovering_first_point) ||
      ! proximity)
    {
      priv->show_pending_point = FALSE;
    }
  else
    {
      priv->show_pending_point = TRUE;

      if (hovering_first_point)
        {
          priv->pending_point = priv->points[0];
        }
      else
        {
          priv->pending_point.x = coords->x;
          priv->pending_point.y = coords->y;

          if (priv->constrain_angle && priv->n_points > 0)
            {
              gdouble start_point_x;
              gdouble start_point_y;

              picman_free_select_tool_get_last_point (fst,
                                                    &start_point_x,
                                                    &start_point_y);

              picman_constrain_line (start_point_x, start_point_y,
                                   &priv->pending_point.x,
                                   &priv->pending_point.y,
                                   PICMAN_CONSTRAIN_LINE_15_DEGREES);
            }
        }
    }

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));

  if (tool->display == NULL)
    {
      PICMAN_TOOL_CLASS (parent_class)->oper_update (tool,
                                                   coords,
                                                   state,
                                                   proximity,
                                                   display);
    }
  else
    {
      picman_free_select_tool_status_update (fst, display, coords, proximity);
    }
}

static void
picman_free_select_tool_cursor_update (PicmanTool         *tool,
                                     const PicmanCoords *coords,
                                     GdkModifierType   state,
                                     PicmanDisplay      *display)
{
  PicmanFreeSelectTool *fst = PICMAN_FREE_SELECT_TOOL (tool);

  if (tool->display == NULL)
    {
      PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool,
                                                     coords,
                                                     state,
                                                     display);
    }
  else
    {
      PicmanCursorModifier modifier;

      if (picman_free_select_tool_is_point_grabbed (fst) &&
          ! picman_free_select_tool_should_close (fst,
                                                display,
                                                NO_CLICK_TIME_AVAILABLE,
                                                coords))
        {
          modifier = PICMAN_CURSOR_MODIFIER_MOVE;
        }
      else
        {
          modifier = PICMAN_CURSOR_MODIFIER_NONE;
        }

      picman_tool_set_cursor (tool, display,
                            picman_tool_control_get_cursor (tool->control),
                            picman_tool_control_get_tool_cursor (tool->control),
                            modifier);
    }
}

static void
picman_free_select_tool_button_press (PicmanTool            *tool,
                                    const PicmanCoords    *coords,
                                    guint32              time,
                                    GdkModifierType      state,
                                    PicmanButtonPressType  press_type,
                                    PicmanDisplay         *display)
{
  PicmanDrawTool              *draw_tool = PICMAN_DRAW_TOOL (tool);
  PicmanFreeSelectTool        *fst       = PICMAN_FREE_SELECT_TOOL (tool);
  PicmanFreeSelectToolPrivate *priv      = GET_PRIVATE (fst);
  PicmanSelectionOptions      *options   = PICMAN_SELECTION_TOOL_GET_OPTIONS (tool);

  if (tool->display && tool->display != display)
    picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);

  if (tool->display == NULL)
    {
      /* First of all handle delegation to the selection mask edit logic
       * if appropriate.
       */
      if (picman_selection_tool_start_edit (PICMAN_SELECTION_TOOL (fst),
                                          display, coords))
        {
          return;
        }

      tool->display = display;

      picman_draw_tool_start (draw_tool, display);

      /* We want to apply the selection operation that was current when
       * the tool was started, so we save this information
       */
      priv->operation_at_start = options->operation;
    }

  picman_tool_control_activate (tool->control);

  picman_draw_tool_pause (draw_tool);

  if (picman_free_select_tool_is_point_grabbed (fst))
    {
      picman_free_select_tool_prepare_for_move (fst);
    }
  else
    {
      PicmanVector2 point_to_add;

      /* Note that we add the pending point (unless it is the first
       * point we add) because the pending point is setup correctly
       * with regards to angle constraints.
       */
      if (priv->n_points > 0)
        {
          point_to_add = priv->pending_point;
        }
      else
        {
          point_to_add.x = coords->x;
          point_to_add.y = coords->y;
        }

      /* No point was grabbed, add a new point and mark this as a
       * segment divider. For a line segment, this will be the only
       * new point. For a free segment, this will be the first point
       * of the free segment.
       */
      picman_free_select_tool_add_point (fst,
                                       point_to_add.x,
                                       point_to_add.y);
      picman_free_select_tool_add_segment_index (fst,
                                               priv->n_points - 1);
    }

  picman_draw_tool_resume (draw_tool);
}

static void
picman_free_select_tool_button_release (PicmanTool              *tool,
                                      const PicmanCoords      *coords,
                                      guint32                time,
                                      GdkModifierType        state,
                                      PicmanButtonReleaseType  release_type,
                                      PicmanDisplay           *display)
{
  PicmanFreeSelectTool        *fst  = PICMAN_FREE_SELECT_TOOL (tool);
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  picman_tool_control_halt (tool->control);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (fst));

  switch (release_type)
    {
    case PICMAN_BUTTON_RELEASE_CLICK:
    case PICMAN_BUTTON_RELEASE_NO_MOTION:
      /* If a click was made, we don't consider the polygon modified */
      priv->polygon_modified = FALSE;

      picman_free_select_tool_handle_click (fst,
                                          coords,
                                          time,
                                          display);
      break;

    case PICMAN_BUTTON_RELEASE_NORMAL:
      picman_free_select_tool_handle_normal_release (fst,
                                                   coords,
                                                   display);
      break;

    case PICMAN_BUTTON_RELEASE_CANCEL:
      picman_free_select_tool_handle_cancel (fst);
      break;

    default:
      break;
    }

  /* Reset */
  priv->polygon_modified = FALSE;

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (fst));
}

static void
picman_free_select_tool_motion (PicmanTool         *tool,
                              const PicmanCoords *coords,
                              guint32           time,
                              GdkModifierType   state,
                              PicmanDisplay      *display)
{
  PicmanFreeSelectTool        *fst       = PICMAN_FREE_SELECT_TOOL (tool);
  PicmanFreeSelectToolPrivate *priv      = GET_PRIVATE (fst);
  PicmanDrawTool              *draw_tool = PICMAN_DRAW_TOOL (tool);

  picman_draw_tool_pause (draw_tool);

  priv->last_coords.x = coords->x;
  priv->last_coords.y = coords->y;

  picman_free_select_tool_update_motion (fst,
                                       coords->x,
                                       coords->y);

  picman_draw_tool_resume (draw_tool);
}

static gboolean
picman_free_select_tool_key_press (PicmanTool    *tool,
                                 GdkEventKey *kevent,
                                 PicmanDisplay *display)
{
  PicmanFreeSelectTool *fst = PICMAN_FREE_SELECT_TOOL (tool);

  switch (kevent->keyval)
    {
    case GDK_KEY_BackSpace:
      picman_free_select_tool_remove_last_segment (fst);
      return TRUE;

    case GDK_KEY_Return:
    case GDK_KEY_KP_Enter:
    case GDK_KEY_ISO_Enter:
      picman_free_select_tool_commit (fst, display);
      return TRUE;

    case GDK_KEY_Escape:
      picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);
      return TRUE;

    default:
      break;
    }

  return FALSE;
}

static void
picman_free_select_tool_modifier_key (PicmanTool        *tool,
                                    GdkModifierType  key,
                                    gboolean         press,
                                    GdkModifierType  state,
                                    PicmanDisplay     *display)
{
  PicmanDrawTool              *draw_tool = PICMAN_DRAW_TOOL (tool);
  PicmanFreeSelectToolPrivate *priv      = GET_PRIVATE (tool);

  if (tool->display == display)
    {
      picman_draw_tool_pause (draw_tool);

      priv->constrain_angle = ((state & picman_get_constrain_behavior_mask ()) ?
                               TRUE : FALSE);

      priv->supress_handles = state & GDK_SHIFT_MASK ? TRUE : FALSE;

      picman_draw_tool_resume (draw_tool);
    }

  PICMAN_TOOL_CLASS (parent_class)->modifier_key (tool,
                                                key,
                                                press,
                                                state,
                                                display);
}

static void
picman_free_select_tool_active_modifier_key (PicmanTool        *tool,
                                           GdkModifierType  key,
                                           gboolean         press,
                                           GdkModifierType  state,
                                           PicmanDisplay     *display)
{
  PicmanDrawTool              *draw_tool = PICMAN_DRAW_TOOL (tool);
  PicmanFreeSelectToolPrivate *priv      = GET_PRIVATE (tool);

  if (tool->display != display)
    return;

  picman_draw_tool_pause (draw_tool);

  priv->constrain_angle = ((state & picman_get_constrain_behavior_mask ()) ?
                           TRUE : FALSE);

  /* If we didn't came here due to a mouse release, immediately update
   * the position of the thing we move.
   */
  if (state & GDK_BUTTON1_MASK)
    {
      picman_free_select_tool_update_motion (PICMAN_FREE_SELECT_TOOL (tool),
                                           priv->last_coords.x,
                                           priv->last_coords.y);
    }

  picman_draw_tool_resume (draw_tool);

  PICMAN_TOOL_CLASS (parent_class)->active_modifier_key (tool,
                                                       key,
                                                       press,
                                                       state,
                                                       display);
}

/**
 * picman_free_select_tool_draw:
 * @draw_tool:
 *
 * Draw the line segments and handles around segment vertices, the
 * latter if the they are in proximity to cursor.
 **/
static void
picman_free_select_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanFreeSelectTool        *fst                   = PICMAN_FREE_SELECT_TOOL (draw_tool);
  PicmanFreeSelectToolPrivate *priv                  = GET_PRIVATE (fst);
  PicmanTool                  *tool                  = PICMAN_TOOL (draw_tool);
  PicmanCanvasGroup           *stroke_group;
  gboolean                   hovering_first_point  = FALSE;
  gboolean                   handles_wants_to_show = FALSE;
  PicmanCoords                 coords                = { priv->last_coords.x,
                                                       priv->last_coords.y,
                                                       /* pad with 0 */ };
  if (! tool->display)
    return;

  hovering_first_point =
    picman_free_select_tool_should_close (fst,
                                        tool->display,
                                        NO_CLICK_TIME_AVAILABLE,
                                        &coords);

  stroke_group = picman_draw_tool_add_stroke_group (draw_tool);

  picman_draw_tool_push_group (draw_tool, stroke_group);
  picman_draw_tool_add_lines (draw_tool,
                            priv->points, priv->n_points,
                            FALSE);
  picman_draw_tool_pop_group (draw_tool);

  /* We always show the handle for the first point, even with button1
   * down, since releasing the button on the first point will close
   * the polygon, so it's a significant state which we must give
   * feedback for
   */
  handles_wants_to_show = (hovering_first_point ||
                           ! picman_tool_control_is_active (tool->control));

  if (handles_wants_to_show &&
      ! priv->supress_handles)
    {
      gint i = 0;
      gint n = 0;

      /* If the first point is hovered while button1 is held down,
       * only draw the first handle, the other handles are not
       * relevant (see comment a few lines up)
       */
      if (picman_tool_control_is_active (tool->control) && hovering_first_point)
        n = MIN (priv->n_segment_indices, 1);
      else
        n = priv->n_segment_indices;

      for (i = 0; i < n; i++)
        {
          PicmanVector2   *point       = NULL;
          gdouble        dist        = 0.0;
          PicmanHandleType handle_type = -1;

          point = &priv->points[priv->segment_indices[i]];

          dist  = picman_draw_tool_calc_distance_square (draw_tool,
                                                       tool->display,
                                                       priv->last_coords.x,
                                                       priv->last_coords.y,
                                                       point->x,
                                                       point->y);

          /* If the cursor is over the point, fill, if it's just
           * close, draw an outline
           */
          if (dist < POINT_GRAB_THRESHOLD_SQ)
            handle_type = PICMAN_HANDLE_FILLED_CIRCLE;
          else if (dist < POINT_SHOW_THRESHOLD_SQ)
            handle_type = PICMAN_HANDLE_CIRCLE;

          if (handle_type != -1)
            {
              PicmanCanvasItem *item;

              item = picman_draw_tool_add_handle (draw_tool, handle_type,
                                                point->x,
                                                point->y,
                                                PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                                PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                                PICMAN_HANDLE_ANCHOR_CENTER);

              if (dist < POINT_GRAB_THRESHOLD_SQ)
                picman_canvas_item_set_highlight (item, TRUE);
            }
        }
    }

  if (priv->show_pending_point)
    {
      PicmanVector2 last = priv->points[priv->n_points - 1];

      picman_draw_tool_push_group (draw_tool, stroke_group);
      picman_draw_tool_add_line (draw_tool,
                               last.x,
                               last.y,
                               priv->pending_point.x,
                               priv->pending_point.y);
      picman_draw_tool_pop_group (draw_tool);
    }
}

static void
picman_free_select_tool_real_select (PicmanFreeSelectTool *fst,
                                   PicmanDisplay        *display)
{
  PicmanSelectionOptions      *options = PICMAN_SELECTION_TOOL_GET_OPTIONS (fst);
  PicmanFreeSelectToolPrivate *priv    = GET_PRIVATE (fst);
  PicmanImage                 *image   = picman_display_get_image (display);

  picman_channel_select_polygon (picman_image_get_mask (image),
                               C_("command", "Free Select"),
                               priv->n_points,
                               priv->points,
                               priv->operation_at_start,
                               options->antialias,
                               options->feather,
                               options->feather_radius,
                               options->feather_radius,
                               TRUE);

  picman_tool_control (PICMAN_TOOL (fst), PICMAN_TOOL_ACTION_HALT, display);
}

void
picman_free_select_tool_get_points (PicmanFreeSelectTool  *fst,
                                  const PicmanVector2  **points,
                                  gint                *n_points)
{
  PicmanFreeSelectToolPrivate *priv = GET_PRIVATE (fst);

  g_return_if_fail (points != NULL && n_points != NULL);

  *points   = priv->points;
  *n_points = priv->n_points;
}
