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

#include "core/picman-transform-utils.h"
#include "core/picmanimage.h"
#include "core/picmandrawable-transform.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmansizebox.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "picmanscaletool.h"
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
  Y3
};


/*  local function prototypes  */

static void    picman_scale_tool_dialog        (PicmanTransformTool  *tr_tool);
static void    picman_scale_tool_dialog_update (PicmanTransformTool  *tr_tool);
static void    picman_scale_tool_prepare       (PicmanTransformTool  *tr_tool);
static void    picman_scale_tool_motion        (PicmanTransformTool  *tr_tool);
static void    picman_scale_tool_recalc_matrix (PicmanTransformTool  *tr_tool);
static gchar * picman_scale_tool_get_undo_desc (PicmanTransformTool  *tr_tool);

static void    picman_scale_tool_size_notify   (GtkWidget          *box,
                                              GParamSpec         *pspec,
                                              PicmanTransformTool  *tr_tool);


G_DEFINE_TYPE (PicmanScaleTool, picman_scale_tool, PICMAN_TYPE_TRANSFORM_TOOL)

#define parent_class picman_scale_tool_parent_class


void
picman_scale_tool_register (PicmanToolRegisterCallback  callback,
                          gpointer                  data)
{
  (* callback) (PICMAN_TYPE_SCALE_TOOL,
                PICMAN_TYPE_TRANSFORM_OPTIONS,
                picman_transform_options_gui,
                PICMAN_CONTEXT_BACKGROUND_MASK,
                "picman-scale-tool",
                _("Scale"),
                _("Scale Tool: Scale the layer, selection or path"),
                N_("_Scale"), "<shift>T",
                NULL, PICMAN_HELP_TOOL_SCALE,
                PICMAN_STOCK_TOOL_SCALE,
                data);
}

static void
picman_scale_tool_class_init (PicmanScaleToolClass *klass)
{
  PicmanTransformToolClass *trans_class = PICMAN_TRANSFORM_TOOL_CLASS (klass);

  trans_class->dialog        = picman_scale_tool_dialog;
  trans_class->dialog_update = picman_scale_tool_dialog_update;
  trans_class->prepare       = picman_scale_tool_prepare;
  trans_class->motion        = picman_scale_tool_motion;
  trans_class->recalc_matrix = picman_scale_tool_recalc_matrix;
  trans_class->get_undo_desc = picman_scale_tool_get_undo_desc;
}

static void
picman_scale_tool_init (PicmanScaleTool *scale_tool)
{
  PicmanTool          *tool    = PICMAN_TOOL (scale_tool);
  PicmanTransformTool *tr_tool = PICMAN_TRANSFORM_TOOL (scale_tool);

  picman_tool_control_set_tool_cursor (tool->control, PICMAN_TOOL_CURSOR_RESIZE);

  tr_tool->progress_text   = _("Scaling");

  tr_tool->use_grid        = TRUE;
  tr_tool->use_handles     = TRUE;
  tr_tool->use_center      = TRUE;
  tr_tool->use_mid_handles = TRUE;
}

static void
picman_scale_tool_dialog (PicmanTransformTool *tr_tool)
{
}

static void
picman_scale_tool_dialog_update (PicmanTransformTool *tr_tool)
{
  PicmanTransformOptions *options = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tr_tool);

  gint width  = ROUND (tr_tool->trans_info[X1] - tr_tool->trans_info[X0]);
  gint height = ROUND (tr_tool->trans_info[Y1] - tr_tool->trans_info[Y0]);

  g_object_set (PICMAN_SCALE_TOOL (tr_tool)->box,
                "width",       width,
                "height",      height,
                "keep-aspect", options->constrain_scale,
                NULL);
}

static void
picman_scale_tool_prepare (PicmanTransformTool *tr_tool)
{
  PicmanScaleTool        *scale   = PICMAN_SCALE_TOOL (tr_tool);
  PicmanTransformOptions *options = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tr_tool);
  PicmanDisplay          *display = PICMAN_TOOL (tr_tool)->display;
  gdouble               xres;
  gdouble               yres;

  tr_tool->trans_info[X0] = (gdouble) tr_tool->x1;
  tr_tool->trans_info[Y0] = (gdouble) tr_tool->y1;
  tr_tool->trans_info[X1] = (gdouble) tr_tool->x2;
  tr_tool->trans_info[Y1] = (gdouble) tr_tool->y2;

  picman_image_get_resolution (picman_display_get_image (display),
                             &xres, &yres);

  if (scale->box)
    {
      g_signal_handlers_disconnect_by_func (scale->box,
                                            picman_scale_tool_size_notify,
                                            tr_tool);
      gtk_widget_destroy (scale->box);
    }

  /*  Need to create a new PicmanSizeBox widget because the initial
   *  width and height is what counts as 100%.
   */
  scale->box =
    g_object_new (PICMAN_TYPE_SIZE_BOX,
                  "width",       tr_tool->x2 - tr_tool->x1,
                  "height",      tr_tool->y2 - tr_tool->y1,
                  "keep-aspect", options->constrain_scale,
                  "unit",        picman_display_get_shell (display)->unit,
                  "xresolution", xres,
                  "yresolution", yres,
                  NULL);

  gtk_container_set_border_width (GTK_CONTAINER (scale->box), 6);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (tr_tool->dialog))),
                      scale->box, FALSE, FALSE, 0);
  gtk_widget_show (scale->box);

  g_signal_connect (scale->box, "notify",
                    G_CALLBACK (picman_scale_tool_size_notify),
                    tr_tool);
}

static void
picman_scale_tool_motion (PicmanTransformTool *tr_tool)
{
  PicmanTransformOptions *options = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tr_tool);
  gdouble              *x1;
  gdouble              *y1;
  gdouble              *x2;
  gdouble              *y2;
  gint                  dir_x;
  gint                  dir_y;
  gdouble               diff_x = tr_tool->curx - tr_tool->lastx;
  gdouble               diff_y = tr_tool->cury - tr_tool->lasty;

  switch (tr_tool->function)
    {
    case TRANSFORM_HANDLE_N:
      diff_x = 0; /* and fall through */
    case TRANSFORM_HANDLE_NW:
      x1 = &tr_tool->trans_info[X0];
      y1 = &tr_tool->trans_info[Y0];
      x2 = &tr_tool->trans_info[X1];
      y2 = &tr_tool->trans_info[Y1];
      dir_x = dir_y = 1;
      break;

    case TRANSFORM_HANDLE_E:
      diff_y = 0; /* and fall through */
    case TRANSFORM_HANDLE_NE:
      x1 = &tr_tool->trans_info[X1];
      y1 = &tr_tool->trans_info[Y0];
      x2 = &tr_tool->trans_info[X0];
      y2 = &tr_tool->trans_info[Y1];
      dir_x = -1;
      dir_y = 1;
      break;

    case TRANSFORM_HANDLE_W:
      diff_y = 0; /* and fall through */
    case TRANSFORM_HANDLE_SW:
      x1 = &tr_tool->trans_info[X0];
      y1 = &tr_tool->trans_info[Y1];
      x2 = &tr_tool->trans_info[X1];
      y2 = &tr_tool->trans_info[Y0];
      dir_x = 1;
      dir_y = -1;
      break;

    case TRANSFORM_HANDLE_S:
      diff_x = 0; /* and fall through */
    case TRANSFORM_HANDLE_SE:
      x1 = &tr_tool->trans_info[X1];
      y1 = &tr_tool->trans_info[Y1];
      x2 = &tr_tool->trans_info[X0];
      y2 = &tr_tool->trans_info[Y0];
      dir_x = dir_y = -1;
      break;

    case TRANSFORM_HANDLE_CENTER:
      tr_tool->trans_info[X0] += diff_x;
      tr_tool->trans_info[Y0] += diff_y;
      tr_tool->trans_info[X1] += diff_x;
      tr_tool->trans_info[Y1] += diff_y;
      tr_tool->trans_info[X2] += diff_x;
      tr_tool->trans_info[Y2] += diff_y;
      tr_tool->trans_info[X3] += diff_x;
      tr_tool->trans_info[Y3] += diff_y;
      return;

    default:
      return;
    }

  *x1 += diff_x;
  *y1 += diff_y;

  /*  if control is being held, constrain the aspect ratio  */
  if (options->constrain_scale)
    {
      /*  FIXME: improve this  */
      gdouble h = tr_tool->trans_info[Y1] - tr_tool->trans_info[Y0];

      switch (tr_tool->function)
        {
        case TRANSFORM_HANDLE_NW:
        case TRANSFORM_HANDLE_SW:
          tr_tool->trans_info[X0] =
            tr_tool->trans_info[X1] - tr_tool->aspect * h;
          break;

        case TRANSFORM_HANDLE_NE:
        case TRANSFORM_HANDLE_SE:
          tr_tool->trans_info[X1] =
            tr_tool->trans_info[X0] + tr_tool->aspect * h;
          break;

        default:
          break;
        }
    }

  if (dir_x > 0)
    {
      if (*x1 >= *x2)
        *x1 = *x2 - 1;
    }
  else
    {
      if (*x1 <= *x2)
        *x1 = *x2 + 1;
    }

  if (dir_y > 0)
    {
      if (*y1 >= *y2)
        *y1 = *y2 - 1;
    }
  else
    {
      if (*y1 <= *y2)
        *y1 = *y2 + 1;
    }
}

static void
picman_scale_tool_recalc_matrix (PicmanTransformTool *tr_tool)
{
  picman_matrix3_identity (&tr_tool->transform);
  picman_transform_matrix_scale (&tr_tool->transform,
                               tr_tool->x1,
                               tr_tool->y1,
                               tr_tool->x2 - tr_tool->x1,
                               tr_tool->y2 - tr_tool->y1,
                               tr_tool->trans_info[X0],
                               tr_tool->trans_info[Y0],
                               tr_tool->trans_info[X1] - tr_tool->trans_info[X0],
                               tr_tool->trans_info[Y1] - tr_tool->trans_info[Y0]);
}

static gchar *
picman_scale_tool_get_undo_desc (PicmanTransformTool *tr_tool)
{
  gint width  = ROUND (tr_tool->trans_info[X1] - tr_tool->trans_info[X0]);
  gint height = ROUND (tr_tool->trans_info[Y1] - tr_tool->trans_info[Y0]);

  return g_strdup_printf (C_("undo-type", "Scale to %d x %d"),
                          width, height);
}

static void
picman_scale_tool_size_notify (GtkWidget         *box,
                             GParamSpec        *pspec,
                             PicmanTransformTool *tr_tool)
{
  PicmanTransformOptions *options = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tr_tool);

  if (! strcmp (pspec->name, "width") ||
      ! strcmp (pspec->name, "height"))
    {
      gint width;
      gint height;
      gint old_width;
      gint old_height;

      g_object_get (box,
                    "width",  &width,
                    "height", &height,
                    NULL);

      old_width  = ROUND (tr_tool->trans_info[X1] - tr_tool->trans_info[X0]);
      old_height = ROUND (tr_tool->trans_info[Y1] - tr_tool->trans_info[Y0]);

      if ((width != old_width) || (height != old_height))
        {
          picman_draw_tool_pause (PICMAN_DRAW_TOOL (tr_tool));

          tr_tool->trans_info[X1] = tr_tool->trans_info[X0] + width;
          tr_tool->trans_info[Y1] = tr_tool->trans_info[Y0] + height;

          picman_transform_tool_push_internal_undo (tr_tool);

          picman_transform_tool_recalc_matrix (tr_tool);

          picman_draw_tool_resume (PICMAN_DRAW_TOOL (tr_tool));
        }
    }
  else if (! strcmp (pspec->name, "keep-aspect"))
    {
      gboolean constrain;

      g_object_get (box,
                    "keep-aspect", &constrain,
                    NULL);

      if (constrain != options->constrain_scale)
        {
          gint width;
          gint height;

          g_object_get (box,
                        "width",  &width,
                        "height", &height,
                        NULL);

          /*  Take the aspect ratio from the size box when the user
           *  activates the constraint by pressing the chain button.
           */
          tr_tool->aspect = (gdouble) width / (gdouble) height;

          g_object_set (options,
                        "constrain-scale", constrain,
                        NULL);
        }
    }
}
