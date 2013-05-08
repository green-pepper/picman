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

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmanpropwidgets.h"
#include "widgets/picmanwidgets-utils.h"

#include "picmanrotatetool.h"
#include "picmanscaletool.h"
#include "picmanunifiedtransformtool.h"
#include "picmantooloptions-gui.h"
#include "picmantransformoptions.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_TYPE,
  PROP_DIRECTION,
  PROP_INTERPOLATION,
  PROP_CLIP,
  PROP_SHOW_PREVIEW,
  PROP_PREVIEW_OPACITY,
  PROP_GRID_TYPE,
  PROP_GRID_SIZE,
  PROP_CONSTRAIN_MOVE,
  PROP_CONSTRAIN_SCALE,
  PROP_CONSTRAIN_ROTATE,
  PROP_CONSTRAIN_SHEAR,
  PROP_CONSTRAIN_PERSPECTIVE,
  PROP_FROMPIVOT_SCALE,
  PROP_FROMPIVOT_SHEAR,
  PROP_FROMPIVOT_PERSPECTIVE,
  PROP_CORNERSNAP,
  PROP_FIXEDPIVOT,
};


static void     picman_transform_options_set_property (GObject         *object,
                                                     guint            property_id,
                                                     const GValue    *value,
                                                     GParamSpec      *pspec);
static void     picman_transform_options_get_property (GObject         *object,
                                                     guint            property_id,
                                                     GValue          *value,
                                                     GParamSpec      *pspec);

static void     picman_transform_options_reset        (PicmanToolOptions *tool_options);

static gboolean picman_transform_options_sync_grid    (GBinding        *binding,
                                                     const GValue    *source_value,
                                                     GValue          *target_value,
                                                     gpointer         user_data);


G_DEFINE_TYPE (PicmanTransformOptions, picman_transform_options,
               PICMAN_TYPE_TOOL_OPTIONS)

#define parent_class picman_transform_options_parent_class


static void
picman_transform_options_class_init (PicmanTransformOptionsClass *klass)
{
  GObjectClass         *object_class  = G_OBJECT_CLASS (klass);
  PicmanToolOptionsClass *options_class = PICMAN_TOOL_OPTIONS_CLASS (klass);

  object_class->set_property = picman_transform_options_set_property;
  object_class->get_property = picman_transform_options_get_property;

  options_class->reset       = picman_transform_options_reset;

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_TYPE,
                                 "type", NULL,
                                 PICMAN_TYPE_TRANSFORM_TYPE,
                                 PICMAN_TRANSFORM_TYPE_LAYER,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_DIRECTION,
                                 "direction",
                                 N_("Direction of transformation"),
                                 PICMAN_TYPE_TRANSFORM_DIRECTION,
                                 PICMAN_TRANSFORM_FORWARD,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_INTERPOLATION,
                                 "interpolation",
                                 N_("Interpolation method"),
                                 PICMAN_TYPE_INTERPOLATION_TYPE,
                                 PICMAN_INTERPOLATION_LINEAR,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_CLIP,
                                 "clip",
                                 N_("How to clip"),
                                 PICMAN_TYPE_TRANSFORM_RESIZE,
                                 PICMAN_TRANSFORM_RESIZE_ADJUST,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_PREVIEW,
                                    "show-preview",
                                    N_("Show a preview of the transformed image"),
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_PREVIEW_OPACITY,
                                   "preview-opacity",
                                   N_("Opacity of the preview image"),
                                   0.0, 1.0, 1.0,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_GRID_TYPE,
                                 "grid-type",
                                 N_("Composition guides such as rule of thirds"),
                                 PICMAN_TYPE_GUIDES_TYPE,
                                 PICMAN_GUIDES_N_LINES,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_GRID_SIZE,
                                "grid-size",
                                N_("Size of a grid cell for variable number of composition guides"),
                                1, 128, 15,
                                PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_CONSTRAIN_MOVE,
                                    "constrain-move",
                                    NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_CONSTRAIN_SCALE,
                                    "constrain-scale",
                                    NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_CONSTRAIN_ROTATE,
                                    "constrain-rotate",
                                    NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_CONSTRAIN_SHEAR,
                                    "constrain-shear",
                                    NULL,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_CONSTRAIN_PERSPECTIVE,
                                    "constrain-perspective",
                                    NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_FROMPIVOT_SCALE,
                                    "frompivot-scale",
                                    NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_FROMPIVOT_SHEAR,
                                    "frompivot-shear",
                                    NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_FROMPIVOT_PERSPECTIVE,
                                    "frompivot-perspective",
                                    NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_CORNERSNAP,
                                    "cornersnap",
                                    NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_FIXEDPIVOT,
                                    "fixedpivot",
                                    NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_transform_options_init (PicmanTransformOptions *options)
{
  options->recursion_level = 3;
}

static void
picman_transform_options_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  PicmanTransformOptions *options = PICMAN_TRANSFORM_OPTIONS (object);

  switch (property_id)
    {
    case PROP_TYPE:
      options->type = g_value_get_enum (value);
      break;
    case PROP_DIRECTION:
      options->direction = g_value_get_enum (value);
      break;
    case PROP_INTERPOLATION:
      options->interpolation = g_value_get_enum (value);
      break;
    case PROP_CLIP:
      options->clip = g_value_get_enum (value);
      break;
    case PROP_SHOW_PREVIEW:
      options->show_preview = g_value_get_boolean (value);
      break;
    case PROP_PREVIEW_OPACITY:
      options->preview_opacity = g_value_get_double (value);
      break;
    case PROP_GRID_TYPE:
      options->grid_type = g_value_get_enum (value);
      break;
    case PROP_GRID_SIZE:
      options->grid_size = g_value_get_int (value);
      break;
    case PROP_CONSTRAIN_MOVE:
      options->constrain_move = g_value_get_boolean (value);
      break;
    case PROP_CONSTRAIN_SCALE:
      options->constrain_scale = g_value_get_boolean (value);
      break;
    case PROP_CONSTRAIN_ROTATE:
      options->constrain_rotate = g_value_get_boolean (value);
      break;
    case PROP_CONSTRAIN_SHEAR:
      options->constrain_shear = g_value_get_boolean (value);
      break;
    case PROP_CONSTRAIN_PERSPECTIVE:
      options->constrain_perspective = g_value_get_boolean (value);
      break;
    case PROP_FROMPIVOT_SCALE:
      options->frompivot_scale = g_value_get_boolean (value);
      break;
    case PROP_FROMPIVOT_SHEAR:
      options->frompivot_shear = g_value_get_boolean (value);
      break;
    case PROP_FROMPIVOT_PERSPECTIVE:
      options->frompivot_perspective = g_value_get_boolean (value);
      break;
    case PROP_CORNERSNAP:
      options->cornersnap = g_value_get_boolean (value);
      break;
    case PROP_FIXEDPIVOT:
      options->fixedpivot = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_transform_options_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  PicmanTransformOptions *options = PICMAN_TRANSFORM_OPTIONS (object);

  switch (property_id)
    {
    case PROP_TYPE:
      g_value_set_enum (value, options->type);
      break;
    case PROP_DIRECTION:
      g_value_set_enum (value, options->direction);
      break;
    case PROP_INTERPOLATION:
      g_value_set_enum (value, options->interpolation);
      break;
    case PROP_CLIP:
      g_value_set_enum (value, options->clip);
      break;
    case PROP_SHOW_PREVIEW:
      g_value_set_boolean (value, options->show_preview);
      break;
    case PROP_PREVIEW_OPACITY:
      g_value_set_double (value, options->preview_opacity);
      break;
    case PROP_GRID_TYPE:
      g_value_set_enum (value, options->grid_type);
      break;
    case PROP_GRID_SIZE:
      g_value_set_int (value, options->grid_size);
      break;
    case PROP_CONSTRAIN_MOVE:
      g_value_set_boolean (value, options->constrain_move);
      break;
    case PROP_CONSTRAIN_SCALE:
      g_value_set_boolean (value, options->constrain_scale);
      break;
    case PROP_CONSTRAIN_ROTATE:
      g_value_set_boolean (value, options->constrain_rotate);
      break;
    case PROP_CONSTRAIN_SHEAR:
      g_value_set_boolean (value, options->constrain_shear);
      break;
    case PROP_CONSTRAIN_PERSPECTIVE:
      g_value_set_boolean (value, options->constrain_perspective);
      break;
    case PROP_FROMPIVOT_SCALE:
      g_value_set_boolean (value, options->frompivot_scale);
      break;
    case PROP_FROMPIVOT_SHEAR:
      g_value_set_boolean (value, options->frompivot_shear);
      break;
    case PROP_FROMPIVOT_PERSPECTIVE:
      g_value_set_boolean (value, options->frompivot_perspective);
      break;
    case PROP_CORNERSNAP:
      g_value_set_boolean (value, options->cornersnap);
      break;
    case PROP_FIXEDPIVOT:
      g_value_set_boolean (value, options->fixedpivot);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_transform_options_reset (PicmanToolOptions *tool_options)
{
  GParamSpec *pspec;

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (tool_options),
                                        "interpolation");

  if (pspec)
    G_PARAM_SPEC_ENUM (pspec)->default_value =
      tool_options->tool_info->picman->config->interpolation_type;

  PICMAN_TOOL_OPTIONS_CLASS (parent_class)->reset (tool_options);
}

/**
 * picman_transform_options_gui:
 * @tool_options: a #PicmanToolOptions
 *
 * Build the Transform Tool Options.
 *
 * Return value: a container holding the transform tool options
 **/
GtkWidget *
picman_transform_options_gui (PicmanToolOptions *tool_options)
{
  GObject     *config = G_OBJECT (tool_options);
  GtkWidget   *vbox   = picman_tool_options_gui (tool_options);
  GtkWidget   *hbox;
  GtkWidget   *box;
  GtkWidget   *label;
  GtkWidget   *frame;
  GtkWidget   *combo;
  GtkWidget   *scale;
  GtkWidget   *grid_box;
  const gchar *constrain_name  = NULL;
  const gchar *constrain_label = NULL;
  const gchar *constrain_tip   = NULL;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Transform:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  box = picman_prop_enum_stock_box_new (config, "type", "picman", 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox), box, FALSE, FALSE, 0);
  gtk_widget_show (box);

  frame = picman_prop_enum_radio_frame_new (config, "direction",
                                          _("Direction"), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  the interpolation menu  */
  frame = picman_frame_new (_("Interpolation:"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  combo = picman_prop_enum_combo_box_new (config, "interpolation", 0, 0);
  gtk_container_add (GTK_CONTAINER (frame), combo);
  gtk_widget_show (combo);

  /*  the clipping menu  */
  frame = picman_frame_new (_("Clipping:"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  combo = picman_prop_enum_combo_box_new (config, "clip", 0, 0);
  gtk_container_add (GTK_CONTAINER (frame), combo);
  gtk_widget_show (combo);

  /*  the preview frame  */
  scale = picman_prop_opacity_spin_scale_new (config, "preview-opacity",
                                            _("Image opacity"));
  frame = picman_prop_expanding_frame_new (config, "show-preview",
                                         _("Show image preview"),
                                         scale, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  the guides frame  */
  frame = picman_frame_new (_("Guides"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  grid_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add (GTK_CONTAINER (frame), grid_box);
  gtk_widget_show (grid_box);

  /*  the guides type menu  */
  combo = picman_prop_enum_combo_box_new (config, "grid-type", 0, 0);
  gtk_box_pack_start (GTK_BOX (grid_box), combo, FALSE, FALSE, 0);
  gtk_widget_show (combo);

  /*  the grid density scale  */
  scale = picman_prop_spin_scale_new (config, "grid-size", NULL,
                                    1.8, 8.0, 0);
  gtk_box_pack_start (GTK_BOX (grid_box), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  g_object_bind_property_full (config, "grid-type",
                               scale,  "visible",
                               G_BINDING_SYNC_CREATE,
                               picman_transform_options_sync_grid,
                               NULL,
                               NULL, NULL);

  if (tool_options->tool_info->tool_type == PICMAN_TYPE_ROTATE_TOOL)
    {
      constrain_name  = "constrain-rotate";
      constrain_label = _("15 degrees  (%s)");
      constrain_tip   = _("Limit rotation steps to 15 degrees");
    }
  else if (tool_options->tool_info->tool_type == PICMAN_TYPE_SCALE_TOOL)
    {
      constrain_name  = "constrain-scale";
      constrain_label = _("Keep aspect  (%s)");
      constrain_tip   = _("Keep the original aspect ratio");
    }

  //TODO: check that the selection tools use the picman_get_*_mask() functions for constrain/etc or change to what they use
  else if (tool_options->tool_info->tool_type == PICMAN_TYPE_UNIFIED_TRANSFORM_TOOL)
    {
      GdkModifierType shift = picman_get_extend_selection_mask ();
      GdkModifierType ctrl  = picman_get_constrain_behavior_mask ();

      struct
      {
        GdkModifierType mod;
        gchar *name;
        gchar *desc;
        gchar *tip;
      }
      opt_list[] =
      {
        { shift, NULL, "Constrain  (%s)" },
        { shift, "constrain-move", "Move",
          "Constrain movement to 45 degree angles from center  (%s)" },
        { shift, "constrain-scale", "Scale",
          "Maintain aspect ratio when scaling  (%s)" },
        { shift, "constrain-rotate", "Rotate",
          "Constrain rotation to 15 degree increments  (%s)" },
        { shift, "constrain-shear", "Shear",
          "Shear along edge direction only  (%s)" },
        { shift, "constrain-perspective", "Perspective",
          "Constrain perspective handles to move along edges and diagonal  (%s)" },

        { ctrl, NULL,
          "From pivot  (%s)" },
        { ctrl, "frompivot-scale", "Scale",
          "Scale from pivot point  (%s)" },
        { ctrl, "frompivot-shear", "Shear",
          "Shear opposite edge by same amount  (%s)" },
        { ctrl, "frompivot-perspective", "Perspective",
          "Maintain position of pivot while changing perspective  (%s)" },

        { 0, NULL,
          "Pivot" },
        { shift, "cornersnap", "Snap  (%s)",
          "Snap pivot to corners and center  (%s)" },
        { 0, "fixedpivot", "Lock",
          "Lock pivot position to canvas" },
      };

      GtkWidget *button;
      gchar     *label;
      gint       i;

      frame = NULL;

      for (i = 0; i < G_N_ELEMENTS (opt_list); i++)
        {
          if (!opt_list[i].name && !opt_list[i].desc)
            {
              frame = NULL;
              continue;
            }

          label = g_strdup_printf (opt_list[i].desc,
                                   picman_get_mod_string (opt_list[i].mod));

          if (opt_list[i].name)
            {
              button = picman_prop_check_button_new (config, opt_list[i].name,
                                                   label);

              gtk_box_pack_start (GTK_BOX (frame ? grid_box : vbox),
                                  button, FALSE, FALSE, 0);

              gtk_widget_show (button);

              g_free (label);
              label = g_strdup_printf (opt_list[i].tip,
                                       picman_get_mod_string (opt_list[i].mod));

              picman_help_set_help_data (button, label, NULL);
            }
          else
            {
              frame = picman_frame_new (label);
              gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
              gtk_widget_show (frame);

              grid_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
              gtk_container_add (GTK_CONTAINER (frame), grid_box);
              gtk_widget_show (grid_box);
            }

          g_free (label);
        }
    }

  if (constrain_label)
    {
      GtkWidget       *button;
      gchar           *label;
      GdkModifierType  constrain_mask;

      constrain_mask = picman_get_extend_selection_mask ();

      label = g_strdup_printf (constrain_label,
                               picman_get_mod_string (constrain_mask));

      button = picman_prop_check_button_new (config, constrain_name, label);
      gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
      gtk_widget_show (button);

      picman_help_set_help_data (button, constrain_tip, NULL);

      g_free (label);
    }

  return vbox;
}

gboolean
picman_transform_options_show_preview (PicmanTransformOptions *options)
{
  g_return_val_if_fail (PICMAN_IS_TRANSFORM_OPTIONS (options), FALSE);

  return (options->show_preview                           &&
          options->type      == PICMAN_TRANSFORM_TYPE_LAYER);
}


/*  private functions  */

static gboolean
picman_transform_options_sync_grid (GBinding     *binding,
                                  const GValue *source_value,
                                  GValue       *target_value,
                                  gpointer      user_data)
{
  PicmanGuidesType type = g_value_get_enum (source_value);

  g_value_set_boolean (target_value,
                       type == PICMAN_GUIDES_N_LINES ||
                       type == PICMAN_GUIDES_SPACING);

  return TRUE;
}
