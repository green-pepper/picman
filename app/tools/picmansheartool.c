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
#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman-transform-utils.h"

#include "widgets/picmanhelp-ids.h"

#include "picmansheartool.h"
#include "picmantoolcontrol.h"
#include "picmantransformoptions.h"

#include "picman-intl.h"


/*  index into trans_info array  */
enum
{
  HORZ_OR_VERT,
  XSHEAR,
  YSHEAR
};

/*  the minimum movement before direction of shear can be determined (pixels) */
#define MIN_MOVE     5

#define SB_WIDTH     10


/*  local function prototypes  */

static void    picman_shear_tool_dialog        (PicmanTransformTool  *tr_tool);
static void    picman_shear_tool_dialog_update (PicmanTransformTool  *tr_tool);

static void    picman_shear_tool_prepare       (PicmanTransformTool  *tr_tool);
static void    picman_shear_tool_motion        (PicmanTransformTool  *tr_tool);
static void    picman_shear_tool_recalc_matrix (PicmanTransformTool  *tr_tool);
static gchar * picman_shear_tool_get_undo_desc (PicmanTransformTool  *tr_tool);

static void    shear_x_mag_changed           (GtkAdjustment      *adj,
                                              PicmanTransformTool  *tr_tool);
static void    shear_y_mag_changed           (GtkAdjustment      *adj,
                                              PicmanTransformTool  *tr_tool);


G_DEFINE_TYPE (PicmanShearTool, picman_shear_tool, PICMAN_TYPE_TRANSFORM_TOOL)


void
picman_shear_tool_register (PicmanToolRegisterCallback  callback,
                          gpointer                  data)
{
  (* callback) (PICMAN_TYPE_SHEAR_TOOL,
                PICMAN_TYPE_TRANSFORM_OPTIONS,
                picman_transform_options_gui,
                0,
                "picman-shear-tool",
                _("Shear"),
                _("Shear Tool: Shear the layer, selection or path"),
                N_("S_hear"), "<shift>S",
                NULL, PICMAN_HELP_TOOL_SHEAR,
                PICMAN_STOCK_TOOL_SHEAR,
                data);
}

static void
picman_shear_tool_class_init (PicmanShearToolClass *klass)
{
  PicmanTransformToolClass *trans_class = PICMAN_TRANSFORM_TOOL_CLASS (klass);

  trans_class->dialog        = picman_shear_tool_dialog;
  trans_class->dialog_update = picman_shear_tool_dialog_update;
  trans_class->prepare       = picman_shear_tool_prepare;
  trans_class->motion        = picman_shear_tool_motion;
  trans_class->recalc_matrix = picman_shear_tool_recalc_matrix;
  trans_class->get_undo_desc = picman_shear_tool_get_undo_desc;
}

static void
picman_shear_tool_init (PicmanShearTool *shear_tool)
{
  PicmanTool          *tool    = PICMAN_TOOL (shear_tool);
  PicmanTransformTool *tr_tool = PICMAN_TRANSFORM_TOOL (shear_tool);

  picman_tool_control_set_tool_cursor (tool->control, PICMAN_TOOL_CURSOR_SHEAR);

  tr_tool->progress_text = _("Shearing");

  tr_tool->use_grid      = TRUE;
}

static void
picman_shear_tool_dialog (PicmanTransformTool *tr_tool)
{
  PicmanShearTool *shear = PICMAN_SHEAR_TOOL (tr_tool);
  GtkWidget     *table;
  GtkWidget     *button;

  table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (tr_tool->dialog))),
                      table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  button = picman_spin_button_new ((GtkObject **) &shear->x_adj,
                                 0, -65536, 65536, 1, 15, 0, 1, 0);
  gtk_entry_set_width_chars (GTK_ENTRY (button), SB_WIDTH);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0, _("Shear magnitude _X:"),
                             0.0, 0.5, button, 1, TRUE);

  g_signal_connect (shear->x_adj, "value-changed",
                    G_CALLBACK (shear_x_mag_changed),
                    tr_tool);

  button = picman_spin_button_new ((GtkObject **) &shear->y_adj,
                                 0, -65536, 65536, 1, 15, 0, 1, 0);
  gtk_entry_set_width_chars (GTK_ENTRY (button), SB_WIDTH);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1, _("Shear magnitude _Y:"),
                             0.0, 0.5, button, 1, TRUE);

  g_signal_connect (shear->y_adj, "value-changed",
                    G_CALLBACK (shear_y_mag_changed),
                    tr_tool);
}

static void
picman_shear_tool_dialog_update (PicmanTransformTool *tr_tool)
{
  PicmanShearTool *shear = PICMAN_SHEAR_TOOL (tr_tool);

  gtk_adjustment_set_value (shear->x_adj, tr_tool->trans_info[XSHEAR]);
  gtk_adjustment_set_value (shear->y_adj, tr_tool->trans_info[YSHEAR]);
}

static void
picman_shear_tool_prepare (PicmanTransformTool *tr_tool)
{
  tr_tool->trans_info[HORZ_OR_VERT] = PICMAN_ORIENTATION_UNKNOWN;
  tr_tool->trans_info[XSHEAR]       = 0.0;
  tr_tool->trans_info[YSHEAR]       = 0.0;
}

static void
picman_shear_tool_motion (PicmanTransformTool *tr_tool)
{
  gdouble diffx = tr_tool->curx - tr_tool->lastx;
  gdouble diffy = tr_tool->cury - tr_tool->lasty;

  /*  If we haven't yet decided on which way to control shearing
   *  decide using the maximum differential
   */

  if (tr_tool->trans_info[HORZ_OR_VERT] == PICMAN_ORIENTATION_UNKNOWN)
    {
      if (abs (diffx) > MIN_MOVE || abs (diffy) > MIN_MOVE)
        {
          if (abs (diffx) > abs (diffy))
            {
              tr_tool->trans_info[HORZ_OR_VERT] = PICMAN_ORIENTATION_HORIZONTAL;
              tr_tool->trans_info[XSHEAR] = 0.0;
            }
          else
            {
              tr_tool->trans_info[HORZ_OR_VERT] = PICMAN_ORIENTATION_VERTICAL;
              tr_tool->trans_info[XSHEAR] = 0.0;
            }
        }
      /*  set the current coords to the last ones  */
      else
        {
          tr_tool->curx = tr_tool->lastx;
          tr_tool->cury = tr_tool->lasty;
        }
    }

  /*  if the direction is known, keep track of the magnitude  */
  if (tr_tool->trans_info[HORZ_OR_VERT] == PICMAN_ORIENTATION_HORIZONTAL)
    {
      if (tr_tool->cury > (tr_tool->ty1 + tr_tool->ty3) / 2)
        tr_tool->trans_info[XSHEAR] += diffx;
      else
        tr_tool->trans_info[XSHEAR] -= diffx;
    }
  else if (tr_tool->trans_info[HORZ_OR_VERT] == PICMAN_ORIENTATION_VERTICAL)
    {
      if (tr_tool->curx > (tr_tool->tx1 + tr_tool->tx2) / 2)
        tr_tool->trans_info[YSHEAR] += diffy;
      else
        tr_tool->trans_info[YSHEAR] -= diffy;
    }
}

static void
picman_shear_tool_recalc_matrix (PicmanTransformTool *tr_tool)
{
  gdouble amount;

  if (tr_tool->trans_info[XSHEAR] == 0.0 &&
      tr_tool->trans_info[YSHEAR] == 0.0)
    {
      tr_tool->trans_info[HORZ_OR_VERT] = PICMAN_ORIENTATION_UNKNOWN;
    }

  if (tr_tool->trans_info[HORZ_OR_VERT] == PICMAN_ORIENTATION_HORIZONTAL)
    amount = tr_tool->trans_info[XSHEAR];
  else
    amount = tr_tool->trans_info[YSHEAR];

  picman_matrix3_identity (&tr_tool->transform);
  picman_transform_matrix_shear (&tr_tool->transform,
                               tr_tool->x1,
                               tr_tool->y1,
                               tr_tool->x2 - tr_tool->x1,
                               tr_tool->y2 - tr_tool->y1,
                               tr_tool->trans_info[HORZ_OR_VERT],
                               amount);
}

static gchar *
picman_shear_tool_get_undo_desc (PicmanTransformTool *tr_tool)
{
  gdouble x = tr_tool->trans_info[XSHEAR];
  gdouble y = tr_tool->trans_info[YSHEAR];

  switch ((gint) tr_tool->trans_info[HORZ_OR_VERT])
    {
    case PICMAN_ORIENTATION_HORIZONTAL:
      return g_strdup_printf (C_("undo-type", "Shear horizontally by %-3.3g"),
                              x);

    case PICMAN_ORIENTATION_VERTICAL:
      return g_strdup_printf (C_("undo-type", "Shear vertically by %-3.3g"),
                              y);

    default:
      /* e.g. user entered numbers but no notification callback */
      return g_strdup_printf (C_("undo-type", "Shear horizontally by %-3.3g, vertically by %-3.3g"),
                              x, y);
    }
}

static void
shear_x_mag_changed (GtkAdjustment     *adj,
                     PicmanTransformTool *tr_tool)
{
  gdouble value = gtk_adjustment_get_value (adj);

  if (value != tr_tool->trans_info[XSHEAR])
    {
      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tr_tool));

      if (tr_tool->trans_info[HORZ_OR_VERT] == PICMAN_ORIENTATION_UNKNOWN)
        tr_tool->trans_info[HORZ_OR_VERT] = PICMAN_ORIENTATION_HORIZONTAL;

      tr_tool->trans_info[XSHEAR] = value;

      picman_transform_tool_push_internal_undo (tr_tool);

      picman_transform_tool_recalc_matrix (tr_tool);

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tr_tool));
    }
}

static void
shear_y_mag_changed (GtkAdjustment     *adj,
                     PicmanTransformTool *tr_tool)
{
  gdouble value = gtk_adjustment_get_value (adj);

  if (value != tr_tool->trans_info[YSHEAR])
    {
      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tr_tool));

      if (tr_tool->trans_info[HORZ_OR_VERT] == PICMAN_ORIENTATION_UNKNOWN)
        tr_tool->trans_info[HORZ_OR_VERT] = PICMAN_ORIENTATION_VERTICAL;

      tr_tool->trans_info[YSHEAR] = value;

      picman_transform_tool_push_internal_undo (tr_tool);

      picman_transform_tool_recalc_matrix (tr_tool);

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tr_tool));
    }
}
