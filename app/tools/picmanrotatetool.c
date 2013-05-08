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
#include <gdk/gdkkeysyms.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman-transform-utils.h"
#include "core/picmanimage.h"

#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "picmanrotatetool.h"
#include "picmantoolcontrol.h"
#include "picmantransformoptions.h"

#include "picman-intl.h"


/*  index into trans_info array  */
enum
{
  ANGLE,
  REAL_ANGLE,
  PIVOT_X,
  PIVOT_Y
};

#define FIFTEEN_DEG  (G_PI / 12.0)

#define SB_WIDTH     10


/*  local function prototypes  */

static gboolean  picman_rotate_tool_key_press     (PicmanTool           *tool,
                                                 GdkEventKey        *kevent,
                                                 PicmanDisplay        *display);

static void      picman_rotate_tool_dialog        (PicmanTransformTool  *tr_tool);
static void      picman_rotate_tool_dialog_update (PicmanTransformTool  *tr_tool);
static void      picman_rotate_tool_prepare       (PicmanTransformTool  *tr_tool);
static void      picman_rotate_tool_motion        (PicmanTransformTool  *tr_tool);
static void      picman_rotate_tool_recalc_matrix (PicmanTransformTool  *tr_tool);
static gchar   * picman_rotate_tool_get_undo_desc (PicmanTransformTool  *tr_tool);

static void      rotate_angle_changed           (GtkAdjustment      *adj,
                                                 PicmanTransformTool  *tr_tool);
static void      rotate_center_changed          (GtkWidget          *entry,
                                                PicmanTransformTool   *tr_tool);


G_DEFINE_TYPE (PicmanRotateTool, picman_rotate_tool, PICMAN_TYPE_TRANSFORM_TOOL)

#define parent_class picman_rotate_tool_parent_class


void
picman_rotate_tool_register (PicmanToolRegisterCallback  callback,
                           gpointer                  data)
{
  (* callback) (PICMAN_TYPE_ROTATE_TOOL,
                PICMAN_TYPE_TRANSFORM_OPTIONS,
                picman_transform_options_gui,
                PICMAN_CONTEXT_BACKGROUND_MASK,
                "picman-rotate-tool",
                _("Rotate"),
                _("Rotate Tool: Rotate the layer, selection or path"),
                N_("_Rotate"), "<shift>R",
                NULL, PICMAN_HELP_TOOL_ROTATE,
                PICMAN_STOCK_TOOL_ROTATE,
                data);
}

static void
picman_rotate_tool_class_init (PicmanRotateToolClass *klass)
{
  PicmanToolClass          *tool_class  = PICMAN_TOOL_CLASS (klass);
  PicmanTransformToolClass *trans_class = PICMAN_TRANSFORM_TOOL_CLASS (klass);

  tool_class->key_press      = picman_rotate_tool_key_press;

  trans_class->dialog        = picman_rotate_tool_dialog;
  trans_class->dialog_update = picman_rotate_tool_dialog_update;
  trans_class->prepare       = picman_rotate_tool_prepare;
  trans_class->motion        = picman_rotate_tool_motion;
  trans_class->recalc_matrix = picman_rotate_tool_recalc_matrix;
  trans_class->get_undo_desc = picman_rotate_tool_get_undo_desc;
}

static void
picman_rotate_tool_init (PicmanRotateTool *rotate_tool)
{
  PicmanTool          *tool    = PICMAN_TOOL (rotate_tool);
  PicmanTransformTool *tr_tool = PICMAN_TRANSFORM_TOOL (rotate_tool);

  picman_tool_control_set_tool_cursor (tool->control, PICMAN_TOOL_CURSOR_ROTATE);

  tr_tool->progress_text = _("Rotating");

  tr_tool->use_grid      = TRUE;
  tr_tool->use_pivot     = TRUE;
}

static gboolean
picman_rotate_tool_key_press (PicmanTool    *tool,
                            GdkEventKey *kevent,
                            PicmanDisplay *display)
{
  PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (tool);

  if (display == draw_tool->display)
    {
      PicmanRotateTool *rotate     = PICMAN_ROTATE_TOOL (tool);
      GtkSpinButton  *angle_spin = GTK_SPIN_BUTTON (rotate->angle_spin_button);

      switch (kevent->keyval)
        {
        case GDK_KEY_Up:
          gtk_spin_button_spin (angle_spin, GTK_SPIN_STEP_FORWARD, 0.0);
          return TRUE;

        case GDK_KEY_Down:
          gtk_spin_button_spin (angle_spin, GTK_SPIN_STEP_BACKWARD, 0.0);
          return TRUE;

        case GDK_KEY_Right:
          gtk_spin_button_spin (angle_spin, GTK_SPIN_PAGE_FORWARD, 0.0);
          return TRUE;

        case GDK_KEY_Left:
          gtk_spin_button_spin (angle_spin, GTK_SPIN_PAGE_BACKWARD, 0.0);
          return TRUE;

        default:
          break;
        }
    }

  return PICMAN_TOOL_CLASS (parent_class)->key_press (tool, kevent, display);
}

static void
picman_rotate_tool_dialog (PicmanTransformTool *tr_tool)
{
  PicmanRotateTool *rotate = PICMAN_ROTATE_TOOL (tr_tool);
  GtkWidget      *table;
  GtkWidget      *button;
  GtkWidget      *scale;
  GtkObject      *adj;

  table = gtk_table_new (4, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacing (GTK_TABLE (table), 1, 6);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (tr_tool->dialog))),
                      table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  button = picman_spin_button_new ((GtkObject **) &rotate->angle_adj,
                                 0, -180, 180, 0.1, 15, 0, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (button), TRUE);
  gtk_entry_set_width_chars (GTK_ENTRY (button), SB_WIDTH);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0, _("_Angle:"),
                             0.0, 0.5, button, 1, TRUE);
  rotate->angle_spin_button = button;

  g_signal_connect (rotate->angle_adj, "value-changed",
                    G_CALLBACK (rotate_angle_changed),
                    tr_tool);

  scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, rotate->angle_adj);
  gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);
  gtk_table_attach (GTK_TABLE (table), scale, 1, 2, 1, 2,
                    GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
  gtk_widget_show (scale);

  button = picman_spin_button_new (&adj, 0, -1, 1, 1, 10, 0, 1, 2);
  gtk_entry_set_width_chars (GTK_ENTRY (button), SB_WIDTH);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 2, _("Center _X:"),
                             0.0, 0.5, button, 1, TRUE);

  rotate->sizeentry = picman_size_entry_new (1, PICMAN_UNIT_PIXEL, "%a",
                                           TRUE, TRUE, FALSE, SB_WIDTH,
                                           PICMAN_SIZE_ENTRY_UPDATE_SIZE);
  picman_size_entry_add_field (PICMAN_SIZE_ENTRY (rotate->sizeentry),
                             GTK_SPIN_BUTTON (button), NULL);
  picman_size_entry_set_pixel_digits (PICMAN_SIZE_ENTRY (rotate->sizeentry), 2);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 3, _("Center _Y:"),
                             0.0, 0.5, rotate->sizeentry, 1, TRUE);

  g_signal_connect (rotate->sizeentry, "value-changed",
                    G_CALLBACK (rotate_center_changed),
                    tr_tool);
}

static void
picman_rotate_tool_dialog_update (PicmanTransformTool *tr_tool)
{
  PicmanRotateTool *rotate = PICMAN_ROTATE_TOOL (tr_tool);

  gtk_adjustment_set_value (rotate->angle_adj,
                            picman_rad_to_deg (tr_tool->trans_info[ANGLE]));

  g_signal_handlers_block_by_func (rotate->sizeentry,
                                   rotate_center_changed,
                                   tr_tool);

  picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (rotate->sizeentry), 0,
                              tr_tool->trans_info[PIVOT_X]);
  picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (rotate->sizeentry), 1,
                              tr_tool->trans_info[PIVOT_Y]);

  g_signal_handlers_unblock_by_func (rotate->sizeentry,
                                     rotate_center_changed,
                                     tr_tool);
}

static void
picman_rotate_tool_prepare (PicmanTransformTool *tr_tool)
{
  PicmanRotateTool *rotate  = PICMAN_ROTATE_TOOL (tr_tool);
  PicmanDisplay    *display = PICMAN_TOOL (tr_tool)->display;
  PicmanImage      *image   = picman_display_get_image (display);
  gdouble         xres;
  gdouble         yres;

  tr_tool->px = (gdouble) (tr_tool->x1 + tr_tool->x2) / 2.0;
  tr_tool->py = (gdouble) (tr_tool->y1 + tr_tool->y2) / 2.0;

  tr_tool->trans_info[ANGLE]      = 0.0;
  tr_tool->trans_info[REAL_ANGLE] = 0.0;
  tr_tool->trans_info[PIVOT_X]    = tr_tool->px;
  tr_tool->trans_info[PIVOT_Y]    = tr_tool->py;

  picman_image_get_resolution (image, &xres, &yres);

  g_signal_handlers_block_by_func (rotate->sizeentry,
                                   rotate_center_changed,
                                   tr_tool);

  picman_size_entry_set_unit (PICMAN_SIZE_ENTRY (rotate->sizeentry),
                            picman_display_get_shell (display)->unit);

  picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (rotate->sizeentry), 0,
                                  xres, FALSE);
  picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (rotate->sizeentry), 1,
                                  yres, FALSE);

  picman_size_entry_set_refval_boundaries (PICMAN_SIZE_ENTRY (rotate->sizeentry), 0,
                                         -65536,
                                         65536 +
                                         picman_image_get_width (image));
  picman_size_entry_set_refval_boundaries (PICMAN_SIZE_ENTRY (rotate->sizeentry), 1,
                                         -65536,
                                         65536 +
                                         picman_image_get_height (image));

  picman_size_entry_set_size (PICMAN_SIZE_ENTRY (rotate->sizeentry), 0,
                            tr_tool->x1, tr_tool->x2);
  picman_size_entry_set_size (PICMAN_SIZE_ENTRY (rotate->sizeentry), 1,
                            tr_tool->y1, tr_tool->y2);

  g_signal_handlers_unblock_by_func (rotate->sizeentry,
                                     rotate_center_changed,
                                     tr_tool);
}

static void
picman_rotate_tool_motion (PicmanTransformTool *tr_tool)
{
  PicmanTransformOptions *options = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tr_tool);
  gdouble               angle1, angle2, angle;
  gdouble               px, py;
  gdouble               x1, y1, x2, y2;

  if (tr_tool->function == TRANSFORM_HANDLE_PIVOT)
    {
      tr_tool->trans_info[PIVOT_X] = tr_tool->curx;
      tr_tool->trans_info[PIVOT_Y] = tr_tool->cury;

      return;
    }

  px = tr_tool->trans_info[PIVOT_X];
  py = tr_tool->trans_info[PIVOT_Y];

  x1 = tr_tool->curx  - px;
  x2 = tr_tool->lastx - px;
  y1 = py - tr_tool->cury;
  y2 = py - tr_tool->lasty;

  /*  find the first angle  */
  angle1 = atan2 (y1, x1);

  /*  find the angle  */
  angle2 = atan2 (y2, x2);

  angle = angle2 - angle1;

  if (angle > G_PI || angle < -G_PI)
    angle = angle2 - ((angle1 < 0) ? 2.0 * G_PI + angle1 : angle1 - 2.0 * G_PI);

  /*  increment the transform tool's angle  */
  tr_tool->trans_info[REAL_ANGLE] += angle;

  /*  limit the angle to between -180 and 180 degrees  */
  if (tr_tool->trans_info[REAL_ANGLE] < - G_PI)
    {
      tr_tool->trans_info[REAL_ANGLE] += 2.0 * G_PI;
    }
  else if (tr_tool->trans_info[REAL_ANGLE] > G_PI)
    {
      tr_tool->trans_info[REAL_ANGLE] -= 2.0 * G_PI;
    }

  /*  constrain the angle to 15-degree multiples if ctrl is held down  */
  if (options->constrain_rotate)
    {
      tr_tool->trans_info[ANGLE] =
        FIFTEEN_DEG * (gint) ((tr_tool->trans_info[REAL_ANGLE] +
                               FIFTEEN_DEG / 2.0) / FIFTEEN_DEG);
    }
  else
    {
      tr_tool->trans_info[ANGLE] = tr_tool->trans_info[REAL_ANGLE];
    }
}

static void
picman_rotate_tool_recalc_matrix (PicmanTransformTool *tr_tool)
{
  tr_tool->px = tr_tool->trans_info[PIVOT_X];
  tr_tool->py = tr_tool->trans_info[PIVOT_Y];

  picman_matrix3_identity (&tr_tool->transform);
  picman_transform_matrix_rotate_center (&tr_tool->transform,
                                       tr_tool->px,
                                       tr_tool->py,
                                       tr_tool->trans_info[ANGLE]);
}

static gchar *
picman_rotate_tool_get_undo_desc (PicmanTransformTool  *tr_tool)
{
  return g_strdup_printf (C_("undo-type",
                             "Rotate by %-3.3g° around (%g, %g)"),
                          picman_rad_to_deg (tr_tool->trans_info[ANGLE]),
                          tr_tool->trans_info[PIVOT_X],
                          tr_tool->trans_info[PIVOT_Y]);
}

static void
rotate_angle_changed (GtkAdjustment     *adj,
                      PicmanTransformTool *tr_tool)
{
  gdouble value = picman_deg_to_rad (gtk_adjustment_get_value (adj));

#define ANGLE_EPSILON 0.0001

  if (ABS (value - tr_tool->trans_info[ANGLE]) > ANGLE_EPSILON)
    {
      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tr_tool));

      tr_tool->trans_info[REAL_ANGLE] = tr_tool->trans_info[ANGLE] = value;

      picman_transform_tool_push_internal_undo (tr_tool);

      picman_transform_tool_recalc_matrix (tr_tool);

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tr_tool));
    }

#undef ANGLE_EPSILON
}

static void
rotate_center_changed (GtkWidget         *widget,
                       PicmanTransformTool *tr_tool)
{
  gdouble px = picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (widget), 0);
  gdouble py = picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (widget), 1);

  if ((px != tr_tool->trans_info[PIVOT_X]) ||
      (py != tr_tool->trans_info[PIVOT_Y]))
    {
      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tr_tool));

      tr_tool->trans_info[PIVOT_X] = px;
      tr_tool->trans_info[PIVOT_Y] = py;
      tr_tool->px = px;
      tr_tool->py = py;

      picman_transform_tool_push_internal_undo (tr_tool);

      picman_transform_tool_recalc_matrix (tr_tool);

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tr_tool));
    }
}
