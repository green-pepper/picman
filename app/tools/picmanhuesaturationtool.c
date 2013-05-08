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

#include "operations/picmanhuesaturationconfig.h"
#include "operations/picmanoperationhuesaturation.h"

#include "core/picmandrawable.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"

#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "picmanhuesaturationtool.h"
#include "picmanimagemapoptions.h"

#include "picman-intl.h"


#define SLIDER_WIDTH  200
#define DA_WIDTH       40
#define DA_HEIGHT      20


/*  local function prototypes  */

static gboolean   picman_hue_saturation_tool_initialize    (PicmanTool         *tool,
                                                          PicmanDisplay      *display,
                                                          GError          **error);

static GeglNode * picman_hue_saturation_tool_get_operation (PicmanImageMapTool *im_tool,
                                                          GObject         **config,
                                                          gchar           **undo_desc);
static void       picman_hue_saturation_tool_dialog        (PicmanImageMapTool *im_tool);
static void       picman_hue_saturation_tool_reset         (PicmanImageMapTool *im_tool);

static void       hue_saturation_config_notify           (GObject               *object,
                                                          GParamSpec            *pspec,
                                                          PicmanHueSaturationTool *hs_tool);

static void       hue_saturation_update_color_areas      (PicmanHueSaturationTool *hs_tool);

static void       hue_saturation_range_callback          (GtkWidget             *widget,
                                                          PicmanHueSaturationTool *hs_tool);
static void       hue_saturation_range_reset_callback    (GtkWidget             *widget,
                                                          PicmanHueSaturationTool *hs_tool);
static void       hue_saturation_hue_changed             (GtkAdjustment         *adjustment,
                                                          PicmanHueSaturationTool *hs_tool);
static void       hue_saturation_lightness_changed       (GtkAdjustment         *adjustment,
                                                          PicmanHueSaturationTool *hs_tool);
static void       hue_saturation_saturation_changed      (GtkAdjustment         *adjustment,
                                                          PicmanHueSaturationTool *hs_tool);
static void       hue_saturation_overlap_changed         (GtkAdjustment         *adjustment,
                                                          PicmanHueSaturationTool *hs_tool);


G_DEFINE_TYPE (PicmanHueSaturationTool, picman_hue_saturation_tool,
               PICMAN_TYPE_IMAGE_MAP_TOOL)

#define parent_class picman_hue_saturation_tool_parent_class


void
picman_hue_saturation_tool_register (PicmanToolRegisterCallback  callback,
                                   gpointer                  data)
{
  (* callback) (PICMAN_TYPE_HUE_SATURATION_TOOL,
                PICMAN_TYPE_IMAGE_MAP_OPTIONS, NULL,
                0,
                "picman-hue-saturation-tool",
                _("Hue-Saturation"),
                _("Hue-Saturation Tool: Adjust hue, saturation, and lightness"),
                N_("Hue-_Saturation..."), NULL,
                NULL, PICMAN_HELP_TOOL_HUE_SATURATION,
                PICMAN_STOCK_TOOL_HUE_SATURATION,
                data);
}

static void
picman_hue_saturation_tool_class_init (PicmanHueSaturationToolClass *klass)
{
  PicmanToolClass         *tool_class    = PICMAN_TOOL_CLASS (klass);
  PicmanImageMapToolClass *im_tool_class = PICMAN_IMAGE_MAP_TOOL_CLASS (klass);

  tool_class->initialize             = picman_hue_saturation_tool_initialize;

  im_tool_class->dialog_desc         = _("Adjust Hue / Lightness / Saturation");
  im_tool_class->settings_name       = "hue-saturation";
  im_tool_class->import_dialog_title = _("Import Hue-Saturation Settings");
  im_tool_class->export_dialog_title = _("Export Hue-Saturation Settings");

  im_tool_class->get_operation       = picman_hue_saturation_tool_get_operation;
  im_tool_class->dialog              = picman_hue_saturation_tool_dialog;
  im_tool_class->reset               = picman_hue_saturation_tool_reset;
}

static void
picman_hue_saturation_tool_init (PicmanHueSaturationTool *hs_tool)
{
}

static gboolean
picman_hue_saturation_tool_initialize (PicmanTool     *tool,
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
			   _("Hue-Saturation operates only on RGB color layers."));
      return FALSE;
    }

  return PICMAN_TOOL_CLASS (parent_class)->initialize (tool, display, error);
}

static GeglNode *
picman_hue_saturation_tool_get_operation (PicmanImageMapTool  *im_tool,
                                        GObject          **config,
                                        gchar            **undo_desc)
{
  PicmanHueSaturationTool *hs_tool = PICMAN_HUE_SATURATION_TOOL (im_tool);

  hs_tool->config = g_object_new (PICMAN_TYPE_HUE_SATURATION_CONFIG, NULL);

  g_signal_connect_object (hs_tool->config, "notify",
                           G_CALLBACK (hue_saturation_config_notify),
                           G_OBJECT (hs_tool), 0);

  *config = G_OBJECT (hs_tool->config);

  return gegl_node_new_child (NULL,
                              "operation", "picman:hue-saturation",
                              "config",    hs_tool->config,
                              NULL);
}


/***************************/
/*  Hue-Saturation dialog  */
/***************************/

static void
picman_hue_saturation_tool_dialog (PicmanImageMapTool *image_map_tool)
{
  PicmanHueSaturationTool   *hs_tool = PICMAN_HUE_SATURATION_TOOL (image_map_tool);
  PicmanHueSaturationConfig *config  = hs_tool->config;
  GtkWidget               *main_vbox;
  GtkWidget               *vbox;
  GtkWidget               *abox;
  GtkWidget               *table;
  GtkWidget               *button;
  GtkWidget               *frame;
  GtkWidget               *hbox;
  GtkObject               *data;
  GtkSizeGroup            *label_group;
  GtkSizeGroup            *spinner_group;
  GSList                  *group = NULL;
  gint                     i;

  const struct
  {
    const gchar *label;
    const gchar *tooltip;
    gint         label_col;
    gint         label_row;
    gint         frame_col;
    gint         frame_row;
  }
  hue_range_table[] =
  {
    { N_("M_aster"), N_("Adjust all colors"), 2, 3, 0, 0 },
    { N_("_R"),      N_("Red"),               2, 1, 2, 0 },
    { N_("_Y"),      N_("Yellow"),            1, 2, 0, 2 },
    { N_("_G"),      N_("Green"),             1, 4, 0, 4 },
    { N_("_C"),      N_("Cyan"),              2, 5, 2, 6 },
    { N_("_B"),      N_("Blue"),              3, 4, 4, 4 },
    { N_("_M"),      N_("Magenta"),           3, 2, 4, 2 }
  };

  main_vbox = picman_image_map_tool_dialog_get_vbox (image_map_tool);

  frame = picman_frame_new (_("Select Primary Color to Adjust"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  abox = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (vbox), abox, TRUE, TRUE, 0);
  gtk_widget_show (abox);

  /*  The table containing hue ranges  */
  table = gtk_table_new (7, 5, FALSE);
  gtk_table_set_col_spacing (GTK_TABLE (table), 0, 4);
  gtk_table_set_col_spacing (GTK_TABLE (table), 3, 4);
  gtk_table_set_row_spacing (GTK_TABLE (table), 0, 2);
  gtk_table_set_row_spacing (GTK_TABLE (table), 5, 2);
  gtk_container_add (GTK_CONTAINER (abox), table);

  /*  the radio buttons for hue ranges  */
  for (i = 0; i < G_N_ELEMENTS (hue_range_table); i++)
    {
      button = gtk_radio_button_new_with_mnemonic (group,
                                                   gettext (hue_range_table[i].label));
      group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
      g_object_set_data (G_OBJECT (button), "picman-item-data",
                         GINT_TO_POINTER (i));

      picman_help_set_help_data (button,
                               gettext (hue_range_table[i].tooltip),
                               NULL);

      if (i == 0)
        {
          gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button), FALSE);

          hs_tool->range_radio = button;
        }

      gtk_table_attach (GTK_TABLE (table), button,
                        hue_range_table[i].label_col,
                        hue_range_table[i].label_col + 1,
                        hue_range_table[i].label_row,
                        hue_range_table[i].label_row + 1,
                        GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);

      if (i > 0)
        {
          PicmanRGB color = { 0.0 };

          frame = gtk_frame_new (NULL);
          gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
          gtk_table_attach (GTK_TABLE (table), frame,
                            hue_range_table[i].frame_col,
                            hue_range_table[i].frame_col + 1,
                            hue_range_table[i].frame_row,
                            hue_range_table[i].frame_row + 1,
                            GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
          gtk_widget_show (frame);

          hs_tool->hue_range_color_area[i - 1] =
            picman_color_area_new (&color, PICMAN_COLOR_AREA_FLAT, 0);
          gtk_widget_set_size_request (hs_tool->hue_range_color_area[i - 1],
                                       DA_WIDTH, DA_HEIGHT);
          gtk_container_add (GTK_CONTAINER (frame),
                             hs_tool->hue_range_color_area[i - 1]);
          gtk_widget_show (hs_tool->hue_range_color_area[i - 1]);
        }

      g_signal_connect (button, "toggled",
                        G_CALLBACK (hue_saturation_range_callback),
                        hs_tool);

      gtk_widget_show (button);
    }

  gtk_widget_show (table);

  label_group  = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  spinner_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  /* Create the 'Overlap' option slider */
  table = gtk_table_new (3, 1, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  data = picman_scale_entry_new (GTK_TABLE (table), 0, 0,
                               _("_Overlap:"), SLIDER_WIDTH, -1,
                               config->overlap * 100.0,
                               0.0, 100.0, 1.0, 15.0, 0,
                               TRUE, 0.0, 0.0,
                               NULL, NULL);
  hs_tool->overlap_data = GTK_ADJUSTMENT (data);

  gtk_size_group_add_widget (label_group, PICMAN_SCALE_ENTRY_LABEL (data));
  gtk_size_group_add_widget (spinner_group, PICMAN_SCALE_ENTRY_SPINBUTTON (data));
  g_object_unref (label_group);
  g_object_unref (spinner_group);

  g_signal_connect (data, "value-changed",
                    G_CALLBACK (hue_saturation_overlap_changed),
                    hs_tool);

  frame = picman_frame_new (_("Adjust Selected Color"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  The table containing sliders  */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  /*  Create the hue scale widget  */
  data = picman_scale_entry_new (GTK_TABLE (table), 0, 0,
                               _("_Hue:"), SLIDER_WIDTH, -1,
                               config->hue[config->range] * 180.0,
                               -180.0, 180.0, 1.0, 15.0, 0,
                               TRUE, 0.0, 0.0,
                               NULL, NULL);
  hs_tool->hue_data = GTK_ADJUSTMENT (data);

  gtk_size_group_add_widget (label_group, PICMAN_SCALE_ENTRY_LABEL (data));
  gtk_size_group_add_widget (spinner_group, PICMAN_SCALE_ENTRY_SPINBUTTON (data));

  g_signal_connect (data, "value-changed",
                    G_CALLBACK (hue_saturation_hue_changed),
                    hs_tool);

  /*  Create the lightness scale widget  */
  data = picman_scale_entry_new (GTK_TABLE (table), 0, 1,
                               _("_Lightness:"), SLIDER_WIDTH, -1,
                               config->lightness[config->range]  * 100.0,
                               -100.0, 100.0, 1.0, 10.0, 0,
                               TRUE, 0.0, 0.0,
                               NULL, NULL);
  hs_tool->lightness_data = GTK_ADJUSTMENT (data);

  gtk_size_group_add_widget (label_group, PICMAN_SCALE_ENTRY_LABEL (data));
  gtk_size_group_add_widget (spinner_group, PICMAN_SCALE_ENTRY_SPINBUTTON (data));

  g_signal_connect (data, "value-changed",
                    G_CALLBACK (hue_saturation_lightness_changed),
                    hs_tool);

  /*  Create the saturation scale widget  */
  data = picman_scale_entry_new (GTK_TABLE (table), 0, 2,
                               _("_Saturation:"), SLIDER_WIDTH, -1,
                               config->saturation[config->range] * 100.0,
                               -100.0, 100.0, 1.0, 10.0, 0,
                               TRUE, 0.0, 0.0,
                               NULL, NULL);
  hs_tool->saturation_data = GTK_ADJUSTMENT (data);

  gtk_size_group_add_widget (label_group, PICMAN_SCALE_ENTRY_LABEL (data));
  gtk_size_group_add_widget (spinner_group, PICMAN_SCALE_ENTRY_SPINBUTTON (data));

  g_signal_connect (hs_tool->saturation_data, "value-changed",
                    G_CALLBACK (hue_saturation_saturation_changed),
                    hs_tool);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  button = gtk_button_new_with_mnemonic (_("R_eset Color"));
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (hue_saturation_range_reset_callback),
                    hs_tool);

  picman_int_radio_group_set_active (GTK_RADIO_BUTTON (hs_tool->range_radio),
                                   config->range);

  hue_saturation_update_color_areas (hs_tool);
}

static void
picman_hue_saturation_tool_reset (PicmanImageMapTool *image_map_tool)
{
  PicmanHueSaturationTool *hs_tool = PICMAN_HUE_SATURATION_TOOL (image_map_tool);
  PicmanHueRange           range   = hs_tool->config->range;

  g_object_freeze_notify (image_map_tool->config);

  if (image_map_tool->default_config)
    {
      picman_config_copy (PICMAN_CONFIG (image_map_tool->default_config),
                        PICMAN_CONFIG (image_map_tool->config),
                        0);
    }
  else
    {
      picman_config_reset (PICMAN_CONFIG (image_map_tool->config));
    }

  g_object_set (hs_tool->config,
                "range", range,
                NULL);

  g_object_thaw_notify (image_map_tool->config);
}

static void
hue_saturation_config_notify (GObject               *object,
                              GParamSpec            *pspec,
                              PicmanHueSaturationTool *hs_tool)
{
  PicmanHueSaturationConfig *config = PICMAN_HUE_SATURATION_CONFIG (object);

  if (! hs_tool->hue_data)
    return;

  if (! strcmp (pspec->name, "range"))
    {
      picman_int_radio_group_set_active (GTK_RADIO_BUTTON (hs_tool->range_radio),
                                       config->range);
    }
  else if (! strcmp (pspec->name, "hue"))
    {
      gtk_adjustment_set_value (hs_tool->hue_data,
                                config->hue[config->range] * 180.0);
    }
  else if (! strcmp (pspec->name, "lightness"))
    {
      gtk_adjustment_set_value (hs_tool->lightness_data,
                                config->lightness[config->range] * 100.0);
    }
  else if (! strcmp (pspec->name, "saturation"))
    {
      gtk_adjustment_set_value (hs_tool->saturation_data,
                                config->saturation[config->range] * 100.0);
    }
  else if (! strcmp (pspec->name, "overlap"))
    {
      gtk_adjustment_set_value (hs_tool->overlap_data,
                                config->overlap * 100.0);
    }

  hue_saturation_update_color_areas (hs_tool);
}

static void
hue_saturation_update_color_areas (PicmanHueSaturationTool *hs_tool)
{
  static PicmanRGB default_colors[6] =
  {
    { 1.0,   0,   0, },
    { 1.0, 1.0,   0, },
    {   0, 1.0,   0, },
    {   0, 1.0, 1.0, },
    {   0,   0, 1.0, },
    { 1.0,   0, 1.0, }
  };

  gint i;

  for (i = 0; i < 6; i++)
    {
      PicmanRGB color = default_colors[i];

      picman_operation_hue_saturation_map (hs_tool->config, &color, i + 1,
                                         &color);

      picman_color_area_set_color (PICMAN_COLOR_AREA (hs_tool->hue_range_color_area[i]),
                                 &color);
    }
}

static void
hue_saturation_range_callback (GtkWidget             *widget,
                               PicmanHueSaturationTool *hs_tool)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    {
      PicmanHueRange range;

      picman_radio_button_update (widget, &range);
      g_object_set (hs_tool->config,
                    "range", range,
                    NULL);
    }
}

static void
hue_saturation_range_reset_callback (GtkWidget             *widget,
                                     PicmanHueSaturationTool *hs_tool)
{
  picman_hue_saturation_config_reset_range (hs_tool->config);
}

static void
hue_saturation_hue_changed (GtkAdjustment         *adjustment,
                            PicmanHueSaturationTool *hs_tool)
{
  gdouble value = gtk_adjustment_get_value (adjustment) / 180.0;

  if (hs_tool->config->hue[hs_tool->config->range] != value)
    {
      g_object_set (hs_tool->config,
                    "hue", value,
                    NULL);
    }
}

static void
hue_saturation_lightness_changed (GtkAdjustment         *adjustment,
                                  PicmanHueSaturationTool *hs_tool)
{
  gdouble value = gtk_adjustment_get_value (adjustment) / 100.0;

  if (hs_tool->config->lightness[hs_tool->config->range] != value)
    {
      g_object_set (hs_tool->config,
                    "lightness", value,
                    NULL);
    }
}

static void
hue_saturation_saturation_changed (GtkAdjustment         *adjustment,
                                   PicmanHueSaturationTool *hs_tool)
{
  gdouble value = gtk_adjustment_get_value (adjustment) / 100.0;

  if (hs_tool->config->saturation[hs_tool->config->range] != value)
    {
      g_object_set (hs_tool->config,
                    "saturation", value,
                    NULL);
    }
}

static void
hue_saturation_overlap_changed (GtkAdjustment         *adjustment,
                                PicmanHueSaturationTool *hs_tool)
{
  gdouble value = gtk_adjustment_get_value (adjustment) / 100.0;

  if (hs_tool->config->overlap != value)
    {
      g_object_set (hs_tool->config,
                    "overlap", value,
                    NULL);
    }
}
