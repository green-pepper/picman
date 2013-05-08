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
#include <gdk/gdkkeysyms.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "config/picmandisplayconfig.h"

#include "core/picman.h"
#include "core/picmanguide.h"
#include "core/picmanimage.h"
#include "core/picmanimage-arrange.h"
#include "core/picmanimage-guides.h"
#include "core/picmanimage-pick-layer.h"
#include "core/picmanimage-undo.h"
#include "core/picmanlayer.h"

#include "vectors/picmanvectors.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-appearance.h"

#include "picmanalignoptions.h"
#include "picmanaligntool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


#define EPSILON  3   /* move distance after which we do a box-select */


/*  local function prototypes  */

static void     picman_align_tool_constructed    (GObject               *object);

static void     picman_align_tool_control        (PicmanTool              *tool,
                                                PicmanToolAction         action,
                                                PicmanDisplay           *display);
static void     picman_align_tool_button_press   (PicmanTool              *tool,
                                                const PicmanCoords      *coords,
                                                guint32                time,
                                                GdkModifierType        state,
                                                PicmanButtonPressType    press_type,
                                                PicmanDisplay           *display);
static void     picman_align_tool_button_release (PicmanTool              *tool,
                                                const PicmanCoords      *coords,
                                                guint32                time,
                                                GdkModifierType        state,
                                                PicmanButtonReleaseType  release_type,
                                                PicmanDisplay           *display);
static void     picman_align_tool_motion         (PicmanTool              *tool,
                                                const PicmanCoords      *coords,
                                                guint32                time,
                                                GdkModifierType        state,
                                                PicmanDisplay           *display);
static gboolean picman_align_tool_key_press      (PicmanTool              *tool,
                                                GdkEventKey           *kevent,
                                                PicmanDisplay           *display);
static void     picman_align_tool_oper_update    (PicmanTool              *tool,
                                                const PicmanCoords      *coords,
                                                GdkModifierType        state,
                                                gboolean               proximity,
                                                PicmanDisplay           *display);
static void     picman_align_tool_status_update  (PicmanTool              *tool,
                                                PicmanDisplay           *display,
                                                GdkModifierType        state,
                                                gboolean               proximity);
static void     picman_align_tool_cursor_update  (PicmanTool              *tool,
                                                const PicmanCoords      *coords,
                                                GdkModifierType        state,
                                                PicmanDisplay           *display);

static void     picman_align_tool_draw           (PicmanDrawTool          *draw_tool);

static void     picman_align_tool_align          (PicmanAlignTool         *align_tool,
                                                PicmanAlignmentType      align_type);

static void     picman_align_tool_object_removed (GObject               *object,
                                                PicmanAlignTool         *align_tool);
static void     picman_align_tool_clear_selected (PicmanAlignTool         *align_tool);


G_DEFINE_TYPE (PicmanAlignTool, picman_align_tool, PICMAN_TYPE_DRAW_TOOL)

#define parent_class picman_align_tool_parent_class


void
picman_align_tool_register (PicmanToolRegisterCallback  callback,
                          gpointer                  data)
{
  (* callback) (PICMAN_TYPE_ALIGN_TOOL,
                PICMAN_TYPE_ALIGN_OPTIONS,
                picman_align_options_gui,
                0,
                "picman-align-tool",
                _("Align"),
                _("Alignment Tool: Align or arrange layers and other objects"),
                N_("_Align"), "Q",
                NULL, PICMAN_HELP_TOOL_ALIGN,
                PICMAN_STOCK_TOOL_ALIGN,
                data);
}

static void
picman_align_tool_class_init (PicmanAlignToolClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  object_class->constructed  = picman_align_tool_constructed;

  tool_class->control        = picman_align_tool_control;
  tool_class->button_press   = picman_align_tool_button_press;
  tool_class->button_release = picman_align_tool_button_release;
  tool_class->motion         = picman_align_tool_motion;
  tool_class->key_press      = picman_align_tool_key_press;
  tool_class->oper_update    = picman_align_tool_oper_update;
  tool_class->cursor_update  = picman_align_tool_cursor_update;

  draw_tool_class->draw      = picman_align_tool_draw;
}

static void
picman_align_tool_init (PicmanAlignTool *align_tool)
{
  PicmanTool *tool = PICMAN_TOOL (align_tool);

  picman_tool_control_set_snap_to     (tool->control, FALSE);
  picman_tool_control_set_precision   (tool->control,
                                     PICMAN_CURSOR_PRECISION_PIXEL_BORDER);
  picman_tool_control_set_tool_cursor (tool->control, PICMAN_TOOL_CURSOR_MOVE);

  align_tool->function = ALIGN_TOOL_IDLE;
}

static void
picman_align_tool_constructed (GObject *object)
{
  PicmanAlignTool    *align_tool = PICMAN_ALIGN_TOOL (object);
  PicmanAlignOptions *options;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  options = PICMAN_ALIGN_TOOL_GET_OPTIONS (align_tool);

  g_signal_connect_object (options, "align-button-clicked",
                           G_CALLBACK (picman_align_tool_align),
                           align_tool, G_CONNECT_SWAPPED);
}

static void
picman_align_tool_control (PicmanTool       *tool,
                         PicmanToolAction  action,
                         PicmanDisplay    *display)
{
  PicmanAlignTool *align_tool = PICMAN_ALIGN_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      picman_align_tool_clear_selected (align_tool);
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_align_tool_button_press (PicmanTool            *tool,
                              const PicmanCoords    *coords,
                              guint32              time,
                              GdkModifierType      state,
                              PicmanButtonPressType  press_type,
                              PicmanDisplay         *display)
{
  PicmanAlignTool *align_tool  = PICMAN_ALIGN_TOOL (tool);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  /*  If the tool was being used in another image... reset it  */
  if (display != tool->display)
    picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);

  tool->display = display;

  picman_tool_control_activate (tool->control);

  align_tool->x2 = align_tool->x1 = coords->x;
  align_tool->y2 = align_tool->y1 = coords->y;

  if (! picman_draw_tool_is_active (PICMAN_DRAW_TOOL (tool)))
    picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

/* some rather complex logic here.  If the user clicks without modifiers,
 * then we start a new list, and use the first object in it as reference.
 * If the user clicks using Shift, or draws a rubber-band box, then
 * we add objects to the list, but do not specify which one should
 * be used as reference.
 */
static void
picman_align_tool_button_release (PicmanTool              *tool,
                                const PicmanCoords      *coords,
                                guint32                time,
                                GdkModifierType        state,
                                PicmanButtonReleaseType  release_type,
                                PicmanDisplay           *display)
{
  PicmanAlignTool    *align_tool = PICMAN_ALIGN_TOOL (tool);
  PicmanAlignOptions *options    = PICMAN_ALIGN_TOOL_GET_OPTIONS (tool);
  PicmanDisplayShell *shell      = picman_display_get_shell (display);
  GObject          *object     = NULL;
  PicmanImage        *image      = picman_display_get_image (display);
  GdkModifierType   extend_mask;
  gint              i;

  extend_mask = picman_get_extend_selection_mask ();

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  picman_tool_control_halt (tool->control);

  if (release_type == PICMAN_BUTTON_RELEASE_CANCEL)
    {
      align_tool->x2 = align_tool->x1;
      align_tool->y2 = align_tool->y1;

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
      return;
    }

  if (! (state & extend_mask)) /* start a new list */
    {
      picman_align_tool_clear_selected (align_tool);
      align_tool->set_reference = FALSE;
    }

  /* if mouse has moved less than EPSILON pixels since button press,
   * select the nearest thing, otherwise make a rubber-band rectangle
   */
  if (hypot (coords->x - align_tool->x1,
             coords->y - align_tool->y1) < EPSILON)
    {
      PicmanVectors *vectors;
      PicmanGuide   *guide;
      PicmanLayer   *layer;
      gint         snap_distance = display->config->snap_distance;

      if (picman_draw_tool_on_vectors (PICMAN_DRAW_TOOL (tool), display,
                                     coords, snap_distance, snap_distance,
                                     NULL, NULL, NULL, NULL, NULL,
                                     &vectors))
        {
          object = G_OBJECT (vectors);
        }
      else if (picman_display_shell_get_show_guides (shell) &&
               (guide = picman_image_find_guide (image,
                                               coords->x, coords->y,
                                               FUNSCALEX (shell, snap_distance),
                                               FUNSCALEY (shell, snap_distance))))
        {
          object = G_OBJECT (guide);
        }
      else
        {
          if ((layer = picman_image_pick_layer_by_bounds (image,
                                                        coords->x, coords->y)))
            {
              object = G_OBJECT (layer);
            }
        }

      if (object)
        {
          if (! g_list_find (align_tool->selected_objects, object))
            {
              align_tool->selected_objects =
                g_list_append (align_tool->selected_objects, object);

              g_signal_connect (object, "removed",
                                G_CALLBACK (picman_align_tool_object_removed),
                                align_tool);

              /* if an object has been selected using unmodified click,
               * it should be used as the reference
               */
              if (! (state & extend_mask))
                align_tool->set_reference = TRUE;
            }
        }
    }
  else  /* FIXME: look for vectors too */
    {
      gint   X0 = MIN (coords->x, align_tool->x1);
      gint   X1 = MAX (coords->x, align_tool->x1);
      gint   Y0 = MIN (coords->y, align_tool->y1);
      gint   Y1 = MAX (coords->y, align_tool->y1);
      GList *all_layers;
      GList *list;

      all_layers = picman_image_get_layer_list (image);

      for (list = all_layers; list; list = g_list_next (list))
        {
          PicmanLayer *layer = list->data;
          gint       x0, y0, x1, y1;

          if (! picman_item_get_visible (PICMAN_ITEM (layer)))
            continue;

          picman_item_get_offset (PICMAN_ITEM (layer), &x0, &y0);
          x1 = x0 + picman_item_get_width  (PICMAN_ITEM (layer));
          y1 = y0 + picman_item_get_height (PICMAN_ITEM (layer));

          if (x0 < X0 || y0 < Y0 || x1 > X1 || y1 > Y1)
            continue;

          if (g_list_find (align_tool->selected_objects, layer))
            continue;

          align_tool->selected_objects =
            g_list_append (align_tool->selected_objects, layer);

          g_signal_connect (layer, "removed",
                            G_CALLBACK (picman_align_tool_object_removed),
                            align_tool);
        }

      g_list_free (all_layers);
    }

  for (i = 0; i < ALIGN_OPTIONS_N_BUTTONS; i++)
    {
      if (options->button[i])
        gtk_widget_set_sensitive (options->button[i],
                                  align_tool->selected_objects != NULL);
    }

  align_tool->x2 = align_tool->x1;
  align_tool->y2 = align_tool->y1;

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static void
picman_align_tool_motion (PicmanTool         *tool,
                        const PicmanCoords *coords,
                        guint32           time,
                        GdkModifierType   state,
                        PicmanDisplay      *display)
{
  PicmanAlignTool *align_tool = PICMAN_ALIGN_TOOL (tool);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  align_tool->x2 = coords->x;
  align_tool->y2 = coords->y;

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static gboolean
picman_align_tool_key_press (PicmanTool    *tool,
                           GdkEventKey *kevent,
                           PicmanDisplay *display)
{
  if (display == tool->display)
    {
      switch (kevent->keyval)
        {
        case GDK_KEY_Escape:
          picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);
          return TRUE;

        default:
          break;
        }
    }

  return FALSE;
}

static void
picman_align_tool_oper_update (PicmanTool         *tool,
                             const PicmanCoords *coords,
                             GdkModifierType   state,
                             gboolean          proximity,
                             PicmanDisplay      *display)
{
  PicmanAlignTool    *align_tool    = PICMAN_ALIGN_TOOL (tool);
  PicmanDisplayShell *shell         = picman_display_get_shell (display);
  PicmanImage        *image         = picman_display_get_image (display);
  gint              snap_distance = display->config->snap_distance;
  gboolean          add;

  add = ((state & picman_get_extend_selection_mask ()) &&
         align_tool->selected_objects);

  if (picman_draw_tool_on_vectors (PICMAN_DRAW_TOOL (tool), display,
                                 coords, snap_distance, snap_distance,
                                 NULL, NULL, NULL, NULL, NULL, NULL))
    {
      if (add)
        align_tool->function = ALIGN_TOOL_ADD_PATH;
      else
        align_tool->function = ALIGN_TOOL_PICK_PATH;
    }
  else if (picman_display_shell_get_show_guides (shell) &&
           picman_image_find_guide (image,
                                  coords->x, coords->y,
                                  FUNSCALEX (shell, snap_distance),
                                  FUNSCALEY (shell, snap_distance)))
    {
      if (add)
        align_tool->function = ALIGN_TOOL_ADD_GUIDE;
      else
        align_tool->function = ALIGN_TOOL_PICK_GUIDE;
    }
  else
    {
      PicmanLayer *layer;

      layer = picman_image_pick_layer_by_bounds (image, coords->x, coords->y);

      if (layer)
        {
          if (add)
            align_tool->function = ALIGN_TOOL_ADD_LAYER;
          else
            align_tool->function = ALIGN_TOOL_PICK_LAYER;
        }
      else
        {
          align_tool->function = ALIGN_TOOL_IDLE;
        }
    }

  picman_align_tool_status_update (tool, display, state, proximity);
}

static void
picman_align_tool_cursor_update (PicmanTool         *tool,
                               const PicmanCoords *coords,
                               GdkModifierType   state,
                               PicmanDisplay      *display)
{
  PicmanAlignTool      *align_tool  = PICMAN_ALIGN_TOOL (tool);
  PicmanToolCursorType  tool_cursor = PICMAN_TOOL_CURSOR_NONE;
  PicmanCursorModifier  modifier    = PICMAN_CURSOR_MODIFIER_NONE;

  /* always add '+' when Shift is pressed, even if nothing is selected */
  if (state & picman_get_extend_selection_mask ())
    modifier = PICMAN_CURSOR_MODIFIER_PLUS;

  switch (align_tool->function)
    {
    case ALIGN_TOOL_IDLE:
      tool_cursor = PICMAN_TOOL_CURSOR_RECT_SELECT;
      break;

    case ALIGN_TOOL_PICK_LAYER:
    case ALIGN_TOOL_ADD_LAYER:
      tool_cursor = PICMAN_TOOL_CURSOR_HAND;
      break;

    case ALIGN_TOOL_PICK_GUIDE:
    case ALIGN_TOOL_ADD_GUIDE:
      tool_cursor = PICMAN_TOOL_CURSOR_MOVE;
      break;

    case ALIGN_TOOL_PICK_PATH:
    case ALIGN_TOOL_ADD_PATH:
      tool_cursor = PICMAN_TOOL_CURSOR_PATHS;
      break;

    case ALIGN_TOOL_DRAG_BOX:
      break;
    }

  picman_tool_control_set_cursor          (tool->control, PICMAN_CURSOR_MOUSE);
  picman_tool_control_set_tool_cursor     (tool->control, tool_cursor);
  picman_tool_control_set_cursor_modifier (tool->control, modifier);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_align_tool_status_update (PicmanTool        *tool,
                               PicmanDisplay     *display,
                               GdkModifierType  state,
                               gboolean         proximity)
{
  PicmanAlignTool   *align_tool = PICMAN_ALIGN_TOOL (tool);
  GdkModifierType  extend_mask;

  extend_mask = picman_get_extend_selection_mask ();

  picman_tool_pop_status (tool, display);

  if (proximity)
    {
      gchar *status = NULL;

      if (! align_tool->selected_objects)
        {
          /* no need to suggest Shift if nothing is selected */
          state |= extend_mask;
        }

      switch (align_tool->function)
        {
        case ALIGN_TOOL_IDLE:
          status = picman_suggest_modifiers (_("Click on a layer, path or guide, "
                                             "or Click-Drag to pick several "
                                             "layers"),
                                           extend_mask & ~state,
                                           NULL, NULL, NULL);
          break;

        case ALIGN_TOOL_PICK_LAYER:
          status = picman_suggest_modifiers (_("Click to pick this layer as "
                                             "first item"),
                                           extend_mask & ~state,
                                           NULL, NULL, NULL);
          break;

        case ALIGN_TOOL_ADD_LAYER:
          status = g_strdup (_("Click to add this layer to the list"));
          break;

        case ALIGN_TOOL_PICK_GUIDE:
          status = picman_suggest_modifiers (_("Click to pick this guide as "
                                             "first item"),
                                           extend_mask & ~state,
                                           NULL, NULL, NULL);
          break;

        case ALIGN_TOOL_ADD_GUIDE:
          status = g_strdup (_("Click to add this guide to the list"));
          break;

        case ALIGN_TOOL_PICK_PATH:
          status = picman_suggest_modifiers (_("Click to pick this path as "
                                             "first item"),
                                           extend_mask & ~state,
                                           NULL, NULL, NULL);
          break;

        case ALIGN_TOOL_ADD_PATH:
          status = g_strdup (_("Click to add this path to the list"));
          break;

        case ALIGN_TOOL_DRAG_BOX:
          break;
        }

      if (status)
        {
          picman_tool_push_status (tool, display, "%s", status);
          g_free (status);
        }
    }
}

static void
picman_align_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanAlignTool *align_tool = PICMAN_ALIGN_TOOL (draw_tool);
  GList         *list;
  gint           x, y, w, h;

  /* draw rubber-band rectangle */
  x = MIN (align_tool->x2, align_tool->x1);
  y = MIN (align_tool->y2, align_tool->y1);
  w = MAX (align_tool->x2, align_tool->x1) - x;
  h = MAX (align_tool->y2, align_tool->y1) - y;

  picman_draw_tool_add_rectangle (draw_tool, FALSE, x, y, w, h);

  for (list = align_tool->selected_objects;
       list;
       list = g_list_next (list))
    {
      if (PICMAN_IS_ITEM (list->data))
        {
          PicmanItem *item = list->data;

          if (PICMAN_IS_VECTORS (item))
            {
              gdouble x1_f, y1_f, x2_f, y2_f;

              picman_vectors_bounds (PICMAN_VECTORS (item),
                                   &x1_f, &y1_f,
                                   &x2_f, &y2_f);
              x = ROUND (x1_f);
              y = ROUND (y1_f);
              w = ROUND (x2_f - x1_f);
              h = ROUND (y2_f - y1_f);
            }
          else
            {
              picman_item_get_offset (item, &x, &y);

              w = picman_item_get_width  (item);
              h = picman_item_get_height (item);
            }

          picman_draw_tool_add_handle (draw_tool, PICMAN_HANDLE_FILLED_SQUARE,
                                     x, y,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                     PICMAN_HANDLE_ANCHOR_NORTH_WEST);
          picman_draw_tool_add_handle (draw_tool, PICMAN_HANDLE_FILLED_SQUARE,
                                     x + w, y,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                     PICMAN_HANDLE_ANCHOR_NORTH_EAST);
          picman_draw_tool_add_handle (draw_tool, PICMAN_HANDLE_FILLED_SQUARE,
                                     x, y + h,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                     PICMAN_HANDLE_ANCHOR_SOUTH_WEST);
          picman_draw_tool_add_handle (draw_tool, PICMAN_HANDLE_FILLED_SQUARE,
                                     x + w, y + h,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                     PICMAN_HANDLE_ANCHOR_SOUTH_EAST);
        }
      else if (PICMAN_IS_GUIDE (list->data))
        {
          PicmanGuide *guide = list->data;
          PicmanImage *image = picman_display_get_image (PICMAN_TOOL (draw_tool)->display);
          gint       x, y;
          gint       w, h;

          switch (picman_guide_get_orientation (guide))
            {
            case PICMAN_ORIENTATION_VERTICAL:
              x = picman_guide_get_position (guide);
              h = picman_image_get_height (image);
              picman_draw_tool_add_handle (draw_tool, PICMAN_HANDLE_FILLED_SQUARE,
                                         x, h,
                                         PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                         PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                         PICMAN_HANDLE_ANCHOR_SOUTH);
              picman_draw_tool_add_handle (draw_tool, PICMAN_HANDLE_FILLED_SQUARE,
                                         x, 0,
                                         PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                         PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                         PICMAN_HANDLE_ANCHOR_NORTH);
              break;

            case PICMAN_ORIENTATION_HORIZONTAL:
              y = picman_guide_get_position (guide);
              w = picman_image_get_width (image);
              picman_draw_tool_add_handle (draw_tool, PICMAN_HANDLE_FILLED_SQUARE,
                                         w, y,
                                         PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                         PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                         PICMAN_HANDLE_ANCHOR_EAST);
              picman_draw_tool_add_handle (draw_tool, PICMAN_HANDLE_FILLED_SQUARE,
                                         0, y,
                                         PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                         PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                         PICMAN_HANDLE_ANCHOR_WEST);
              break;

            default:
              break;
            }
        }
    }
}

static void
picman_align_tool_align (PicmanAlignTool     *align_tool,
                       PicmanAlignmentType  align_type)
{
  PicmanAlignOptions *options = PICMAN_ALIGN_TOOL_GET_OPTIONS (align_tool);
  PicmanImage        *image;
  GObject          *reference_object = NULL;
  GList            *list;
  gint              offset = 0;

  /* if nothing is selected, just return */
  if (! align_tool->selected_objects)
    return;

  image  = picman_display_get_image (PICMAN_TOOL (align_tool)->display);

  switch (align_type)
    {
    case PICMAN_ALIGN_LEFT:
    case PICMAN_ALIGN_HCENTER:
    case PICMAN_ALIGN_RIGHT:
    case PICMAN_ALIGN_TOP:
    case PICMAN_ALIGN_VCENTER:
    case PICMAN_ALIGN_BOTTOM:
      offset = 0;
      break;

    case PICMAN_ARRANGE_LEFT:
    case PICMAN_ARRANGE_HCENTER:
    case PICMAN_ARRANGE_RIGHT:
    case PICMAN_ARRANGE_TOP:
    case PICMAN_ARRANGE_VCENTER:
    case PICMAN_ARRANGE_BOTTOM:
      offset = options->offset_x;
      break;
    }

  /* if only one object is selected, use the image as reference
   * if multiple objects are selected, use the first one as reference if
   * "set_reference" is TRUE, otherwise use NULL.
   */

  list = align_tool->selected_objects;

  switch (options->align_reference)
    {
    case PICMAN_ALIGN_REFERENCE_IMAGE:
      reference_object = G_OBJECT (image);
      break;

    case PICMAN_ALIGN_REFERENCE_FIRST:
      if (g_list_length (list) == 1)
        {
          reference_object = G_OBJECT (image);
        }
      else
        {
          if (align_tool->set_reference)
            {
              reference_object = G_OBJECT (list->data);
              list = g_list_next (list);
            }
          else
            {
              reference_object = NULL;
            }
        }
      break;

    case PICMAN_ALIGN_REFERENCE_SELECTION:
      reference_object = G_OBJECT (picman_image_get_mask (image));
      break;

    case PICMAN_ALIGN_REFERENCE_ACTIVE_LAYER:
      reference_object = G_OBJECT (picman_image_get_active_layer (image));
      break;

    case PICMAN_ALIGN_REFERENCE_ACTIVE_CHANNEL:
      reference_object = G_OBJECT (picman_image_get_active_channel (image));
      break;

    case PICMAN_ALIGN_REFERENCE_ACTIVE_PATH:
      g_print ("reference = active path not yet handled.\n");
      break;
    }

  if (! reference_object)
    return;

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (align_tool));

  picman_image_arrange_objects (image, list,
                              align_type,
                              reference_object,
                              align_type,
                              offset);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (align_tool));

  picman_image_flush (image);
}

static void
picman_align_tool_object_removed (GObject       *object,
                                PicmanAlignTool *align_tool)
{
  picman_draw_tool_pause (PICMAN_DRAW_TOOL (align_tool));

  if (align_tool->selected_objects)
    g_signal_handlers_disconnect_by_func (object,
                                          picman_align_tool_object_removed,
                                          align_tool);

  align_tool->selected_objects = g_list_remove (align_tool->selected_objects,
                                                object);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (align_tool));
}

static void
picman_align_tool_clear_selected (PicmanAlignTool *align_tool)
{
  picman_draw_tool_pause (PICMAN_DRAW_TOOL (align_tool));

  while (align_tool->selected_objects)
    picman_align_tool_object_removed (align_tool->selected_objects->data,
                                    align_tool);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (align_tool));
}
