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

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "operations/picmancolorbalanceconfig.h"

#include "core/picmandrawable.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"

#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "picmancolorbalancetool.h"
#include "picmanimagemapoptions.h"

#include "picman-intl.h"


/*  local function prototypes  */

static gboolean   picman_color_balance_tool_initialize    (PicmanTool         *tool,
                                                         PicmanDisplay      *display,
                                                         GError          **error);

static GeglNode * picman_color_balance_tool_get_operation (PicmanImageMapTool *im_tool,
                                                         GObject         **config,
                                                         gchar           **undo_desc);
static void       picman_color_balance_tool_dialog        (PicmanImageMapTool *im_tool);
static void       picman_color_balance_tool_reset         (PicmanImageMapTool *im_tool);

static void     color_balance_config_notify        (GObject              *object,
                                                    GParamSpec           *pspec,
                                                    PicmanColorBalanceTool *cb_tool);

static void     color_balance_range_callback       (GtkWidget            *widget,
                                                    PicmanColorBalanceTool *cb_tool);
static void     color_balance_range_reset_callback (GtkWidget            *widget,
                                                    PicmanColorBalanceTool *cb_tool);
static void     color_balance_preserve_toggled     (GtkWidget            *widget,
                                                    PicmanColorBalanceTool *cb_tool);
static void     color_balance_cr_changed           (GtkAdjustment        *adj,
                                                    PicmanColorBalanceTool *cb_tool);
static void     color_balance_mg_changed           (GtkAdjustment        *adj,
                                                    PicmanColorBalanceTool *cb_tool);
static void     color_balance_yb_changed           (GtkAdjustment        *adj,
                                                    PicmanColorBalanceTool *cb_tool);


G_DEFINE_TYPE (PicmanColorBalanceTool, picman_color_balance_tool,
               PICMAN_TYPE_IMAGE_MAP_TOOL)

#define parent_class picman_color_balance_tool_parent_class


void
picman_color_balance_tool_register (PicmanToolRegisterCallback  callback,
                                  gpointer                  data)
{
  (* callback) (PICMAN_TYPE_COLOR_BALANCE_TOOL,
                PICMAN_TYPE_IMAGE_MAP_OPTIONS, NULL,
                0,
                "picman-color-balance-tool",
                _("Color Balance"),
                _("Color Balance Tool: Adjust color distribution"),
                N_("Color _Balance..."), NULL,
                NULL, PICMAN_HELP_TOOL_COLOR_BALANCE,
                PICMAN_STOCK_TOOL_COLOR_BALANCE,
                data);
}

static void
picman_color_balance_tool_class_init (PicmanColorBalanceToolClass *klass)
{
  PicmanToolClass         *tool_class    = PICMAN_TOOL_CLASS (klass);
  PicmanImageMapToolClass *im_tool_class = PICMAN_IMAGE_MAP_TOOL_CLASS (klass);

  tool_class->initialize             = picman_color_balance_tool_initialize;

  im_tool_class->dialog_desc         = _("Adjust Color Balance");
  im_tool_class->settings_name       = "color-balance";
  im_tool_class->import_dialog_title = _("Import Color Balance Settings");
  im_tool_class->export_dialog_title = _("Export Color Balance Settings");

  im_tool_class->get_operation       = picman_color_balance_tool_get_operation;
  im_tool_class->dialog              = picman_color_balance_tool_dialog;
  im_tool_class->reset               = picman_color_balance_tool_reset;
}

static void
picman_color_balance_tool_init (PicmanColorBalanceTool *cb_tool)
{
}

static gboolean
picman_color_balance_tool_initialize (PicmanTool     *tool,
                                    PicmanDisplay  *display,
                                    GError      **error)
{
  PicmanImage    *image    = picman_display_get_image (display);
  PicmanDrawable *drawable = picman_image_get_active_drawable (image);

  if (! drawable)
    return FALSE;

  if (! picman_drawable_is_rgb (drawable))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Color Balance operates only on RGB color layers."));
      return FALSE;
    }

  return PICMAN_TOOL_CLASS (parent_class)->initialize (tool, display, error);
}

static GeglNode *
picman_color_balance_tool_get_operation (PicmanImageMapTool  *im_tool,
                                       GObject          **config,
                                       gchar            **undo_desc)
{
  PicmanColorBalanceTool *cb_tool = PICMAN_COLOR_BALANCE_TOOL (im_tool);

  cb_tool->config = g_object_new (PICMAN_TYPE_COLOR_BALANCE_CONFIG, NULL);

  g_signal_connect_object (cb_tool->config, "notify",
                           G_CALLBACK (color_balance_config_notify),
                           G_OBJECT (cb_tool), 0);

  *config = G_OBJECT (cb_tool->config);

  return gegl_node_new_child (NULL,
                              "operation", "picman:color-balance",
                              "config",    cb_tool->config,
                              NULL);
}


/**************************/
/*  Color Balance dialog  */
/**************************/

static GtkAdjustment *
create_levels_scale (gdouble        value,
                     const gchar   *left,
                     const gchar   *right,
                     GtkWidget     *table,
                     gint           col)
{
  GtkWidget     *label;
  GtkWidget     *slider;
  GtkWidget     *spinbutton;
  GtkAdjustment *adj;

  label = gtk_label_new (left);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, col, col + 1,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (label);

  spinbutton = picman_spin_button_new ((GtkObject **) &adj,
                                     value, -100.0, 100.0,
                                     1.0, 10.0, 0.0, 1.0, 0);

  slider = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, adj);
  gtk_scale_set_draw_value (GTK_SCALE (slider), FALSE);
  gtk_widget_set_size_request (slider, 100, -1);
  gtk_table_attach_defaults (GTK_TABLE (table), slider, 1, 2, col, col + 1);
  gtk_widget_show (slider);

  label = gtk_label_new (right);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, col, col + 1,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (label);

  gtk_table_attach (GTK_TABLE (table), spinbutton, 3, 4, col, col + 1,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (spinbutton);

  return adj;
}

static void
picman_color_balance_tool_dialog (PicmanImageMapTool *image_map_tool)
{
  PicmanColorBalanceTool   *cb_tool = PICMAN_COLOR_BALANCE_TOOL (image_map_tool);
  PicmanColorBalanceConfig *config  = cb_tool->config;
  GtkWidget              *main_vbox;
  GtkWidget              *vbox;
  GtkWidget              *hbox;
  GtkWidget              *table;
  GtkWidget              *button;
  GtkWidget              *frame;

  main_vbox = picman_image_map_tool_dialog_get_vbox (image_map_tool);

  frame = picman_enum_radio_frame_new (PICMAN_TYPE_TRANSFER_MODE,
                                     gtk_label_new (_("Select Range to Adjust")),
                                     G_CALLBACK (color_balance_range_callback),
                                     cb_tool,
                                     &cb_tool->range_radio);
  picman_int_radio_group_set_active (GTK_RADIO_BUTTON (cb_tool->range_radio),
                                   config->range);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  frame = picman_frame_new (_("Adjust Color Levels"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  /*  The table containing sliders  */
  table = gtk_table_new (3, 4, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  cb_tool->cyan_red_adj =
    create_levels_scale (config->cyan_red[config->range] * 100.0,
                         _("Cyan"), _("Red"),
                         table, 0);

  g_signal_connect (cb_tool->cyan_red_adj, "value-changed",
                    G_CALLBACK (color_balance_cr_changed),
                    cb_tool);

  cb_tool->magenta_green_adj =
    create_levels_scale (config->magenta_green[config->range] * 100.0,
                         _("Magenta"), _("Green"),
                         table, 1);

  g_signal_connect (cb_tool->magenta_green_adj, "value-changed",
                    G_CALLBACK (color_balance_mg_changed),
                    cb_tool);

  cb_tool->yellow_blue_adj =
    create_levels_scale (config->yellow_blue[config->range] * 100.0,
                         _("Yellow"), _("Blue"),
                         table, 2);

  g_signal_connect (cb_tool->yellow_blue_adj, "value-changed",
                    G_CALLBACK (color_balance_yb_changed),
                    cb_tool);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  button = gtk_button_new_with_mnemonic (_("R_eset Range"));
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (color_balance_range_reset_callback),
                    cb_tool);

  cb_tool->preserve_toggle =
    gtk_check_button_new_with_mnemonic (_("Preserve _luminosity"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb_tool->preserve_toggle),
                                config->preserve_luminosity);
  gtk_box_pack_end (GTK_BOX (main_vbox), cb_tool->preserve_toggle,
                    FALSE, FALSE, 0);
  gtk_widget_show (cb_tool->preserve_toggle);

  g_signal_connect (cb_tool->preserve_toggle, "toggled",
                    G_CALLBACK (color_balance_preserve_toggled),
                    cb_tool);
}

static void
picman_color_balance_tool_reset (PicmanImageMapTool *im_tool)
{
  PicmanColorBalanceTool *cb_tool = PICMAN_COLOR_BALANCE_TOOL (im_tool);
  PicmanTransferMode      range   = cb_tool->config->range;

  g_object_freeze_notify (im_tool->config);

  if (im_tool->default_config)
    {
      picman_config_copy (PICMAN_CONFIG (im_tool->default_config),
                        PICMAN_CONFIG (im_tool->config),
                        0);
    }
  else
    {
      picman_config_reset (PICMAN_CONFIG (im_tool->config));
    }

  g_object_set (cb_tool->config,
                "range", range,
                NULL);

  g_object_thaw_notify (im_tool->config);
}

static void
color_balance_config_notify (GObject              *object,
                             GParamSpec           *pspec,
                             PicmanColorBalanceTool *cb_tool)
{
  PicmanColorBalanceConfig *config = PICMAN_COLOR_BALANCE_CONFIG (object);

  if (! cb_tool->cyan_red_adj)
    return;

  if (! strcmp (pspec->name, "range"))
    {
      picman_int_radio_group_set_active (GTK_RADIO_BUTTON (cb_tool->range_radio),
                                       config->range);
    }
  else if (! strcmp (pspec->name, "cyan-red"))
    {
      gtk_adjustment_set_value (cb_tool->cyan_red_adj,
                                config->cyan_red[config->range] * 100.0);
    }
  else if (! strcmp (pspec->name, "magenta-green"))
    {
      gtk_adjustment_set_value (cb_tool->magenta_green_adj,
                                config->magenta_green[config->range] * 100.0);
    }
  else if (! strcmp (pspec->name, "yellow-blue"))
    {
      gtk_adjustment_set_value (cb_tool->yellow_blue_adj,
                                config->yellow_blue[config->range] * 100.0);
    }
  else if (! strcmp (pspec->name, "preserve-luminosity"))
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb_tool->preserve_toggle),
                                    config->preserve_luminosity);
    }
}

static void
color_balance_range_callback (GtkWidget            *widget,
                              PicmanColorBalanceTool *cb_tool)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    {
      PicmanTransferMode range;

      picman_radio_button_update (widget, &range);
      g_object_set (cb_tool->config,
                    "range", range,
                    NULL);
    }
}

static void
color_balance_range_reset_callback (GtkWidget            *widget,
                                    PicmanColorBalanceTool *cb_tool)
{
  picman_color_balance_config_reset_range (cb_tool->config);
}

static void
color_balance_preserve_toggled (GtkWidget            *widget,
                                PicmanColorBalanceTool *cb_tool)
{
  gboolean active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

  if (cb_tool->config->preserve_luminosity != active)
    {
      g_object_set (cb_tool->config,
                    "preserve-luminosity", active,
                    NULL);
    }
}

static void
color_balance_cr_changed (GtkAdjustment        *adjustment,
                          PicmanColorBalanceTool *cb_tool)
{
  gdouble value = gtk_adjustment_get_value (adjustment) / 100.0;

  if (cb_tool->config->cyan_red[cb_tool->config->range] != value)
    {
      g_object_set (cb_tool->config,
                    "cyan-red", value,
                    NULL);
    }
}

static void
color_balance_mg_changed (GtkAdjustment        *adjustment,
                          PicmanColorBalanceTool *cb_tool)
{
  gdouble value = gtk_adjustment_get_value (adjustment) / 100.0;

  if (cb_tool->config->magenta_green[cb_tool->config->range] != value)
    {
      g_object_set (cb_tool->config,
                    "magenta-green", value,
                    NULL);
    }
}

static void
color_balance_yb_changed (GtkAdjustment        *adjustment,
                          PicmanColorBalanceTool *cb_tool)
{
  gdouble value = gtk_adjustment_get_value (adjustment) / 100.0;

  if (cb_tool->config->yellow_blue[cb_tool->config->range] != value)
    {
      g_object_set (cb_tool->config,
                    "yellow-blue", value,
                    NULL);
    }
}
