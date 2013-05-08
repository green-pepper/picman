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
#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "operations/picmancurvesconfig.h"
#include "operations/picmanoperationcurves.h"

#include "core/picman.h"
#include "core/picmancurve.h"
#include "core/picmancurve-map.h"
#include "core/picmandrawable.h"
#include "core/picmandrawable-histogram.h"
#include "core/picmanerror.h"
#include "core/picmanhistogram.h"
#include "core/picmanimage.h"

#include "widgets/picmancolorbar.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmancurveview.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"

#include "picmancurvestool.h"
#include "picmanhistogramoptions.h"

#include "picman-intl.h"


#define GRAPH_SIZE 256
#define BAR_SIZE    12
#define RADIUS       4


/*  local function prototypes  */

static gboolean   picman_curves_tool_initialize     (PicmanTool             *tool,
                                                   PicmanDisplay          *display,
                                                   GError              **error);
static void       picman_curves_tool_button_release (PicmanTool             *tool,
                                                   const PicmanCoords     *coords,
                                                   guint32               time,
                                                   GdkModifierType       state,
                                                   PicmanButtonReleaseType release_type,
                                                   PicmanDisplay          *display);
static gboolean   picman_curves_tool_key_press      (PicmanTool             *tool,
                                                   GdkEventKey          *kevent,
                                                   PicmanDisplay          *display);
static void       picman_curves_tool_oper_update    (PicmanTool             *tool,
                                                   const PicmanCoords     *coords,
                                                   GdkModifierType       state,
                                                   gboolean              proximity,
                                                   PicmanDisplay          *display);

static void       picman_curves_tool_color_picked   (PicmanColorTool        *color_tool,
                                                   PicmanColorPickState    pick_state,
                                                   const Babl           *sample_format,
                                                   const PicmanRGB        *color,
                                                   gint                  color_index);
static GeglNode * picman_curves_tool_get_operation  (PicmanImageMapTool     *image_map_tool,
                                                   GObject             **config,
                                                   gchar               **undo_desc);
static void       picman_curves_tool_dialog         (PicmanImageMapTool     *image_map_tool);
static void       picman_curves_tool_reset          (PicmanImageMapTool     *image_map_tool);
static gboolean   picman_curves_tool_settings_import(PicmanImageMapTool     *image_map_tool,
                                                   const gchar          *filename,
                                                   GError              **error);
static gboolean   picman_curves_tool_settings_export(PicmanImageMapTool     *image_map_tool,
                                                   const gchar          *filename,
                                                   GError              **error);

static void       picman_curves_tool_export_setup   (PicmanSettingsBox      *settings_box,
                                                   GtkFileChooserDialog *dialog,
                                                   gboolean              export,
                                                   PicmanCurvesTool       *tool);
static void       picman_curves_tool_config_notify  (GObject              *object,
                                                   GParamSpec           *pspec,
                                                   PicmanCurvesTool       *tool);

static void       curves_channel_callback         (GtkWidget            *widget,
                                                   PicmanCurvesTool       *tool);
static void       curves_channel_reset_callback   (GtkWidget            *widget,
                                                   PicmanCurvesTool       *tool);

static gboolean   curves_menu_sensitivity         (gint                  value,
                                                   gpointer              data);

static void       curves_curve_type_callback      (GtkWidget            *widget,
                                                   PicmanCurvesTool       *tool);


G_DEFINE_TYPE (PicmanCurvesTool, picman_curves_tool, PICMAN_TYPE_IMAGE_MAP_TOOL)

#define parent_class picman_curves_tool_parent_class

static PicmanRGB channel_colors[PICMAN_HISTOGRAM_RGB] =
{
  { 0.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0, 1.0 },
  { 0.0, 1.0, 0.0, 1.0 },
  { 0.0, 0.0, 1.0, 1.0 },
  { 0.5, 0.5, 0.5, 1.0 }
};


/*  public functions  */

void
picman_curves_tool_register (PicmanToolRegisterCallback  callback,
                           gpointer                  data)
{
  (* callback) (PICMAN_TYPE_CURVES_TOOL,
                PICMAN_TYPE_HISTOGRAM_OPTIONS,
                picman_color_options_gui,
                0,
                "picman-curves-tool",
                _("Curves"),
                _("Curves Tool: Adjust color curves"),
                N_("_Curves..."), NULL,
                NULL, PICMAN_HELP_TOOL_CURVES,
                PICMAN_STOCK_TOOL_CURVES,
                data);
}


/*  private functions  */

static void
picman_curves_tool_class_init (PicmanCurvesToolClass *klass)
{
  PicmanToolClass         *tool_class       = PICMAN_TOOL_CLASS (klass);
  PicmanColorToolClass    *color_tool_class = PICMAN_COLOR_TOOL_CLASS (klass);
  PicmanImageMapToolClass *im_tool_class    = PICMAN_IMAGE_MAP_TOOL_CLASS (klass);

  tool_class->initialize             = picman_curves_tool_initialize;
  tool_class->button_release         = picman_curves_tool_button_release;
  tool_class->key_press              = picman_curves_tool_key_press;
  tool_class->oper_update            = picman_curves_tool_oper_update;

  color_tool_class->picked           = picman_curves_tool_color_picked;

  im_tool_class->dialog_desc         = _("Adjust Color Curves");
  im_tool_class->settings_name       = "curves";
  im_tool_class->import_dialog_title = _("Import Curves");
  im_tool_class->export_dialog_title = _("Export Curves");

  im_tool_class->get_operation       = picman_curves_tool_get_operation;
  im_tool_class->dialog              = picman_curves_tool_dialog;
  im_tool_class->reset               = picman_curves_tool_reset;
  im_tool_class->settings_import     = picman_curves_tool_settings_import;
  im_tool_class->settings_export     = picman_curves_tool_settings_export;
}

static void
picman_curves_tool_init (PicmanCurvesTool *tool)
{
  gint i;

  for (i = 0; i < G_N_ELEMENTS (tool->picked_color); i++)
    tool->picked_color[i] = -1.0;
}

static gboolean
picman_curves_tool_initialize (PicmanTool     *tool,
                             PicmanDisplay  *display,
                             GError      **error)
{
  PicmanCurvesTool *c_tool   = PICMAN_CURVES_TOOL (tool);
  PicmanImage      *image    = picman_display_get_image (display);
  PicmanDrawable   *drawable = picman_image_get_active_drawable (image);
  PicmanHistogram  *histogram;

  if (! PICMAN_TOOL_CLASS (parent_class)->initialize (tool, display, error))
    {
      return FALSE;
    }

  /*  always pick colors  */
  picman_color_tool_enable (PICMAN_COLOR_TOOL (tool),
                          PICMAN_COLOR_TOOL_GET_OPTIONS (tool));

  picman_int_combo_box_set_sensitivity (PICMAN_INT_COMBO_BOX (c_tool->channel_menu),
                                      curves_menu_sensitivity, drawable, NULL);

  histogram = picman_histogram_new ();
  picman_drawable_calculate_histogram (drawable, histogram);
  picman_histogram_view_set_background (PICMAN_HISTOGRAM_VIEW (c_tool->graph),
                                      histogram);
  picman_histogram_unref (histogram);

  return TRUE;
}

static void
picman_curves_tool_button_release (PicmanTool              *tool,
                                 const PicmanCoords      *coords,
                                 guint32                time,
                                 GdkModifierType        state,
                                 PicmanButtonReleaseType  release_type,
                                 PicmanDisplay           *display)
{
  PicmanCurvesTool   *c_tool = PICMAN_CURVES_TOOL (tool);
  PicmanCurvesConfig *config = c_tool->config;

  if (state & GDK_SHIFT_MASK)
    {
      PicmanCurve *curve = config->curve[config->channel];
      gdouble    value = c_tool->picked_color[config->channel];
      gint       closest;

      closest = picman_curve_get_closest_point (curve, value);

      picman_curve_view_set_selected (PICMAN_CURVE_VIEW (c_tool->graph),
                                    closest);

      picman_curve_set_point (curve, closest,
                            value, picman_curve_map_value (curve, value));
    }
  else if (state & picman_get_toggle_behavior_mask ())
    {
      gint i;

      for (i = 0; i < 5; i++)
        {
          PicmanCurve *curve = config->curve[i];
          gdouble    value = c_tool->picked_color[i];
          gint       closest;

          closest = picman_curve_get_closest_point (curve, value);

          picman_curve_view_set_selected (PICMAN_CURVE_VIEW (c_tool->graph),
                                        closest);

          picman_curve_set_point (curve, closest,
                                value, picman_curve_map_value (curve, value));
        }
    }

  /*  chain up to halt the tool */
  PICMAN_TOOL_CLASS (parent_class)->button_release (tool, coords, time, state,
                                                  release_type, display);
}

static gboolean
picman_curves_tool_key_press (PicmanTool    *tool,
                            GdkEventKey *kevent,
                            PicmanDisplay *display)
{
  PicmanCurvesTool *c_tool = PICMAN_CURVES_TOOL (tool);

  if (tool->display && c_tool->graph)
    {
      if (gtk_widget_event (c_tool->graph, (GdkEvent *) kevent))
        return TRUE;
    }

  return PICMAN_TOOL_CLASS (parent_class)->key_press (tool, kevent, display);
}

static void
picman_curves_tool_oper_update (PicmanTool         *tool,
                              const PicmanCoords *coords,
                              GdkModifierType   state,
                              gboolean          proximity,
                              PicmanDisplay      *display)
{
  PicmanColorPickMode  mode;
  const gchar       *status;

  PICMAN_TOOL_CLASS (parent_class)->oper_update (tool, coords, state, proximity,
                                               display);

  picman_tool_pop_status (tool, display);

  if (state & GDK_SHIFT_MASK)
    {
      mode   = PICMAN_COLOR_PICK_MODE_PALETTE;
      status = _("Click to add a control point");
    }
  else if (state & picman_get_toggle_behavior_mask ())
    {
      mode   = PICMAN_COLOR_PICK_MODE_PALETTE;
      status = _("Click to add control points to all channels");
    }
  else
    {
      mode   = PICMAN_COLOR_PICK_MODE_NONE;
      status = _("Click to locate on curve (try Shift, Ctrl)");
    }

  PICMAN_COLOR_TOOL (tool)->pick_mode = mode;

  if (proximity)
    picman_tool_push_status (tool, display, "%s", status);
}

static void
picman_curves_tool_color_picked (PicmanColorTool      *color_tool,
                               PicmanColorPickState  pick_state,
                               const Babl         *sample_format,
                               const PicmanRGB      *color,
                               gint                color_index)
{
  PicmanCurvesTool *tool = PICMAN_CURVES_TOOL (color_tool);
  PicmanDrawable   *drawable;

  drawable = PICMAN_IMAGE_MAP_TOOL (tool)->drawable;

  tool->picked_color[PICMAN_HISTOGRAM_RED]   = color->r;
  tool->picked_color[PICMAN_HISTOGRAM_GREEN] = color->g;
  tool->picked_color[PICMAN_HISTOGRAM_BLUE]  = color->b;

  if (picman_drawable_has_alpha (drawable))
    tool->picked_color[PICMAN_HISTOGRAM_ALPHA] = color->a;

  tool->picked_color[PICMAN_HISTOGRAM_VALUE] = MAX (MAX (color->r, color->g),
                                                  color->b);

  picman_curve_view_set_xpos (PICMAN_CURVE_VIEW (tool->graph),
                            tool->picked_color[tool->config->channel]);
}

static GeglNode *
picman_curves_tool_get_operation (PicmanImageMapTool  *image_map_tool,
                                GObject          **config,
                                gchar            **undo_desc)
{
  PicmanCurvesTool *tool = PICMAN_CURVES_TOOL (image_map_tool);

  tool->config = g_object_new (PICMAN_TYPE_CURVES_CONFIG, NULL);

  g_signal_connect_object (tool->config, "notify",
                           G_CALLBACK (picman_curves_tool_config_notify),
                           tool, 0);

  *config = G_OBJECT (tool->config);

  return gegl_node_new_child (NULL,
                              "operation", "picman:curves",
                              "config",    tool->config,
                              NULL);
}


/*******************/
/*  Curves dialog  */
/*******************/

static void
picman_curves_tool_dialog (PicmanImageMapTool *image_map_tool)
{
  PicmanCurvesTool   *tool         = PICMAN_CURVES_TOOL (image_map_tool);
  PicmanToolOptions  *tool_options = PICMAN_TOOL_GET_OPTIONS (image_map_tool);
  PicmanCurvesConfig *config       = tool->config;
  GtkListStore     *store;
  GtkSizeGroup     *label_group;
  GtkWidget        *main_vbox;
  GtkWidget        *vbox;
  GtkWidget        *hbox;
  GtkWidget        *hbox2;
  GtkWidget        *label;
  GtkWidget        *frame;
  GtkWidget        *table;
  GtkWidget        *button;
  GtkWidget        *bar;
  GtkWidget        *combo;

  g_signal_connect (image_map_tool->settings_box, "file-dialog-setup",
                    G_CALLBACK (picman_curves_tool_export_setup),
                    image_map_tool);

  main_vbox   = picman_image_map_tool_dialog_get_vbox (image_map_tool);
  label_group = picman_image_map_tool_dialog_get_label_group (image_map_tool);

  /*  The combo box for selecting channels  */
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
  tool->channel_menu =
    picman_enum_combo_box_new_with_model (PICMAN_ENUM_STORE (store));
  g_object_unref (store);

  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (tool->channel_menu),
                                 config->channel);
  picman_enum_combo_box_set_stock_prefix (PICMAN_ENUM_COMBO_BOX (tool->channel_menu),
                                        "picman-channel");
  gtk_box_pack_start (GTK_BOX (hbox), tool->channel_menu, FALSE, FALSE, 0);
  gtk_widget_show (tool->channel_menu);

  g_signal_connect (tool->channel_menu, "changed",
                    G_CALLBACK (curves_channel_callback),
                    tool);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), tool->channel_menu);

  button = gtk_button_new_with_mnemonic (_("R_eset Channel"));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (curves_channel_reset_callback),
                    tool);

  /*  The histogram scale radio buttons  */
  hbox2 = picman_prop_enum_stock_box_new (G_OBJECT (tool_options),
                                        "histogram-scale", "picman-histogram",
                                        0, 0);
  gtk_box_pack_end (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);
  gtk_widget_show (hbox2);

  /*  The table for the color bars and the graph  */
  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, TRUE, TRUE, 0);

  /*  The left color bar  */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_table_attach (GTK_TABLE (table), vbox, 0, 1, 0, 1,
                    GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_widget_show (vbox);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, RADIUS);
  gtk_widget_show (frame);

  tool->yrange = picman_color_bar_new (GTK_ORIENTATION_VERTICAL);
  gtk_widget_set_size_request (tool->yrange, BAR_SIZE, -1);
  gtk_container_add (GTK_CONTAINER (frame), tool->yrange);
  gtk_widget_show (tool->yrange);

  /*  The curves graph  */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 0, 1,
                    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
  gtk_widget_show (frame);

  tool->graph = picman_curve_view_new ();
  picman_curve_view_set_range_x (PICMAN_CURVE_VIEW (tool->graph), 0, 255);
  picman_curve_view_set_range_y (PICMAN_CURVE_VIEW (tool->graph), 0, 255);
  gtk_widget_set_size_request (tool->graph,
                               GRAPH_SIZE + RADIUS * 2,
                               GRAPH_SIZE + RADIUS * 2);
  g_object_set (tool->graph,
                "border-width", RADIUS,
                "subdivisions", 1,
                NULL);
  picman_curve_view_set_curve (PICMAN_CURVE_VIEW (tool->graph),
                             config->curve[config->channel],
                             &channel_colors[config->channel]);
  gtk_container_add (GTK_CONTAINER (frame), tool->graph);
  gtk_widget_show (tool->graph);

  picman_histogram_options_connect_view (PICMAN_HISTOGRAM_OPTIONS (tool_options),
                                       PICMAN_HISTOGRAM_VIEW (tool->graph));

  /*  The bottom color bar  */
  hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_table_attach (GTK_TABLE (table), hbox2, 1, 2, 1, 2,
                    GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (hbox2);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox2), frame, TRUE, TRUE, RADIUS);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_set_homogeneous (GTK_BOX (vbox), TRUE);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  tool->xrange = picman_color_bar_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_set_size_request (tool->xrange, -1, BAR_SIZE / 2);
  gtk_box_pack_start (GTK_BOX (vbox), tool->xrange, TRUE, TRUE, 0);
  gtk_widget_show (tool->xrange);

  bar = picman_color_bar_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (vbox), bar, TRUE, TRUE, 0);
  gtk_widget_show (bar);

  gtk_widget_show (table);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_end (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("Curve _type:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  tool->curve_type = combo = picman_enum_combo_box_new (PICMAN_TYPE_CURVE_TYPE);
  picman_enum_combo_box_set_stock_prefix (PICMAN_ENUM_COMBO_BOX (combo),
                                        "picman-curve");
  picman_int_combo_box_connect (PICMAN_INT_COMBO_BOX (combo), 0,
                              G_CALLBACK (curves_curve_type_callback),
                              tool);
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
  gtk_widget_show (combo);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);
}

static void
picman_curves_tool_reset (PicmanImageMapTool *image_map_tool)
{
  PicmanCurvesTool       *tool = PICMAN_CURVES_TOOL (image_map_tool);
  PicmanCurvesConfig     *default_config;
  PicmanHistogramChannel  channel;

  default_config = PICMAN_CURVES_CONFIG (image_map_tool->default_config);

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      if (default_config)
        {
          PicmanCurveType curve_type = tool->config->curve[channel]->curve_type;

          g_object_freeze_notify (G_OBJECT (tool->config->curve[channel]));

          picman_config_copy (PICMAN_CONFIG (default_config->curve[channel]),
                            PICMAN_CONFIG (tool->config->curve[channel]),
                            0);

          g_object_set (tool->config->curve[channel],
                        "curve-type", curve_type,
                        NULL);

          g_object_thaw_notify (G_OBJECT (tool->config->curve[channel]));
        }
      else
        {
          picman_curve_reset (tool->config->curve[channel], FALSE);
        }
    }
}

static gboolean
picman_curves_tool_settings_import (PicmanImageMapTool  *image_map_tool,
                                  const gchar       *filename,
                                  GError           **error)
{
  PicmanCurvesTool *tool = PICMAN_CURVES_TOOL (image_map_tool);
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

  if (g_str_has_prefix (header, "# PICMAN Curves File\n"))
    {
      gboolean success;

      rewind (file);

      success = picman_curves_config_load_cruft (tool->config, file, error);

      fclose (file);

      return success;
    }

  fclose (file);

  return PICMAN_IMAGE_MAP_TOOL_CLASS (parent_class)->settings_import (image_map_tool,
                                                                    filename,
                                                                    error);
}

static gboolean
picman_curves_tool_settings_export (PicmanImageMapTool  *image_map_tool,
                                  const gchar       *filename,
                                  GError           **error)
{
  PicmanCurvesTool *tool = PICMAN_CURVES_TOOL (image_map_tool);

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

      success = picman_curves_config_save_cruft (tool->config, file, error);

      fclose (file);

      return success;
    }

  return PICMAN_IMAGE_MAP_TOOL_CLASS (parent_class)->settings_export (image_map_tool,
                                                                    filename,
                                                                    error);
}

static void
picman_curves_tool_export_setup (PicmanSettingsBox      *settings_box,
                               GtkFileChooserDialog *dialog,
                               gboolean              export,
                               PicmanCurvesTool       *tool)
{
  GtkWidget *button;

  if (! export)
    return;

  button = gtk_check_button_new_with_mnemonic (_("Use _old curves file format"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                tool->export_old_format);
  gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (dialog), button);
  gtk_widget_show (button);

  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &tool->export_old_format);
}

static void
picman_curves_tool_config_notify (GObject        *object,
                                GParamSpec     *pspec,
                                PicmanCurvesTool *tool)
{
  PicmanCurvesConfig *config = PICMAN_CURVES_CONFIG (object);
  PicmanCurve        *curve  = config->curve[config->channel];

  if (! tool->xrange)
    return;

  if (! strcmp (pspec->name, "channel"))
    {
      PicmanHistogramChannel channel;

      picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (tool->channel_menu),
                                     config->channel);

      switch (config->channel)
        {
          guchar r[256];
          guchar g[256];
          guchar b[256];

        case PICMAN_HISTOGRAM_VALUE:
        case PICMAN_HISTOGRAM_ALPHA:
        case PICMAN_HISTOGRAM_RGB:
          picman_curve_get_uchar (curve, sizeof (r), r);

          picman_color_bar_set_buffers (PICMAN_COLOR_BAR (tool->xrange),
                                      r, r, r);
          break;

        case PICMAN_HISTOGRAM_RED:
        case PICMAN_HISTOGRAM_GREEN:
        case PICMAN_HISTOGRAM_BLUE:
          picman_curve_get_uchar (config->curve[PICMAN_HISTOGRAM_RED],
                                sizeof (r), r);
          picman_curve_get_uchar (config->curve[PICMAN_HISTOGRAM_GREEN],
                                sizeof (g), g);
          picman_curve_get_uchar (config->curve[PICMAN_HISTOGRAM_BLUE],
                                sizeof (b), b);

          picman_color_bar_set_buffers (PICMAN_COLOR_BAR (tool->xrange),
                                      r, g, b);
          break;
        }

      picman_histogram_view_set_channel (PICMAN_HISTOGRAM_VIEW (tool->graph),
                                       config->channel);
      picman_curve_view_set_xpos (PICMAN_CURVE_VIEW (tool->graph),
                                tool->picked_color[config->channel]);

      picman_color_bar_set_channel (PICMAN_COLOR_BAR (tool->yrange),
                                  config->channel);

      picman_curve_view_remove_all_backgrounds (PICMAN_CURVE_VIEW (tool->graph));

      for (channel = PICMAN_HISTOGRAM_VALUE;
           channel <= PICMAN_HISTOGRAM_ALPHA;
           channel++)
        {
          if (channel == config->channel)
            {
              picman_curve_view_set_curve (PICMAN_CURVE_VIEW (tool->graph), curve,
                                         &channel_colors[channel]);
            }
          else
            {
              picman_curve_view_add_background (PICMAN_CURVE_VIEW (tool->graph),
                                              config->curve[channel],
                                              &channel_colors[channel]);
            }
        }

      picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (tool->curve_type),
                                     curve->curve_type);
    }
  else if (! strcmp (pspec->name, "curve"))
    {
      picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (tool->curve_type),
                                     curve->curve_type);
    }
}

static void
curves_channel_callback (GtkWidget      *widget,
                         PicmanCurvesTool *tool)
{
  PicmanCurvesConfig *config = tool->config;
  gint              value;

  if (picman_int_combo_box_get_active (PICMAN_INT_COMBO_BOX (widget), &value) &&
      config->channel != value)
    {
      g_object_set (config,
                    "channel", value,
                    NULL);
    }
}

static void
curves_channel_reset_callback (GtkWidget      *widget,
                               PicmanCurvesTool *tool)
{
  picman_curve_reset (tool->config->curve[tool->config->channel], FALSE);
}

static gboolean
curves_menu_sensitivity (gint      value,
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
curves_curve_type_callback (GtkWidget      *widget,
                            PicmanCurvesTool *tool)
{
  gint value;

  if (picman_int_combo_box_get_active (PICMAN_INT_COMBO_BOX (widget), &value))
    {
      PicmanCurvesConfig *config     = tool->config;
      PicmanCurveType     curve_type = value;

      if (config->curve[config->channel]->curve_type != curve_type)
        picman_curve_set_curve_type (config->curve[config->channel], curve_type);
    }
}
