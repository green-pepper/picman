/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picmanbrush.h"
#include "core/picmantempbuf.h"
#include "core/picmantoolinfo.h"

#include "paint/picmanpaintoptions.h"

#include "widgets/picmanpropwidgets.h"
#include "widgets/picmanspinscale.h"
#include "widgets/picmanviewablebox.h"
#include "widgets/picmanwidgets-constructors.h"
#include "widgets/picmanwidgets-utils.h"

#include "picmanairbrushtool.h"
#include "picmanclonetool.h"
#include "picmanconvolvetool.h"
#include "picmandodgeburntool.h"
#include "picmanerasertool.h"
#include "picmanhealtool.h"
#include "picmaninktool.h"
#include "picmanpaintoptions-gui.h"
#include "picmanpenciltool.h"
#include "picmanperspectiveclonetool.h"
#include "picmansmudgetool.h"
#include "picmantooloptions-gui.h"

#include "picman-intl.h"


static void picman_paint_options_gui_reset_size  (GtkWidget        *button,
                                                PicmanPaintOptions *paint_options);
static void picman_paint_options_gui_reset_aspect_ratio
                                               (GtkWidget        *button,
                                                PicmanPaintOptions *paint_options);
static void picman_paint_options_gui_reset_angle (GtkWidget        *button,
                                                PicmanPaintOptions *paint_options);

static GtkWidget * dynamics_options_gui        (PicmanPaintOptions *paint_options,
                                                GType             tool_type);
static GtkWidget * jitter_options_gui          (PicmanPaintOptions *paint_options,
                                                GType             tool_type);
static GtkWidget * smoothing_options_gui       (PicmanPaintOptions *paint_options,
                                                GType             tool_type);


/*  public functions  */

GtkWidget *
picman_paint_options_gui (PicmanToolOptions *tool_options)
{
  GObject          *config  = G_OBJECT (tool_options);
  PicmanPaintOptions *options = PICMAN_PAINT_OPTIONS (tool_options);
  GtkWidget        *vbox    = picman_tool_options_gui (tool_options);
  GtkWidget        *hbox;
  GtkWidget        *menu;
  GtkWidget        *label;
  GtkWidget        *scale;
  GType             tool_type;

  tool_type = tool_options->tool_info->tool_type;

  /*  the paint mode menu  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Mode:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  menu = picman_prop_paint_mode_menu_new (config, "paint-mode", TRUE, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), menu, TRUE, TRUE, 0);
  gtk_widget_show (menu);

  if (tool_type == PICMAN_TYPE_ERASER_TOOL     ||
      tool_type == PICMAN_TYPE_CONVOLVE_TOOL   ||
      tool_type == PICMAN_TYPE_DODGE_BURN_TOOL ||
      tool_type == PICMAN_TYPE_SMUDGE_TOOL)
    {
      gtk_widget_set_sensitive (menu, FALSE);
      gtk_widget_set_sensitive (label, FALSE);
    }

  /*  the opacity scale  */
  scale = picman_prop_opacity_spin_scale_new (config, "opacity",
                                            _("Opacity"));
  gtk_box_pack_start (GTK_BOX (vbox), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  /*  the brush  */
  if (g_type_is_a (tool_type, PICMAN_TYPE_BRUSH_TOOL))
    {
      GtkWidget *button;
      GtkWidget *hbox;
      GtkWidget *frame;

      button = picman_prop_brush_box_new (NULL, PICMAN_CONTEXT (tool_options),
                                        _("Brush"), 2,
                                        "brush-view-type", "brush-view-size",
                                        "picman-brush-editor");
      gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
      gtk_widget_show (button);

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      gtk_widget_show (hbox);

      scale = picman_prop_spin_scale_new (config, "brush-size",
                                        _("Size"),
                                        1.0, 10.0, 2);
      picman_spin_scale_set_scale_limits (PICMAN_SPIN_SCALE (scale), 1.0, 1000.0);
      picman_spin_scale_set_gamma (PICMAN_SPIN_SCALE (scale), 1.7);
      gtk_box_pack_start (GTK_BOX (hbox), scale, TRUE, TRUE, 0);
      gtk_widget_show (scale);

      button = picman_stock_button_new (PICMAN_STOCK_RESET, NULL);
      gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
      gtk_image_set_from_stock (GTK_IMAGE (gtk_bin_get_child (GTK_BIN (button))),
                                PICMAN_STOCK_RESET, GTK_ICON_SIZE_MENU);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
      gtk_widget_show (button);

      g_signal_connect (button, "clicked",
                        G_CALLBACK (picman_paint_options_gui_reset_size),
                        options);

      picman_help_set_help_data (button,
                               _("Reset size to brush's native size"), NULL);

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      gtk_widget_show (hbox);

      scale = picman_prop_spin_scale_new (config, "brush-aspect-ratio",
                                        _("Aspect Ratio"),
                                        0.1, 1.0, 2);
      gtk_box_pack_start (GTK_BOX (hbox), scale, TRUE, TRUE, 0);
      gtk_widget_show (scale);

      button = picman_stock_button_new (PICMAN_STOCK_RESET, NULL);
      gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
      gtk_image_set_from_stock (GTK_IMAGE (gtk_bin_get_child (GTK_BIN (button))),
                                PICMAN_STOCK_RESET, GTK_ICON_SIZE_MENU);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
      gtk_widget_show (button);

      g_signal_connect (button, "clicked",
                        G_CALLBACK (picman_paint_options_gui_reset_aspect_ratio),
                        options);

      picman_help_set_help_data (button,
                               _("Reset aspect ratio to brush's native"), NULL);

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      gtk_widget_show (hbox);

      scale = picman_prop_spin_scale_new (config, "brush-angle",
                                        _("Angle"),
                                        1.0, 5.0, 2);
      gtk_box_pack_start (GTK_BOX (hbox), scale, TRUE, TRUE, 0);
      gtk_widget_show (scale);

      button = picman_stock_button_new (PICMAN_STOCK_RESET, NULL);
      gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
      gtk_image_set_from_stock (GTK_IMAGE (gtk_bin_get_child (GTK_BIN (button))),
                                PICMAN_STOCK_RESET, GTK_ICON_SIZE_MENU);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
      gtk_widget_show (button);

      g_signal_connect (button, "clicked",
                        G_CALLBACK (picman_paint_options_gui_reset_angle),
                        options);

      picman_help_set_help_data (button,
                               _("Reset angle to zero"), NULL);

      button = picman_prop_dynamics_box_new (NULL, PICMAN_CONTEXT (tool_options),
                                           _("Dynamics"), 2,
                                           "dynamics-view-type",
                                           "dynamics-view-size",
                                           "picman-dynamics-editor");
      gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
      gtk_widget_show (button);

      frame = dynamics_options_gui (options, tool_type);
      gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
      gtk_widget_show (frame);

      frame = jitter_options_gui (options, tool_type);
      gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
      gtk_widget_show (frame);
    }

  /*  the "smooth stroke" options  */
  if (g_type_is_a (tool_type, PICMAN_TYPE_PAINT_TOOL))
    {
      GtkWidget *frame;

      frame = smoothing_options_gui (options, tool_type);
      gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
      gtk_widget_show (frame);
    }

  /*  the "incremental" toggle  */
  if (tool_type == PICMAN_TYPE_PENCIL_TOOL     ||
      tool_type == PICMAN_TYPE_PAINTBRUSH_TOOL ||
      tool_type == PICMAN_TYPE_ERASER_TOOL)
    {
      GtkWidget *button;

      button = picman_prop_enum_check_button_new (config,
                                                "application-mode",
                                                _("Incremental"),
                                                PICMAN_PAINT_CONSTANT,
                                                PICMAN_PAINT_INCREMENTAL);
      gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
      gtk_widget_show (button);
    }

  /* the "hard edge" toggle */
  if (tool_type == PICMAN_TYPE_ERASER_TOOL            ||
      tool_type == PICMAN_TYPE_CLONE_TOOL             ||
      tool_type == PICMAN_TYPE_HEAL_TOOL              ||
      tool_type == PICMAN_TYPE_PERSPECTIVE_CLONE_TOOL ||
      tool_type == PICMAN_TYPE_CONVOLVE_TOOL          ||
      tool_type == PICMAN_TYPE_DODGE_BURN_TOOL        ||
      tool_type == PICMAN_TYPE_SMUDGE_TOOL)
    {
      GtkWidget *button;

      button = picman_prop_check_button_new (config, "hard", _("Hard edge"));
      gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
      gtk_widget_show (button);
    }

  return vbox;
}


/*  private functions  */

static GtkWidget *
dynamics_options_gui (PicmanPaintOptions *paint_options,
                      GType             tool_type)
{
  GObject   *config = G_OBJECT (paint_options);
  GtkWidget *frame;
  GtkWidget *inner_frame;
  GtkWidget *label;
  GtkWidget *scale;
  GtkWidget *menu;
  GtkWidget *combo;
  GtkWidget *checkbox;
  GtkWidget *vbox;
  GtkWidget *inner_vbox;
  GtkWidget *hbox;
  GtkWidget *box;

  frame = picman_prop_expander_new (config, "dynamics-expanded",
                                  _("Dynamics Options"));

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  inner_frame = picman_frame_new (_("Fade Options"));
  gtk_box_pack_start (GTK_BOX (vbox), inner_frame, FALSE, FALSE, 0);
  gtk_widget_show (inner_frame);

  inner_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add (GTK_CONTAINER (inner_frame), inner_vbox);
  gtk_widget_show (inner_vbox);

  /*  the fade-out scale & unitmenu  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  scale = picman_prop_spin_scale_new (config, "fade-length",
                                    _("Fade length"), 1.0, 50.0, 0);
  picman_spin_scale_set_scale_limits (PICMAN_SPIN_SCALE (scale), 1.0, 1000.0);
  gtk_box_pack_start (GTK_BOX (hbox), scale, TRUE, TRUE, 0);
  gtk_widget_show (scale);

  menu = picman_prop_unit_combo_box_new (config, "fade-unit");
  gtk_box_pack_start (GTK_BOX (hbox), menu, FALSE, FALSE, 0);
  gtk_widget_show (menu);

#if 0
  /* FIXME pixel digits */
  g_object_set_data (G_OBJECT (menu), "set_digits", spinbutton);
  picman_unit_menu_set_pixel_digits (PICMAN_UNIT_MENU (menu), 0);
#endif

  /*  the repeat type  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Repeat:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  combo = picman_prop_enum_combo_box_new (config, "fade-repeat", 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
  gtk_widget_show (combo);

  checkbox = picman_prop_check_button_new (config, "fade-reverse",
                                         _("Reverse"));
  gtk_box_pack_start (GTK_BOX (inner_vbox), checkbox, FALSE, FALSE, 0);
  gtk_widget_show (checkbox);

  /* Color UI */
  if (g_type_is_a (tool_type, PICMAN_TYPE_PAINTBRUSH_TOOL))
    {
      inner_frame = picman_frame_new (_("Color Options"));
      gtk_box_pack_start (GTK_BOX (vbox), inner_frame, FALSE, FALSE, 0);
      gtk_widget_show (inner_frame);

      box = picman_prop_gradient_box_new (NULL, PICMAN_CONTEXT (config),
                                        _("Gradient"), 2,
                                        "gradient-view-type",
                                        "gradient-view-size",
                                        "gradient-reverse",
                                        "picman-gradient-editor");
      gtk_container_add (GTK_CONTAINER (inner_frame), box);
      gtk_widget_show (box);
    }

  return frame;
}

static GtkWidget *
jitter_options_gui (PicmanPaintOptions *paint_options,
                    GType             tool_type)
{
  GObject   *config = G_OBJECT (paint_options);
  GtkWidget *frame;
  GtkWidget *scale;

  scale = picman_prop_spin_scale_new (config, "jitter-amount",
                                    _("Amount"),
                                    0.01, 1.0, 2);

  frame = picman_prop_expanding_frame_new (config, "use-jitter",
                                         _("Apply Jitter"),
                                         scale, NULL);

  return frame;
}

static GtkWidget *
smoothing_options_gui (PicmanPaintOptions *paint_options,
                       GType             tool_type)
{
  GObject   *config = G_OBJECT (paint_options);
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *scale;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);

  frame = picman_prop_expanding_frame_new (config, "use-smoothing",
                                         _("Smooth stroke"),
                                         vbox, NULL);

  scale = picman_prop_spin_scale_new (config, "smoothing-quality",
                                    _("Quality"),
                                    1, 10, 1);
  gtk_box_pack_start (GTK_BOX (vbox), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  scale = picman_prop_spin_scale_new (config, "smoothing-factor",
                                    _("Weight"),
                                    1, 10, 1);
  gtk_box_pack_start (GTK_BOX (vbox), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  return frame;
}

static void
picman_paint_options_gui_reset_size (GtkWidget        *button,
                                   PicmanPaintOptions *paint_options)
{
 PicmanBrush *brush = picman_context_get_brush (PICMAN_CONTEXT (paint_options));

 if (brush)
   {
     g_object_set (paint_options,
                   "brush-size",
                   (gdouble) MAX (picman_temp_buf_get_width  (brush->mask),
                                  picman_temp_buf_get_height (brush->mask)),
                   NULL);
   }
}

static void
picman_paint_options_gui_reset_aspect_ratio (GtkWidget        *button,
                                           PicmanPaintOptions *paint_options)
{
   g_object_set (paint_options,
                 "brush-aspect-ratio", 0.0,
                 NULL);
}

static void
picman_paint_options_gui_reset_angle (GtkWidget        *button,
                                    PicmanPaintOptions *paint_options)
{
   g_object_set (paint_options,
                 "brush-angle", 0.0,
                 NULL);
}
