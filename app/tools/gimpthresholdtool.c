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

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "operations/picmanthresholdconfig.h"

#include "core/picmandrawable.h"
#include "core/picmandrawable-histogram.h"
#include "core/picmanerror.h"
#include "core/picmanhistogram.h"
#include "core/picmanimage.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanhistogrambox.h"
#include "widgets/picmanhistogramview.h"

#include "display/picmandisplay.h"

#include "picmanhistogramoptions.h"
#include "picmanthresholdtool.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void       picman_threshold_tool_finalize        (GObject           *object);

static gboolean   picman_threshold_tool_initialize      (PicmanTool          *tool,
                                                       PicmanDisplay       *display,
                                                       GError           **error);

static GeglNode * picman_threshold_tool_get_operation   (PicmanImageMapTool  *im_tool,
                                                       GObject          **config,
                                                       gchar            **undo_desc);
static void       picman_threshold_tool_dialog          (PicmanImageMapTool  *im_tool);

static void       picman_threshold_tool_config_notify   (GObject           *object,
                                                       GParamSpec        *pspec,
                                                       PicmanThresholdTool *t_tool);

static void       picman_threshold_tool_histogram_range (PicmanHistogramView *view,
                                                       gint               start,
                                                       gint               end,
                                                       PicmanThresholdTool *t_tool);
static void       picman_threshold_tool_auto_clicked    (GtkWidget         *button,
                                                       PicmanThresholdTool *t_tool);


G_DEFINE_TYPE (PicmanThresholdTool, picman_threshold_tool,
               PICMAN_TYPE_IMAGE_MAP_TOOL)

#define parent_class picman_threshold_tool_parent_class


void
picman_threshold_tool_register (PicmanToolRegisterCallback  callback,
                              gpointer                  data)
{
  (* callback) (PICMAN_TYPE_THRESHOLD_TOOL,
                PICMAN_TYPE_HISTOGRAM_OPTIONS,
                picman_histogram_options_gui,
                0,
                "picman-threshold-tool",
                _("Threshold"),
                _("Threshold Tool: Reduce image to two colors using a threshold"),
                N_("_Threshold..."), NULL,
                NULL, PICMAN_HELP_TOOL_THRESHOLD,
                PICMAN_STOCK_TOOL_THRESHOLD,
                data);
}

static void
picman_threshold_tool_class_init (PicmanThresholdToolClass *klass)
{
  GObjectClass          *object_class  = G_OBJECT_CLASS (klass);
  PicmanToolClass         *tool_class    = PICMAN_TOOL_CLASS (klass);
  PicmanImageMapToolClass *im_tool_class = PICMAN_IMAGE_MAP_TOOL_CLASS (klass);

  object_class->finalize             = picman_threshold_tool_finalize;

  tool_class->initialize             = picman_threshold_tool_initialize;

  im_tool_class->dialog_desc         = _("Apply Threshold");
  im_tool_class->settings_name       = "threshold";
  im_tool_class->import_dialog_title = _("Import Threshold Settings");
  im_tool_class->export_dialog_title = _("Export Threshold Settings");

  im_tool_class->get_operation       = picman_threshold_tool_get_operation;
  im_tool_class->dialog              = picman_threshold_tool_dialog;
}

static void
picman_threshold_tool_init (PicmanThresholdTool *t_tool)
{
  t_tool->histogram = picman_histogram_new ();
}

static void
picman_threshold_tool_finalize (GObject *object)
{
  PicmanThresholdTool *t_tool = PICMAN_THRESHOLD_TOOL (object);

  if (t_tool->histogram)
    {
      picman_histogram_unref (t_tool->histogram);
      t_tool->histogram = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
picman_threshold_tool_initialize (PicmanTool     *tool,
                                PicmanDisplay  *display,
                                GError      **error)
{
  PicmanThresholdTool *t_tool   = PICMAN_THRESHOLD_TOOL (tool);
  PicmanImage         *image    = picman_display_get_image (display);
  PicmanDrawable      *drawable = picman_image_get_active_drawable (image);

  if (! PICMAN_TOOL_CLASS (parent_class)->initialize (tool, display, error))
    {
      return FALSE;
    }

  picman_drawable_calculate_histogram (drawable, t_tool->histogram);
  picman_histogram_view_set_histogram (t_tool->histogram_box->view,
                                     t_tool->histogram);

  return TRUE;
}

static GeglNode *
picman_threshold_tool_get_operation (PicmanImageMapTool  *image_map_tool,
                                   GObject          **config,
                                   gchar            **undo_desc)
{
  PicmanThresholdTool *t_tool = PICMAN_THRESHOLD_TOOL (image_map_tool);

  t_tool->config = g_object_new (PICMAN_TYPE_THRESHOLD_CONFIG, NULL);

  g_signal_connect_object (t_tool->config, "notify",
                           G_CALLBACK (picman_threshold_tool_config_notify),
                           G_OBJECT (t_tool), 0);

  *config = G_OBJECT (t_tool->config);

  return gegl_node_new_child (NULL,
                              "operation", "picman:threshold",
                              "config",    t_tool->config,
                              NULL);
}


/**********************/
/*  Threshold dialog  */
/**********************/

static void
picman_threshold_tool_dialog (PicmanImageMapTool *image_map_tool)
{
  PicmanThresholdTool   *t_tool       = PICMAN_THRESHOLD_TOOL (image_map_tool);
  PicmanToolOptions     *tool_options = PICMAN_TOOL_GET_OPTIONS (image_map_tool);
  PicmanThresholdConfig *config       = t_tool->config;
  GtkWidget           *main_vbox;
  GtkWidget           *hbox;
  GtkWidget           *menu;
  GtkWidget           *box;
  GtkWidget           *button;

  main_vbox = picman_image_map_tool_dialog_get_vbox (image_map_tool);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  menu = picman_prop_enum_stock_box_new (G_OBJECT (tool_options),
                                       "histogram-scale", "picman-histogram",
                                       0, 0);
  gtk_box_pack_end (GTK_BOX (hbox), menu, FALSE, FALSE, 0);
  gtk_widget_show (menu);

  box = picman_histogram_box_new ();
  gtk_box_pack_start (GTK_BOX (main_vbox), box, TRUE, TRUE, 0);
  gtk_widget_show (box);

  t_tool->histogram_box = PICMAN_HISTOGRAM_BOX (box);

  picman_histogram_view_set_range (t_tool->histogram_box->view,
                                 config->low  * 255.999,
                                 config->high * 255.999);

  g_signal_connect (t_tool->histogram_box->view, "range-changed",
                    G_CALLBACK (picman_threshold_tool_histogram_range),
                    t_tool);

  picman_histogram_options_connect_view (PICMAN_HISTOGRAM_OPTIONS (tool_options),
                                       t_tool->histogram_box->view);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  button = gtk_button_new_with_mnemonic (_("_Auto"));
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  picman_help_set_help_data (button, _("Automatically adjust to optimal "
                                     "binarization threshold"), NULL);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (picman_threshold_tool_auto_clicked),
                    t_tool);
}

static void
picman_threshold_tool_config_notify (GObject           *object,
                                   GParamSpec        *pspec,
                                   PicmanThresholdTool *t_tool)
{
  PicmanThresholdConfig *config = PICMAN_THRESHOLD_CONFIG (object);

  if (! t_tool->histogram_box)
    return;

  picman_histogram_view_set_range (t_tool->histogram_box->view,
                                 config->low  * 255.999,
                                 config->high * 255.999);
}

static void
picman_threshold_tool_histogram_range (PicmanHistogramView *widget,
                                     gint               start,
                                     gint               end,
                                     PicmanThresholdTool *t_tool)
{
  gdouble low  = start / 255.0;
  gdouble high = end   / 255.0;

  if (low  != t_tool->config->low ||
      high != t_tool->config->high)
    {
      g_object_set (t_tool->config,
                    "low",  low,
                    "high", high,
                    NULL);
    }
}

static void
picman_threshold_tool_auto_clicked (GtkWidget         *button,
                                  PicmanThresholdTool *t_tool)
{
  PicmanDrawable *drawable = PICMAN_IMAGE_MAP_TOOL (t_tool)->drawable;
  gdouble       low;

  low = picman_histogram_get_threshold (t_tool->histogram,
                                      picman_drawable_is_rgb (drawable) ?
                                      PICMAN_HISTOGRAM_RGB :
                                      PICMAN_HISTOGRAM_VALUE,
                                      0, 255);

  picman_histogram_view_set_range (t_tool->histogram_box->view,
                                 low, 255.0);
}
