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

#include <stdlib.h>
#include <stdarg.h>

#include <gegl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmanboundary.h"
#include "core/picmanimage.h"
#include "core/picmanimage-guides.h"
#include "core/picmanimage-item-list.h"
#include "core/picmanimage-undo.h"
#include "core/picmanitem-linked.h"
#include "core/picmanlayer.h"
#include "core/picmanlayermask.h"
#include "core/picmanprojection.h"
#include "core/picmanselection.h"
#include "core/picmanundostack.h"

#include "vectors/picmanvectors.h"

#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-appearance.h"
#include "display/picmandisplayshell-selection.h"
#include "display/picmandisplayshell-transform.h"

#include "picmandrawtool.h"
#include "picmaneditselectiontool.h"
#include "picmantoolcontrol.h"
#include "tool_manager.h"

#include "picman-intl.h"


#define ARROW_VELOCITY 25


typedef struct _PicmanEditSelectionTool      PicmanEditSelectionTool;
typedef struct _PicmanEditSelectionToolClass PicmanEditSelectionToolClass;

struct _PicmanEditSelectionTool
{
  PicmanDrawTool        parent_instance;

  gint                origx, origy;    /*  Last x and y coords               */
  gint                cumlx, cumly;    /*  Cumulative changes to x and yed   */
  gint                x, y;            /*  Current x and y coords            */
  gint                num_segs_in;     /*  Num seg in selection boundary     */
  gint                num_segs_out;    /*  Num seg in selection boundary     */
  PicmanBoundSeg       *segs_in;         /*  Pointer to the channel sel. segs  */
  PicmanBoundSeg       *segs_out;        /*  Pointer to the channel sel. segs  */

  gint                x1, y1;          /*  Bounding box of selection mask    */
  gint                x2, y2;

  gdouble             center_x;        /*  Where to draw the mark of center  */
  gdouble             center_y;

  PicmanTranslateMode   edit_mode;       /*  Translate the mask or layer?      */

  gboolean            first_move;      /*  Don't push undos after the first  */

  gboolean            propagate_release;

  gboolean            constrain;       /*  Constrain the movement            */
  gdouble             start_x, start_y;/*  Coords when button was pressed    */
  gdouble             last_x,  last_y; /*  Previous coords sent to _motion   */
};

struct _PicmanEditSelectionToolClass
{
  PicmanDrawToolClass   parent_class;
};


static void       picman_edit_selection_tool_button_release      (PicmanTool                    *tool,
                                                                const PicmanCoords            *coords,
                                                                guint32                      time,
                                                                GdkModifierType              state,
                                                                PicmanButtonReleaseType        release_type,
                                                                PicmanDisplay                 *display);
static void       picman_edit_selection_tool_motion              (PicmanTool                    *tool,
                                                                const PicmanCoords            *coords,
                                                                guint32                      time,
                                                                GdkModifierType              state,
                                                                PicmanDisplay                 *display);
static void       picman_edit_selection_tool_active_modifier_key (PicmanTool                    *tool,
                                                                GdkModifierType              key,
                                                                gboolean                     press,
                                                                GdkModifierType              state,
                                                                PicmanDisplay                 *display);
static void       picman_edit_selection_tool_draw                (PicmanDrawTool                *tool);

static PicmanItem * picman_edit_selection_tool_get_active_item     (const PicmanEditSelectionTool *edit_select,
                                                                const PicmanImage             *image);


G_DEFINE_TYPE (PicmanEditSelectionTool, picman_edit_selection_tool,
               PICMAN_TYPE_DRAW_TOOL)

#define parent_class picman_edit_selection_tool_parent_class


static void
picman_edit_selection_tool_class_init (PicmanEditSelectionToolClass *klass)
{
  PicmanToolClass     *tool_class   = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_class   = PICMAN_DRAW_TOOL_CLASS (klass);

  tool_class->button_release      = picman_edit_selection_tool_button_release;
  tool_class->motion              = picman_edit_selection_tool_motion;
  tool_class->active_modifier_key = picman_edit_selection_tool_active_modifier_key;

  draw_class->draw                = picman_edit_selection_tool_draw;
}

static void
picman_edit_selection_tool_init (PicmanEditSelectionTool *edit_selection_tool)
{
  PicmanTool *tool = PICMAN_TOOL (edit_selection_tool);

  picman_tool_control_set_motion_mode (tool->control, PICMAN_MOTION_MODE_COMPRESS);

  edit_selection_tool->origx      = 0;
  edit_selection_tool->origy      = 0;

  edit_selection_tool->cumlx      = 0;
  edit_selection_tool->cumly      = 0;

  edit_selection_tool->first_move = TRUE;

  edit_selection_tool->constrain  = FALSE;
}

static void
picman_edit_selection_tool_calc_coords (PicmanEditSelectionTool *edit_select,
                                      gdouble                x,
                                      gdouble                y)
{
  gdouble x1, y1;
  gdouble dx, dy;

  dx = x - edit_select->origx;
  dy = y - edit_select->origy;

  x1 = edit_select->x1 + dx;
  y1 = edit_select->y1 + dy;

  edit_select->x = (gint) floor (x1) - (edit_select->x1 - edit_select->origx);
  edit_select->y = (gint) floor (y1) - (edit_select->y1 - edit_select->origy);
}

void
picman_edit_selection_tool_start (PicmanTool          *parent_tool,
                                PicmanDisplay       *display,
                                const PicmanCoords  *coords,
                                PicmanTranslateMode  edit_mode,
                                gboolean           propagate_release)
{
  PicmanEditSelectionTool *edit_select;
  PicmanTool              *tool;
  PicmanDisplayShell      *shell;
  PicmanImage             *image;
  PicmanItem              *active_item;
  PicmanChannel           *channel;
  gint                   off_x, off_y;
  const PicmanBoundSeg    *segs_in;
  const PicmanBoundSeg    *segs_out;
  const gchar           *undo_desc;

  edit_select = g_object_new (PICMAN_TYPE_EDIT_SELECTION_TOOL,
                              "tool-info", parent_tool->tool_info,
                              NULL);

  edit_select->propagate_release = propagate_release;

  tool = PICMAN_TOOL (edit_select);

  shell = picman_display_get_shell (display);
  image = picman_display_get_image (display);

  /*  Make a check to see if it should be a floating selection translation  */
  if ((edit_mode == PICMAN_TRANSLATE_MODE_MASK_TO_LAYER ||
       edit_mode == PICMAN_TRANSLATE_MODE_MASK_COPY_TO_LAYER) &&
      picman_image_get_floating_selection (image))
    {
      edit_mode = PICMAN_TRANSLATE_MODE_FLOATING_SEL;
    }

  if (edit_mode == PICMAN_TRANSLATE_MODE_LAYER)
    {
      PicmanLayer *layer = picman_image_get_active_layer (image);

      if (picman_layer_is_floating_sel (layer))
        edit_mode = PICMAN_TRANSLATE_MODE_FLOATING_SEL;
    }

  edit_select->edit_mode = edit_mode;

  active_item = picman_edit_selection_tool_get_active_item (edit_select, image);

  switch (edit_select->edit_mode)
    {
    case PICMAN_TRANSLATE_MODE_VECTORS:
    case PICMAN_TRANSLATE_MODE_CHANNEL:
    case PICMAN_TRANSLATE_MODE_LAYER_MASK:
    case PICMAN_TRANSLATE_MODE_LAYER:
      undo_desc = PICMAN_ITEM_GET_CLASS (active_item)->translate_desc;
      break;

    case PICMAN_TRANSLATE_MODE_MASK:
      undo_desc = _("Move Selection");
      break;

    default:
      undo_desc = _("Move Floating Selection");
      break;
    }

  picman_image_undo_group_start (image,
                               edit_select->edit_mode ==
                               PICMAN_TRANSLATE_MODE_MASK ?
                               PICMAN_UNDO_GROUP_MASK :
                               PICMAN_UNDO_GROUP_ITEM_DISPLACE,
                               undo_desc);

  picman_item_get_offset (active_item, &off_x, &off_y);

  edit_select->x = edit_select->origx = coords->x - off_x;
  edit_select->y = edit_select->origy = coords->y - off_y;

  /* Remember starting point for use in constrained movement */
  edit_select->start_x = coords->x;
  edit_select->start_y = coords->y;

  edit_select->constrain = FALSE;

  switch (edit_select->edit_mode)
    {
    case PICMAN_TRANSLATE_MODE_CHANNEL:
    case PICMAN_TRANSLATE_MODE_LAYER_MASK:
      channel = PICMAN_CHANNEL (active_item);
     break;

    default:
      channel = picman_image_get_mask (image);
      break;
    }

  picman_channel_boundary (channel,
                         &segs_in, &segs_out,
                         &edit_select->num_segs_in, &edit_select->num_segs_out,
                         0, 0, 0, 0);

  edit_select->segs_in = g_memdup (segs_in,
                                   edit_select->num_segs_in * sizeof (PicmanBoundSeg));

  edit_select->segs_out = g_memdup (segs_out,
                                    edit_select->num_segs_out * sizeof (PicmanBoundSeg));

  if (edit_select->edit_mode == PICMAN_TRANSLATE_MODE_VECTORS)
    {
      edit_select->x1 = 0;
      edit_select->y1 = 0;
      edit_select->x2 = picman_image_get_width  (image);
      edit_select->y2 = picman_image_get_height (image);
    }
  else
    {
      /*  find the bounding box of the selection mask -
       *  this is used for the case of a PICMAN_TRANSLATE_MODE_MASK_TO_LAYER,
       *  where the translation will result in floating the selection
       *  mask and translating the resulting layer
       */
      picman_item_mask_bounds (active_item,
                             &edit_select->x1, &edit_select->y1,
                             &edit_select->x2, &edit_select->y2);
    }

  picman_edit_selection_tool_calc_coords (edit_select,
                                        edit_select->origx,
                                        edit_select->origy);

  {
    gint x1, y1, x2, y2;

    switch (edit_select->edit_mode)
      {
      case PICMAN_TRANSLATE_MODE_CHANNEL:
        picman_channel_bounds (PICMAN_CHANNEL (active_item),
                             &x1, &y1, &x2, &y2);
        break;

      case PICMAN_TRANSLATE_MODE_LAYER_MASK:
        picman_channel_bounds (PICMAN_CHANNEL (active_item),
                             &x1, &y1, &x2, &y2);
        x1 += off_x;
        y1 += off_y;
        x2 += off_x;
        y2 += off_y;
        break;

      case PICMAN_TRANSLATE_MODE_MASK:
        picman_channel_bounds (picman_image_get_mask (image),
                             &x1, &y1, &x2, &y2);
        break;

      case PICMAN_TRANSLATE_MODE_MASK_TO_LAYER:
      case PICMAN_TRANSLATE_MODE_MASK_COPY_TO_LAYER:
        x1 = edit_select->x1 + off_x;
        y1 = edit_select->y1 + off_y;
        x2 = edit_select->x2 + off_x;
        y2 = edit_select->y2 + off_y;
        break;

      case PICMAN_TRANSLATE_MODE_LAYER:
      case PICMAN_TRANSLATE_MODE_FLOATING_SEL:
        x1 = off_x;
        y1 = off_y;
        x2 = x1 + picman_item_get_width  (active_item);
        y2 = y1 + picman_item_get_height (active_item);

        if (picman_item_get_linked (active_item))
          {
            GList *linked;
            GList *list;

            linked = picman_image_item_list_get_list (image,
                                                    active_item,
                                                    PICMAN_ITEM_TYPE_LAYERS,
                                                    PICMAN_ITEM_SET_LINKED);

            linked = picman_image_item_list_filter (active_item, linked,
                                                  TRUE, FALSE);

            /*  Expand the rectangle to include all linked layers as well  */
            for (list = linked; list; list = g_list_next (list))
              {
                PicmanItem *item = list->data;
                gint      x3, y3;
                gint      x4, y4;

                picman_item_get_offset (item, &x3, &y3);

                x4 = x3 + picman_item_get_width  (item);
                y4 = y3 + picman_item_get_height (item);

                x1 = MIN (x1, x3);
                y1 = MIN (y1, y3);
                x2 = MAX (x2, x4);
                y2 = MAX (y2, y4);
              }

            g_list_free (linked);
          }
        break;

      case PICMAN_TRANSLATE_MODE_VECTORS:
        {
          gdouble  xd1, yd1, xd2, yd2;

          picman_vectors_bounds (PICMAN_VECTORS (active_item),
                               &xd1, &yd1, &xd2, &yd2);

          if (picman_item_get_linked (active_item))
            {
              /*  Expand the rectangle to include all linked layers as well  */

              GList *linked;
              GList *list;

              linked = picman_image_item_list_get_list (image,
                                                      active_item,
                                                      PICMAN_ITEM_TYPE_VECTORS,
                                                      PICMAN_ITEM_SET_LINKED);

              linked = picman_image_item_list_filter (active_item, linked,
                                                    TRUE, FALSE);

              for (list = linked; list; list = g_list_next (list))
                {
                  PicmanItem *item = list->data;
                  gdouble   x3, y3;
                  gdouble   x4, y4;

                  picman_vectors_bounds (PICMAN_VECTORS (item), &x3, &y3, &x4, &y4);

                  xd1 = MIN (xd1, x3);
                  yd1 = MIN (yd1, y3);
                  xd2 = MAX (xd2, x4);
                  yd2 = MAX (yd2, y4);
                }
            }

          x1 = ROUND (floor (xd1));
          y1 = ROUND (floor (yd1));
          x2 = ROUND (ceil (xd2));
          y2 = ROUND (ceil (yd2));
        }
        break;
      }

    picman_tool_control_set_snap_offsets (tool->control,
                                        x1 - coords->x,
                                        y1 - coords->y,
                                        x2 - x1,
                                        y2 - y1);

    /* Save where to draw the mark of the center */
    edit_select->center_x = (x1 + x2) / 2.0;
    edit_select->center_y = (y1 + y2) / 2.0;
  }

  tool_manager_push_tool (display->picman, tool);

  picman_tool_control_activate (tool->control);
  tool->display = display;

  /*  pause the current selection  */
  picman_display_shell_selection_pause (shell);

  /* initialize the statusbar display */
  picman_tool_push_status_coords (tool, display,
                                picman_tool_control_get_precision (tool->control),
                                _("Move: "), 0, ", ", 0, NULL);

  picman_draw_tool_start (PICMAN_DRAW_TOOL (edit_select), display);
}


static void
picman_edit_selection_tool_button_release (PicmanTool              *tool,
                                         const PicmanCoords      *coords,
                                         guint32                time,
                                         GdkModifierType        state,
                                         PicmanButtonReleaseType  release_type,
                                         PicmanDisplay           *display)
{
  PicmanEditSelectionTool *edit_select = PICMAN_EDIT_SELECTION_TOOL (tool);
  PicmanDisplayShell      *shell       = picman_display_get_shell (display);
  PicmanImage             *image       = picman_display_get_image (display);
  PicmanItem              *active_item;

  /*  resume the current selection  */
  picman_display_shell_selection_resume (shell);

  picman_tool_pop_status (tool, display);

  picman_tool_control_halt (tool->control);

  /*  Stop and free the selection core  */
  picman_draw_tool_stop (PICMAN_DRAW_TOOL (edit_select));

  tool_manager_pop_tool (display->picman);

  active_item = picman_edit_selection_tool_get_active_item (edit_select, image);

  picman_edit_selection_tool_calc_coords (edit_select,
                                        coords->x,
                                        coords->y);

  /* PICMAN_TRANSLATE_MODE_MASK is performed here at movement end, not 'live' like
   *  the other translation types.
   */
  if (edit_select->edit_mode == PICMAN_TRANSLATE_MODE_MASK)
    {
      /* move the selection -- whether there has been movement or not!
       * (to ensure that there's something on the undo stack)
       */
      picman_item_translate (PICMAN_ITEM (picman_image_get_mask (image)),
                           edit_select->cumlx,
                           edit_select->cumly,
                           TRUE);
    }

  /*  PICMAN_TRANSLATE_MODE_CHANNEL and PICMAN_TRANSLATE_MODE_LAYER_MASK
   *  need to be preformed after thawing the undo.
   */
  if (edit_select->edit_mode == PICMAN_TRANSLATE_MODE_CHANNEL ||
      edit_select->edit_mode == PICMAN_TRANSLATE_MODE_LAYER_MASK)
    {
      /* move the channel -- whether there has been movement or not!
       * (to ensure that there's something on the undo stack)
       */
      picman_item_translate (active_item,
                           edit_select->cumlx,
                           edit_select->cumly,
                           TRUE);
    }

  if (edit_select->edit_mode == PICMAN_TRANSLATE_MODE_VECTORS ||
      edit_select->edit_mode == PICMAN_TRANSLATE_MODE_CHANNEL ||
      edit_select->edit_mode == PICMAN_TRANSLATE_MODE_LAYER)
    {
      if ((release_type != PICMAN_BUTTON_RELEASE_CANCEL) &&
          (edit_select->cumlx != 0 ||
           edit_select->cumly != 0))
        {
          if (picman_item_get_linked (active_item))
            {
              /*  translate all linked channels as well  */

              GList *linked;

              linked = picman_image_item_list_get_list (image,
                                                      active_item,
                                                      PICMAN_ITEM_TYPE_CHANNELS,
                                                      PICMAN_ITEM_SET_LINKED);

              linked = picman_image_item_list_filter (active_item, linked,
                                                    TRUE, FALSE);

              picman_image_item_list_translate (image,
                                              linked,
                                              edit_select->cumlx,
                                              edit_select->cumly,
                                              TRUE);

              g_list_free (linked);
            }
        }
    }

  picman_image_undo_group_end (image);

  if (release_type == PICMAN_BUTTON_RELEASE_CANCEL)
    {
      /* Operation cancelled - undo the undo-group! */
      picman_image_undo (image);
    }

  picman_image_flush (image);

  g_free (edit_select->segs_in);
  g_free (edit_select->segs_out);

  edit_select->segs_in      = NULL;
  edit_select->segs_out     = NULL;
  edit_select->num_segs_in  = 0;
  edit_select->num_segs_out = 0;

  if (edit_select->propagate_release &&
      tool_manager_get_active (display->picman))
    {
      tool_manager_button_release_active (display->picman,
                                          coords, time, state,
                                          display);
    }

  g_object_unref (edit_select);
}

static void
picman_edit_selection_tool_update_motion (PicmanEditSelectionTool *edit_select,
                                        gdouble                new_x,
                                        gdouble                new_y,
                                        PicmanDisplay           *display)
{
  PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (edit_select);
  PicmanTool     *tool      = PICMAN_TOOL (edit_select);
  PicmanImage    *image     = picman_display_get_image (display);
  PicmanItem     *active_item;
  gint          off_x, off_y;
  gdouble       motion_x, motion_y;
  gint          x, y;

  gdk_flush ();

  picman_draw_tool_pause (draw_tool);

  active_item = picman_edit_selection_tool_get_active_item (edit_select, image);

  picman_item_get_offset (active_item, &off_x, &off_y);

  if (edit_select->constrain)
    {
      picman_constrain_line (edit_select->start_x, edit_select->start_y,
                           &new_x, &new_y,
                           PICMAN_CONSTRAIN_LINE_45_DEGREES);
    }

  motion_x = new_x - off_x;
  motion_y = new_y - off_y;

  /* now do the actual move. */

  picman_edit_selection_tool_calc_coords (edit_select,
                                        motion_x,
                                        motion_y);
  x = edit_select->x;
  y = edit_select->y;

  /* if there has been movement, move the selection  */
  if (edit_select->origx != x || edit_select->origy != y)
    {
      gint    xoffset;
      gint    yoffset;
      GError *error = NULL;

      xoffset = x - edit_select->origx;
      yoffset = y - edit_select->origy;

      edit_select->cumlx += xoffset;
      edit_select->cumly += yoffset;

      switch (edit_select->edit_mode)
        {
        case PICMAN_TRANSLATE_MODE_LAYER_MASK:
        case PICMAN_TRANSLATE_MODE_MASK:
          /*  we don't do the actual edit selection move here.  */
          edit_select->origx = x;
          edit_select->origy = y;
          break;

        case PICMAN_TRANSLATE_MODE_VECTORS:
        case PICMAN_TRANSLATE_MODE_CHANNEL:
          edit_select->origx = x;
          edit_select->origy = y;

          /*  fallthru  */

        case PICMAN_TRANSLATE_MODE_LAYER:
          /*  for CHANNEL_TRANSLATE, only translate the linked layers
           *  and vectors on-the-fly, the channel is translated
           *  on button_release.
           */
          if (edit_select->edit_mode != PICMAN_TRANSLATE_MODE_CHANNEL)
            picman_item_translate (active_item, xoffset, yoffset,
                                 edit_select->first_move);

          if (picman_item_get_linked (active_item))
            {
              /*  translate all linked layers & vectors as well  */

              GList *linked;

              linked = picman_image_item_list_get_list (image,
                                                      active_item,
                                                      PICMAN_ITEM_TYPE_LAYERS |
                                                      PICMAN_ITEM_TYPE_VECTORS,
                                                      PICMAN_ITEM_SET_LINKED);

              linked = picman_image_item_list_filter (active_item, linked,
                                                    TRUE, FALSE);

              picman_image_item_list_translate (image,
                                              linked,
                                              xoffset, yoffset,
                                              edit_select->first_move);

              g_list_free (linked);
            }
          break;

        case PICMAN_TRANSLATE_MODE_MASK_TO_LAYER:
        case PICMAN_TRANSLATE_MODE_MASK_COPY_TO_LAYER:
          if (! picman_selection_float (PICMAN_SELECTION (picman_image_get_mask (image)),
                                      PICMAN_DRAWABLE (active_item),
                                      picman_get_user_context (display->picman),
                                      edit_select->edit_mode ==
                                      PICMAN_TRANSLATE_MODE_MASK_TO_LAYER,
                                      0, 0, &error))
            {
              /* no region to float, abort safely */
              picman_message_literal (display->picman, G_OBJECT (display),
				    PICMAN_MESSAGE_WARNING,
				    error->message);
              g_clear_error (&error);
              picman_draw_tool_resume (draw_tool);

              return;
            }

          edit_select->origx -= edit_select->x1;
          edit_select->origy -= edit_select->y1;
          edit_select->x2    -= edit_select->x1;
          edit_select->y2    -= edit_select->y1;
          edit_select->x1     = 0;
          edit_select->y1     = 0;

          edit_select->edit_mode = PICMAN_TRANSLATE_MODE_FLOATING_SEL;

          active_item =
            PICMAN_ITEM (picman_image_get_active_drawable (image));

          /* fall through */

        case PICMAN_TRANSLATE_MODE_FLOATING_SEL:
          picman_item_translate (active_item, xoffset, yoffset,
                               edit_select->first_move);
          break;
        }

      edit_select->first_move = FALSE;
    }

  picman_projection_flush (picman_image_get_projection (image));

  picman_tool_pop_status (tool, display);
  picman_tool_push_status_coords (tool, display,
                                picman_tool_control_get_precision (tool->control),
                                _("Move: "),
                                edit_select->cumlx,
                                ", ",
                                edit_select->cumly,
                                NULL);

  picman_draw_tool_resume (draw_tool);
}


static void
picman_edit_selection_tool_motion (PicmanTool         *tool,
                                 const PicmanCoords *coords,
                                 guint32           time,
                                 GdkModifierType   state,
                                 PicmanDisplay      *display)
{
  PicmanEditSelectionTool *edit_select = PICMAN_EDIT_SELECTION_TOOL (tool);

  edit_select->last_x = coords->x;
  edit_select->last_y = coords->y;

  picman_edit_selection_tool_update_motion (edit_select,
                                          coords->x, coords->y,
                                          display);
}

static void
picman_edit_selection_tool_active_modifier_key (PicmanTool        *tool,
                                              GdkModifierType  key,
                                              gboolean         press,
                                              GdkModifierType  state,
                                              PicmanDisplay     *display)
{
  PicmanEditSelectionTool *edit_select = PICMAN_EDIT_SELECTION_TOOL (tool);

  edit_select->constrain = (state & picman_get_constrain_behavior_mask () ?
                            TRUE : FALSE);

  /* If we didn't came here due to a mouse release, immediately update
   * the position of the thing we move.
   */
  if (state & GDK_BUTTON1_MASK)
    {
      picman_edit_selection_tool_update_motion (edit_select,
                                              edit_select->last_x,
                                              edit_select->last_y,
                                              display);
    }
}

static void
picman_edit_selection_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanEditSelectionTool *edit_select = PICMAN_EDIT_SELECTION_TOOL (draw_tool);
  PicmanDisplay           *display     = PICMAN_TOOL (draw_tool)->display;
  PicmanImage             *image       = picman_display_get_image (display);
  PicmanItem              *active_item;

  active_item = picman_edit_selection_tool_get_active_item (edit_select, image);

  switch (edit_select->edit_mode)
    {
    case PICMAN_TRANSLATE_MODE_CHANNEL:
    case PICMAN_TRANSLATE_MODE_LAYER_MASK:
    case PICMAN_TRANSLATE_MODE_MASK:
      {
        gboolean floating_sel = FALSE;
        gint     off_x        = 0;
        gint     off_y        = 0;

        if (edit_select->edit_mode == PICMAN_TRANSLATE_MODE_MASK)
          {
            PicmanLayer *layer = picman_image_get_active_layer (image);

            if (layer)
              floating_sel = picman_layer_is_floating_sel (layer);
          }
        else
          {
            picman_item_get_offset (active_item, &off_x, &off_y);
          }

        if (! floating_sel && edit_select->segs_in)
          {
            picman_draw_tool_add_boundary (draw_tool,
                                         edit_select->segs_in,
                                         edit_select->num_segs_in,
                                         NULL,
                                         edit_select->cumlx + off_x,
                                         edit_select->cumly + off_y);
          }

        if (edit_select->segs_out)
          {
            picman_draw_tool_add_boundary (draw_tool,
                                         edit_select->segs_out,
                                         edit_select->num_segs_out,
                                         NULL,
                                         edit_select->cumlx + off_x,
                                         edit_select->cumly + off_y);
          }
        else if (edit_select->edit_mode != PICMAN_TRANSLATE_MODE_MASK)
          {
            picman_draw_tool_add_rectangle (draw_tool,
                                          FALSE,
                                          edit_select->cumlx + off_x,
                                          edit_select->cumly + off_y,
                                          picman_item_get_width  (active_item),
                                          picman_item_get_height (active_item));
          }
      }
      break;

    case PICMAN_TRANSLATE_MODE_MASK_TO_LAYER:
    case PICMAN_TRANSLATE_MODE_MASK_COPY_TO_LAYER:
      {
        gint off_x;
        gint off_y;

        picman_item_get_offset (active_item, &off_x, &off_y);

        picman_draw_tool_add_rectangle (draw_tool,
                                      FALSE,
                                      edit_select->x1 + off_x,
                                      edit_select->y1 + off_y,
                                      edit_select->x2 - edit_select->x1,
                                      edit_select->y2 - edit_select->y1);
      }
      break;

    case PICMAN_TRANSLATE_MODE_LAYER:
      {
        PicmanItem *active_item;
        gint      x1, y1, x2, y2;

        active_item = PICMAN_ITEM (picman_image_get_active_layer (image));

        picman_item_get_offset (active_item, &x1, &y1);

        x2 = x1 + picman_item_get_width  (active_item);
        y2 = y1 + picman_item_get_height (active_item);

        if (picman_item_get_linked (active_item))
          {
            /*  Expand the rectangle to include all linked layers as well  */

            GList *linked;
            GList *list;

            linked = picman_image_item_list_get_list (image,
                                                    active_item,
                                                    PICMAN_ITEM_TYPE_LAYERS,
                                                    PICMAN_ITEM_SET_LINKED);

            linked = picman_image_item_list_filter (active_item, linked,
                                                  TRUE, FALSE);

            for (list = linked; list; list = g_list_next (list))
              {
                PicmanItem *item = list->data;
                gint      x3, y3;
                gint      x4, y4;

                picman_item_get_offset (item, &x3, &y3);

                x4 = x3 + picman_item_get_width  (item);
                y4 = y3 + picman_item_get_height (item);

                x1 = MIN (x1, x3);
                y1 = MIN (y1, y3);
                x2 = MAX (x2, x4);
                y2 = MAX (y2, y4);
              }

            g_list_free (linked);
          }

        picman_draw_tool_add_rectangle (draw_tool, FALSE,
                                      x1, y1,
                                      x2 - x1, y2 - y1);
      }
      break;

    case PICMAN_TRANSLATE_MODE_VECTORS:
      {
        PicmanItem *active_item;
        gdouble   x1, y1, x2, y2;

        active_item = PICMAN_ITEM (picman_image_get_active_vectors (image));

        picman_vectors_bounds (PICMAN_VECTORS (active_item), &x1, &y1, &x2, &y2);

        if (picman_item_get_linked (active_item))
          {
            /*  Expand the rectangle to include all linked vectors as well  */

            GList *linked;
            GList *list;

            linked = picman_image_item_list_get_list (image,
                                                    active_item,
                                                    PICMAN_ITEM_TYPE_VECTORS,
                                                    PICMAN_ITEM_SET_LINKED);

            linked = picman_image_item_list_filter (active_item, linked,
                                                  TRUE, FALSE);

            for (list = linked; list; list = g_list_next (list))
              {
                PicmanItem *item = list->data;
                gdouble   x3, y3;
                gdouble   x4, y4;

                picman_vectors_bounds (PICMAN_VECTORS (item), &x3, &y3, &x4, &y4);

                x1 = MIN (x1, x3);
                y1 = MIN (y1, y3);
                x2 = MAX (x2, x4);
                y2 = MAX (y2, y4);
              }

            g_list_free (linked);
          }

        x1 = floor (x1);
        y1 = floor (y1);
        x2 = ceil (x2);
        y2 = ceil (y2);

        picman_draw_tool_add_rectangle (draw_tool, FALSE,
                                      x1, y1,
                                      x2 - x1, y2 - y1);
      }
      break;

    case PICMAN_TRANSLATE_MODE_FLOATING_SEL:
      picman_draw_tool_add_boundary (draw_tool,
                                   edit_select->segs_in,
                                   edit_select->num_segs_in,
                                   NULL,
                                   edit_select->cumlx,
                                   edit_select->cumly);
      break;
    }

  /* Mark the center because we snap to it */
  picman_draw_tool_add_handle (draw_tool,
                             PICMAN_HANDLE_CROSS,
                             edit_select->center_x + edit_select->cumlx,
                             edit_select->center_y + edit_select->cumly,
                             PICMAN_TOOL_HANDLE_SIZE_SMALL,
                             PICMAN_TOOL_HANDLE_SIZE_SMALL,
                             PICMAN_HANDLE_ANCHOR_CENTER);

  PICMAN_DRAW_TOOL_CLASS (parent_class)->draw (draw_tool);
}

static PicmanItem *
picman_edit_selection_tool_get_active_item (const PicmanEditSelectionTool *edit_select,
                                          const PicmanImage             *image)
{
  PicmanItem *active_item;

  if (edit_select->edit_mode == PICMAN_TRANSLATE_MODE_VECTORS)
    active_item = PICMAN_ITEM (picman_image_get_active_vectors (image));
  else
    active_item = PICMAN_ITEM (picman_image_get_active_drawable (image));

  return active_item;
}

static gint
process_event_queue_keys (GdkEventKey *kevent,
                          ... /* GdkKeyType, GdkModifierType, value ... 0 */)
{

#define FILTER_MAX_KEYS 50

  va_list          argp;
  GdkEvent        *event;
  GList           *event_list = NULL;
  GList           *list;
  guint            keys[FILTER_MAX_KEYS];
  GdkModifierType  modifiers[FILTER_MAX_KEYS];
  gint             values[FILTER_MAX_KEYS];
  gint             i      = 0;
  gint             n_keys = 0;
  gint             value  = 0;
  gboolean         done   = FALSE;
  GtkWidget       *orig_widget;

  va_start (argp, kevent);

  while (n_keys < FILTER_MAX_KEYS &&
         (keys[n_keys] = va_arg (argp, guint)) != 0)
    {
      modifiers[n_keys] = va_arg (argp, GdkModifierType);
      values[n_keys]    = va_arg (argp, gint);
      n_keys++;
    }

  va_end (argp);

  for (i = 0; i < n_keys; i++)
    if (kevent->keyval                 == keys[i] &&
        (kevent->state & modifiers[i]) == modifiers[i])
      value += values[i];

  orig_widget = gtk_get_event_widget ((GdkEvent *) kevent);

  while (gdk_events_pending () > 0 && ! done)
    {
      gboolean discard_event = FALSE;

      event = gdk_event_get ();

      if (! event || orig_widget != gtk_get_event_widget (event))
        {
          done = TRUE;
        }
      else
        {
          if (event->any.type == GDK_KEY_PRESS)
            {
              for (i = 0; i < n_keys; i++)
                if (event->key.keyval                 == keys[i] &&
                    (event->key.state & modifiers[i]) == modifiers[i])
                  {
                    discard_event = TRUE;
                    value += values[i];
                  }

              if (! discard_event)
                done = TRUE;
            }
          /* should there be more types here? */
          else if (event->any.type != GDK_KEY_RELEASE &&
                   event->any.type != GDK_MOTION_NOTIFY &&
                   event->any.type != GDK_EXPOSE)
            done = FALSE;
        }

      if (! event)
        ; /* Do nothing */
      else if (! discard_event)
        event_list = g_list_prepend (event_list, event);
      else
        gdk_event_free (event);
    }

  event_list = g_list_reverse (event_list);

  /* unget the unused events and free the list */
  for (list = event_list; list; list = g_list_next (list))
    {
      gdk_event_put ((GdkEvent *) list->data);
      gdk_event_free ((GdkEvent *) list->data);
    }

  g_list_free (event_list);

  return value;

#undef FILTER_MAX_KEYS
}

gboolean
picman_edit_selection_tool_key_press (PicmanTool    *tool,
                                    GdkEventKey *kevent,
                                    PicmanDisplay *display)
{
  PicmanTransformType translate_type;

  if (kevent->state & GDK_MOD1_MASK)
    translate_type = PICMAN_TRANSFORM_TYPE_SELECTION;
  else if (kevent->state & picman_get_toggle_behavior_mask ())
    translate_type = PICMAN_TRANSFORM_TYPE_PATH;
  else
    translate_type = PICMAN_TRANSFORM_TYPE_LAYER;

  return picman_edit_selection_tool_translate (tool, kevent, translate_type,
                                             display);
}

gboolean
picman_edit_selection_tool_translate (PicmanTool          *tool,
                                    GdkEventKey       *kevent,
                                    PicmanTransformType  translate_type,
                                    PicmanDisplay       *display)
{
  gint               inc_x     = 0;
  gint               inc_y     = 0;
  PicmanUndo          *undo;
  gboolean           push_undo = TRUE;
  PicmanImage         *image     = picman_display_get_image (display);
  PicmanItem          *item      = NULL;
  PicmanTranslateMode  edit_mode = PICMAN_TRANSLATE_MODE_MASK;
  PicmanUndoType       undo_type = PICMAN_UNDO_GROUP_MASK;
  const gchar       *undo_desc = NULL;
  gint               velocity;

  /* bail out early if it is not an arrow key event */

  if (kevent->keyval != GDK_KEY_Left &&
      kevent->keyval != GDK_KEY_Right &&
      kevent->keyval != GDK_KEY_Up &&
      kevent->keyval != GDK_KEY_Down)
    return FALSE;

  /*  adapt arrow velocity to the zoom factor when holding <shift>  */
  velocity = (ARROW_VELOCITY /
              picman_zoom_model_get_factor (picman_display_get_shell (display)->zoom));
  velocity = MAX (1.0, velocity);

  /*  check the event queue for key events with the same modifier mask
   *  as the current event, allowing only GDK_SHIFT_MASK to vary between
   *  them.
   */
  inc_x = process_event_queue_keys (kevent,
                                    GDK_KEY_Left,
                                    kevent->state | GDK_SHIFT_MASK,
                                    -1 * velocity,

                                    GDK_KEY_Left,
                                    kevent->state & ~GDK_SHIFT_MASK,
                                    -1,

                                    GDK_KEY_Right,
                                    kevent->state | GDK_SHIFT_MASK,
                                    1 * velocity,

                                    GDK_KEY_Right,
                                    kevent->state & ~GDK_SHIFT_MASK,
                                    1,

                                    0);

  inc_y = process_event_queue_keys (kevent,
                                    GDK_KEY_Up,
                                    kevent->state | GDK_SHIFT_MASK,
                                    -1 * velocity,

                                    GDK_KEY_Up,
                                    kevent->state & ~GDK_SHIFT_MASK,
                                    -1,

                                    GDK_KEY_Down,
                                    kevent->state | GDK_SHIFT_MASK,
                                    1 * velocity,

                                    GDK_KEY_Down,
                                    kevent->state & ~GDK_SHIFT_MASK,
                                    1,

                                    0);

  if (inc_x != 0 || inc_y != 0)
    {
      switch (translate_type)
        {
        case PICMAN_TRANSFORM_TYPE_SELECTION:
          item = PICMAN_ITEM (picman_image_get_mask (image));

          edit_mode = PICMAN_TRANSLATE_MODE_MASK;
          undo_type = PICMAN_UNDO_GROUP_MASK;
          break;

        case PICMAN_TRANSFORM_TYPE_PATH:
          item = PICMAN_ITEM (picman_image_get_active_vectors (image));

          edit_mode = PICMAN_TRANSLATE_MODE_VECTORS;
          undo_type = PICMAN_UNDO_GROUP_ITEM_DISPLACE;
          break;

        case PICMAN_TRANSFORM_TYPE_LAYER:
          item = PICMAN_ITEM (picman_image_get_active_drawable (image));

          if (item)
            {
              if (PICMAN_IS_LAYER_MASK (item))
                {
                  edit_mode = PICMAN_TRANSLATE_MODE_LAYER_MASK;
                }
              else if (PICMAN_IS_CHANNEL (item))
                {
                  edit_mode = PICMAN_TRANSLATE_MODE_CHANNEL;
                }
              else if (picman_layer_is_floating_sel (PICMAN_LAYER (item)))
                {
                  edit_mode = PICMAN_TRANSLATE_MODE_FLOATING_SEL;
                }
              else
                {
                  edit_mode = PICMAN_TRANSLATE_MODE_LAYER;
                }

              undo_type = PICMAN_UNDO_GROUP_ITEM_DISPLACE;
            }
          break;
        }
    }

  if (! item)
    return TRUE;

  switch (edit_mode)
    {
    case PICMAN_TRANSLATE_MODE_FLOATING_SEL:
      undo_desc = _("Move Floating Selection");
      break;

    default:
      undo_desc = PICMAN_ITEM_GET_CLASS (item)->translate_desc;
      break;
    }

  /* compress undo */
  undo = picman_image_undo_can_compress (image, PICMAN_TYPE_UNDO_STACK, undo_type);

  if (undo                                                         &&
      g_object_get_data (G_OBJECT (undo),
                         "edit-selection-tool") == (gpointer) tool &&
      g_object_get_data (G_OBJECT (undo),
                         "edit-selection-item") == (gpointer) item &&
      g_object_get_data (G_OBJECT (undo),
                         "edit-selection-type") == GINT_TO_POINTER (edit_mode))
    {
      push_undo = FALSE;
    }

  if (push_undo)
    {
      if (picman_image_undo_group_start (image, undo_type, undo_desc))
        {
          undo = picman_image_undo_can_compress (image,
                                               PICMAN_TYPE_UNDO_STACK,
                                               undo_type);

          if (undo)
            {
              g_object_set_data (G_OBJECT (undo), "edit-selection-tool",
                                 tool);
              g_object_set_data (G_OBJECT (undo), "edit-selection-item",
                                 item);
              g_object_set_data (G_OBJECT (undo), "edit-selection-type",
                                 GINT_TO_POINTER (edit_mode));
            }
        }
    }

  switch (edit_mode)
    {
    case PICMAN_TRANSLATE_MODE_LAYER_MASK:
    case PICMAN_TRANSLATE_MODE_MASK:
      picman_item_translate (item, inc_x, inc_y, push_undo);
      break;

    case PICMAN_TRANSLATE_MODE_MASK_TO_LAYER:
    case PICMAN_TRANSLATE_MODE_MASK_COPY_TO_LAYER:
      /*  this won't happen  */
      break;

    case PICMAN_TRANSLATE_MODE_VECTORS:
    case PICMAN_TRANSLATE_MODE_CHANNEL:
    case PICMAN_TRANSLATE_MODE_LAYER:
      picman_item_translate (item, inc_x, inc_y, push_undo);

      /*  translate all linked items as well  */
      if (picman_item_get_linked (item))
        picman_item_linked_translate (item, inc_x, inc_y, push_undo);
      break;

    case PICMAN_TRANSLATE_MODE_FLOATING_SEL:
      picman_item_translate (item, inc_x, inc_y, push_undo);
      break;
    }

  if (push_undo)
    picman_image_undo_group_end (image);
  else
    picman_undo_refresh_preview (undo,
                               picman_get_user_context (display->picman));

  picman_image_flush (image);

  return TRUE;
}
