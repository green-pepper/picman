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

#include "operations/picmanbrightnesscontrastconfig.h"

#include "core/picmandrawable.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-constructors.h"

#include "display/picmandisplay.h"

#include "picmanbrightnesscontrasttool.h"
#include "picmanimagemapoptions.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


#define SLIDER_WIDTH 200


static void   picman_brightness_contrast_tool_button_press   (PicmanTool              *tool,
                                                            const PicmanCoords      *coords,
                                                            guint32                time,
                                                            GdkModifierType        state,
                                                            PicmanButtonPressType    press_type,
                                                            PicmanDisplay           *display);
static void   picman_brightness_contrast_tool_button_release (PicmanTool              *tool,
                                                            const PicmanCoords      *coords,
                                                            guint32                time,
                                                            GdkModifierType        state,
                                                            PicmanButtonReleaseType  release_type,
                                                            PicmanDisplay           *display);
static void   picman_brightness_contrast_tool_motion         (PicmanTool              *tool,
                                                            const PicmanCoords      *coords,
                                                            guint32                time,
                                                            GdkModifierType        state,
                                                            PicmanDisplay           *display);

static GeglNode *
              picman_brightness_contrast_tool_get_operation  (PicmanImageMapTool      *image_map_tool,
                                                            GObject              **config,
                                                            gchar                **undo_desc);
static void   picman_brightness_contrast_tool_dialog         (PicmanImageMapTool      *image_map_tool);

static void   brightness_contrast_config_notify            (GObject                    *object,
                                                            GParamSpec                 *pspec,
                                                            PicmanBrightnessContrastTool *bc_tool);

static void   brightness_contrast_brightness_changed       (GtkAdjustment              *adj,
                                                            PicmanBrightnessContrastTool *bc_tool);
static void   brightness_contrast_contrast_changed         (GtkAdjustment              *adj,
                                                            PicmanBrightnessContrastTool *bc_tool);

static void   brightness_contrast_to_levels_callback       (GtkWidget                  *widget,
                                                            PicmanBrightnessContrastTool *bc_tool);


G_DEFINE_TYPE (PicmanBrightnessContrastTool, picman_brightness_contrast_tool,
               PICMAN_TYPE_IMAGE_MAP_TOOL)

#define parent_class picman_brightness_contrast_tool_parent_class


void
picman_brightness_contrast_tool_register (PicmanToolRegisterCallback  callback,
                                        gpointer                  data)
{
  (* callback) (PICMAN_TYPE_BRIGHTNESS_CONTRAST_TOOL,
                PICMAN_TYPE_IMAGE_MAP_OPTIONS, NULL,
                0,
                "picman-brightness-contrast-tool",
                _("Brightness-Contrast"),
                _("Brightness/Contrast Tool: Adjust brightness and contrast"),
                N_("B_rightness-Contrast..."), NULL,
                NULL, PICMAN_HELP_TOOL_BRIGHTNESS_CONTRAST,
                PICMAN_STOCK_TOOL_BRIGHTNESS_CONTRAST,
                data);
}

static void
picman_brightness_contrast_tool_class_init (PicmanBrightnessContrastToolClass *klass)
{
  PicmanToolClass         *tool_class    = PICMAN_TOOL_CLASS (klass);
  PicmanImageMapToolClass *im_tool_class = PICMAN_IMAGE_MAP_TOOL_CLASS (klass);

  tool_class->button_press           = picman_brightness_contrast_tool_button_press;
  tool_class->button_release         = picman_brightness_contrast_tool_button_release;
  tool_class->motion                 = picman_brightness_contrast_tool_motion;

  im_tool_class->dialog_desc         = _("Adjust Brightness and Contrast");
  im_tool_class->settings_name       = "brightness-contrast";
  im_tool_class->import_dialog_title = _("Import Brightness-Contrast settings");
  im_tool_class->export_dialog_title = _("Export Brightness-Contrast settings");

  im_tool_class->get_operation       = picman_brightness_contrast_tool_get_operation;
  im_tool_class->dialog              = picman_brightness_contrast_tool_dialog;
}

static void
picman_brightness_contrast_tool_init (PicmanBrightnessContrastTool *bc_tool)
{
}

static GeglNode *
picman_brightness_contrast_tool_get_operation (PicmanImageMapTool  *im_tool,
                                             GObject          **config,
                                             gchar            **undo_desc)
{
  PicmanBrightnessContrastTool *bc_tool = PICMAN_BRIGHTNESS_CONTRAST_TOOL (im_tool);

  bc_tool->config = g_object_new (PICMAN_TYPE_BRIGHTNESS_CONTRAST_CONFIG, NULL);

  g_signal_connect_object (bc_tool->config, "notify",
                           G_CALLBACK (brightness_contrast_config_notify),
                           G_OBJECT (bc_tool), 0);

  *config = G_OBJECT (bc_tool->config);

  return gegl_node_new_child (NULL,
                              "operation", "picman:brightness-contrast",
                              "config",    bc_tool->config,
                              NULL);
}

static void
picman_brightness_contrast_tool_button_press (PicmanTool            *tool,
                                            const PicmanCoords    *coords,
                                            guint32              time,
                                            GdkModifierType      state,
                                            PicmanButtonPressType  press_type,
                                            PicmanDisplay         *display)
{
  PicmanBrightnessContrastTool *bc_tool = PICMAN_BRIGHTNESS_CONTRAST_TOOL (tool);

  bc_tool->x  = coords->x - bc_tool->config->contrast   * 127.0;
  bc_tool->y  = coords->y + bc_tool->config->brightness * 127.0;
  bc_tool->dx =   bc_tool->config->contrast   * 127.0;
  bc_tool->dy = - bc_tool->config->brightness * 127.0;

  picman_tool_control_activate (tool->control);
  tool->display = display;
}

static void
picman_brightness_contrast_tool_button_release (PicmanTool              *tool,
                                              const PicmanCoords      *coords,
                                              guint32                time,
                                              GdkModifierType        state,
                                              PicmanButtonReleaseType  release_type,
                                              PicmanDisplay           *display)
{
  PicmanBrightnessContrastTool *bc_tool = PICMAN_BRIGHTNESS_CONTRAST_TOOL (tool);
  PicmanImageMapTool           *im_tool = PICMAN_IMAGE_MAP_TOOL (tool);

  picman_tool_control_halt (tool->control);

  if (bc_tool->dx == 0 && bc_tool->dy == 0)
    return;

  if (release_type == PICMAN_BUTTON_RELEASE_CANCEL)
    picman_config_reset (PICMAN_CONFIG (bc_tool->config));

  picman_image_map_tool_preview (im_tool);
}

static void
picman_brightness_contrast_tool_motion (PicmanTool         *tool,
                                      const PicmanCoords *coords,
                                      guint32           time,
                                      GdkModifierType   state,
                                      PicmanDisplay      *display)
{
  PicmanBrightnessContrastTool *bc_tool = PICMAN_BRIGHTNESS_CONTRAST_TOOL (tool);

  bc_tool->dx =   (coords->x - bc_tool->x);
  bc_tool->dy = - (coords->y - bc_tool->y);

  g_object_set (bc_tool->config,
                "brightness", CLAMP (bc_tool->dy, -127.0, 127.0) / 127.0,
                "contrast",   CLAMP (bc_tool->dx, -127.0, 127.0) / 127.0,
                NULL);
}


/********************************/
/*  Brightness Contrast dialog  */
/********************************/

static void
picman_brightness_contrast_tool_dialog (PicmanImageMapTool *image_map_tool)
{
  PicmanBrightnessContrastTool   *bc_tool;
  PicmanBrightnessContrastConfig *config;
  GtkWidget                    *main_vbox;
  GtkWidget                    *table;
  GtkWidget                    *button;
  GtkObject                    *data;

  bc_tool = PICMAN_BRIGHTNESS_CONTRAST_TOOL (image_map_tool);
  config  = bc_tool->config;

  main_vbox = picman_image_map_tool_dialog_get_vbox (image_map_tool);

  /*  The table containing sliders  */
  table = gtk_table_new (2, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  /*  Create the brightness scale widget  */
  data = picman_scale_entry_new (GTK_TABLE (table), 0, 0,
                               _("_Brightness:"), SLIDER_WIDTH, -1,
                               config->brightness * 127.0,
                               -127.0, 127.0, 1.0, 10.0, 0,
                               TRUE, 0.0, 0.0,
                               NULL, NULL);
  bc_tool->brightness_data = GTK_ADJUSTMENT (data);

  g_signal_connect (data, "value-changed",
                    G_CALLBACK (brightness_contrast_brightness_changed),
                    bc_tool);

  /*  Create the contrast scale widget  */
  data = picman_scale_entry_new (GTK_TABLE (table), 0, 1,
                               _("Con_trast:"), SLIDER_WIDTH, -1,
                               config->contrast * 127.0,
                               -127.0, 127.0, 1.0, 10.0, 0,
                               TRUE, 0.0, 0.0,
                               NULL, NULL);
  bc_tool->contrast_data = GTK_ADJUSTMENT (data);

  g_signal_connect (data, "value-changed",
                    G_CALLBACK (brightness_contrast_contrast_changed),
                    bc_tool);

  button = picman_stock_button_new (PICMAN_STOCK_TOOL_LEVELS,
                                  _("Edit these Settings as Levels"));
  gtk_box_pack_start (GTK_BOX (main_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (brightness_contrast_to_levels_callback),
                    bc_tool);
}

static void
brightness_contrast_config_notify (GObject                    *object,
                                   GParamSpec                 *pspec,
                                   PicmanBrightnessContrastTool *bc_tool)
{
  PicmanBrightnessContrastConfig *config;

  config = PICMAN_BRIGHTNESS_CONTRAST_CONFIG (object);

  if (! bc_tool->brightness_data)
    return;

  if (! strcmp (pspec->name, "brightness"))
    {
      gtk_adjustment_set_value (bc_tool->brightness_data,
                                config->brightness * 127.0);
    }
  else if (! strcmp (pspec->name, "contrast"))
    {
      gtk_adjustment_set_value (bc_tool->contrast_data,
                                config->contrast * 127.0);
    }
}

static void
brightness_contrast_brightness_changed (GtkAdjustment              *adjustment,
                                        PicmanBrightnessContrastTool *bc_tool)
{
  PicmanBrightnessContrastConfig *config = bc_tool->config;
  gdouble                       value;

  value = gtk_adjustment_get_value (adjustment) / 127.0;

  if (config->brightness != value)
    {
      g_object_set (config,
                    "brightness", value,
                    NULL);
    }
}

static void
brightness_contrast_contrast_changed (GtkAdjustment              *adjustment,
                                      PicmanBrightnessContrastTool *bc_tool)
{
  PicmanBrightnessContrastConfig *config = bc_tool->config;
  gdouble                       value;

  value = gtk_adjustment_get_value (adjustment) / 127.0;

  if (config->contrast != value)
    {
      g_object_set (config,
                    "contrast", value,
                    NULL);
    }
}

static void
brightness_contrast_to_levels_callback (GtkWidget                  *widget,
                                        PicmanBrightnessContrastTool *bc_tool)
{
  PicmanLevelsConfig *levels;

  levels = picman_brightness_contrast_config_to_levels_config (bc_tool->config);

  picman_image_map_tool_edit_as (PICMAN_IMAGE_MAP_TOOL (bc_tool),
                               "picman-levels-tool",
                               PICMAN_CONFIG (levels));

  g_object_unref (levels);
}
