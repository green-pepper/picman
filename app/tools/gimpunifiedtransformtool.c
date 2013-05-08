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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picmanchannel.h"
#include "core/picman-transform-utils.h"
#include "core/picmanimage.h"
#include "core/picmandrawable-transform.h"
#include "core/picman-utils.h"

#include "vectors/picmanvectors.h"
#include "vectors/picmanstroke.h"

#include "widgets/picmanhelp-ids.h"

#include "display/picmancanvasgroup.h"
#include "display/picmancanvashandle.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-transform.h"

#include "picmanunifiedtransformtool.h"
#include "picmantoolcontrol.h"
#include "picmantransformoptions.h"

#include "picman-intl.h"


/*  index into trans_info array  */
enum
{
  X0,
  Y0,
  X1,
  Y1,
  X2,
  Y2,
  X3,
  Y3,
  PIVOT_X,
  PIVOT_Y,
};


/*  local function prototypes  */

static void            picman_unified_transform_tool_dialog        (PicmanTransformTool *tr_tool);
static void            picman_unified_transform_tool_dialog_update (PicmanTransformTool *tr_tool);
static void            picman_unified_transform_tool_prepare       (PicmanTransformTool *tr_tool);
static void            picman_unified_transform_tool_motion        (PicmanTransformTool *tr_tool);
static void            picman_unified_transform_tool_recalc_matrix (PicmanTransformTool *tr_tool);
static gchar *         picman_unified_transform_tool_get_undo_desc (PicmanTransformTool *tr_tool);
static TransformAction picman_unified_transform_tool_pick_function (PicmanTransformTool *tr_tool,
                                                                  const PicmanCoords  *coords,
                                                                  GdkModifierType    state,
                                                                  PicmanDisplay       *display);
static void            picman_unified_transform_tool_cursor_update (PicmanTransformTool  *tr_tool,
                                                                  PicmanCursorType     *cursor,
                                                                  PicmanCursorModifier *modifier);
static void            picman_unified_transform_tool_draw_gui      (PicmanTransformTool *tr_tool,
                                                                  gint               handle_w,
                                                                  gint               handle_h);


G_DEFINE_TYPE (PicmanUnifiedTransformTool, picman_unified_transform_tool,
               PICMAN_TYPE_TRANSFORM_TOOL)


void
picman_unified_transform_tool_register (PicmanToolRegisterCallback  callback,
                                      gpointer                  data)
{
  (* callback) (PICMAN_TYPE_UNIFIED_TRANSFORM_TOOL,
                PICMAN_TYPE_TRANSFORM_OPTIONS,
                picman_transform_options_gui,
                PICMAN_CONTEXT_BACKGROUND_MASK,
                "picman-unified-transform-tool",
                _("Unified Transform"),
                _("Unified Transform Tool: "
                  "Transform the layer, selection or path"),
                N_("_Unified Transform"), "<shift>L",
                NULL, PICMAN_HELP_TOOL_UNIFIED_TRANSFORM,
                PICMAN_STOCK_TOOL_UNIFIED_TRANSFORM,
                data);
}

static void
picman_unified_transform_tool_class_init (PicmanUnifiedTransformToolClass *klass)
{
  PicmanTransformToolClass *trans_class = PICMAN_TRANSFORM_TOOL_CLASS (klass);

  trans_class->dialog        = picman_unified_transform_tool_dialog;
  trans_class->dialog_update = picman_unified_transform_tool_dialog_update;
  trans_class->prepare       = picman_unified_transform_tool_prepare;
  trans_class->motion        = picman_unified_transform_tool_motion;
  trans_class->recalc_matrix = picman_unified_transform_tool_recalc_matrix;
  trans_class->get_undo_desc = picman_unified_transform_tool_get_undo_desc;
  trans_class->pick_function = picman_unified_transform_tool_pick_function;
  trans_class->cursor_update = picman_unified_transform_tool_cursor_update;
  trans_class->draw_gui      = picman_unified_transform_tool_draw_gui;
}

static void
picman_unified_transform_tool_init (PicmanUnifiedTransformTool *unified_tool)
{
  PicmanTransformTool *tr_tool = PICMAN_TRANSFORM_TOOL (unified_tool);

  tr_tool->progress_text = _("Unified transform");

  tr_tool->use_grid    = TRUE;
  tr_tool->use_handles = TRUE;
}

static gboolean
transform_is_convex (PicmanVector2 *pos)
{
  return picman_transform_polygon_is_convex (pos[0].x, pos[0].y,
                                           pos[1].x, pos[1].y,
                                           pos[2].x, pos[2].y,
                                           pos[3].x, pos[3].y);
}

static inline gdouble
dotprod (PicmanVector2 a,
         PicmanVector2 b)
{
  return a.x * b.x + a.y * b.y;
}

static inline gdouble
norm (PicmanVector2 a)
{
  return sqrt (dotprod (a, a));
}

static inline PicmanVector2
vectorsubtract (PicmanVector2 a,
                PicmanVector2 b)
{
  PicmanVector2 c;

  c.x = a.x - b.x;
  c.y = a.y - b.y;

  return c;
}

static inline PicmanVector2
vectoradd (PicmanVector2 a,
           PicmanVector2 b)
{
  PicmanVector2 c;

  c.x = a.x + b.x;
  c.y = a.y + b.y;

  return c;
}

static inline PicmanVector2
scalemult (PicmanVector2 a,
           gdouble     b)
{
  PicmanVector2 c;

  c.x = a.x * b;
  c.y = a.y * b;

  return c;
}

static inline PicmanVector2
vectorproject (PicmanVector2 a,
               PicmanVector2 b)
{
  return scalemult (b, dotprod (a, b) / dotprod (b, b));
}

/* finds the clockwise angle between the vectors given, 0-2π */
static inline gdouble
calcangle (PicmanVector2 a,
           PicmanVector2 b)
{
  gdouble angle, angle2;
  gdouble length = norm (a) * norm (b);

  angle = acos (dotprod (a, b)/length);
  angle2 = b.y;
  b.y = -b.x;
  b.x = angle2;
  angle2 = acos (dotprod (a, b)/length);

  return ((angle2 > G_PI/2.) ? angle : 2*G_PI-angle);
}

static inline PicmanVector2
rotate2d (PicmanVector2 p,
          gdouble     angle)
{
  PicmanVector2 ret;

  ret.x = cos (angle) * p.x-sin (angle) * p.y;
  ret.y = sin (angle) * p.x+cos (angle) * p.y;

  return ret;
}

static inline PicmanVector2
lineintersect (PicmanVector2 p1, PicmanVector2 p2,
               PicmanVector2 q1, PicmanVector2 q2)
{
  gdouble     denom, u;
  PicmanVector2 p;

  denom = (q2.y - q1.y) * (p2.x - p1.x) - (q2.x - q1.x) * (p2.y - p1.y);
  if (denom == 0.0)
    {
      p.x = (p1.x + p2.x + q1.x + q2.x) / 4;
      p.y = (p1.y + p2.y + q1.y + q2.y) / 4;
    }
  else
    {
      u = (q2.x - q1.x) * (p1.y - q1.y) - (q2.y - q1.y) * (p1.x - q1.x);
      u /= denom;

      p.x = p1.x + u * (p2.x - p1.x);
      p.y = p1.y + u * (p2.y - p1.y);
    }

  return p;
}

static inline PicmanVector2
getpivotdelta (PicmanTransformTool *tr_tool,
               PicmanVector2       *oldpos,
               PicmanVector2       *newpos,
               PicmanVector2        pivot)
{
  PicmanMatrix3 transform_before, transform_after;
  PicmanVector2 delta;

  picman_matrix3_identity (&transform_before);
  picman_matrix3_identity (&transform_after);

  picman_transform_matrix_perspective (&transform_before,
                                     tr_tool->x1,
                                     tr_tool->y1,
                                     tr_tool->x2 - tr_tool->x1,
                                     tr_tool->y2 - tr_tool->y1,
                                     oldpos[0].x, oldpos[0].y,
                                     oldpos[1].x, oldpos[1].y,
                                     oldpos[2].x, oldpos[2].y,
                                     oldpos[3].x, oldpos[3].y);
  picman_transform_matrix_perspective (&transform_after,
                                     tr_tool->x1,
                                     tr_tool->y1,
                                     tr_tool->x2 - tr_tool->x1,
                                     tr_tool->y2 - tr_tool->y1,
                                     newpos[0].x, newpos[0].y,
                                     newpos[1].x, newpos[1].y,
                                     newpos[2].x, newpos[2].y,
                                     newpos[3].x, newpos[3].y);
  picman_matrix3_invert (&transform_before);
  picman_matrix3_mult (&transform_after, &transform_before);
  picman_matrix3_transform_point (&transform_before,
                                pivot.x, pivot.y, &delta.x, &delta.y);

  delta = vectorsubtract (delta, pivot);

  return delta;
}

static gboolean
point_is_inside_polygon (gint n, gdouble *x, gdouble *y, gdouble px, gdouble py)
{
  int i, j;
  gboolean odd = FALSE;

  for (i = 0, j = n - 1; i < n; j = i++)
    {
      if ((y[i] < py && y[j] >= py) ||
          (y[j] < py && y[i] >= py))
        {
          if (x[i] + (py - y[i]) / (y[j] - y[i]) * (x[j] - x[i]) < px)
            odd = !odd;
        }
    }

  return odd;
}

static gboolean
point_is_inside_polygon_pos (PicmanVector2 *pos, PicmanVector2 point)
{
  return point_is_inside_polygon (4,
                                  (gdouble[4]){ pos[0].x, pos[1].x,
                                                pos[3].x, pos[2].x },
                                  (gdouble[4]){ pos[0].y, pos[1].y,
                                                pos[3].y, pos[2].y },
                                  point.x, point.y);
}

static gchar*
get_friendly_operation_name (TransformAction op)
{
  switch (op)
    {
    case TRANSFORM_HANDLE_NONE:
      return "";
    case TRANSFORM_HANDLE_NW_P:
    case TRANSFORM_HANDLE_NE_P:
    case TRANSFORM_HANDLE_SW_P:
    case TRANSFORM_HANDLE_SE_P:
      return "Change perspective";
    case TRANSFORM_HANDLE_NW:
    case TRANSFORM_HANDLE_NE:
    case TRANSFORM_HANDLE_SW:
    case TRANSFORM_HANDLE_SE:
      return "Scale";
    case TRANSFORM_HANDLE_N:
    case TRANSFORM_HANDLE_S:
    case TRANSFORM_HANDLE_E:
    case TRANSFORM_HANDLE_W:
      return "Scale";
    case TRANSFORM_HANDLE_CENTER:
      return "Move";
    case TRANSFORM_HANDLE_PIVOT:
      return "Move pivot";
    case TRANSFORM_HANDLE_N_S:
    case TRANSFORM_HANDLE_S_S:
    case TRANSFORM_HANDLE_E_S:
    case TRANSFORM_HANDLE_W_S:
      return "Shear";
    case TRANSFORM_HANDLE_ROTATION:
      return "Rotate";
    default:
      g_assert_not_reached();
    }
}

static TransformAction
picman_unified_transform_tool_pick_function (PicmanTransformTool *tr_tool,
                                           const PicmanCoords  *coords,
                                           GdkModifierType    state,
                                           PicmanDisplay       *display)
{
  PicmanTool        *tool = PICMAN_TOOL (tr_tool);
  TransformAction  ret = TRANSFORM_HANDLE_NONE;
  TransformAction  i;

  for (i = TRANSFORM_HANDLE_NONE + 1; i < TRANSFORM_HANDLE_NUM; i++)
    {
      if (tr_tool->handles[i] &&
          picman_canvas_item_hit (tr_tool->handles[i], coords->x, coords->y))
        {
          ret = i;
          break;
        }
    }

  if (ret == TRANSFORM_HANDLE_NONE)
    {
      /* points passed in clockwise order */
      if (point_is_inside_polygon (4,
                                   (gdouble[4]){ tr_tool->tx1, tr_tool->tx2,
                                                 tr_tool->tx4, tr_tool->tx3 },
                                   (gdouble[4]){ tr_tool->ty1, tr_tool->ty2,
                                                 tr_tool->ty4, tr_tool->ty3 },
                                   coords->x, coords->y))
        ret = TRANSFORM_HANDLE_CENTER;
      else
        ret = TRANSFORM_HANDLE_ROTATION;
    }

  picman_tool_pop_status (tool, tool->display);

  if (ret != TRANSFORM_HANDLE_NONE)
    picman_tool_push_status (tool, tool->display, "%s",
                           get_friendly_operation_name (ret));

  return ret;
}

static void
gethandlegeometry (PicmanTransformTool *tr_tool,
                   PicmanVector2       *position,
                   gdouble           *angle)
{
  PicmanVector2      o[] = { { .x = tr_tool->tx1, .y = tr_tool->ty1 },
                           { .x = tr_tool->tx2, .y = tr_tool->ty2 },
                           { .x = tr_tool->tx3, .y = tr_tool->ty3 },
                           { .x = tr_tool->tx4, .y = tr_tool->ty4 } },
                   right = { .x = 1.0, .y = 0.0 },
                   up =    { .x = 0.0, .y = 1.0 };

  if (position)
    {
      position[0] = o[0];
      position[1] = o[1];
      position[2] = o[2];
      position[3] = o[3];
    }

  angle[0] = calcangle (vectorsubtract (o[1], o[0]), right);
  angle[1] = calcangle (vectorsubtract (o[3], o[2]), right);
  angle[2] = calcangle (vectorsubtract (o[3], o[1]), up);
  angle[3] = calcangle (vectorsubtract (o[2], o[0]), up);

  angle[4] = (angle[0] + angle[3]) / 2.0;
  angle[5] = (angle[0] + angle[2]) / 2.0;
  angle[6] = (angle[1] + angle[3]) / 2.0;
  angle[7] = (angle[1] + angle[2]) / 2.0;
}

static void
picman_unified_transform_tool_cursor_update (PicmanTransformTool  *tr_tool,
                                           PicmanCursorType     *cursor,
                                           PicmanCursorModifier *modifier)
{
  PicmanToolCursorType toolcursor = PICMAN_TOOL_CURSOR_NONE;
  gdouble angle[8];
  gint i;
  PicmanCursorType map[8];
  PicmanVector2 pos[4], this, that;
  gboolean flip = FALSE, side = FALSE, setcursor = TRUE;

  map[0] = PICMAN_CURSOR_CORNER_TOP_LEFT;
  map[1] = PICMAN_CURSOR_CORNER_TOP;
  map[2] = PICMAN_CURSOR_CORNER_TOP_RIGHT;
  map[3] = PICMAN_CURSOR_CORNER_RIGHT;
  map[4] = PICMAN_CURSOR_CORNER_BOTTOM_RIGHT;
  map[5] = PICMAN_CURSOR_CORNER_BOTTOM;
  map[6] = PICMAN_CURSOR_CORNER_BOTTOM_LEFT;
  map[7] = PICMAN_CURSOR_CORNER_LEFT;

  gethandlegeometry (tr_tool, pos, angle);

  for (i = 0; i < 8; i++)
    angle[i] = round(angle[i] * 180. / G_PI / 45.);

  switch (tr_tool->function)
    {
    case TRANSFORM_HANDLE_NW_P:
    case TRANSFORM_HANDLE_NW:
      i = (gint)angle[4] + 0;
      this = pos[0];
      that = pos[3];
      break;

    case TRANSFORM_HANDLE_NE_P:
    case TRANSFORM_HANDLE_NE:
      i = (gint)angle[5] + 2;
      this = pos[1];
      that = pos[2];
      break;

    case TRANSFORM_HANDLE_SW_P:
    case TRANSFORM_HANDLE_SW:
      i = (gint)angle[6] + 6;
      this = pos[2];
      that = pos[1];
      break;

    case TRANSFORM_HANDLE_SE_P:
    case TRANSFORM_HANDLE_SE:
      i = (gint)angle[7] + 4;
      this = pos[3];
      that = pos[0];
      break;

    case TRANSFORM_HANDLE_N:
    case TRANSFORM_HANDLE_N_S:
      i = (gint)angle[0] + 1;
      this = vectoradd (pos[0], pos[1]);
      that = vectoradd (pos[2], pos[3]);
      side = TRUE;
      break;

    case TRANSFORM_HANDLE_S:
    case TRANSFORM_HANDLE_S_S:
      i = (gint)angle[1] + 5;
      this = vectoradd (pos[2], pos[3]);
      that = vectoradd (pos[0], pos[1]);
      side = TRUE;
      break;

    case TRANSFORM_HANDLE_E:
    case TRANSFORM_HANDLE_E_S:
      i = (gint)angle[2] + 3;
      this = vectoradd (pos[1], pos[3]);
      that = vectoradd (pos[0], pos[2]);
      side = TRUE;
      break;

    case TRANSFORM_HANDLE_W:
    case TRANSFORM_HANDLE_W_S:
      i = (gint)angle[3] + 7;
      this = vectoradd (pos[0], pos[2]);
      that = vectoradd (pos[1], pos[3]);
      side = TRUE;
      break;

    default:
      setcursor = FALSE;
      break;
    }

  if (setcursor)
    {
      i %= 8;

      switch (map[i])
        {
        case PICMAN_CURSOR_CORNER_TOP_LEFT:
          if (this.x + this.y > that.x + that.y)
            flip = TRUE;
          break;
        case PICMAN_CURSOR_CORNER_TOP:
          if (this.y > that.y)
            flip = TRUE;
          break;
        case PICMAN_CURSOR_CORNER_TOP_RIGHT:
          if (this.x - this.y < that.x - that.y)
            flip = TRUE;
          break;
        case PICMAN_CURSOR_CORNER_RIGHT:
          if (this.x < that.x)
            flip = TRUE;
          break;
        case PICMAN_CURSOR_CORNER_BOTTOM_RIGHT:
          if (this.x + this.y < that.x + that.y)
            flip = TRUE;
          break;
        case PICMAN_CURSOR_CORNER_BOTTOM:
          if (this.y < that.y)
            flip = TRUE;
          break;
        case PICMAN_CURSOR_CORNER_BOTTOM_LEFT:
          if (this.x - this.y > that.x - that.y)
            flip = TRUE;
          break;
        case PICMAN_CURSOR_CORNER_LEFT:
          if (this.x > that.x)
            flip = TRUE;
          break;
        default:
          g_assert_not_reached ();
        }
      if (flip)
        *cursor = map[(i + 4) % 8];
      else
        *cursor = map[i];
      if (side)
        *cursor += 8;
    }

  /* parent class handles *cursor and *modifier for most handles */
  switch (tr_tool->function)
    {
    case TRANSFORM_HANDLE_NONE:
    case TRANSFORM_CREATING:
      toolcursor = PICMAN_TOOL_CURSOR_NONE;
      break;
    case TRANSFORM_HANDLE_NW_P:
    case TRANSFORM_HANDLE_NE_P:
    case TRANSFORM_HANDLE_SW_P:
    case TRANSFORM_HANDLE_SE_P:
      toolcursor = PICMAN_TOOL_CURSOR_PERSPECTIVE;
      break;
    case TRANSFORM_HANDLE_NW:
    case TRANSFORM_HANDLE_NE:
    case TRANSFORM_HANDLE_SW:
    case TRANSFORM_HANDLE_SE:
    case TRANSFORM_HANDLE_N:
    case TRANSFORM_HANDLE_S:
    case TRANSFORM_HANDLE_E:
    case TRANSFORM_HANDLE_W:
      toolcursor = PICMAN_TOOL_CURSOR_RESIZE;
      break;
    case TRANSFORM_HANDLE_CENTER:
      toolcursor = PICMAN_TOOL_CURSOR_MOVE;
      break;
    case TRANSFORM_HANDLE_PIVOT:
      toolcursor = PICMAN_TOOL_CURSOR_ROTATE;
      *modifier = PICMAN_CURSOR_MODIFIER_MOVE;
      break;
    case TRANSFORM_HANDLE_N_S:
    case TRANSFORM_HANDLE_S_S:
    case TRANSFORM_HANDLE_E_S:
    case TRANSFORM_HANDLE_W_S:
      toolcursor = PICMAN_TOOL_CURSOR_SHEAR;
      break;
    case TRANSFORM_HANDLE_ROTATION:
      toolcursor = PICMAN_TOOL_CURSOR_ROTATE;
      break;
    default:
      g_assert_not_reached();
    }

  /* parent class sets cursor and cursor_modifier */
  picman_tool_control_set_tool_cursor (PICMAN_TOOL (tr_tool)->control, toolcursor);
}

static void
picman_unified_transform_tool_draw_gui (PicmanTransformTool *tr_tool,
                                      gint               handle_w,
                                      gint               handle_h)
{
  PicmanDrawTool    *draw_tool = PICMAN_DRAW_TOOL (tr_tool);
  PicmanCanvasGroup *stroke_group;
  gint             d, i;
  gdouble          angle[8];
  PicmanVector2      o[4], t[4];

  gethandlegeometry (tr_tool, o, angle);

  for (i = 0; i < 4; i++)
    {
      PicmanCanvasItem *h;

      /*  draw the scale handles  */
      h = picman_draw_tool_add_handle (draw_tool,
                                     PICMAN_HANDLE_SQUARE,
                                     o[i].x, o[i].y,
                                     handle_w * 1.5, handle_h * 1.5,
                                     PICMAN_HANDLE_ANCHOR_CENTER);
      picman_canvas_handle_set_angles (h, angle[i + 4], 0.0);
      tr_tool->handles[TRANSFORM_HANDLE_NW + i] = h;

      /*  draw the perspective handles  */
      h = picman_draw_tool_add_handle (draw_tool,
                                     PICMAN_HANDLE_DIAMOND,
                                     o[i].x, o[i].y,
                                     handle_w * 0.8, handle_h * 0.8,
                                     PICMAN_HANDLE_ANCHOR_CENTER);
      picman_canvas_handle_set_angles (h, angle[i + 4], 0.0);
      tr_tool->handles[TRANSFORM_HANDLE_NW_P + i] = h;
    }

  /*  draw the side handles  */
  t[0] = scalemult (vectoradd (o[0], o[1]), 0.5);
  t[1] = scalemult (vectoradd (o[2], o[3]), 0.5);
  t[2] = scalemult (vectoradd (o[1], o[3]), 0.5);
  t[3] = scalemult (vectoradd (o[2], o[0]), 0.5);

  for (i = 0; i < 4; i++)
    {
      PicmanCanvasItem *h;

      h = picman_draw_tool_add_handle (draw_tool,
                                     PICMAN_HANDLE_SQUARE,
                                     t[i].x, t[i].y,
                                     handle_w, handle_h,
                                     PICMAN_HANDLE_ANCHOR_CENTER);
      picman_canvas_handle_set_angles (h, angle[i], 0.0);
      tr_tool->handles[TRANSFORM_HANDLE_N + i] = h;
    }

  /*  draw the shear handles  */
  t[0] = scalemult (vectoradd (           o[0]      , scalemult (o[1], 3.0)), 0.25);
  t[1] = scalemult (vectoradd (scalemult (o[2], 3.0),            o[3]      ), 0.25);
  t[2] = scalemult (vectoradd (           o[1]      , scalemult (o[3], 3.0)), 0.25);
  t[3] = scalemult (vectoradd (scalemult (o[2], 3.0),            o[0]      ), 0.25);

  for (i = 0; i < 4; i++)
    {
      PicmanCanvasItem *h;

      h = picman_draw_tool_add_handle (draw_tool,
                                     PICMAN_HANDLE_FILLED_DIAMOND,
                                     t[i].x, t[i].y,
                                     handle_w, handle_h,
                                     PICMAN_HANDLE_ANCHOR_CENTER);
      picman_canvas_handle_set_angles (h, angle[i], 0.0);
      tr_tool->handles[TRANSFORM_HANDLE_N_S + i] = h;
    }

  /*  draw the rotation center axis handle  */
  d = MIN (handle_w, handle_h);

  stroke_group = picman_draw_tool_add_stroke_group (draw_tool);

  tr_tool->handles[TRANSFORM_HANDLE_PIVOT] = PICMAN_CANVAS_ITEM (stroke_group);

  picman_draw_tool_push_group (draw_tool, stroke_group);

  picman_draw_tool_add_handle (draw_tool,
                             PICMAN_HANDLE_CIRCLE,
                             tr_tool->tpx, tr_tool->tpy,
                             d, d,
                             PICMAN_HANDLE_ANCHOR_CENTER);
  picman_draw_tool_add_handle (draw_tool,
                             PICMAN_HANDLE_CROSS,
                             tr_tool->tpx, tr_tool->tpy,
                             d, d,
                             PICMAN_HANDLE_ANCHOR_CENTER);

  picman_draw_tool_pop_group (draw_tool);

  /* draw an item at 40,80 in screen coordinates */
  //gint x, y;
  //picman_display_shell_untransform_xy (picman_display_get_shell (tool->display),
  //                                 40, 80, &x, &y, TRUE);
  //picman_draw_tool_add_handle (draw_tool,
  //                           PICMAN_HANDLE_SQUARE,
  //                           x, y,
  //                           5, 5,
  //                           PICMAN_HANDLE_ANCHOR_CENTER);
}

static void
picman_unified_transform_tool_dialog (PicmanTransformTool *tr_tool)
{
  PicmanUnifiedTransformTool *unified = PICMAN_UNIFIED_TRANSFORM_TOOL (tr_tool);
  GtkWidget                *content_area;
  GtkWidget                *frame;
  GtkWidget                *table;
  gint                      x, y;

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (tr_tool->dialog));

  frame = picman_frame_new (_("Transform Matrix"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 6);
  gtk_box_pack_start (GTK_BOX (content_area), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  for (y = 0; y < 3; y++)
    for (x = 0; x < 3; x++)
      {
        GtkWidget *label = gtk_label_new (" ");

        gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.0);
        gtk_label_set_width_chars (GTK_LABEL (label), 12);
        gtk_table_attach (GTK_TABLE (table), label,
                          x, x + 1, y, y + 1, GTK_EXPAND, GTK_FILL, 0, 0);
        gtk_widget_show (label);

        unified->label[y][x] = label;
      }
}

static void
picman_unified_transform_tool_dialog_update (PicmanTransformTool *tr_tool)
{
  PicmanUnifiedTransformTool *unified = PICMAN_UNIFIED_TRANSFORM_TOOL (tr_tool);
  gint                      x, y;

  for (y = 0; y < 3; y++)
    for (x = 0; x < 3; x++)
      {
        gchar buf[32];

        g_snprintf (buf, sizeof (buf),
                    "%10.5f", tr_tool->transform.coeff[y][x]);

        gtk_label_set_text (GTK_LABEL (unified->label[y][x]), buf);
      }
}

static void
picman_unified_transform_tool_prepare (PicmanTransformTool *tr_tool)
{
  tr_tool->trans_info[PIVOT_X] = (gdouble) (tr_tool->x1 + tr_tool->x2) / 2.0;
  tr_tool->trans_info[PIVOT_Y] = (gdouble) (tr_tool->y1 + tr_tool->y2) / 2.0;

  tr_tool->trans_info[X0] = (gdouble) tr_tool->x1;
  tr_tool->trans_info[Y0] = (gdouble) tr_tool->y1;
  tr_tool->trans_info[X1] = (gdouble) tr_tool->x2;
  tr_tool->trans_info[Y1] = (gdouble) tr_tool->y1;
  tr_tool->trans_info[X2] = (gdouble) tr_tool->x1;
  tr_tool->trans_info[Y2] = (gdouble) tr_tool->y2;
  tr_tool->trans_info[X3] = (gdouble) tr_tool->x2;
  tr_tool->trans_info[Y3] = (gdouble) tr_tool->y2;
}

static void
picman_unified_transform_tool_motion (PicmanTransformTool *transform_tool)
{
  PicmanTransformOptions *options = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (transform_tool);
  gdouble              *x[4], *y[4];
  gdouble              *newpivot_x, *newpivot_y;

  PicmanVector2           oldpos[5], newpos[4];
  PicmanVector2           cur   = { .x = transform_tool->curx,
                                  .y = transform_tool->cury };
  PicmanVector2           mouse = { .x = transform_tool->mousex,
                                  .y = transform_tool->mousey };
  PicmanVector2           d;
  PicmanVector2           pivot;

  gint                  i;

  gboolean              fixedpivot = options->fixedpivot;

  TransformAction       function = transform_tool->function;

  for (i = 0; i < 4; i++)
    {
      x[i] = &transform_tool->trans_info[X0+i*2];
      y[i] = &transform_tool->trans_info[Y0+i*2];
      newpos[i].x = oldpos[i].x = transform_tool->prev_trans_info[0][X0+i*2];
      newpos[i].y = oldpos[i].y = transform_tool->prev_trans_info[0][Y0+i*2];
    }

  /* put center point in this array too */
  oldpos[4].x = (oldpos[0].x + oldpos[1].x + oldpos[2].x + oldpos[3].x) / 4.;
  oldpos[4].y = (oldpos[0].y + oldpos[1].y + oldpos[2].y + oldpos[3].y) / 4.;

  d = vectorsubtract (cur, mouse);

  newpivot_x = &transform_tool->trans_info[PIVOT_X];
  newpivot_y = &transform_tool->trans_info[PIVOT_Y];

  pivot.x = transform_tool->prev_trans_info[0][PIVOT_X];
  pivot.y = transform_tool->prev_trans_info[0][PIVOT_Y];

  /* move */
  if (function == TRANSFORM_HANDLE_CENTER)
    {
      if (options->constrain_move)
        {
          /* snap to 45 degree vectors from starting point */
          gdouble angle = 16. * calcangle ((PicmanVector2){1., 0.}, d) / (2.*G_PI);
          gdouble dist = norm (d) / sqrt (2);

          if (angle < 1. || angle >= 15.)
            d.y = 0;
          else if (angle < 3.)
            d.y = -(d.x = dist);
          else if (angle < 5.)
            d.x = 0;
          else if (angle < 7.)
            d.x = d.y = -dist;
          else if (angle < 9.)
            d.y = 0;
          else if (angle < 11.)
            d.x = -(d.y = dist);
          else if (angle < 13.)
            d.x = 0;
          else if (angle < 15.)
            d.x = d.y = dist;
        }

      for (i = 0; i < 4; i++)
        newpos[i] = vectoradd (oldpos[i], d);
    }

  /* rotate */
  if (function == TRANSFORM_HANDLE_ROTATION)
    {
      gdouble angle = calcangle (vectorsubtract (cur, pivot), vectorsubtract (mouse, pivot));

      if (options->constrain_rotate)
        {
          /* round to 15 degree multiple */
          angle /= 2*G_PI/24.;
          angle = round (angle);
          angle *= 2*G_PI/24.;
        }

      for (i = 0; i < 4; i++)
        newpos[i] = vectoradd (pivot,
                               rotate2d (vectorsubtract (oldpos[i], pivot),
                                         angle));

      fixedpivot = TRUE;
    }

  /* move rotation axis */
  if (function == TRANSFORM_HANDLE_PIVOT)
    {
      pivot = vectoradd (pivot, d);

      if (options->cornersnap)
        {
          /* snap to corner points and center */
          gint    closest = 0;
          gdouble closest_dist = G_MAXDOUBLE, dist;

          for (i = 0; i < 5; i++)
            {
              dist = norm (vectorsubtract (pivot, oldpos[i]));
              if (dist < closest_dist)
                {
                  closest_dist = dist;
                  closest = i;
                }
            }

          if (closest_dist *
              picman_display_get_shell (PICMAN_TOOL (transform_tool)->display)->scale_x < 50)
            {
              pivot = oldpos[closest];
            }
        }

      fixedpivot = TRUE;
    }

  /* scaling via corner */
  if (function == TRANSFORM_HANDLE_NW ||
      function == TRANSFORM_HANDLE_NE ||
      function == TRANSFORM_HANDLE_SE ||
      function == TRANSFORM_HANDLE_SW)
    {
      /* Scaling through scale handles means translating one corner point,
       * with all sides at constant angles. */

      gint this, left, right, opposite;

      /* 0: northwest, 1: northeast, 2: southwest, 3: southeast */
      if (function == TRANSFORM_HANDLE_NW)
        {
          this = 0; left = 1; right = 2; opposite = 3;
        }
      else if (function == TRANSFORM_HANDLE_NE)
        {
          this = 1; left = 3; right = 0; opposite = 2;
        }
      else if (function == TRANSFORM_HANDLE_SW)
        {
          this = 2; left = 0; right = 3; opposite = 1;
        }
      else if (function == TRANSFORM_HANDLE_SE)
        {
          this = 3; left = 2; right = 1; opposite = 0;
        }
      else
        g_assert_not_reached ();

      /* when the keep aspect transformation constraint is enabled,
       * the translation shall only be along the diagonal that runs
       * trough this corner point.
       */
      if (options->constrain_scale)
        {
          /* restrict to movement along the diagonal */
          PicmanVector2 diag = vectorsubtract (oldpos[this], oldpos[opposite]);

          d = vectorproject (d, diag);
        }

      /* Move the corner being interacted with */
      /*    rp---------tp
       *   /           /\ <- d, the interaction vector
       *  /           /  tp
       * op----------/
       *
       */
      newpos[this] = vectoradd (oldpos[this], d);

      /* Where the corner to the right and left would go, need these to form
       * lines to intersect with the sides */
      /*    rp----------/
       *   /\          /\
       *  /  nr       /  nt
       * op----------lp
       *              \
       *               nl
       */

      newpos[right] = vectoradd (oldpos[right], d);
      newpos[left] = vectoradd (oldpos[left], d);

      /* Now we just need to find the intersection of op-rp and nr-nt.
       *    rp----------/
       *   /           /
       *  /  nr==========nt
       * op----------/
       *
       */
      newpos[right] = lineintersect (newpos[right], newpos[this], oldpos[opposite], oldpos[right]);
      newpos[left] = lineintersect (newpos[left], newpos[this], oldpos[opposite], oldpos[left]);
      /*    /-----------/
       *   /           /
       *  rp============nt
       * op----------/
       *
       */

      /*
       *
       *  /--------------/
       * /--------------/
       *
       */

      if (options->frompivot_scale && transform_is_convex (newpos) && transform_is_convex (oldpos))
        {
          /* transform the pivot point before the interaction and after, and move everything by
           * this difference */
          //TODO the handle doesn't actually end up where the mouse cursor is
          PicmanVector2 delta = getpivotdelta (transform_tool, oldpos, newpos, pivot);
          for (i = 0; i < 4; i++)
            newpos[i] = vectorsubtract (newpos[i], delta);

          fixedpivot = TRUE;
        }
    }

  /* scaling via sides */
  if (function == TRANSFORM_HANDLE_N ||
      function == TRANSFORM_HANDLE_E ||
      function == TRANSFORM_HANDLE_S ||
      function == TRANSFORM_HANDLE_W)
    {
      gint        this_l, this_r, opp_l, opp_r;
      PicmanVector2 side_l, side_r, midline;

      /* 0: northwest, 1: northeast, 2: southwest, 3: southeast */
      if (function == TRANSFORM_HANDLE_N)
        {
          this_l = 1; this_r = 0;
        }
      else if (function == TRANSFORM_HANDLE_E)
        {
          this_l = 3; this_r = 1;
        }
      else if (function == TRANSFORM_HANDLE_S)
        {
          this_l = 2; this_r = 3;
        }
      else if (function == TRANSFORM_HANDLE_W)
        {
          this_l = 0; this_r = 2;
        }
      else
        g_assert_not_reached ();

      opp_l = 3 - this_r; opp_r = 3 - this_l;

      side_l = vectorsubtract (oldpos[opp_l], oldpos[this_l]);
      side_r = vectorsubtract (oldpos[opp_r], oldpos[this_r]);
      midline = vectoradd (side_l, side_r);

      /* restrict to movement along the midline */
      d = vectorproject (d, midline);

      if (options->constrain_scale)
        {
          PicmanVector2 before, after, effective_pivot = pivot;
          gdouble     distance;

          if (!options->frompivot_scale)
            {
              /* center of the opposite side is pivot */
              effective_pivot = scalemult (vectoradd (oldpos[opp_l], oldpos[opp_r]), 0.5);
            }
          /* get the difference between the distance from the pivot to where
           * interaction started and the distance from the pivot to where
           * cursor is now, and scale all corners distance from the pivot
           * with this factor */
          before = vectorsubtract (effective_pivot, mouse);
          after = vectorsubtract (effective_pivot, cur);
          after = vectorproject (after, before);

          distance = 0.5 * (after.x / before.x + after.y / before.y);

          for (i = 0; i < 4; i++)
            newpos[i] = vectoradd (effective_pivot,
                                   scalemult (vectorsubtract (oldpos[i],
                                                              effective_pivot),
                                              distance));
        }
      else
        {
          /* just move the side */
          newpos[this_l] = vectoradd (oldpos[this_l], d);
          newpos[this_r] = vectoradd (oldpos[this_r], d);
        }

      if (!options->constrain_scale && options->frompivot_scale &&
          transform_is_convex (newpos) && transform_is_convex (oldpos))
        {
          PicmanVector2 delta = getpivotdelta (transform_tool,
                                             oldpos, newpos, pivot);
          for (i = 0; i < 4; i++)
            newpos[i] = vectorsubtract (newpos[i], delta);

          fixedpivot = TRUE;
        }
    }

  /* shear */
  if (function == TRANSFORM_HANDLE_N_S ||
      function == TRANSFORM_HANDLE_E_S ||
      function == TRANSFORM_HANDLE_S_S ||
      function == TRANSFORM_HANDLE_W_S)
    {
      gint this_l, this_r;

      /* set up indices for this edge and the opposite edge */
      if (function == TRANSFORM_HANDLE_N_S)
        {
          this_l = 1; this_r = 0;
        }
      else if (function == TRANSFORM_HANDLE_W_S)
        {
          this_l = 0; this_r = 2;
        }
      else if (function == TRANSFORM_HANDLE_S_S)
        {
          this_l = 2; this_r = 3;
        }
      else if (function == TRANSFORM_HANDLE_E_S)
        {
          this_l = 3; this_r = 1;
        }
      else
        g_assert_not_reached ();

      if (options->constrain_shear)
        {
          /* restrict to movement along the side */
          PicmanVector2 side = vectorsubtract (oldpos[this_r], oldpos[this_l]);

          d = vectorproject (d, side);
        }

      newpos[this_l] = vectoradd (oldpos[this_l], d);
      newpos[this_r] = vectoradd (oldpos[this_r], d);

      if (options->frompivot_shear && transform_is_convex (newpos) &&
          transform_is_convex (oldpos))
        {
          PicmanVector2 delta = getpivotdelta (transform_tool,
                                             oldpos, newpos, pivot);
          for (i = 0; i < 4; i++)
            newpos[i] = vectorsubtract (newpos[i], delta);

          fixedpivot = TRUE;
        }
    }

  /* perspective transform */
  if (function == TRANSFORM_HANDLE_NW_P ||
      function == TRANSFORM_HANDLE_NE_P ||
      function == TRANSFORM_HANDLE_SE_P ||
      function == TRANSFORM_HANDLE_SW_P)
    {
      gint this, left, right, opposite;

      /* 0: northwest, 1: northeast, 2: southwest, 3: southeast */
      if (function == TRANSFORM_HANDLE_NW_P)
        {
          this = 0; left = 1; right = 2; opposite = 3;
        }
      else if (function == TRANSFORM_HANDLE_NE_P)
        {
          this = 1; left = 3; right = 0; opposite = 2;
        }
      else if (function == TRANSFORM_HANDLE_SW_P)
        {
          this = 2; left = 0; right = 3; opposite = 1;
        }
      else if (function == TRANSFORM_HANDLE_SE_P)
        {
          this = 3; left = 2; right = 1; opposite = 0;
        }
      else
        g_assert_not_reached ();

      if (options->constrain_perspective)
        {
          /* when the constrain transformation constraint is enabled,
           * the translation shall only be either along the side
           * angles of the two sides that run to this corner point, or
           * along the diagonal that runs trough this corner point.
           */
          PicmanVector2 proj[4];
          gdouble     rej[4];

          for (i = 0; i < 4; i++)
            {
              if (i == this)
                continue;

              /* get the vectors along the sides and the diagonal */
              proj[i] = vectorsubtract (oldpos[this], oldpos[i]);

              /* project d on each candidate vector and see which has
               * the shortest rejection
               */
              proj[i] = vectorproject (d, proj[i]);
              rej[i] = norm (vectorsubtract (d, proj[i]));
            }

          if (rej[left] < rej[right] && rej[left] < rej[opposite])
            d = proj[left];
          else if (rej[right] < rej[opposite])
            d = proj[right];
          else
            d = proj[opposite];
        }

      newpos[this] = vectoradd (oldpos[this], d);

      if (options->frompivot_perspective && transform_is_convex (newpos) &&
          transform_is_convex (oldpos))
        {
          PicmanVector2 delta = getpivotdelta (transform_tool,
                                             oldpos, newpos, pivot);

          for (i = 0; i < 4; i++)
            newpos[i] = vectorsubtract (newpos[i], delta);

          fixedpivot = TRUE;
        }
    }

  for (i = 0; i < 4; i++)
    {
      *x[i] = newpos[i].x;
      *y[i] = newpos[i].y;
    }

  /* this will have been set to TRUE if an operation used the pivot in
   * addition to being a user option
   */
  if (!fixedpivot && transform_is_convex (newpos) &&
      transform_is_convex (oldpos) && point_is_inside_polygon_pos (oldpos, pivot))
    {
      PicmanVector2 delta = getpivotdelta (transform_tool, oldpos, newpos, pivot);
      pivot = vectoradd (pivot, delta);
    }

  /* set unconditionally: if options get toggled during operation, we
   * have to move pivot back
   */
  *newpivot_x = pivot.x;
  *newpivot_y = pivot.y;
}

static void
picman_unified_transform_tool_recalc_matrix (PicmanTransformTool *tr_tool)
{
  tr_tool->px = tr_tool->trans_info[PIVOT_X];
  tr_tool->py = tr_tool->trans_info[PIVOT_Y];

  picman_matrix3_identity (&tr_tool->transform);
  picman_transform_matrix_perspective (&tr_tool->transform,
                                     tr_tool->x1,
                                     tr_tool->y1,
                                     tr_tool->x2 - tr_tool->x1,
                                     tr_tool->y2 - tr_tool->y1,
                                     tr_tool->trans_info[X0],
                                     tr_tool->trans_info[Y0],
                                     tr_tool->trans_info[X1],
                                     tr_tool->trans_info[Y1],
                                     tr_tool->trans_info[X2],
                                     tr_tool->trans_info[Y2],
                                     tr_tool->trans_info[X3],
                                     tr_tool->trans_info[Y3]);
}

static gchar *
picman_unified_transform_tool_get_undo_desc (PicmanTransformTool *tr_tool)
{
  return g_strdup (C_("undo-type", "Unified Transform"));
}
