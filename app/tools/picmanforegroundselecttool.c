/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanForegroundSelectTool
 * Copyright (C) 2005  Sven Neumann <sven@picman.org>
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

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmanchannel-combine.h"
#include "core/picmanchannel-select.h"
#include "core/picmandrawable-foreground-extract.h"
#include "core/picmanimage.h"
#include "core/picmanprogress.h"
#include "core/picmanscanconvert.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "picmanforegroundselecttool.h"
#include "picmanforegroundselectoptions.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


typedef struct
{
  gint         width;
  gboolean     background;
  gint         num_points;
  PicmanVector2 *points;
} FgSelectStroke;


static void   picman_foreground_select_tool_constructed    (GObject          *object);
static void   picman_foreground_select_tool_finalize       (GObject          *object);

static void   picman_foreground_select_tool_control        (PicmanTool         *tool,
                                                          PicmanToolAction    action,
                                                          PicmanDisplay      *display);
static void   picman_foreground_select_tool_oper_update    (PicmanTool         *tool,
                                                          const PicmanCoords *coords,
                                                          GdkModifierType   state,
                                                          gboolean          proximity,
                                                          PicmanDisplay      *display);
static void   picman_foreground_select_tool_modifier_key   (PicmanTool         *tool,
                                                          GdkModifierType   key,
                                                          gboolean          press,
                                                          GdkModifierType   state,
                                                          PicmanDisplay      *display);
static void   picman_foreground_select_tool_cursor_update  (PicmanTool         *tool,
                                                          const PicmanCoords *coords,
                                                          GdkModifierType   state,
                                                          PicmanDisplay      *display);
static gboolean  picman_foreground_select_tool_key_press   (PicmanTool         *tool,
                                                          GdkEventKey      *kevent,
                                                          PicmanDisplay      *display);
static void   picman_foreground_select_tool_button_press   (PicmanTool         *tool,
                                                          const PicmanCoords *coords,
                                                          guint32           time,
                                                          GdkModifierType   state,
                                                          PicmanButtonPressType press_type,
                                                          PicmanDisplay      *display);
static void   picman_foreground_select_tool_button_release (PicmanTool         *tool,
                                                          const PicmanCoords *coords,
                                                          guint32           time,
                                                          GdkModifierType   state,
                                                          PicmanButtonReleaseType release_type,
                                                          PicmanDisplay      *display);
static void   picman_foreground_select_tool_motion         (PicmanTool         *tool,
                                                          const PicmanCoords *coords,
                                                          guint32           time,
                                                          GdkModifierType   state,
                                                          PicmanDisplay      *display);

static void   picman_foreground_select_tool_draw           (PicmanDrawTool     *draw_tool);

static void   picman_foreground_select_tool_select   (PicmanFreeSelectTool *free_sel,
                                                    PicmanDisplay        *display);

static void   picman_foreground_select_tool_set_mask (PicmanForegroundSelectTool *fg_select,
                                                    PicmanDisplay              *display,
                                                    PicmanChannel              *mask);
static void   picman_foreground_select_tool_apply    (PicmanForegroundSelectTool *fg_select,
                                                    PicmanDisplay              *display);

static void   picman_foreground_select_tool_stroke   (PicmanChannel              *mask,
                                                    FgSelectStroke           *stroke);

static void   picman_foreground_select_tool_push_stroke (PicmanForegroundSelectTool    *fg_select,
                                                       PicmanDisplay                 *display,
                                                       PicmanForegroundSelectOptions *options);

static void   picman_foreground_select_options_notify (PicmanForegroundSelectOptions *options,
                                                     GParamSpec                  *pspec,
                                                     PicmanForegroundSelectTool    *fg_select);


G_DEFINE_TYPE (PicmanForegroundSelectTool, picman_foreground_select_tool,
               PICMAN_TYPE_FREE_SELECT_TOOL)

#define parent_class picman_foreground_select_tool_parent_class


void
picman_foreground_select_tool_register (PicmanToolRegisterCallback  callback,
                                      gpointer                  data)
{
  (* callback) (PICMAN_TYPE_FOREGROUND_SELECT_TOOL,
                PICMAN_TYPE_FOREGROUND_SELECT_OPTIONS,
                picman_foreground_select_options_gui,
                PICMAN_CONTEXT_FOREGROUND_MASK | PICMAN_CONTEXT_BACKGROUND_MASK,
                "picman-foreground-select-tool",
                _("Foreground Select"),
                _("Foreground Select Tool: Select a region containing foreground objects"),
                N_("F_oreground Select"), NULL,
                NULL, PICMAN_HELP_TOOL_FOREGROUND_SELECT,
                PICMAN_STOCK_TOOL_FOREGROUND_SELECT,
                data);
}

static void
picman_foreground_select_tool_class_init (PicmanForegroundSelectToolClass *klass)
{
  GObjectClass            *object_class    = G_OBJECT_CLASS (klass);
  PicmanToolClass           *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass       *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);
  PicmanFreeSelectToolClass *free_select_tool_class;

  free_select_tool_class = PICMAN_FREE_SELECT_TOOL_CLASS (klass);

  object_class->constructed      = picman_foreground_select_tool_constructed;
  object_class->finalize         = picman_foreground_select_tool_finalize;

  tool_class->control            = picman_foreground_select_tool_control;
  tool_class->oper_update        = picman_foreground_select_tool_oper_update;
  tool_class->modifier_key       = picman_foreground_select_tool_modifier_key;
  tool_class->cursor_update      = picman_foreground_select_tool_cursor_update;
  tool_class->key_press          = picman_foreground_select_tool_key_press;
  tool_class->button_press       = picman_foreground_select_tool_button_press;
  tool_class->button_release     = picman_foreground_select_tool_button_release;
  tool_class->motion             = picman_foreground_select_tool_motion;

  draw_tool_class->draw          = picman_foreground_select_tool_draw;

  free_select_tool_class->select = picman_foreground_select_tool_select;
}

static void
picman_foreground_select_tool_init (PicmanForegroundSelectTool *fg_select)
{
  PicmanTool *tool = PICMAN_TOOL (fg_select);

  picman_tool_control_set_scroll_lock (tool->control, FALSE);
  picman_tool_control_set_preserve    (tool->control, FALSE);
  picman_tool_control_set_dirty_mask  (tool->control,
                                     PICMAN_DIRTY_IMAGE_SIZE |
                                     PICMAN_DIRTY_ACTIVE_DRAWABLE);
  picman_tool_control_set_precision   (tool->control,
                                     PICMAN_CURSOR_PRECISION_PIXEL_CENTER);
  picman_tool_control_set_tool_cursor (tool->control,
                                     PICMAN_TOOL_CURSOR_FREE_SELECT);

  picman_tool_control_set_action_value_2 (tool->control,
                                        "tools/tools-foreground-select-brush-size-set");

  fg_select->idle_id = 0;
  fg_select->stroke  = NULL;
  fg_select->strokes = NULL;
  fg_select->mask    = NULL;
}

static void
picman_foreground_select_tool_constructed (GObject *object)
{
  PicmanToolOptions *options = PICMAN_TOOL_GET_OPTIONS (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_signal_connect_object (options, "notify",
                           G_CALLBACK (picman_foreground_select_options_notify),
                           object, 0);
}

static void
picman_foreground_select_tool_finalize (GObject *object)
{
  PicmanForegroundSelectTool *fg_select = PICMAN_FOREGROUND_SELECT_TOOL (object);

  if (fg_select->stroke)
    g_warning ("%s: stroke should be NULL at this point", G_STRLOC);

  if (fg_select->strokes)
    g_warning ("%s: strokes should be NULL at this point", G_STRLOC);

#if 0
  if (fg_select->state)
    g_warning ("%s: state should be NULL at this point", G_STRLOC);
#endif

  if (fg_select->mask)
    g_warning ("%s: mask should be NULL at this point", G_STRLOC);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_foreground_select_tool_control (PicmanTool       *tool,
                                     PicmanToolAction  action,
                                     PicmanDisplay    *display)
{
  PicmanForegroundSelectTool *fg_select = PICMAN_FOREGROUND_SELECT_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      {
        GList *list;

        picman_foreground_select_tool_set_mask (fg_select, display, NULL);

        for (list = fg_select->strokes; list; list = list->next)
          {
            FgSelectStroke *stroke = list->data;

            g_free (stroke->points);
            g_slice_free (FgSelectStroke, stroke);
          }

        g_list_free (fg_select->strokes);
        fg_select->strokes = NULL;

#if 0
        if (fg_select->state)
          {
            picman_drawable_foreground_extract_siox_done (fg_select->state);
            fg_select->state = NULL;
          }
#endif

        tool->display = NULL;
      }
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_foreground_select_tool_oper_update (PicmanTool         *tool,
                                         const PicmanCoords *coords,
                                         GdkModifierType   state,
                                         gboolean          proximity,
                                         PicmanDisplay      *display)
{
  PicmanForegroundSelectTool *fg_select = PICMAN_FOREGROUND_SELECT_TOOL (tool);
  const gchar              *status    = NULL;

  PICMAN_TOOL_CLASS (parent_class)->oper_update (tool, coords, state, proximity,
                                               display);

  if (fg_select->mask && display == tool->display)
    {
      PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (tool);

      picman_draw_tool_pause (draw_tool);

      fg_select->last_coords = *coords;

      picman_draw_tool_resume (draw_tool);

      if (fg_select->strokes)
        status = _("Add more strokes or press Enter to accept the selection");
      else
        status = _("Mark foreground by painting on the object to extract");
    }
  else
    {
      if (PICMAN_SELECTION_TOOL (tool)->function == SELECTION_SELECT)
        status = _("Roughly outline the object to extract");
    }

  if (proximity && status)
    picman_tool_replace_status (tool, display, "%s", status);
}

static void
picman_foreground_select_tool_modifier_key (PicmanTool        *tool,
                                          GdkModifierType  key,
                                          gboolean         press,
                                          GdkModifierType  state,
                                          PicmanDisplay     *display)
{
  if (key == picman_get_toggle_behavior_mask ())
    {
      PicmanForegroundSelectOptions *options;

      options = PICMAN_FOREGROUND_SELECT_TOOL_GET_OPTIONS (tool);

      g_object_set (options,
                    "background", ! options->background,
                    NULL);
    }
}

static void
picman_foreground_select_tool_cursor_update (PicmanTool         *tool,
                                           const PicmanCoords *coords,
                                           GdkModifierType   state,
                                           PicmanDisplay      *display)
{
  PicmanForegroundSelectTool *fg_select = PICMAN_FOREGROUND_SELECT_TOOL (tool);

  if (fg_select->mask)
    {
      PicmanForegroundSelectOptions *options;

      options = PICMAN_FOREGROUND_SELECT_TOOL_GET_OPTIONS (tool);

      picman_tool_control_set_toggled (tool->control, options->background);

      switch (PICMAN_SELECTION_TOOL (tool)->function)
        {
        case SELECTION_MOVE_MASK:
        case SELECTION_MOVE:
        case SELECTION_MOVE_COPY:
        case SELECTION_ANCHOR:
          return;
        default:
          break;
        }
    }

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static gboolean
picman_foreground_select_tool_key_press (PicmanTool    *tool,
                                       GdkEventKey *kevent,
                                       PicmanDisplay *display)
{
  PicmanForegroundSelectTool *fg_select = PICMAN_FOREGROUND_SELECT_TOOL (tool);

  if (display != tool->display)
    return FALSE;

#if 0
  if (fg_select->state)
#endif
  if (fg_select->mask) /* dunno if that's the right condition */
    {
      switch (kevent->keyval)
        {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
        case GDK_KEY_ISO_Enter:
          picman_foreground_select_tool_apply (fg_select, display);
          return TRUE;

        case GDK_KEY_Escape:
          picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);
          return TRUE;

        default:
          return FALSE;
        }
    }
  else
    {
      return PICMAN_TOOL_CLASS (parent_class)->key_press (tool,
                                                        kevent,
                                                        display);
    }
}

static void
picman_foreground_select_tool_button_press (PicmanTool            *tool,
                                          const PicmanCoords    *coords,
                                          guint32              time,
                                          GdkModifierType      state,
                                          PicmanButtonPressType  press_type,
                                          PicmanDisplay         *display)
{
  PicmanForegroundSelectTool *fg_select = PICMAN_FOREGROUND_SELECT_TOOL (tool);
  PicmanDrawTool             *draw_tool = PICMAN_DRAW_TOOL (tool);

  if (fg_select->mask)
    {
      PicmanVector2 point = picman_vector2_new (coords->x, coords->y);

      picman_draw_tool_pause (draw_tool);

      if (picman_draw_tool_is_active (draw_tool) && draw_tool->display != display)
        picman_draw_tool_stop (draw_tool);

      picman_tool_control_activate (tool->control);

      fg_select->last_coords = *coords;

      g_return_if_fail (fg_select->stroke == NULL);
      fg_select->stroke = g_array_new (FALSE, FALSE, sizeof (PicmanVector2));

      g_array_append_val (fg_select->stroke, point);

      if (! picman_draw_tool_is_active (draw_tool))
        picman_draw_tool_start (draw_tool, display);

      picman_draw_tool_resume (draw_tool);
    }
  else
    {
      PICMAN_TOOL_CLASS (parent_class)->button_press (tool, coords, time, state,
                                                    press_type, display);
    }
}

static void
picman_foreground_select_tool_button_release (PicmanTool              *tool,
                                            const PicmanCoords      *coords,
                                            guint32                time,
                                            GdkModifierType        state,
                                            PicmanButtonReleaseType  release_type,
                                            PicmanDisplay           *display)
{
  PicmanForegroundSelectTool *fg_select = PICMAN_FOREGROUND_SELECT_TOOL (tool);

  if (fg_select->mask)
    {
      PicmanForegroundSelectOptions *options;

      options = PICMAN_FOREGROUND_SELECT_TOOL_GET_OPTIONS (tool);

      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

      picman_tool_control_halt (tool->control);

      picman_foreground_select_tool_push_stroke (fg_select, display, options);

      picman_free_select_tool_select (PICMAN_FREE_SELECT_TOOL (tool), display);

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
    }
  else
    {
      PICMAN_TOOL_CLASS (parent_class)->button_release (tool,
                                                      coords, time, state,
                                                      release_type,
                                                      display);
    }
}

static void
picman_foreground_select_tool_motion (PicmanTool         *tool,
                                    const PicmanCoords *coords,
                                    guint32           time,
                                    GdkModifierType   state,
                                    PicmanDisplay      *display)
{
  PicmanForegroundSelectTool *fg_select = PICMAN_FOREGROUND_SELECT_TOOL (tool);

  if (fg_select->mask)
    {
      PicmanVector2 *last = &g_array_index (fg_select->stroke,
                                          PicmanVector2,
                                          fg_select->stroke->len - 1);

      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

      fg_select->last_coords = *coords;

      if (last->x != (gint) coords->x || last->y != (gint) coords->y)
        {
          PicmanVector2 point = picman_vector2_new (coords->x, coords->y);

          g_array_append_val (fg_select->stroke, point);
        }

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
    }
  else
    {
      PICMAN_TOOL_CLASS (parent_class)->motion (tool,
                                              coords, time, state, display);
    }
}

static void
picman_foreground_select_tool_get_area (PicmanChannel *mask,
                                      gint        *x1,
                                      gint        *y1,
                                      gint        *x2,
                                      gint        *y2)
{
  gint width;
  gint height;

  picman_channel_bounds (mask, x1, y1, x2, y2);

  width  = *x2 - *x1;
  height = *y2 - *y1;

  *x1 = MAX (*x1 - width  / 2, 0);
  *y1 = MAX (*y1 - height / 2, 0);
  *x2 = MIN (*x2 + width  / 2, picman_item_get_width  (PICMAN_ITEM (mask)));
  *y2 = MIN (*y2 + height / 2, picman_item_get_height (PICMAN_ITEM (mask)));
}

static void
picman_foreground_select_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanForegroundSelectTool    *fg_select = PICMAN_FOREGROUND_SELECT_TOOL (draw_tool);
  PicmanTool                    *tool      = PICMAN_TOOL (draw_tool);
  PicmanForegroundSelectOptions *options;

  options = PICMAN_FOREGROUND_SELECT_TOOL_GET_OPTIONS (tool);

  if (fg_select->stroke)
    {
      picman_draw_tool_add_pen (draw_tool,
                              (const PicmanVector2 *) fg_select->stroke->data,
                              fg_select->stroke->len,
                              PICMAN_CONTEXT (options),
                              (options->background ?
                               PICMAN_ACTIVE_COLOR_BACKGROUND :
                               PICMAN_ACTIVE_COLOR_FOREGROUND),
                              options->stroke_width);
    }

  if (fg_select->mask)
    {
      PicmanDisplayShell   *shell = picman_display_get_shell (draw_tool->display);
      gint                x     = fg_select->last_coords.x;
      gint                y     = fg_select->last_coords.y;
      gdouble             radius;

      radius = (options->stroke_width / shell->scale_y) / 2;

      /*  warn if the user is drawing outside of the working area  */
      if (FALSE)
        {
          gint x1, y1;
          gint x2, y2;

          picman_foreground_select_tool_get_area (fg_select->mask,
                                                &x1, &y1, &x2, &y2);

          if (x < x1 + radius || x > x2 - radius ||
              y < y1 + radius || y > y2 - radius)
            {
              picman_draw_tool_add_rectangle (draw_tool, FALSE,
                                            x1, y1,
                                            x2 - x1, y2 - y1);
            }
        }

      picman_draw_tool_add_arc (draw_tool, FALSE,
                              x - radius, y - radius,
                              2 * radius, 2 * radius,
                              0.0, 2.0 * G_PI);
    }
  else
    {
      PICMAN_DRAW_TOOL_CLASS (parent_class)->draw (draw_tool);
    }
}

static void
picman_foreground_select_tool_select (PicmanFreeSelectTool *free_sel,
                                    PicmanDisplay        *display)
{
  PicmanForegroundSelectTool    *fg_select;
  PicmanForegroundSelectOptions *options;
  PicmanImage                   *image = picman_display_get_image (display);
  PicmanDrawable                *drawable;
  PicmanScanConvert             *scan_convert;
  PicmanChannel                 *mask;
  const PicmanVector2           *points;
  gint                         n_points;

  drawable  = picman_image_get_active_drawable (image);
  fg_select = PICMAN_FOREGROUND_SELECT_TOOL (free_sel);
  options   = PICMAN_FOREGROUND_SELECT_TOOL_GET_OPTIONS (free_sel);

  if (fg_select->idle_id)
    {
      g_source_remove (fg_select->idle_id);
      fg_select->idle_id = 0;
    }

  if (! drawable)
    return;

  scan_convert = picman_scan_convert_new ();

  picman_free_select_tool_get_points (free_sel,
                                    &points,
                                    &n_points);

  picman_scan_convert_add_polyline (scan_convert,
                                  n_points,
                                  points,
                                  TRUE);

  mask = picman_channel_new (image,
                           picman_image_get_width (image),
                           picman_image_get_height (image),
                           "foreground-extraction", NULL);

  picman_scan_convert_render_value (scan_convert,
                                  picman_drawable_get_buffer (PICMAN_DRAWABLE (mask)),
                                  0, 0, 0.5);
  picman_scan_convert_free (scan_convert);

  if (fg_select->strokes)
    {
      GList *list;

      picman_set_busy (image->picman);

      /*  apply foreground and background markers  */
      for (list = fg_select->strokes; list; list = list->next)
        picman_foreground_select_tool_stroke (mask, list->data);

#if 0
      if (fg_select->state)
        picman_drawable_foreground_extract_siox (PICMAN_DRAWABLE (mask),
                                               fg_select->state,
                                               fg_select->refinement,
                                               options->smoothness,
                                               options->sensitivity,
                                               ! options->contiguous,
                                               PICMAN_PROGRESS (fg_select));
#endif

      fg_select->refinement = SIOX_REFINEMENT_NO_CHANGE;

      picman_unset_busy (image->picman);
    }
  else
    {
      gint x1, y1;
      gint x2, y2;

      g_object_set (options, "background", FALSE, NULL);

      picman_foreground_select_tool_get_area (mask, &x1, &y1, &x2, &y2);

#if 0
      if (fg_select->state)
        g_warning ("state should be NULL here");

      fg_select->state =
        picman_drawable_foreground_extract_siox_init (drawable,
                                                    x1, y1, x2 - x1, y2 - y1);
#endif
    }

  picman_foreground_select_tool_set_mask (fg_select, display, mask);

  g_object_unref (mask);
}

static void
picman_foreground_select_tool_set_mask (PicmanForegroundSelectTool *fg_select,
                                      PicmanDisplay              *display,
                                      PicmanChannel              *mask)
{
  PicmanTool                    *tool = PICMAN_TOOL (fg_select);
  PicmanForegroundSelectOptions *options;

  options = PICMAN_FOREGROUND_SELECT_TOOL_GET_OPTIONS (tool);

  if (fg_select->mask == mask)
    return;

  if (fg_select->mask)
    {
      g_object_unref (fg_select->mask);
      fg_select->mask = NULL;
    }

  if (mask)
    {
      PicmanRGB color;

      fg_select->mask = g_object_ref (mask);

      picman_foreground_select_options_get_mask_color (options, &color);
      picman_display_shell_set_mask (picman_display_get_shell (display),
                                   PICMAN_DRAWABLE (mask), &color);
    }
  else
    {
      picman_display_shell_set_mask (picman_display_get_shell (display),
                                   NULL, NULL);
    }

  if (mask)
    {
      picman_tool_control_set_tool_cursor        (tool->control,
                                                PICMAN_TOOL_CURSOR_PAINTBRUSH);
      picman_tool_control_set_toggle_tool_cursor (tool->control,
                                                PICMAN_TOOL_CURSOR_ERASER);

      picman_tool_control_set_toggled (tool->control, options->background);
    }
  else
    {
      picman_tool_control_set_tool_cursor        (tool->control,
                                                PICMAN_TOOL_CURSOR_FREE_SELECT);
      picman_tool_control_set_toggle_tool_cursor (tool->control,
                                                PICMAN_TOOL_CURSOR_FREE_SELECT);

      picman_tool_control_set_toggled (tool->control, FALSE);
    }
}

static void
picman_foreground_select_tool_apply (PicmanForegroundSelectTool *fg_select,
                                   PicmanDisplay              *display)
{
  PicmanTool             *tool    = PICMAN_TOOL (fg_select);
  PicmanSelectionOptions *options = PICMAN_SELECTION_TOOL_GET_OPTIONS (tool);
  PicmanImage            *image   = picman_display_get_image (display);

  g_return_if_fail (fg_select->mask != NULL);

  picman_channel_select_channel (picman_image_get_mask (image),
                               C_("command", "Foreground Select"),
                               fg_select->mask, 0, 0,
                               options->operation,
                               options->feather,
                               options->feather_radius,
                               options->feather_radius);

  picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);

  picman_image_flush (image);
}

static void
picman_foreground_select_tool_stroke (PicmanChannel    *mask,
                                    FgSelectStroke *stroke)
{
  PicmanScanConvert *scan_convert = picman_scan_convert_new ();

  if (stroke->num_points == 1)
    {
      PicmanVector2 points[2];

      points[0] = points[1] = stroke->points[0];

      points[1].x += 0.01;
      points[1].y += 0.01;

      picman_scan_convert_add_polyline (scan_convert, 2, points, FALSE);
    }
  else
    {
      picman_scan_convert_add_polyline (scan_convert,
                                      stroke->num_points, stroke->points,
                                      FALSE);
    }

  picman_scan_convert_stroke (scan_convert,
                            stroke->width,
                            PICMAN_JOIN_ROUND, PICMAN_CAP_ROUND, 10.0,
                            0.0, NULL);
  picman_scan_convert_compose_value (scan_convert,
                                   picman_drawable_get_buffer (PICMAN_DRAWABLE (mask)),
                                   0, 0, stroke->background ? 0.0 : 1.0);
  picman_scan_convert_free (scan_convert);
}

static void
picman_foreground_select_tool_push_stroke (PicmanForegroundSelectTool    *fg_select,
                                         PicmanDisplay                 *display,
                                         PicmanForegroundSelectOptions *options)
{
  PicmanDisplayShell *shell = picman_display_get_shell (display);
  FgSelectStroke   *stroke;

  g_return_if_fail (fg_select->stroke != NULL);

  stroke = g_slice_new (FgSelectStroke);

  stroke->background = options->background;
  stroke->width      = ROUND ((gdouble) options->stroke_width / shell->scale_y);
  stroke->num_points = fg_select->stroke->len;
  stroke->points     = (PicmanVector2 *) g_array_free (fg_select->stroke, FALSE);

  fg_select->stroke = NULL;

  fg_select->strokes = g_list_append (fg_select->strokes, stroke);

  fg_select->refinement |= (stroke->background ?
                            SIOX_REFINEMENT_ADD_BACKGROUND :
                            SIOX_REFINEMENT_ADD_FOREGROUND);
}

static gboolean
picman_foreground_select_tool_idle_select (PicmanForegroundSelectTool *fg_select)
{
  PicmanTool *tool = PICMAN_TOOL (fg_select);

  fg_select->idle_id = 0;

  if (tool->display)
    picman_free_select_tool_select (PICMAN_FREE_SELECT_TOOL (tool), tool->display);

  return FALSE;
}

/* To compress close notify signals, the process is delayed by */
#define MINIMUM_DELAY 300

static void
picman_foreground_select_options_notify (PicmanForegroundSelectOptions *options,
                                       GParamSpec                  *pspec,
                                       PicmanForegroundSelectTool    *fg_select)
{
  SioxRefinementType refinement = 0;

  if (! fg_select->mask)
    return;

#if 0
  if (strcmp (pspec->name, "smoothness") == 0)
    {
      refinement = SIOX_REFINEMENT_CHANGE_SMOOTHNESS;
    }
  else if (strcmp (pspec->name, "contiguous") == 0)
    {
      refinement = SIOX_REFINEMENT_CHANGE_MULTIBLOB;
    }
  else if (g_str_has_prefix (pspec->name, "sensitivity"))
    {
      refinement = SIOX_REFINEMENT_CHANGE_SENSITIVITY;
    }
#endif

  if (refinement && fg_select->strokes)
    {
      fg_select->refinement |= refinement;

      if (fg_select->idle_id)
        g_source_remove (fg_select->idle_id);

      fg_select->idle_id = 
        g_timeout_add_full (G_PRIORITY_LOW, MINIMUM_DELAY,
                            (GSourceFunc) picman_foreground_select_tool_idle_select,
                            fg_select, NULL);
    }

  if (g_str_has_prefix (pspec->name, "mask-color"))
    {
      PicmanTool *tool = PICMAN_TOOL (fg_select);

      if (tool->display && fg_select->mask)
        {
          PicmanRGB color;

          picman_foreground_select_options_get_mask_color (options, &color);
          picman_display_shell_set_mask (picman_display_get_shell (tool->display),
                                       PICMAN_DRAWABLE (fg_select->mask), &color);
        }
    }
}
