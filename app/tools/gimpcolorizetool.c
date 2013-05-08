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

#include "operations/picmancolorizeconfig.h"

#include "core/picmandrawable.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"

#include "widgets/picmancolorpanel.h"
#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "picmancolorizetool.h"
#include "picmancoloroptions.h"

#include "picman-intl.h"


#define SLIDER_WIDTH  200
#define SPINNER_WIDTH 4


/*  local function prototypes  */

static gboolean   picman_colorize_tool_initialize    (PicmanTool         *tool,
                                                    PicmanDisplay      *display,
                                                    GError          **error);

static GeglNode * picman_colorize_tool_get_operation (PicmanImageMapTool *im_tool,
                                                    GObject         **config,
                                                    gchar           **undo_desc);
static void       picman_colorize_tool_dialog        (PicmanImageMapTool *im_tool);
static void       picman_colorize_tool_color_picked  (PicmanImageMapTool *im_tool,
                                                    gpointer          identifier,
                                                    const Babl       *sample_format,
                                                    const PicmanRGB    *color);

static void       picman_colorize_tool_config_notify (GObject          *object,
                                                    GParamSpec       *pspec,
                                                    PicmanColorizeTool *col_tool);

static void       colorize_hue_changed             (GtkAdjustment    *adj,
                                                    PicmanColorizeTool *col_tool);
static void       colorize_saturation_changed      (GtkAdjustment    *adj,
                                                    PicmanColorizeTool *col_tool);
static void       colorize_lightness_changed       (GtkAdjustment    *adj,
                                                    PicmanColorizeTool *col_tool);
static void       colorize_color_changed           (GtkWidget        *button,
                                                    PicmanColorizeTool *col_tool);


G_DEFINE_TYPE (PicmanColorizeTool, picman_colorize_tool, PICMAN_TYPE_IMAGE_MAP_TOOL)

#define parent_class picman_colorize_tool_parent_class


void
picman_colorize_tool_register (PicmanToolRegisterCallback  callback,
                             gpointer                  data)
{
  (* callback) (PICMAN_TYPE_COLORIZE_TOOL,
                PICMAN_TYPE_COLOR_OPTIONS,
                picman_color_options_gui,
                0,
                "picman-colorize-tool",
                _("Colorize"),
                _("Colorize Tool: Colorize the image"),
                N_("Colori_ze..."), NULL,
                NULL, PICMAN_HELP_TOOL_COLORIZE,
                PICMAN_STOCK_TOOL_COLORIZE,
                data);
}

static void
picman_colorize_tool_class_init (PicmanColorizeToolClass *klass)
{
  PicmanToolClass         *tool_class    = PICMAN_TOOL_CLASS (klass);
  PicmanImageMapToolClass *im_tool_class = PICMAN_IMAGE_MAP_TOOL_CLASS (klass);

  tool_class->initialize             = picman_colorize_tool_initialize;

  im_tool_class->dialog_desc         = _("Colorize the Image");
  im_tool_class->settings_name       = "colorize";
  im_tool_class->import_dialog_title = _("Import Colorize Settings");
  im_tool_class->export_dialog_title = _("Export Colorize Settings");

  im_tool_class->get_operation       = picman_colorize_tool_get_operation;
  im_tool_class->dialog              = picman_colorize_tool_dialog;
  im_tool_class->color_picked        = picman_colorize_tool_color_picked;
}

static void
picman_colorize_tool_init (PicmanColorizeTool *col_tool)
{
}

static gboolean
picman_colorize_tool_initialize (PicmanTool     *tool,
                               PicmanDisplay  *display,
                               GError      **error)
{
  PicmanImage    *image    = picman_display_get_image (display);
  PicmanDrawable *drawable = picman_image_get_active_drawable (image);

  if (! drawable)
    return FALSE;

  if (picman_drawable_is_gray (drawable))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Colorize does not operate on grayscale layers."));
      return FALSE;
    }

  if (! PICMAN_TOOL_CLASS (parent_class)->initialize (tool, display, error))
    {
      return FALSE;
    }

  return TRUE;
}

static GeglNode *
picman_colorize_tool_get_operation (PicmanImageMapTool  *im_tool,
                                  GObject          **config,
                                  gchar            **undo_desc)
{
  PicmanColorizeTool *col_tool = PICMAN_COLORIZE_TOOL (im_tool);

  col_tool->config = g_object_new (PICMAN_TYPE_COLORIZE_CONFIG, NULL);

  g_signal_connect_object (col_tool->config, "notify",
                           G_CALLBACK (picman_colorize_tool_config_notify),
                           G_OBJECT (col_tool), 0);

  *config = G_OBJECT (col_tool->config);

  return gegl_node_new_child (NULL,
                              "operation", "picman:colorize",
                              "config",    col_tool->config,
                              NULL);
}


/***************************/
/*  Hue-Saturation dialog  */
/***************************/

static void
picman_colorize_tool_dialog (PicmanImageMapTool *image_map_tool)
{
  PicmanColorizeTool *col_tool = PICMAN_COLORIZE_TOOL (image_map_tool);
  GtkWidget        *main_vbox;
  GtkWidget        *table;
  GtkWidget        *frame;
  GtkWidget        *vbox;
  GtkWidget        *hbox;
  GtkWidget        *button;
  GtkObject        *data;
  PicmanRGB           color;

  main_vbox = picman_image_map_tool_dialog_get_vbox (image_map_tool);

  frame = picman_frame_new (_("Select Color"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  The table containing sliders  */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  /*  Create the hue scale widget  */
  data = picman_scale_entry_new (GTK_TABLE (table), 0, 0,
                               _("_Hue:"), SLIDER_WIDTH, SPINNER_WIDTH,
                               col_tool->config->hue * 360.0,
                               0.0, 359.99, 1.0, 15.0, 0,
                               TRUE, 0.0, 0.0,
                               NULL, NULL);
  col_tool->hue_data = GTK_ADJUSTMENT (data);

  g_signal_connect (data, "value-changed",
                    G_CALLBACK (colorize_hue_changed),
                    col_tool);

  /*  Create the saturation scale widget  */
  data = picman_scale_entry_new (GTK_TABLE (table), 0, 1,
                               _("_Saturation:"), SLIDER_WIDTH, SPINNER_WIDTH,
                               col_tool->config->saturation * 100.0,
                               0.0, 100.0, 1.0, 10.0, 0,
                               TRUE, 0.0, 0.0,
                               NULL, NULL);
  col_tool->saturation_data = GTK_ADJUSTMENT (data);

  g_signal_connect (data, "value-changed",
                    G_CALLBACK (colorize_saturation_changed),
                    col_tool);

  /*  Create the lightness scale widget  */
  data = picman_scale_entry_new (GTK_TABLE (table), 0, 2,
                               _("_Lightness:"), SLIDER_WIDTH, SPINNER_WIDTH,
                               col_tool->config->lightness * 100.0,
                               -100.0, 100.0, 1.0, 10.0, 0,
                               TRUE, 0.0, 0.0,
                               NULL, NULL);
  col_tool->lightness_data = GTK_ADJUSTMENT (data);

  g_signal_connect (data, "value-changed",
                    G_CALLBACK (colorize_lightness_changed),
                    col_tool);

  /*  Create the color button  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  picman_colorize_config_get_color (col_tool->config, &color);

  col_tool->color_button = picman_color_panel_new (_("Colorize Color"),
                                                 &color,
                                                 PICMAN_COLOR_AREA_FLAT,
                                                 128, 24);
  picman_color_button_set_update (PICMAN_COLOR_BUTTON (col_tool->color_button),
                                TRUE);
  picman_color_panel_set_context (PICMAN_COLOR_PANEL (col_tool->color_button),
                                PICMAN_CONTEXT (PICMAN_TOOL_GET_OPTIONS (col_tool)));
  gtk_box_pack_start (GTK_BOX (hbox), col_tool->color_button, TRUE, TRUE, 0);
  gtk_widget_show (col_tool->color_button);

  g_signal_connect (col_tool->color_button, "color-changed",
                    G_CALLBACK (colorize_color_changed),
                    col_tool);

  button = picman_image_map_tool_add_color_picker (image_map_tool,
                                                 "colorize",
                                                 PICMAN_STOCK_COLOR_PICKER_GRAY,
                                                 _("Pick color from image"));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);
}

static void
picman_colorize_tool_color_picked (PicmanImageMapTool *im_tool,
                                 gpointer          identifier,
                                 const Babl       *sample_format,
                                 const PicmanRGB    *color)
{
  PicmanColorizeTool *col_tool = PICMAN_COLORIZE_TOOL (im_tool);

  picman_colorize_config_set_color (col_tool->config, color);
}

static void
picman_colorize_tool_config_notify (GObject          *object,
                                  GParamSpec       *pspec,
                                  PicmanColorizeTool *col_tool)
{
  PicmanColorizeConfig *config = PICMAN_COLORIZE_CONFIG (object);
  PicmanRGB             color;

  if (! col_tool->hue_data)
    return;

  if (! strcmp (pspec->name, "hue"))
    {
      gtk_adjustment_set_value (col_tool->hue_data,
                                config->hue * 360.0);
    }
  else if (! strcmp (pspec->name, "saturation"))
    {
      gtk_adjustment_set_value (col_tool->saturation_data,
                                config->saturation * 100.0);
    }
  else if (! strcmp (pspec->name, "lightness"))
    {
      gtk_adjustment_set_value (col_tool->lightness_data,
                                config->lightness * 100.0);
    }

  picman_colorize_config_get_color (col_tool->config, &color);
  picman_color_button_set_color (PICMAN_COLOR_BUTTON (col_tool->color_button),
                               &color);
}

static void
colorize_hue_changed (GtkAdjustment    *adjustment,
                      PicmanColorizeTool *col_tool)
{
  gdouble value = gtk_adjustment_get_value (adjustment) / 360.0;

  if (col_tool->config->hue != value)
    {
      g_object_set (col_tool->config,
                    "hue", value,
                    NULL);
    }
}

static void
colorize_saturation_changed (GtkAdjustment    *adjustment,
                             PicmanColorizeTool *col_tool)
{
  gdouble value = gtk_adjustment_get_value (adjustment) / 100.0;

  if (col_tool->config->saturation != value)
    {
      g_object_set (col_tool->config,
                    "saturation", value,
                    NULL);
    }
}

static void
colorize_lightness_changed (GtkAdjustment    *adjustment,
                            PicmanColorizeTool *col_tool)
{
  gdouble value = gtk_adjustment_get_value (adjustment) / 100.0;

  if (col_tool->config->lightness != value)
    {
      g_object_set (col_tool->config,
                    "lightness", value,
                    NULL);
    }
}

static void
colorize_color_changed (GtkWidget        *button,
                        PicmanColorizeTool *col_tool)
{
  PicmanRGB color;

  picman_color_button_get_color (PICMAN_COLOR_BUTTON (button), &color);
  picman_colorize_config_set_color (col_tool->config, &color);
}
