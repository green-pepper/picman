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

#include <errno.h>

#include <glib/gstdio.h>
#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "gegl/picman-babl.h"

#include "operations/picmanlevelsconfig.h"
#include "operations/picmanoperationlevels.h"

#include "core/picmandrawable.h"
#include "core/picmandrawable-histogram.h"
#include "core/picmanerror.h"
#include "core/picmanhistogram.h"
#include "core/picmanimage.h"

#include "widgets/picmancolorbar.h"
#include "widgets/picmanhandlebar.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanhistogramview.h"
#include "widgets/picmanwidgets-constructors.h"

#include "display/picmandisplay.h"

#include "picmanhistogramoptions.h"
#include "picmanlevelstool.h"

#include "picman-intl.h"


#define PICK_LOW_INPUT    (1 << 0)
#define PICK_GAMMA        (1 << 1)
#define PICK_HIGH_INPUT   (1 << 2)
#define PICK_ALL_CHANNELS (1 << 8)

#define HISTOGRAM_WIDTH    256
#define GRADIENT_HEIGHT     12
#define CONTROL_HEIGHT      10


/*  local function prototypes  */

static void       picman_levels_tool_finalize       (GObject           *object);

static gboolean   picman_levels_tool_initialize     (PicmanTool          *tool,
                                                   PicmanDisplay       *display,
                                                   GError           **error);

static GeglNode * picman_levels_tool_get_operation  (PicmanImageMapTool  *im_tool,
                                                   GObject          **config,
                                                   gchar            **undo_desc);
static void       picman_levels_tool_dialog         (PicmanImageMapTool  *im_tool);
static void       picman_levels_tool_reset          (PicmanImageMapTool  *im_tool);
static gboolean   picman_levels_tool_settings_import(PicmanImageMapTool  *im_tool,
                                                   const gchar       *filename,
                                                   GError           **error);
static gboolean   picman_levels_tool_settings_export(PicmanImageMapTool  *im_tool,
                                                   const gchar       *filename,
                                                   GError           **error);
static void       picman_levels_tool_color_picked   (PicmanImageMapTool  *im_tool,
                                                   gpointer           identifier,
                                                   const Babl        *sample_format,
                                                   const PicmanRGB     *color);

static void       picman_levels_tool_export_setup   (PicmanSettingsBox   *settings_box,
                                                   GtkFileChooserDialog *dialog,
                                                   gboolean           export,
                                                   PicmanLevelsTool    *tool);
static void       picman_levels_tool_config_notify  (GObject           *object,
                                                   GParamSpec        *pspec,
                                                   PicmanLevelsTool    *tool);

static void       levels_update_input_bar         (PicmanLevelsTool    *tool);

static void       levels_channel_callback         (GtkWidget         *widget,
                                                   PicmanLevelsTool    *tool);
static void       levels_channel_reset_callback   (GtkWidget         *widget,
                                                   PicmanLevelsTool    *tool);

static gboolean   levels_menu_sensitivity         (gint               value,
                                                   gpointer           data);

static void       levels_stretch_callback         (GtkWidget         *widget,
                                                   PicmanLevelsTool    *tool);
static void       levels_low_input_changed        (GtkAdjustment     *adjustment,
                                                   PicmanLevelsTool    *tool);
static void       levels_gamma_changed            (GtkAdjustment     *adjustment,
                                                   PicmanLevelsTool    *tool);
static void       levels_linear_gamma_changed     (GtkAdjustment     *adjustment,
                                                   PicmanLevelsTool    *tool);
static void       levels_high_input_changed       (GtkAdjustment     *adjustment,
                                                   PicmanLevelsTool    *tool);
static void       levels_low_output_changed       (GtkAdjustment     *adjustment,
                                                   PicmanLevelsTool    *tool);
static void       levels_high_output_changed      (GtkAdjustment     *adjustment,
                                                   PicmanLevelsTool    *tool);

static void       levels_to_curves_callback       (GtkWidget         *widget,
                                                   PicmanLevelsTool    *tool);


G_DEFINE_TYPE (PicmanLevelsTool, picman_levels_tool, PICMAN_TYPE_IMAGE_MAP_TOOL)

#define parent_class picman_levels_tool_parent_class


void
picman_levels_tool_register (PicmanToolRegisterCallback  callback,
                           gpointer                  data)
{
  (* callback) (PICMAN_TYPE_LEVELS_TOOL,
                PICMAN_TYPE_HISTOGRAM_OPTIONS,
                picman_color_options_gui,
                0,
                "picman-levels-tool",
                _("Levels"),
                _("Levels Tool: Adjust color levels"),
                N_("_Levels..."), NULL,
                NULL, PICMAN_HELP_TOOL_LEVELS,
                PICMAN_STOCK_TOOL_LEVELS,
                data);
}

static void
picman_levels_tool_class_init (PicmanLevelsToolClass *klass)
{
  GObjectClass          *object_class  = G_OBJECT_CLASS (klass);
  PicmanToolClass         *tool_class    = PICMAN_TOOL_CLASS (klass);
  PicmanImageMapToolClass *im_tool_class = PICMAN_IMAGE_MAP_TOOL_CLASS (klass);

  object_class->finalize             = picman_levels_tool_finalize;

  tool_class->initialize             = picman_levels_tool_initialize;

  im_tool_class->dialog_desc         = _("Adjust Color Levels");
  im_tool_class->settings_name       = "levels";
  im_tool_class->import_dialog_title = _("Import Levels");
  im_tool_class->export_dialog_title = _("Export Levels");

  im_tool_class->get_operation       = picman_levels_tool_get_operation;
  im_tool_class->dialog              = picman_levels_tool_dialog;
  im_tool_class->reset               = picman_levels_tool_reset;
  im_tool_class->settings_import     = picman_levels_tool_settings_import;
  im_tool_class->settings_export     = picman_levels_tool_settings_export;
  im_tool_class->color_picked        = picman_levels_tool_color_picked;
}

static void
picman_levels_tool_init (PicmanLevelsTool *tool)
{
  tool->histogram = picman_histogram_new ();
}

static void
picman_levels_tool_finalize (GObject *object)
{
  PicmanLevelsTool *tool = PICMAN_LEVELS_TOOL (object);

  if (tool->histogram)
    {
      picman_histogram_unref (tool->histogram);
      tool->histogram = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
picman_levels_tool_initialize (PicmanTool     *tool,
                             PicmanDisplay  *display,
                             GError      **error)
{
  PicmanLevelsTool   *l_tool   = PICMAN_LEVELS_TOOL (tool);
  PicmanImage        *image    = picman_display_get_image (display);
  PicmanDrawable     *drawable = picman_image_get_active_drawable (image);
  PicmanLevelsConfig *config   = l_tool->config;
  gdouble           scale_factor;
  gdouble           step;
  gint              digits;

  if (! PICMAN_TOOL_CLASS (parent_class)->initialize (tool, display, error))
    {
      return FALSE;
    }

  picman_int_combo_box_set_sensitivity (PICMAN_INT_COMBO_BOX (l_tool->channel_menu),
                                      levels_menu_sensitivity, drawable, NULL);

  picman_drawable_calculate_histogram (drawable, l_tool->histogram);
  picman_histogram_view_set_histogram (PICMAN_HISTOGRAM_VIEW (l_tool->histogram_view),
                                     l_tool->histogram);

  if (picman_drawable_get_precision (drawable) == PICMAN_PRECISION_U8)
    {
      scale_factor = 255.0;
      step         = 1.0;
      digits       = 0;
    }
  else
    {
      scale_factor = 100.0;
      step         = 0.1;
      digits       = 1;
    }

  l_tool->ui_scale_factor = scale_factor;

  g_object_freeze_notify (G_OBJECT (l_tool->low_input));
  g_object_freeze_notify (G_OBJECT (l_tool->high_input));
  g_object_freeze_notify (G_OBJECT (l_tool->gamma_linear));
  g_object_freeze_notify (G_OBJECT (l_tool->low_output));
  g_object_freeze_notify (G_OBJECT (l_tool->high_output));

  gtk_adjustment_configure (l_tool->low_input,
                            config->low_input[config->channel] * scale_factor,
                            0, scale_factor, step, 10, 0);

  gtk_adjustment_configure (l_tool->high_input,
                            config->high_input[config->channel] * scale_factor,
                            0, scale_factor, step, 10, 0);

  gtk_adjustment_configure (l_tool->gamma_linear,
                            scale_factor / 2.0,
                            0, scale_factor, 0.1, 1.0, 0);

  gtk_adjustment_configure (l_tool->low_output,
                            config->low_output[config->channel] * scale_factor,
                            0, scale_factor, step, 10, 0);

  gtk_adjustment_configure (l_tool->high_output,
                            config->high_output[config->channel] * scale_factor,
                            0, scale_factor, step, 10, 0);

  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (l_tool->low_input_spinbutton),
                              digits);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (l_tool->high_input_spinbutton),
                              digits);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (l_tool->low_output_spinbutton),
                              digits);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (l_tool->high_output_spinbutton),
                              digits);

  g_object_thaw_notify (G_OBJECT (l_tool->low_input));
  g_object_thaw_notify (G_OBJECT (l_tool->high_input));
  g_object_thaw_notify (G_OBJECT (l_tool->gamma_linear));
  g_object_thaw_notify (G_OBJECT (l_tool->low_output));
  g_object_thaw_notify (G_OBJECT (l_tool->high_output));

  return TRUE;
}

static GeglNode *
picman_levels_tool_get_operation (PicmanImageMapTool  *im_tool,
                                GObject          **config,
                                gchar            **undo_desc)
{
  PicmanLevelsTool *tool = PICMAN_LEVELS_TOOL (im_tool);

  tool->config = g_object_new (PICMAN_TYPE_LEVELS_CONFIG, NULL);

  g_signal_connect_object (tool->config, "notify",
                           G_CALLBACK (picman_levels_tool_config_notify),
                           G_OBJECT (tool), 0);

  *config = G_OBJECT (tool->config);

  return gegl_node_new_child (NULL,
                              "operation", "picman:levels",
                              "config",    tool->config,
                              NULL);
}


/*******************/
/*  Levels dialog  */
/*******************/

static GtkWidget *
picman_levels_tool_color_picker_new (PicmanLevelsTool *tool,
                                   guint           value)
{
  const gchar *stock_id;
  const gchar *help;

  switch (value & 0xF)
    {
    case PICK_LOW_INPUT:
      stock_id = PICMAN_STOCK_COLOR_PICKER_BLACK;
      help     = _("Pick black point");
      break;
    case PICK_GAMMA:
      stock_id = PICMAN_STOCK_COLOR_PICKER_GRAY;
      help     = _("Pick gray point");
      break;
    case PICK_HIGH_INPUT:
      stock_id = PICMAN_STOCK_COLOR_PICKER_WHITE;
      help     = _("Pick white point");
      break;
    default:
      return NULL;
    }

  return picman_image_map_tool_add_color_picker (PICMAN_IMAGE_MAP_TOOL (tool),
                                               GUINT_TO_POINTER (value),
                                               stock_id,
                                               help);
}

static void
picman_levels_tool_dialog (PicmanImageMapTool *image_map_tool)
{
  PicmanLevelsTool   *tool         = PICMAN_LEVELS_TOOL (image_map_tool);
  PicmanToolOptions  *tool_options = PICMAN_TOOL_GET_OPTIONS (image_map_tool);
  PicmanLevelsConfig *config       = tool->config;
  GtkListStore     *store;
  GtkSizeGroup     *label_group;
  GtkWidget        *main_vbox;
  GtkWidget        *vbox;
  GtkWidget        *vbox2;
  GtkWidget        *vbox3;
  GtkWidget        *hbox;
  GtkWidget        *hbox2;
  GtkWidget        *label;
  GtkWidget        *menu;
  GtkWidget        *frame;
  GtkWidget        *hbbox;
  GtkWidget        *button;
  GtkWidget        *spinbutton;
  GtkWidget        *bar;
  GtkObject        *data;
  gint              border;

  g_signal_connect (image_map_tool->settings_box, "file-dialog-setup",
                    G_CALLBACK (picman_levels_tool_export_setup),
                    image_map_tool);

  main_vbox   = picman_image_map_tool_dialog_get_vbox (image_map_tool);
  label_group = picman_image_map_tool_dialog_get_label_group (image_map_tool);

  /*  The option menu for selecting channels  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("Cha_nnel:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  gtk_size_group_add_widget (label_group, label);

  store = picman_enum_store_new_with_range (PICMAN_TYPE_HISTOGRAM_CHANNEL,
                                          PICMAN_HISTOGRAM_VALUE,
                                          PICMAN_HISTOGRAM_ALPHA);
  menu = picman_enum_combo_box_new_with_model (PICMAN_ENUM_STORE (store));
  g_object_unref (store);

  g_signal_connect (menu, "changed",
                    G_CALLBACK (levels_channel_callback),
                    tool);
  picman_enum_combo_box_set_stock_prefix (PICMAN_ENUM_COMBO_BOX (menu),
                                        "picman-channel");
  gtk_box_pack_start (GTK_BOX (hbox), menu, FALSE, FALSE, 0);
  gtk_widget_show (menu);

  tool->channel_menu = menu;

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), menu);

  button = gtk_button_new_with_mnemonic (_("R_eset Channel"));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (levels_channel_reset_callback),
                    tool);

  menu = picman_prop_enum_stock_box_new (G_OBJECT (tool_options),
                                       "histogram-scale", "picman-histogram",
                                       0, 0);
  gtk_box_pack_end (GTK_BOX (hbox), menu, FALSE, FALSE, 0);
  gtk_widget_show (menu);

  /*  Input levels frame  */
  frame = picman_frame_new (_("Input Levels"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox2);
  gtk_widget_show (vbox2);

  tool->histogram_view = picman_histogram_view_new (FALSE);
  gtk_box_pack_start (GTK_BOX (vbox2), tool->histogram_view, TRUE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (tool->histogram_view));

  picman_histogram_options_connect_view (PICMAN_HISTOGRAM_OPTIONS (tool_options),
                                       PICMAN_HISTOGRAM_VIEW (tool->histogram_view));

  g_object_get (tool->histogram_view, "border-width", &border, NULL);

  vbox3 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox3), border);
  gtk_box_pack_start (GTK_BOX (vbox2), vbox3, FALSE, FALSE, 0);
  gtk_widget_show (vbox3);

  tool->input_bar = g_object_new (PICMAN_TYPE_COLOR_BAR, NULL);
  gtk_widget_set_size_request (tool->input_bar, -1, GRADIENT_HEIGHT / 2);
  gtk_box_pack_start (GTK_BOX (vbox3), tool->input_bar, FALSE, FALSE, 0);
  gtk_widget_show (tool->input_bar);

  bar = g_object_new (PICMAN_TYPE_COLOR_BAR, NULL);
  gtk_widget_set_size_request (bar, -1, GRADIENT_HEIGHT / 2);
  gtk_box_pack_start (GTK_BOX (vbox3), bar, FALSE, FALSE, 0);
  gtk_widget_show (bar);

  tool->input_sliders = g_object_new (PICMAN_TYPE_HANDLE_BAR, NULL);
  gtk_widget_set_size_request (tool->input_sliders, -1, CONTROL_HEIGHT);
  gtk_box_pack_start (GTK_BOX (vbox3), tool->input_sliders, FALSE, FALSE, 0);
  gtk_widget_show (tool->input_sliders);

  g_signal_connect_swapped (tool->input_bar, "button-press-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (tool->input_sliders)->button_press_event),
                            tool->input_sliders);

  g_signal_connect_swapped (tool->input_bar, "button-release-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (tool->input_sliders)->button_release_event),
                            tool->input_sliders);

  g_signal_connect_swapped (tool->input_bar, "motion-notify-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (tool->input_sliders)->motion_notify_event),
                            tool->input_sliders);

  g_signal_connect_swapped (bar, "button-press-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (tool->input_sliders)->button_press_event),
                            tool->input_sliders);

  g_signal_connect_swapped (bar, "button-release-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (tool->input_sliders)->button_release_event),
                            tool->input_sliders);

  g_signal_connect_swapped (bar, "motion-notify-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (tool->input_sliders)->motion_notify_event),
                            tool->input_sliders);

  /*  Horizontal box for input levels spinbuttons  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /*  low input spin  */
  hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);
  gtk_widget_show (hbox2);

  button = picman_levels_tool_color_picker_new (tool, PICK_LOW_INPUT);
  gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  spinbutton = picman_spin_button_new (&data,
                                     config->low_input[config->channel] * 255.0,
                                     0, 255, 1, 10, 0, 0.5, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);
  tool->low_input_spinbutton = spinbutton;

  tool->low_input = GTK_ADJUSTMENT (data);
  g_signal_connect (tool->low_input, "value-changed",
                    G_CALLBACK (levels_low_input_changed),
                    tool);

  picman_handle_bar_set_adjustment (PICMAN_HANDLE_BAR (tool->input_sliders), 0,
                                  tool->low_input);

  /*  input gamma spin  */
  spinbutton = picman_spin_button_new (&data,
                                     config->gamma[config->channel],
                                     0.1, 10, 0.01, 0.1, 0, 0.5, 2);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, TRUE, FALSE, 0);
  picman_help_set_help_data (spinbutton, _("Gamma"), NULL);
  gtk_widget_show (spinbutton);

  tool->gamma = GTK_ADJUSTMENT (data);
  g_signal_connect (tool->gamma, "value-changed",
                    G_CALLBACK (levels_gamma_changed),
                    tool);

  tool->gamma_linear = GTK_ADJUSTMENT (gtk_adjustment_new (127, 0, 255,
                                                           0.1, 1.0, 0.0));
  g_signal_connect (tool->gamma_linear, "value-changed",
                    G_CALLBACK (levels_linear_gamma_changed),
                    tool);

  picman_handle_bar_set_adjustment (PICMAN_HANDLE_BAR (tool->input_sliders), 1,
                                  tool->gamma_linear);
  g_object_unref (tool->gamma_linear);

  /*  high input spin  */
  hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_end (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);
  gtk_widget_show (hbox2);

  button = picman_levels_tool_color_picker_new (tool, PICK_HIGH_INPUT);
  gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  spinbutton = picman_spin_button_new (&data,
                                     config->high_input[config->channel] * 255.0,
                                     0, 255, 1, 10, 0, 0.5, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);
  tool->high_input_spinbutton = spinbutton;

  tool->high_input = GTK_ADJUSTMENT (data);
  g_signal_connect (tool->high_input, "value-changed",
                    G_CALLBACK (levels_high_input_changed),
                    tool);

  picman_handle_bar_set_adjustment (PICMAN_HANDLE_BAR (tool->input_sliders), 2,
                                  tool->high_input);

  /*  Output levels frame  */
  frame = picman_frame_new (_("Output Levels"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), border);
  gtk_container_add (GTK_CONTAINER (frame), vbox2);
  gtk_widget_show (vbox2);

  tool->output_bar = g_object_new (PICMAN_TYPE_COLOR_BAR, NULL);
  gtk_widget_set_size_request (tool->output_bar, -1, GRADIENT_HEIGHT);
  gtk_box_pack_start (GTK_BOX (vbox2), tool->output_bar, FALSE, FALSE, 0);
  gtk_widget_show (tool->output_bar);

  tool->output_sliders = g_object_new (PICMAN_TYPE_HANDLE_BAR, NULL);
  gtk_widget_set_size_request (tool->output_sliders, -1, CONTROL_HEIGHT);
  gtk_box_pack_start (GTK_BOX (vbox2), tool->output_sliders, FALSE, FALSE, 0);
  gtk_widget_show (tool->output_sliders);

  g_signal_connect_swapped (tool->output_bar, "button-press-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (tool->output_sliders)->button_press_event),
                            tool->output_sliders);

  g_signal_connect_swapped (tool->output_bar, "button-release-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (tool->output_sliders)->button_release_event),
                            tool->output_sliders);

  g_signal_connect_swapped (tool->output_bar, "motion-notify-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (tool->output_sliders)->motion_notify_event),
                            tool->output_sliders);

  /*  Horizontal box for levels spin widgets  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /*  low output spin  */
  spinbutton = picman_spin_button_new (&data,
                                     config->low_output[config->channel] * 255.0,
                                     0, 255, 1, 10, 0, 0.5, 0);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);
  tool->low_output_spinbutton = spinbutton;

  tool->low_output = GTK_ADJUSTMENT (data);
  g_signal_connect (tool->low_output, "value-changed",
                    G_CALLBACK (levels_low_output_changed),
                    tool);

  picman_handle_bar_set_adjustment (PICMAN_HANDLE_BAR (tool->output_sliders), 0,
                                  tool->low_output);

  /*  high output spin  */
  spinbutton = picman_spin_button_new (&data,
                                     config->high_output[config->channel] * 255.0,
                                     0, 255, 1, 10, 0, 0.5, 0);
  gtk_box_pack_end (GTK_BOX (hbox), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);
  tool->high_output_spinbutton = spinbutton;

  tool->high_output = GTK_ADJUSTMENT (data);
  g_signal_connect (tool->high_output, "value-changed",
                    G_CALLBACK (levels_high_output_changed),
                    tool);

  picman_handle_bar_set_adjustment (PICMAN_HANDLE_BAR (tool->output_sliders), 2,
                                  tool->high_output);


  /*  all channels frame  */
  frame = picman_frame_new (_("All Channels"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_widget_show (hbox);

  hbbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_end (GTK_BOX (hbox), hbbox, FALSE, FALSE, 0);
  gtk_widget_show (hbbox);

  button = gtk_button_new_with_mnemonic (_("_Auto"));
  gtk_box_pack_start (GTK_BOX (hbbox), button, FALSE, FALSE, 0);
  picman_help_set_help_data (button, _("Adjust levels automatically"), NULL);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (levels_stretch_callback),
                    tool);

  button = picman_levels_tool_color_picker_new (tool,
                                              PICK_LOW_INPUT | PICK_ALL_CHANNELS);
  gtk_box_pack_start (GTK_BOX (hbbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = picman_levels_tool_color_picker_new (tool,
                                              PICK_GAMMA | PICK_ALL_CHANNELS);
  gtk_box_pack_start (GTK_BOX (hbbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = picman_levels_tool_color_picker_new (tool,
                                              PICK_HIGH_INPUT | PICK_ALL_CHANNELS);
  gtk_box_pack_start (GTK_BOX (hbbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = picman_stock_button_new (PICMAN_STOCK_TOOL_CURVES,
                                  _("Edit these Settings as Curves"));
  gtk_box_pack_start (GTK_BOX (main_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (levels_to_curves_callback),
                    tool);

  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (tool->channel_menu),
                                 config->channel);
}

static void
picman_levels_tool_reset (PicmanImageMapTool *image_map_tool)
{
  PicmanLevelsTool       *tool    = PICMAN_LEVELS_TOOL (image_map_tool);
  PicmanHistogramChannel  channel = tool->config->channel;

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

  g_object_set (tool->config,
                "channel", channel,
                NULL);

  g_object_thaw_notify (image_map_tool->config);
}

static gboolean
picman_levels_tool_settings_import (PicmanImageMapTool  *image_map_tool,
                                  const gchar       *filename,
                                  GError           **error)
{
  PicmanLevelsTool *tool = PICMAN_LEVELS_TOOL (image_map_tool);
  FILE           *file;
  gchar           header[64];

  file = g_fopen (filename, "rt");

  if (! file)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename),
                   g_strerror (errno));
      return FALSE;
    }

  if (! fgets (header, sizeof (header), file))
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not read header from '%s': %s"),
                   picman_filename_to_utf8 (filename),
                   g_strerror (errno));
      fclose (file);
      return FALSE;
    }

  if (g_str_has_prefix (header, "# PICMAN Levels File\n"))
    {
      gboolean success;

      rewind (file);

      success = picman_levels_config_load_cruft (tool->config, file, error);

      fclose (file);

      return success;
    }

  fclose (file);

  return PICMAN_IMAGE_MAP_TOOL_CLASS (parent_class)->settings_import (image_map_tool,
                                                                    filename,
                                                                    error);
}

static gboolean
picman_levels_tool_settings_export (PicmanImageMapTool  *image_map_tool,
                                  const gchar       *filename,
                                  GError           **error)
{
  PicmanLevelsTool *tool = PICMAN_LEVELS_TOOL (image_map_tool);

  if (tool->export_old_format)
    {
      FILE     *file;
      gboolean  success;

      file = g_fopen (filename, "wt");

      if (! file)
        {
          g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                       _("Could not open '%s' for writing: %s"),
                       picman_filename_to_utf8 (filename),
                       g_strerror (errno));
          return FALSE;
        }

      success = picman_levels_config_save_cruft (tool->config, file, error);

      fclose (file);

      return success;
    }

  return PICMAN_IMAGE_MAP_TOOL_CLASS (parent_class)->settings_export (image_map_tool,
                                                                    filename,
                                                                    error);
}

static void
picman_levels_tool_export_setup (PicmanSettingsBox      *settings_box,
                               GtkFileChooserDialog *dialog,
                               gboolean              export,
                               PicmanLevelsTool       *tool)
{
  GtkWidget *button;

  if (! export)
    return;

  button = gtk_check_button_new_with_mnemonic (_("Use _old levels file format"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                tool->export_old_format);
  gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (dialog), button);
  gtk_widget_show (button);

  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &tool->export_old_format);
}

static void
picman_levels_tool_config_notify (GObject        *object,
                                GParamSpec     *pspec,
                                PicmanLevelsTool *tool)
{
  PicmanLevelsConfig *config       = PICMAN_LEVELS_CONFIG (object);
  gdouble           scale_factor = tool->ui_scale_factor;

  if (! tool->low_input)
    return;

  if (! strcmp (pspec->name, "channel"))
    {
      picman_histogram_view_set_channel (PICMAN_HISTOGRAM_VIEW (tool->histogram_view),
                                       config->channel);
      picman_color_bar_set_channel (PICMAN_COLOR_BAR (tool->output_bar),
                                  config->channel);
      picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (tool->channel_menu),
                                     config->channel);
    }
  else if (! strcmp (pspec->name, "gamma")     ||
           ! strcmp (pspec->name, "low-input") ||
           ! strcmp (pspec->name, "high-input"))
    {
      g_object_freeze_notify (G_OBJECT (tool->low_input));
      g_object_freeze_notify (G_OBJECT (tool->high_input));
      g_object_freeze_notify (G_OBJECT (tool->gamma_linear));

      gtk_adjustment_set_upper (tool->low_input,  scale_factor);
      gtk_adjustment_set_lower (tool->high_input, 0);

      gtk_adjustment_set_lower (tool->gamma_linear, 0);
      gtk_adjustment_set_upper (tool->gamma_linear, scale_factor);

      gtk_adjustment_set_value (tool->low_input,
                                config->low_input[config->channel]  *
                                scale_factor);
      gtk_adjustment_set_value (tool->gamma,
                                config->gamma[config->channel]);
      gtk_adjustment_set_value (tool->high_input,
                                config->high_input[config->channel] *
                                scale_factor);

      gtk_adjustment_set_upper (tool->low_input,
                                gtk_adjustment_get_value (tool->high_input));
      gtk_adjustment_set_lower (tool->high_input,
                                gtk_adjustment_get_value (tool->low_input));

      gtk_adjustment_set_lower (tool->gamma_linear,
                                gtk_adjustment_get_value (tool->low_input));
      gtk_adjustment_set_upper (tool->gamma_linear,
                                gtk_adjustment_get_value (tool->high_input));

      g_object_thaw_notify (G_OBJECT (tool->low_input));
      g_object_thaw_notify (G_OBJECT (tool->high_input));
      g_object_thaw_notify (G_OBJECT (tool->gamma_linear));

      levels_update_input_bar (tool);
    }
  else if (! strcmp (pspec->name, "low-output"))
    {
      gtk_adjustment_set_value (tool->low_output,
                                config->low_output[config->channel] *
                                scale_factor);
    }
  else if (! strcmp (pspec->name, "high-output"))
    {
      gtk_adjustment_set_value (tool->high_output,
                                config->high_output[config->channel] *
                                scale_factor);
    }
}

static void
levels_update_input_bar (PicmanLevelsTool *tool)
{
  PicmanLevelsConfig *config = tool->config;

  switch (config->channel)
    {
      gdouble value;

    case PICMAN_HISTOGRAM_VALUE:
    case PICMAN_HISTOGRAM_ALPHA:
    case PICMAN_HISTOGRAM_RGB:
      {
        guchar v[256];
        gint   i;

        for (i = 0; i < 256; i++)
          {
            value = picman_operation_levels_map_input (config,
                                                     config->channel,
                                                     i / 255.0);
            v[i] = CLAMP (value, 0.0, 1.0) * 255.999;
          }

        picman_color_bar_set_buffers (PICMAN_COLOR_BAR (tool->input_bar),
                                    v, v, v);
      }
      break;

    case PICMAN_HISTOGRAM_RED:
    case PICMAN_HISTOGRAM_GREEN:
    case PICMAN_HISTOGRAM_BLUE:
      {
        guchar r[256];
        guchar g[256];
        guchar b[256];
        gint   i;

        for (i = 0; i < 256; i++)
          {
            value = picman_operation_levels_map_input (config,
                                                     PICMAN_HISTOGRAM_RED,
                                                     i / 255.0);
            r[i] = CLAMP (value, 0.0, 1.0) * 255.999;

            value = picman_operation_levels_map_input (config,
                                                     PICMAN_HISTOGRAM_GREEN,
                                                     i / 255.0);
            g[i] = CLAMP (value, 0.0, 1.0) * 255.999;

            value = picman_operation_levels_map_input (config,
                                                     PICMAN_HISTOGRAM_BLUE,
                                                     i / 255.0);
            b[i] = CLAMP (value, 0.0, 1.0) * 255.999;
          }

        picman_color_bar_set_buffers (PICMAN_COLOR_BAR (tool->input_bar),
                                    r, g, b);
      }
      break;
    }
}

static void
levels_channel_callback (GtkWidget      *widget,
                         PicmanLevelsTool *tool)
{
  gint value;

  if (picman_int_combo_box_get_active (PICMAN_INT_COMBO_BOX (widget), &value) &&
      tool->config->channel != value)
    {
      g_object_set (tool->config,
                    "channel", value,
                    NULL);
    }
}

static void
levels_channel_reset_callback (GtkWidget      *widget,
                               PicmanLevelsTool *tool)
{
  picman_levels_config_reset_channel (tool->config);
}

static gboolean
levels_menu_sensitivity (gint      value,
                         gpointer  data)
{
  PicmanDrawable         *drawable = PICMAN_DRAWABLE (data);
  PicmanHistogramChannel  channel  = value;

  switch (channel)
    {
    case PICMAN_HISTOGRAM_VALUE:
      return TRUE;

    case PICMAN_HISTOGRAM_RED:
    case PICMAN_HISTOGRAM_GREEN:
    case PICMAN_HISTOGRAM_BLUE:
      return picman_drawable_is_rgb (drawable);

    case PICMAN_HISTOGRAM_ALPHA:
      return picman_drawable_has_alpha (drawable);

    case PICMAN_HISTOGRAM_RGB:
      return FALSE;
    }

  return FALSE;
}

static void
levels_stretch_callback (GtkWidget      *widget,
                         PicmanLevelsTool *tool)
{
  PicmanDrawable *drawable = PICMAN_IMAGE_MAP_TOOL (tool)->drawable;

  picman_levels_config_stretch (tool->config, tool->histogram,
                              picman_drawable_is_rgb (drawable));
}

static void
levels_linear_gamma_update (PicmanLevelsTool *tool)
{
  gdouble low_input  = gtk_adjustment_get_value (tool->low_input);
  gdouble high_input = gtk_adjustment_get_value (tool->high_input);
  gdouble delta, mid, tmp, value;

  delta = (high_input - low_input) / 2.0;
  mid   = low_input + delta;
  tmp   = log10 (1.0 / tool->config->gamma[tool->config->channel]);
  value = mid + delta * tmp;

  gtk_adjustment_set_value (tool->gamma_linear, value);
}

static void
levels_linear_gamma_changed (GtkAdjustment  *adjustment,
                             PicmanLevelsTool *tool)
{
  gdouble low_input  = gtk_adjustment_get_value (tool->low_input);
  gdouble high_input = gtk_adjustment_get_value (tool->high_input);
  gdouble delta, mid, tmp, value;

  delta = (high_input - low_input) / 2.0;

  if (delta >= 0.5)
    {
      mid   = low_input + delta;
      tmp   = (gtk_adjustment_get_value (adjustment) - mid) / delta;
      value = 1.0 / pow (10, tmp);

      /*  round the gamma value to the nearest 1/100th  */
      value = floor (value * 100 + 0.5) / 100.0;

      gtk_adjustment_set_value (tool->gamma, value);
    }
}

static void
levels_low_input_changed (GtkAdjustment  *adjustment,
                          PicmanLevelsTool *tool)
{
  PicmanLevelsConfig *config = tool->config;
  gint              value  = ROUND (gtk_adjustment_get_value (adjustment));

  gtk_adjustment_set_lower (tool->high_input, value);
  gtk_adjustment_set_lower (tool->gamma_linear, value);

  if (config->low_input[config->channel] != value / tool->ui_scale_factor)
    {
      g_object_set (config,
                    "low-input", value / tool->ui_scale_factor,
                    NULL);
    }

  levels_linear_gamma_update (tool);
}

static void
levels_gamma_changed (GtkAdjustment  *adjustment,
                      PicmanLevelsTool *tool)
{
  PicmanLevelsConfig *config = tool->config;
  gdouble           value  = gtk_adjustment_get_value (adjustment);

  if (config->gamma[config->channel] != value)
    {
      g_object_set (config,
                    "gamma", value,
                    NULL);
    }

  levels_linear_gamma_update (tool);
}

static void
levels_high_input_changed (GtkAdjustment  *adjustment,
                           PicmanLevelsTool *tool)
{
  PicmanLevelsConfig *config = tool->config;
  gint              value  = ROUND (gtk_adjustment_get_value (adjustment));

  gtk_adjustment_set_upper (tool->low_input, value);
  gtk_adjustment_set_upper (tool->gamma_linear, value);

  if (config->high_input[config->channel] != value / tool->ui_scale_factor)
    {
      g_object_set (config,
                    "high-input", value / tool->ui_scale_factor,
                    NULL);
    }

  levels_linear_gamma_update (tool);
}

static void
levels_low_output_changed (GtkAdjustment  *adjustment,
                           PicmanLevelsTool *tool)
{
  PicmanLevelsConfig *config = tool->config;
  gint              value  = ROUND (gtk_adjustment_get_value (adjustment));

  if (config->low_output[config->channel] != value / tool->ui_scale_factor)
    {
      g_object_set (config,
                    "low-output", value / tool->ui_scale_factor,
                    NULL);
    }
}

static void
levels_high_output_changed (GtkAdjustment  *adjustment,
                            PicmanLevelsTool *tool)
{
  PicmanLevelsConfig *config = tool->config;
  gint              value  = ROUND (gtk_adjustment_get_value (adjustment));

  if (config->high_output[config->channel] != value / tool->ui_scale_factor)
    {
      g_object_set (config,
                    "high-output", value / tool->ui_scale_factor,
                    NULL);
    }
}

static void
levels_input_adjust_by_color (PicmanLevelsConfig     *config,
                              guint                 value,
                              PicmanHistogramChannel  channel,
                              const PicmanRGB        *color)
{
  switch (value & 0xF)
    {
    case PICK_LOW_INPUT:
      picman_levels_config_adjust_by_colors (config, channel, color, NULL, NULL);
      break;
    case PICK_GAMMA:
      picman_levels_config_adjust_by_colors (config, channel, NULL, color, NULL);
      break;
    case PICK_HIGH_INPUT:
      picman_levels_config_adjust_by_colors (config, channel, NULL, NULL, color);
      break;
    default:
      break;
    }
}

static void
picman_levels_tool_color_picked (PicmanImageMapTool *color_tool,
                               gpointer          identifier,
                               const Babl       *sample_format,
                               const PicmanRGB    *color)
{
  PicmanLevelsTool *tool  = PICMAN_LEVELS_TOOL (color_tool);
  guint           value = GPOINTER_TO_UINT (identifier);

  if (value & PICK_ALL_CHANNELS &&
      picman_babl_format_get_base_type (sample_format) == PICMAN_RGB)
    {
      PicmanHistogramChannel  channel;

      /*  first reset the value channel  */
      switch (value & 0xF)
        {
        case PICK_LOW_INPUT:
          tool->config->low_input[PICMAN_HISTOGRAM_VALUE] = 0.0;
          break;
        case PICK_GAMMA:
          tool->config->gamma[PICMAN_HISTOGRAM_VALUE] = 1.0;
          break;
        case PICK_HIGH_INPUT:
          tool->config->high_input[PICMAN_HISTOGRAM_VALUE] = 1.0;
          break;
        default:
          break;
        }

      /*  then adjust all color channels  */
      for (channel = PICMAN_HISTOGRAM_RED;
           channel <= PICMAN_HISTOGRAM_BLUE;
           channel++)
        {
          levels_input_adjust_by_color (tool->config,
                                        value, channel, color);
        }
    }
  else
    {
      levels_input_adjust_by_color (tool->config,
                                    value, tool->config->channel, color);
    }
}

static void
levels_to_curves_callback (GtkWidget      *widget,
                           PicmanLevelsTool *tool)
{
  PicmanCurvesConfig *curves;

  curves = picman_levels_config_to_curves_config (tool->config);

  picman_image_map_tool_edit_as (PICMAN_IMAGE_MAP_TOOL (tool),
                               "picman-curves-tool",
                               PICMAN_CONFIG (curves));

  g_object_unref (curves);
}
