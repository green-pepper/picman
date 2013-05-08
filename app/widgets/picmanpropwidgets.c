/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanpropwidgets.c
 * Copyright (C) 2002-2004  Michael Natterer <mitch@picman.org>
 *                          Sven Neumann <sven@picman.org>
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
#include <stdlib.h>

#include <gegl.h>
#include <gegl-paramspecs.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontext.h"
#include "core/picmanviewable.h"

#include "picmancolorpanel.h"
#include "picmandnd.h"
#include "picmaniconpicker.h"
#include "picmanlanguagecombobox.h"
#include "picmanlanguageentry.h"
#include "picmanscalebutton.h"
#include "picmanspinscale.h"
#include "picmanview.h"
#include "picmanpropwidgets.h"
#include "picmanwidgets-constructors.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


/*  utility function prototypes  */

static void         set_param_spec     (GObject     *object,
                                        GtkWidget   *widget,
                                        GParamSpec  *param_spec);
static GParamSpec * get_param_spec     (GObject     *object);

static GParamSpec * find_param_spec    (GObject     *object,
                                        const gchar *property_name,
                                        const gchar *strloc);
static GParamSpec * check_param_spec   (GObject     *object,
                                        const gchar *property_name,
                                        GType         type,
                                        const gchar *strloc);
static GParamSpec * check_param_spec_w (GObject     *object,
                                        const gchar *property_name,
                                        GType         type,
                                        const gchar *strloc);

static gboolean     get_numeric_values (GObject     *object,
                                        GParamSpec  *param_spec,
                                        gdouble     *value,
                                        gdouble     *lower,
                                        gdouble     *upper,
                                        const gchar *strloc);

static void         connect_notify     (GObject     *config,
                                        const gchar *property_name,
                                        GCallback    callback,
                                        gpointer     callback_data);


/*********************/
/*  expanding frame  */
/*********************/

GtkWidget *
picman_prop_expanding_frame_new (GObject      *config,
                               const gchar  *property_name,
                               const gchar  *button_label,
                               GtkWidget    *child,
                               GtkWidget   **button)
{
  GtkWidget *frame;
  GtkWidget *toggle;
  gboolean   value;

  if (! check_param_spec_w (config, property_name,
                            G_TYPE_PARAM_BOOLEAN, G_STRFUNC))
    return NULL;

  frame = picman_frame_new (NULL);

  toggle = picman_prop_check_button_new (config, property_name, button_label);
  gtk_frame_set_label_widget (GTK_FRAME (frame), toggle);
  gtk_widget_show (toggle);

  gtk_container_add (GTK_CONTAINER (frame), child);

  g_object_get (config,
                property_name, &value,
                NULL);

  if (value)
    gtk_widget_show (child);

  g_signal_connect_object (toggle, "toggled",
                           G_CALLBACK (picman_toggle_button_set_visible),
                           child, 0);

  if (button)
    *button = toggle;

  return frame;
}


/****************/
/*  paint menu  */
/****************/

static void   picman_prop_paint_menu_callback (GtkWidget   *widget,
                                             GObject     *config);
static void   picman_prop_paint_menu_notify   (GObject     *config,
                                             GParamSpec  *param_spec,
                                             GtkWidget   *menu);

/**
 * picman_prop_paint_mode_menu_new:
 * @config:             #PicmanConfig object to which property is attached.
 * @property_name:      Name of Enum property.
 * @with_behind_mode:   Whether to include "Behind" mode in the menu.
 * @with_replace_modes: Whether to include the "Replace", "Erase" and
 *                      "Anti Erase" modes in the menu.
 *
 * Creates a #PicmanPaintModeMenu widget to display and set the specified
 * Enum property, for which the enum must be #PicmanLayerModeEffects.
 *
 * Return value: The newly created #PicmanPaintModeMenu widget.
 *
 * Since PICMAN 2.4
 */
GtkWidget *
picman_prop_paint_mode_menu_new (GObject     *config,
                               const gchar *property_name,
                               gboolean     with_behind_mode,
                               gboolean     with_replace_modes)
{
  GParamSpec *param_spec;
  GtkWidget  *menu;
  gint        value;

  param_spec = check_param_spec_w (config, property_name,
                                   G_TYPE_PARAM_ENUM, G_STRFUNC);
  if (! param_spec)
    return NULL;

  g_object_get (config,
                property_name, &value,
                NULL);

  menu = picman_paint_mode_menu_new (with_behind_mode, with_replace_modes);

  picman_int_combo_box_connect (PICMAN_INT_COMBO_BOX (menu),
                              value,
                              G_CALLBACK (picman_prop_paint_menu_callback),
                              config);

  set_param_spec (G_OBJECT (menu), menu, param_spec);

  connect_notify (config, property_name,
                  G_CALLBACK (picman_prop_paint_menu_notify),
                  menu);

  return menu;
}

static void
picman_prop_paint_menu_callback (GtkWidget *widget,
                               GObject   *config)
{
  GParamSpec *param_spec;
  gint        value;

  param_spec = get_param_spec (G_OBJECT (widget));
  if (! param_spec)
    return;

  if (picman_int_combo_box_get_active (PICMAN_INT_COMBO_BOX (widget), &value))
    {
      g_object_set (config,
                    param_spec->name, value,
                    NULL);
    }
}

static void
picman_prop_paint_menu_notify (GObject    *config,
                             GParamSpec *param_spec,
                             GtkWidget  *menu)
{
  gint value;

  g_object_get (config,
                param_spec->name, &value,
                NULL);

  g_signal_handlers_block_by_func (menu,
                                   picman_prop_paint_menu_callback,
                                   config);

  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (menu), value);

  g_signal_handlers_unblock_by_func (menu,
                                     picman_prop_paint_menu_callback,
                                     config);

}


/******************/
/*  color button  */
/******************/

static void   picman_prop_color_button_callback (GtkWidget  *widget,
                                               GObject    *config);
static void   picman_prop_color_button_notify   (GObject    *config,
                                               GParamSpec *param_spec,
                                               GtkWidget  *button);

/**
 * picman_prop_color_button_new:
 * @config:        #PicmanConfig object to which property is attached.
 * @property_name: Name of #PicmanRGB property.
 * @title:         Title of the #PicmanColorPanel that is to be created
 * @width:         Width of color button.
 * @height:        Height of color button.
 * @type:          How transparency is represented.
 *
 * Creates a #PicmanColorPanel to set and display the value of a #PicmanRGB
 * property.  Pressing the button brings up a color selector dialog.
 *
 * Return value:  A new #PicmanColorPanel widget.
 *
 * Since PICMAN 2.4
 */
GtkWidget *
picman_prop_color_button_new (GObject           *config,
                            const gchar       *property_name,
                            const gchar       *title,
                            gint               width,
                            gint               height,
                            PicmanColorAreaType  type)
{
  GParamSpec *param_spec;
  GtkWidget  *button;
  PicmanRGB    *value;

  param_spec = check_param_spec_w (config, property_name,
                                   PICMAN_TYPE_PARAM_RGB, G_STRFUNC);
  if (! param_spec)
    return NULL;

  g_object_get (config,
                property_name, &value,
                NULL);

  button = picman_color_panel_new (title, value, type, width, height);
  g_free (value);

  set_param_spec (G_OBJECT (button), button, param_spec);

  g_signal_connect (button, "color-changed",
                    G_CALLBACK (picman_prop_color_button_callback),
                    config);

  connect_notify (config, property_name,
                  G_CALLBACK (picman_prop_color_button_notify),
                  button);

  return button;
}

static void
picman_prop_color_button_callback (GtkWidget *button,
                                 GObject   *config)
{
  GParamSpec *param_spec;
  PicmanRGB     value;

  param_spec = get_param_spec (G_OBJECT (button));
  if (! param_spec)
    return;

  picman_color_button_get_color (PICMAN_COLOR_BUTTON (button), &value);

  g_signal_handlers_block_by_func (config,
                                   picman_prop_color_button_notify,
                                   button);

  g_object_set (config,
                param_spec->name, &value,
                NULL);

  g_signal_handlers_unblock_by_func (config,
                                     picman_prop_color_button_notify,
                                     button);
}

static void
picman_prop_color_button_notify (GObject    *config,
                               GParamSpec *param_spec,
                               GtkWidget  *button)
{
  PicmanRGB *value;

  g_object_get (config,
                param_spec->name, &value,
                NULL);

  g_signal_handlers_block_by_func (button,
                                   picman_prop_color_button_callback,
                                   config);

  picman_color_button_set_color (PICMAN_COLOR_BUTTON (button), value);

  g_free (value);

  g_signal_handlers_unblock_by_func (button,
                                     picman_prop_color_button_callback,
                                     config);
}


/******************/
/*  scale button  */
/******************/

static void   picman_prop_scale_button_callback (GtkWidget  *widget,
                                               gdouble     value,
                                               GObject    *config);
static void   picman_prop_scale_button_notify   (GObject    *config,
                                               GParamSpec *param_spec,
                                               GtkWidget  *button);

/**
 * picman_prop_scale_button_new:
 * @config:        #PicmanConfig object to which property is attached.
 * @property_name: Name of gdouble property
 *
 * Creates a #PicmanScaleButton to set and display the value of a
 * gdouble property in a very space-efficient way.
 *
 * Return value:  A new #PicmanScaleButton widget.
 *
 * Since PICMAN 2.6
 */
GtkWidget *
picman_prop_scale_button_new (GObject     *config,
                            const gchar *property_name)
{
  GParamSpec *param_spec;
  GtkWidget  *button;
  gdouble     value;

  param_spec = check_param_spec_w (config, property_name,
                                   G_TYPE_PARAM_DOUBLE, G_STRFUNC);

  if (! param_spec)
    return NULL;

  g_object_get (config,
                param_spec->name, &value,
                NULL);

  button = picman_scale_button_new (value,
                                  G_PARAM_SPEC_DOUBLE (param_spec)->minimum,
                                  G_PARAM_SPEC_DOUBLE (param_spec)->maximum);

  set_param_spec (G_OBJECT (button), button, param_spec);

  g_signal_connect (button, "value-changed",
                    G_CALLBACK (picman_prop_scale_button_callback),
                    config);

  connect_notify (config, property_name,
                  G_CALLBACK (picman_prop_scale_button_notify),
                  button);

  return button;
}

static void
picman_prop_scale_button_callback (GtkWidget *button,
                                 gdouble    value,
                                 GObject   *config)
{
  GParamSpec *param_spec;

  param_spec = get_param_spec (G_OBJECT (button));
  if (! param_spec)
    return;

  g_signal_handlers_block_by_func (config,
                                   picman_prop_scale_button_notify,
                                   button);

  g_object_set (config,
                param_spec->name, value,
                NULL);

  g_signal_handlers_unblock_by_func (config,
                                     picman_prop_scale_button_notify,
                                     button);
}

static void
picman_prop_scale_button_notify (GObject    *config,
                               GParamSpec *param_spec,
                               GtkWidget  *button)
{
  gdouble value;

  g_object_get (config,
                param_spec->name, &value,
                NULL);

  g_signal_handlers_block_by_func (button,
                                   picman_prop_scale_button_callback,
                                   config);

  gtk_scale_button_set_value (GTK_SCALE_BUTTON (button), value);

  g_signal_handlers_unblock_by_func (button,
                                     picman_prop_scale_button_callback,
                                     config);
}


/*****************/
/*  adjustments  */
/*****************/

static void   picman_prop_adjustment_callback (GtkAdjustment *adjustment,
                                             GObject       *config);
static void   picman_prop_adjustment_notify   (GObject       *config,
                                             GParamSpec    *param_spec,
                                             GtkAdjustment *adjustment);

/**
 * picman_prop_spin_scale_new:
 * @config:        #PicmanConfig object to which property is attached.
 * @property_name: Name of gdouble property
 *
 * Creates a #PicmanSpinScale to set and display the value of a
 * gdouble property in a very space-efficient way.
 *
 * Return value:  A new #PicmanSpinScale widget.
 *
 * Since PICMAN 2.8
 */
GtkWidget *
picman_prop_spin_scale_new (GObject     *config,
                          const gchar *property_name,
                          const gchar *label,
                          gdouble      step_increment,
                          gdouble      page_increment,
                          gint         digits)
{
  GParamSpec *param_spec;
  GtkObject  *adjustment;
  GtkWidget  *scale;
  gdouble     value;
  gdouble     lower;
  gdouble     upper;

  param_spec = find_param_spec (config, property_name, G_STRFUNC);
  if (! param_spec)
    return NULL;

  if (! get_numeric_values (config,
                            param_spec, &value, &lower, &upper, G_STRFUNC))
    return NULL;

  if (! G_IS_PARAM_SPEC_DOUBLE (param_spec))
    digits = 0;

  adjustment = gtk_adjustment_new (value, lower, upper,
                                   step_increment, page_increment, 0.0);

  scale = picman_spin_scale_new (GTK_ADJUSTMENT (adjustment), label, digits);

  set_param_spec (G_OBJECT (adjustment), scale, param_spec);

  if (GEGL_IS_PARAM_SPEC_DOUBLE (param_spec))
    {
      GeglParamSpecDouble *gspec = GEGL_PARAM_SPEC_DOUBLE (param_spec);
      picman_spin_scale_set_scale_limits (PICMAN_SPIN_SCALE (scale),
                                        gspec->ui_minimum, gspec->ui_maximum);
      picman_spin_scale_set_gamma (PICMAN_SPIN_SCALE (scale), gspec->ui_gamma);
    }

  if (GEGL_IS_PARAM_SPEC_INT (param_spec))
    {
      GeglParamSpecInt *gspec = GEGL_PARAM_SPEC_INT (param_spec);
      picman_spin_scale_set_scale_limits (PICMAN_SPIN_SCALE (scale),
                                        gspec->ui_minimum, gspec->ui_maximum);
      picman_spin_scale_set_gamma (PICMAN_SPIN_SCALE (scale), gspec->ui_gamma);
    }


  g_signal_connect (adjustment, "value-changed",
                    G_CALLBACK (picman_prop_adjustment_callback),
                    config);

  connect_notify (config, property_name,
                  G_CALLBACK (picman_prop_adjustment_notify),
                  adjustment);

  return scale;
}

/**
 * picman_prop_opacity_spin_scale_new:
 * @config:        #PicmanConfig object to which property is attached.
 * @property_name: Name of gdouble property
 *
 * Creates a #PicmanSpinScale to set and display the value of a
 * gdouble property in a very space-efficient way.
 *
 * Return value:  A new #PicmanSpinScale widget.
 *
 * Since PICMAN 2.8
 */
GtkWidget *
picman_prop_opacity_spin_scale_new (GObject     *config,
                                  const gchar *property_name,
                                  const gchar *label)
{
  GParamSpec *param_spec;
  GtkObject  *adjustment;
  GtkWidget  *scale;
  gdouble     value;
  gdouble     lower;
  gdouble     upper;

  param_spec = check_param_spec_w (config, property_name,
                                   G_TYPE_PARAM_DOUBLE, G_STRFUNC);
  if (! param_spec)
    return NULL;

  g_object_get (config, property_name, &value, NULL);

  value *= 100.0;
  lower = G_PARAM_SPEC_DOUBLE (param_spec)->minimum * 100.0;
  upper = G_PARAM_SPEC_DOUBLE (param_spec)->maximum * 100.0;

  adjustment = gtk_adjustment_new (value, lower, upper, 1.0, 10.0, 0.0);

  scale = picman_spin_scale_new (GTK_ADJUSTMENT (adjustment), label, 1);

  set_param_spec (G_OBJECT (adjustment), scale, param_spec);
  g_object_set_data (G_OBJECT (adjustment),
                     "opacity-scale", GINT_TO_POINTER (TRUE));

  g_signal_connect (adjustment, "value-changed",
                    G_CALLBACK (picman_prop_adjustment_callback),
                    config);

  connect_notify (config, property_name,
                  G_CALLBACK (picman_prop_adjustment_notify),
                  adjustment);

  return scale;
}

static void
picman_prop_adjustment_callback (GtkAdjustment *adjustment,
                               GObject       *config)
{
  GParamSpec *param_spec;
  gdouble     value;

  param_spec = get_param_spec (G_OBJECT (adjustment));
  if (! param_spec)
    return;

  value = gtk_adjustment_get_value (adjustment);

  if (G_IS_PARAM_SPEC_INT (param_spec))
    {
      g_object_set (config,
                    param_spec->name, (gint) value,
                    NULL);
    }
  else if (G_IS_PARAM_SPEC_UINT (param_spec))
    {
      g_object_set (config,
                    param_spec->name, (guint) value,
                    NULL);
    }
  else if (G_IS_PARAM_SPEC_LONG (param_spec))
    {
      g_object_set (config,
                    param_spec->name, (glong) value,
                    NULL);
    }
  else if (G_IS_PARAM_SPEC_ULONG (param_spec))
    {
      g_object_set (config,
                    param_spec->name, (gulong) value,
                    NULL);
    }
  else if (G_IS_PARAM_SPEC_INT64 (param_spec))
    {
      g_object_set (config,
                    param_spec->name, (gint64) value,
                    NULL);
    }
  else if (G_IS_PARAM_SPEC_UINT64 (param_spec))
    {
      g_object_set (config,
                    param_spec->name, (guint64) value,
                    NULL);
    }
  else if (G_IS_PARAM_SPEC_DOUBLE (param_spec))
    {
      if (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (adjustment),
                                              "opacity-scale")))
        value /= 100.0;

      g_object_set (config, param_spec->name, value, NULL);
    }
}

static void
picman_prop_adjustment_notify (GObject       *config,
                             GParamSpec    *param_spec,
                             GtkAdjustment *adjustment)
{
  gdouble value;

  if (G_IS_PARAM_SPEC_INT (param_spec))
    {
      gint int_value;

      g_object_get (config, param_spec->name, &int_value, NULL);

      value = int_value;
    }
  else if (G_IS_PARAM_SPEC_UINT (param_spec))
    {
      guint uint_value;

      g_object_get (config, param_spec->name, &uint_value, NULL);

      value = uint_value;
    }
  else if (G_IS_PARAM_SPEC_LONG (param_spec))
    {
      glong long_value;

      g_object_get (config, param_spec->name, &long_value, NULL);

      value = long_value;
    }
  else if (G_IS_PARAM_SPEC_ULONG (param_spec))
    {
      gulong ulong_value;

      g_object_get (config, param_spec->name, &ulong_value, NULL);

      value = ulong_value;
    }
  else if (G_IS_PARAM_SPEC_INT64 (param_spec))
    {
      gint64 int64_value;

      g_object_get (config, param_spec->name, &int64_value, NULL);

      value = int64_value;
    }
  else if (G_IS_PARAM_SPEC_UINT64 (param_spec))
    {
      guint64 uint64_value;

      g_object_get (config, param_spec->name, &uint64_value, NULL);

#if defined _MSC_VER && (_MSC_VER < 1300)
      value = (gint64) uint64_value;
#else
      value = uint64_value;
#endif
    }
  else if (G_IS_PARAM_SPEC_DOUBLE (param_spec))
    {
      g_object_get (config, param_spec->name, &value, NULL);

      if (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (adjustment),
                                              "opacity-scale")))
        value *= 100.0;
    }
  else
    {
      g_warning ("%s: unhandled param spec of type %s",
                 G_STRFUNC, G_PARAM_SPEC_TYPE_NAME (param_spec));
      return;
    }

  if (gtk_adjustment_get_value (adjustment) != value)
    {
      g_signal_handlers_block_by_func (adjustment,
                                       picman_prop_adjustment_callback,
                                       config);

      gtk_adjustment_set_value (adjustment, value);

      g_signal_handlers_unblock_by_func (adjustment,
                                         picman_prop_adjustment_callback,
                                         config);
    }
}


/*************/
/*  view  */
/*************/

static void   picman_prop_view_drop   (GtkWidget    *menu,
                                     gint          x,
                                     gint          y,
                                     PicmanViewable *viewable,
                                     gpointer      data);
static void   picman_prop_view_notify (GObject      *config,
                                     GParamSpec   *param_spec,
                                     GtkWidget    *view);

/**
 * picman_prop_view_new:
 * @config:        #PicmanConfig object to which property is attached.
 * @context:       a #Picmancontext.
 * @property_name: Name of #PicmanViewable property.
 * @size:          Width and height of preview display.
 *
 * Creates a widget to display the value of a #PicmanViewable property.
 *
 * Return value:  A new #PicmanView widget.
 *
 * Since PICMAN 2.4
 */
GtkWidget *
picman_prop_view_new (GObject     *config,
                    const gchar *property_name,
                    PicmanContext *context,
                    gint         size)
{
  GParamSpec   *param_spec;
  GtkWidget    *view;
  PicmanViewable *viewable;

  param_spec = check_param_spec_w (config, property_name,
                                   G_TYPE_PARAM_OBJECT, G_STRFUNC);
  if (! param_spec)
    return NULL;

  if (! g_type_is_a (param_spec->value_type, PICMAN_TYPE_VIEWABLE))
    {
      g_warning ("%s: property '%s' of %s is not a PicmanViewable",
                 G_STRFUNC, property_name,
                 g_type_name (G_TYPE_FROM_INSTANCE (config)));
      return NULL;
    }

  view = picman_view_new_by_types (context,
                                 PICMAN_TYPE_VIEW,
                                 param_spec->value_type,
                                 size, 0, FALSE);

  if (! view)
    {
      g_warning ("%s: cannot create view for type '%s'",
                 G_STRFUNC, g_type_name (param_spec->value_type));
      return NULL;
    }

  g_object_get (config,
                property_name, &viewable,
                NULL);

  if (viewable)
    {
      picman_view_set_viewable (PICMAN_VIEW (view), viewable);
      g_object_unref (viewable);
    }

  set_param_spec (G_OBJECT (view), view, param_spec);

  picman_dnd_viewable_dest_add (view, param_spec->value_type,
                              picman_prop_view_drop,
                              config);

  connect_notify (config, property_name,
                  G_CALLBACK (picman_prop_view_notify),
                  view);

  return view;
}

static void
picman_prop_view_drop (GtkWidget    *view,
                     gint          x,
                     gint          y,
                     PicmanViewable *viewable,
                     gpointer      data)
{
  GObject    *config;
  GParamSpec *param_spec;

  param_spec = get_param_spec (G_OBJECT (view));
  if (! param_spec)
    return;

  config = G_OBJECT (data);

  g_object_set (config,
                param_spec->name, viewable,
                NULL);
}

static void
picman_prop_view_notify (GObject      *config,
                       GParamSpec   *param_spec,
                       GtkWidget    *view)
{
  PicmanViewable *viewable;

  g_object_get (config,
                param_spec->name, &viewable,
                NULL);

  picman_view_set_viewable (PICMAN_VIEW (view), viewable);

  if (viewable)
    g_object_unref (viewable);
}


/***********************
 *  number pair entry  *
 ***********************/

typedef struct
{
  GObject     *config;
  const gchar *left_number_property;
  const gchar *right_number_property;
  const gchar *default_left_number_property;
  const gchar *default_right_number_property;
  const gchar *user_override_property;
} PicmanPropNumberPairEntryData;

static void
picman_prop_number_pair_entry_data_free (PicmanPropNumberPairEntryData *data)
{
  g_slice_free (PicmanPropNumberPairEntryData, data);
}


static void  picman_prop_number_pair_entry_config_notify   (GObject                     *config,
                                                          GParamSpec                  *param_spec,
                                                          GtkEntry                    *entry);
static void  picman_prop_number_pair_entry_number_pair_numbers_changed
                                                         (GtkWidget                   *widget,
                                                          PicmanPropNumberPairEntryData *data);
static void  picman_prop_number_pair_entry_number_pair_user_override_notify
                                                         (GtkWidget                   *entry,
                                                          GParamSpec                  *param_spec,
                                                          PicmanPropNumberPairEntryData *data);


/**
 * picman_prop_number_pair_entry_new:
 * @config:                        Object to which properties are attached.
 * @left_number_property:          Name of double property for left number.
 * @right_number_property:         Name of double property for right number.
 * @default_left_number_property:  Name of double property for default left number.
 * @default_right_number_property: Name of double property for default right number.
 * @user_override_property:        Name of boolean property for user override mode.
 * @connect_numbers_changed:       %TRUE to connect to the widgets "numbers-changed"
 *                                 signal, %FALSE to not connect.
 * @connect_ratio_changed:         %TRUE to connect to the widgets "ratio-changed"
 *                                 signal, %FALSE to not connect.
 * @separators:
 * @allow_simplification:
 * @min_valid_value:
 * @max_valid_value:         What to pass to picman_number_pair_entry_new ().
 *
 * Return value: A #PicmanNumberPairEntry widget.
 */
GtkWidget *
picman_prop_number_pair_entry_new (GObject     *config,
                                 const gchar *left_number_property,
                                 const gchar *right_number_property,
                                 const gchar *default_left_number_property,
                                 const gchar *default_right_number_property,
                                 const gchar *user_override_property,
                                 gboolean     connect_numbers_changed,
                                 gboolean     connect_ratio_changed,
                                 const gchar *separators,
                                 gboolean     allow_simplification,
                                 gdouble      min_valid_value,
                                 gdouble      max_valid_value)
{
  PicmanPropNumberPairEntryData *data;
  GtkWidget                   *number_pair_entry;
  gdouble                      left_number;
  gdouble                      right_number;
  gdouble                      default_left_number;
  gdouble                      default_right_number;
  gboolean                     user_override;


  /* Setup config data */

  data = g_slice_new (PicmanPropNumberPairEntryData);

  data->config                        = config;
  data->left_number_property          = left_number_property;
  data->right_number_property         = right_number_property;
  data->default_left_number_property  = default_left_number_property;
  data->default_right_number_property = default_right_number_property;
  data->user_override_property        = user_override_property;


  /* Read current values of config properties */

  g_object_get (config,
                left_number_property,          &left_number,
                right_number_property,         &right_number,
                default_left_number_property,  &default_left_number,
                default_right_number_property, &default_right_number,
                user_override_property,        &user_override,
                NULL);


  /* Create a PicmanNumberPairEntry and setup with config property values */

  number_pair_entry = picman_number_pair_entry_new (separators,
                                                  allow_simplification,
                                                  min_valid_value,
                                                  max_valid_value);

  g_object_set_data_full (G_OBJECT (number_pair_entry),
                          "picman-prop-number-pair-entry-data", data,
                          (GDestroyNotify) picman_prop_number_pair_entry_data_free);

  gtk_entry_set_width_chars (GTK_ENTRY (number_pair_entry), 7);

  picman_number_pair_entry_set_user_override  (PICMAN_NUMBER_PAIR_ENTRY (number_pair_entry),
                                             user_override);
  picman_number_pair_entry_set_values         (PICMAN_NUMBER_PAIR_ENTRY (number_pair_entry),
                                             left_number,
                                             right_number);
  picman_number_pair_entry_set_default_values (PICMAN_NUMBER_PAIR_ENTRY (number_pair_entry),
                                             default_left_number,
                                             default_right_number);


  /* Connect to PicmanNumberPairEntry signals */

  if (connect_ratio_changed)
    g_signal_connect (number_pair_entry, "ratio-changed",
                      G_CALLBACK (picman_prop_number_pair_entry_number_pair_numbers_changed),
                      data);

  if (connect_numbers_changed)
    g_signal_connect (number_pair_entry, "numbers-changed",
                      G_CALLBACK (picman_prop_number_pair_entry_number_pair_numbers_changed),
                      data);

  g_signal_connect (number_pair_entry, "notify::user-override",
                    G_CALLBACK (picman_prop_number_pair_entry_number_pair_user_override_notify),
                    data);


  /* Connect to connfig object signals */

  connect_notify (config, left_number_property,
                  G_CALLBACK (picman_prop_number_pair_entry_config_notify),
                  number_pair_entry);
  connect_notify (config, right_number_property,
                  G_CALLBACK (picman_prop_number_pair_entry_config_notify),
                  number_pair_entry);
  connect_notify (config, default_left_number_property,
                  G_CALLBACK (picman_prop_number_pair_entry_config_notify),
                  number_pair_entry);
  connect_notify (config, default_right_number_property,
                  G_CALLBACK (picman_prop_number_pair_entry_config_notify),
                  number_pair_entry);
  connect_notify (config, user_override_property,
                  G_CALLBACK (picman_prop_number_pair_entry_config_notify),
                  number_pair_entry);


  /* Done */

  return number_pair_entry;
}

static void
picman_prop_number_pair_entry_config_notify (GObject    *config,
                                           GParamSpec *param_spec,
                                           GtkEntry   *number_pair_entry)
{
  PicmanPropNumberPairEntryData *data =
    g_object_get_data (G_OBJECT (number_pair_entry),
                       "picman-prop-number-pair-entry-data");

  g_return_if_fail (data != NULL);

  if (strcmp (param_spec->name, data->left_number_property)  == 0 ||
      strcmp (param_spec->name, data->right_number_property) == 0)
    {
      gdouble left_number;
      gdouble right_number;

      g_object_get (config,
                    data->left_number_property,  &left_number,
                    data->right_number_property, &right_number,
                    NULL);

      picman_number_pair_entry_set_values (PICMAN_NUMBER_PAIR_ENTRY (number_pair_entry),
                                         left_number,
                                         right_number);
    }
  else if (strcmp (param_spec->name, data->default_left_number_property)  == 0 ||
           strcmp (param_spec->name, data->default_right_number_property) == 0)
    {
      gdouble default_left_number;
      gdouble default_right_number;

      g_object_get (config,
                    data->default_left_number_property,  &default_left_number,
                    data->default_right_number_property, &default_right_number,
                    NULL);

      picman_number_pair_entry_set_default_values (PICMAN_NUMBER_PAIR_ENTRY (number_pair_entry),
                                                 default_left_number,
                                                 default_right_number);
    }
  else if (strcmp (param_spec->name, data->user_override_property)  == 0)
    {
      gboolean user_override;

      g_object_get (config,
                    data->user_override_property, &user_override,
                    NULL);

      picman_number_pair_entry_set_user_override (PICMAN_NUMBER_PAIR_ENTRY (number_pair_entry),
                                                user_override);
    }
}

static void
picman_prop_number_pair_entry_number_pair_numbers_changed (GtkWidget                   *widget,
                                                         PicmanPropNumberPairEntryData *data)
{
  gdouble left_number;
  gdouble right_number;

  picman_number_pair_entry_get_values (PICMAN_NUMBER_PAIR_ENTRY (widget),
                                     &left_number,
                                     &right_number);

  g_object_set (data->config,
                data->left_number_property,  left_number,
                data->right_number_property, right_number,
                NULL);
}

static void
picman_prop_number_pair_entry_number_pair_user_override_notify (GtkWidget                   *entry,
                                                              GParamSpec                  *param_spec,
                                                              PicmanPropNumberPairEntryData *data)

{
  gboolean old_config_user_override;
  gboolean new_config_user_override;

  g_object_get (data->config,
                data->user_override_property, &old_config_user_override,
                NULL);

  new_config_user_override =
    picman_number_pair_entry_get_user_override (PICMAN_NUMBER_PAIR_ENTRY (entry));

  /* Only set when property changed, to avoid deadlocks */
  if (new_config_user_override != old_config_user_override)
    g_object_set (data->config,
                  data->user_override_property, new_config_user_override,
                  NULL);
}


/************************/
/*  language combo-box  */
/************************/

static void   picman_prop_language_combo_box_callback (GtkWidget  *combo,
                                                     GObject    *config);
static void   picman_prop_language_combo_box_notify   (GObject    *config,
                                                     GParamSpec *param_spec,
                                                     GtkWidget  *combo);

GtkWidget *
picman_prop_language_combo_box_new (GObject     *config,
                                  const gchar *property_name)
{
  GParamSpec *param_spec;
  GtkWidget  *combo;
  gchar      *value;

  param_spec = check_param_spec_w (config, property_name,
                                   G_TYPE_PARAM_STRING, G_STRFUNC);
  if (! param_spec)
    return NULL;

  combo = picman_language_combo_box_new ();

  g_object_get (config,
                property_name, &value,
                NULL);

  picman_language_combo_box_set_code (PICMAN_LANGUAGE_COMBO_BOX (combo), value);
  g_free (value);

  set_param_spec (G_OBJECT (combo), combo, param_spec);

  g_signal_connect (combo, "changed",
                    G_CALLBACK (picman_prop_language_combo_box_callback),
                    config);

  connect_notify (config, property_name,
                  G_CALLBACK (picman_prop_language_combo_box_notify),
                  combo);

  return combo;
}

static void
picman_prop_language_combo_box_callback (GtkWidget *combo,
                                       GObject   *config)
{
  GParamSpec *param_spec;
  gchar      *code;

  param_spec = get_param_spec (G_OBJECT (combo));
  if (! param_spec)
    return;

  code = picman_language_combo_box_get_code (PICMAN_LANGUAGE_COMBO_BOX (combo));

  g_signal_handlers_block_by_func (config,
                                   picman_prop_language_combo_box_notify,
                                   combo);

  g_object_set (config,
                param_spec->name, code,
                NULL);

  g_signal_handlers_unblock_by_func (config,
                                     picman_prop_language_combo_box_notify,
                                     combo);

  g_free (code);
}

static void
picman_prop_language_combo_box_notify (GObject    *config,
                                     GParamSpec *param_spec,
                                     GtkWidget  *combo)
{
  gchar *value;

  g_object_get (config,
                param_spec->name, &value,
                NULL);

  g_signal_handlers_block_by_func (combo,
                                   picman_prop_language_combo_box_callback,
                                   config);

  picman_language_combo_box_set_code (PICMAN_LANGUAGE_COMBO_BOX (combo), value);

  g_signal_handlers_unblock_by_func (combo,
                                     picman_prop_language_combo_box_callback,
                                     config);

  g_free (value);
}


/********************/
/*  language entry  */
/********************/

static void   picman_prop_language_entry_callback (GtkWidget  *entry,
                                                 GObject    *config);
static void   picman_prop_language_entry_notify   (GObject    *config,
                                                 GParamSpec *param_spec,
                                                 GtkWidget  *entry);

GtkWidget *
picman_prop_language_entry_new (GObject     *config,
                              const gchar *property_name)
{
  GParamSpec *param_spec;
  GtkWidget  *entry;
  gchar      *value;

  param_spec = check_param_spec_w (config, property_name,
                                   G_TYPE_PARAM_STRING, G_STRFUNC);
  if (! param_spec)
    return NULL;

  entry = picman_language_entry_new ();

  g_object_get (config,
                property_name, &value,
                NULL);

  picman_language_entry_set_code (PICMAN_LANGUAGE_ENTRY (entry), value);
  g_free (value);

  set_param_spec (G_OBJECT (entry), entry, param_spec);

  g_signal_connect (entry, "changed",
                    G_CALLBACK (picman_prop_language_entry_callback),
                    config);

  connect_notify (config, property_name,
                  G_CALLBACK (picman_prop_language_entry_notify),
                  entry);

  return entry;
}

static void
picman_prop_language_entry_callback (GtkWidget *entry,
                                   GObject   *config)
{
  GParamSpec  *param_spec;
  const gchar *code;

  param_spec = get_param_spec (G_OBJECT (entry));
  if (! param_spec)
    return;

  code = picman_language_entry_get_code (PICMAN_LANGUAGE_ENTRY (entry));

  g_signal_handlers_block_by_func (config,
                                   picman_prop_language_entry_notify,
                                   entry);

  g_object_set (config,
                param_spec->name, code,
                NULL);

  g_signal_handlers_unblock_by_func (config,
                                     picman_prop_language_entry_notify,
                                     entry);
}

static void
picman_prop_language_entry_notify (GObject    *config,
                                 GParamSpec *param_spec,
                                 GtkWidget  *entry)
{
  gchar *value;

  g_object_get (config,
                param_spec->name, &value,
                NULL);

  g_signal_handlers_block_by_func (entry,
                                   picman_prop_language_entry_callback,
                                   config);

  picman_language_entry_set_code (PICMAN_LANGUAGE_ENTRY (entry), value);

  g_signal_handlers_unblock_by_func (entry,
                                     picman_prop_language_entry_callback,
                                     config);

  g_free (value);
}


/*****************/
/*  icon picker  */
/*****************/

static void   picman_prop_icon_picker_callback (GtkWidget  *picker,
                                              GParamSpec *param_spec,
                                              GObject    *config);
static void   picman_prop_icon_picker_notify   (GObject    *config,
                                              GParamSpec *param_spec,
                                              GtkWidget  *picker);

GtkWidget *
picman_prop_icon_picker_new (PicmanViewable *viewable,
                           Picman         *picman)
{
  GObject     *object         = G_OBJECT (viewable);
  GtkWidget   *picker         = NULL;
  GdkPixbuf   *pixbuf_value   = NULL;
  gchar       *stock_id_value = NULL;

  picker = picman_icon_picker_new (picman);

  g_object_get (object,
                "stock-id", &stock_id_value,
                "icon-pixbuf", &pixbuf_value,
                NULL);

  picman_icon_picker_set_stock_id (PICMAN_ICON_PICKER (picker), stock_id_value);
  picman_icon_picker_set_icon_pixbuf (PICMAN_ICON_PICKER (picker), pixbuf_value);

  g_signal_connect (picker, "notify::icon-pixbuf",
                    G_CALLBACK (picman_prop_icon_picker_callback),
                    object);

  g_signal_connect (picker, "notify::stock-id",
                    G_CALLBACK (picman_prop_icon_picker_callback),
                    object);

  connect_notify (object, "stock-id",
                  G_CALLBACK (picman_prop_icon_picker_notify),
                  picker);

  connect_notify (object, "icon-pixbuf",
                  G_CALLBACK (picman_prop_icon_picker_notify),
                  picker);

  if (stock_id_value)
    g_free (stock_id_value);
  if (pixbuf_value)
    g_object_unref (pixbuf_value);

  return picker;
}

static void
picman_prop_icon_picker_callback (GtkWidget  *picker,
                                GParamSpec *param_spec,
                                GObject    *config)
{
  g_signal_handlers_block_by_func (config,
                                   picman_prop_icon_picker_notify,
                                   picker);

  if (! strcmp (param_spec->name, "stock-id"))
    {
      const gchar *value = picman_icon_picker_get_stock_id (PICMAN_ICON_PICKER (picker));

      g_object_set (config,
                    "stock-id", value,
                    NULL);

    }
  else if (! strcmp (param_spec->name, "icon-pixbuf"))
    {
      GdkPixbuf *value = picman_icon_picker_get_icon_pixbuf (PICMAN_ICON_PICKER (picker));

      g_object_set (config,
                    "icon-pixbuf", value,
                    NULL);
    }


  g_signal_handlers_unblock_by_func (config,
                                     picman_prop_icon_picker_notify,
                                     picker);
}

static void
picman_prop_icon_picker_notify (GObject    *config,
                              GParamSpec *param_spec,
                              GtkWidget  *picker)
{
  g_signal_handlers_block_by_func (picker,
                                   picman_prop_icon_picker_callback,
                                   config);

  if (!strcmp (param_spec->name, "stock-id"))
    {
      gchar *value = NULL;

      g_object_get (config,
                    "stock-id", &value,
                    NULL);

      picman_icon_picker_set_stock_id (PICMAN_ICON_PICKER (picker), value);

      if (value)
        g_free (value);
    }
  else if (!strcmp (param_spec->name, "icon-pixbuf"))
    {
      GdkPixbuf *value = NULL;

      g_object_get (config,
                    "icon-pixbuf", &value,
                    NULL);

      picman_icon_picker_set_icon_pixbuf (PICMAN_ICON_PICKER (picker), value);

      if (value)
        g_object_unref (value);
    }

  g_signal_handlers_unblock_by_func (picker,
                                     picman_prop_icon_picker_callback,
                                     config);
}


/***********/
/*  table  */
/***********/

static void
picman_prop_table_chain_toggled (PicmanChainButton *chain,
                               GtkAdjustment   *x_adj)
{
  GtkAdjustment *y_adj;

  y_adj = g_object_get_data (G_OBJECT (x_adj), "y-adjustment");

  if (picman_chain_button_get_active (chain))
    {
      GBinding *binding;

      binding = g_object_bind_property (x_adj, "value",
                                        y_adj,   "value",
                                        G_BINDING_BIDIRECTIONAL);

      g_object_set_data (G_OBJECT (chain), "binding", binding);
    }
  else
    {
      GBinding *binding;

      binding = g_object_get_data (G_OBJECT (chain), "binding");

      g_object_unref (binding);
      g_object_set_data (G_OBJECT (chain), "binding", NULL);
    }
}

static void
picman_prop_table_new_seed_clicked (GtkButton     *button,
                                  GtkAdjustment *adj)
{
  guint32 value = g_random_int_range (gtk_adjustment_get_lower (adj),
                                      gtk_adjustment_get_upper (adj));

  gtk_adjustment_set_value (adj, value);
}

GtkWidget *
picman_prop_table_new (GObject              *config,
                     GType                 owner_type,
                     PicmanContext          *context,
                     PicmanCreatePickerFunc  create_picker_func,
                     gpointer              picker_creator)
{
  GtkWidget     *table;
  GtkSizeGroup  *size_group;
  GParamSpec   **param_specs;
  guint          n_param_specs;
  gint           i;
  gint           row = 0;
  GtkAdjustment *last_x_adj = NULL;
  gint           last_x_row = 0;

  g_return_val_if_fail (G_IS_OBJECT (config), NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);

  param_specs = g_object_class_list_properties (G_OBJECT_GET_CLASS (config),
                                                &n_param_specs);

  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  table = gtk_table_new (3, 1, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);

  for (i = 0; i < n_param_specs; i++)
    {
      GParamSpec  *pspec  = param_specs[i];
      GtkWidget   *widget = NULL;
      const gchar *label  = NULL;

      /*  ignore properties of parent classes of owner_type  */
      if (! g_type_is_a (pspec->owner_type, owner_type))
        continue;

      if (G_IS_PARAM_SPEC_STRING (pspec))
        {
          static GQuark multiline_quark = 0;

          if (! multiline_quark)
            multiline_quark = g_quark_from_static_string ("multiline");

          if (PICMAN_IS_PARAM_SPEC_CONFIG_PATH (pspec))
            {
              widget = picman_prop_file_chooser_button_new (config,
                                                          pspec->name,
                                                          g_param_spec_get_nick (pspec),
                                                          GTK_FILE_CHOOSER_ACTION_OPEN);
            }
          else if (g_param_spec_get_qdata (pspec, multiline_quark))
            {
              GtkTextBuffer *buffer;
              GtkWidget     *view;

              buffer = picman_prop_text_buffer_new (config, pspec->name, -1);
              view = gtk_text_view_new_with_buffer (buffer);

              widget = gtk_scrolled_window_new (NULL, NULL);
              gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget),
                                                   GTK_SHADOW_IN);
              gtk_container_add (GTK_CONTAINER (widget), view);
              gtk_widget_show (view);
            }
          else
            {
              widget = picman_prop_entry_new (config, pspec->name, -1);
            }

          label  = g_param_spec_get_nick (pspec);
        }
      else if (G_IS_PARAM_SPEC_BOOLEAN (pspec))
        {
          widget = picman_prop_check_button_new (config, pspec->name,
                                               g_param_spec_get_nick (pspec));
        }
      else if (G_IS_PARAM_SPEC_ENUM (pspec))
        {
          widget = picman_prop_enum_combo_box_new (config, pspec->name, 0, 0);
          label = g_param_spec_get_nick (pspec);
        }
      else if (GEGL_IS_PARAM_SPEC_SEED (pspec))
        {
          GtkAdjustment *adj;
          GtkWidget     *scale;
          GtkWidget     *button;

          widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);

          scale = picman_prop_spin_scale_new (config, pspec->name,
                                            g_param_spec_get_nick (pspec),
                                            1.0, 1.0, 0);
          gtk_box_pack_start (GTK_BOX (widget), scale, TRUE, TRUE, 0);
          gtk_widget_show (scale);

          button = gtk_button_new_with_label (_("New Seed"));
          gtk_box_pack_start (GTK_BOX (widget), button, FALSE, FALSE, 0);
          gtk_widget_show (button);

          adj = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (scale));

          g_signal_connect (button, "clicked",
                            G_CALLBACK (picman_prop_table_new_seed_clicked),
                            adj);
        }
      else if (G_IS_PARAM_SPEC_INT (pspec)   ||
               G_IS_PARAM_SPEC_UINT (pspec)  ||
               G_IS_PARAM_SPEC_FLOAT (pspec) ||
               G_IS_PARAM_SPEC_DOUBLE (pspec))
        {
          GtkAdjustment *adj;
          gint           digits = (G_IS_PARAM_SPEC_FLOAT (pspec) ||
                                   G_IS_PARAM_SPEC_DOUBLE (pspec)) ? 2 : 0;

          widget = picman_prop_spin_scale_new (config, pspec->name,
                                             g_param_spec_get_nick (pspec),
                                             1.0, 1.0, digits);

          adj = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget));

          if (g_str_has_suffix (pspec->name, "x") ||
              g_str_has_suffix (pspec->name, "width"))
            {
              last_x_adj = adj;
              last_x_row = row;
            }
          else if ((g_str_has_suffix (pspec->name, "y") ||
                    g_str_has_suffix (pspec->name, "height")) &&
                   last_x_adj != NULL &&
                   last_x_row == row - 1)
            {
              GtkWidget *chain = picman_chain_button_new (PICMAN_CHAIN_RIGHT);

              gtk_table_attach (GTK_TABLE (table), chain,
                                3, 4, last_x_row, row + 1,
                                GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL,
                                0, 0);
              gtk_widget_show (chain);

              if (gtk_adjustment_get_value (last_x_adj) ==
                  gtk_adjustment_get_value (adj))
                {
                  GBinding *binding;

                  picman_chain_button_set_active (PICMAN_CHAIN_BUTTON (chain), TRUE);

                  binding = g_object_bind_property (last_x_adj, "value",
                                                    adj,        "value",
                                                    G_BINDING_BIDIRECTIONAL);

                  g_object_set_data (G_OBJECT (chain), "binding", binding);
                }

              g_signal_connect (chain, "toggled",
                                G_CALLBACK (picman_prop_table_chain_toggled),
                                last_x_adj);

              g_object_set_data (G_OBJECT (last_x_adj), "y-adjustment", adj);
            }
        }
      else if (PICMAN_IS_PARAM_SPEC_RGB (pspec))
        {
          GtkWidget *button;

          widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);

          button = picman_prop_color_button_new (config, pspec->name,
                                               g_param_spec_get_nick (pspec),
                                               128, 24,
                                               PICMAN_COLOR_AREA_SMALL_CHECKS);
          picman_color_button_set_update (PICMAN_COLOR_BUTTON (button), TRUE);
          picman_color_panel_set_context (PICMAN_COLOR_PANEL (button), context);
          gtk_box_pack_start (GTK_BOX (widget), button, TRUE, TRUE, 0);
          gtk_widget_show (button);

          button = create_picker_func (picker_creator,
                                       pspec->name,
                                       PICMAN_STOCK_COLOR_PICKER_GRAY,
                                       _("Pick color from image"));
          gtk_box_pack_start (GTK_BOX (widget), button, FALSE, FALSE, 0);
          gtk_widget_show (button);

          label = g_param_spec_get_nick (pspec);
        }
      else
        {
          g_warning ("%s: not supported: %s (%s)\n", G_STRFUNC,
                     g_type_name (G_TYPE_FROM_INSTANCE (pspec)), pspec->name);
        }

      if (widget)
        {
          if (label)
            {
              picman_table_attach_aligned (GTK_TABLE (table), 0, row,
                                         label, 0.0, 0.5,
                                         widget, 2, FALSE);
            }
          else
            {
              gtk_table_attach_defaults (GTK_TABLE (table), widget,
                                         0, 3, row, row + 1);
              gtk_widget_show (widget);
            }

          row++;
        }
    }

  g_object_unref (size_group);

  g_free (param_specs);

  return table;
}


/*******************************/
/*  private utility functions  */
/*******************************/

static GQuark picman_prop_widgets_param_spec_quark (void) G_GNUC_CONST;

#define PARAM_SPEC_QUARK (picman_prop_widgets_param_spec_quark ())

static GQuark
picman_prop_widgets_param_spec_quark (void)
{
  static GQuark param_spec_quark = 0;

  if (! param_spec_quark)
    param_spec_quark = g_quark_from_static_string ("picman-config-param-spec");

  return param_spec_quark;
}

static void
set_param_spec (GObject     *object,
                GtkWidget   *widget,
                GParamSpec  *param_spec)
{
  if (object)
    {
      g_object_set_qdata (object, PARAM_SPEC_QUARK, param_spec);
    }

  if (widget)
    {
      const gchar *blurb = g_param_spec_get_blurb (param_spec);

      if (blurb)
        picman_help_set_help_data (widget, gettext (blurb), NULL);
    }
}

static GParamSpec *
get_param_spec (GObject *object)
{
  return g_object_get_qdata (object, PARAM_SPEC_QUARK);
}

static GParamSpec *
find_param_spec (GObject     *object,
                 const gchar *property_name,
                 const gchar *strloc)
{
  GParamSpec *param_spec;

  param_spec = g_object_class_find_property (G_OBJECT_GET_CLASS (object),
                                             property_name);

  if (! param_spec)
    g_warning ("%s: %s has no property named '%s'",
               strloc,
               g_type_name (G_TYPE_FROM_INSTANCE (object)),
               property_name);

  return param_spec;
}

static GParamSpec *
check_param_spec (GObject     *object,
                  const gchar *property_name,
                  GType        type,
                  const gchar *strloc)
{
  GParamSpec *param_spec;

  param_spec = find_param_spec (object, property_name, strloc);

  if (param_spec && ! g_type_is_a (G_TYPE_FROM_INSTANCE (param_spec), type))
    {
      g_warning ("%s: property '%s' of %s is not a %s",
                 strloc,
                 param_spec->name,
                 g_type_name (param_spec->owner_type),
                 g_type_name (type));
      return NULL;
    }

  return param_spec;
}

static GParamSpec *
check_param_spec_w (GObject     *object,
                    const gchar *property_name,
                    GType        type,
                    const gchar *strloc)
{
  GParamSpec *param_spec;

  param_spec = check_param_spec (object, property_name, type, strloc);

  if (param_spec &&
      (param_spec->flags & G_PARAM_WRITABLE) == 0)
    {
      g_warning ("%s: property '%s' of %s is writable",
                 strloc,
                 param_spec->name,
                 g_type_name (param_spec->owner_type));
      return NULL;
    }

  return param_spec;
}

static gboolean
get_numeric_values (GObject     *object,
                    GParamSpec  *param_spec,
                    gdouble     *value,
                    gdouble     *lower,
                    gdouble     *upper,
                    const gchar *strloc)
{
  if (G_IS_PARAM_SPEC_INT (param_spec))
    {
      GParamSpecInt *int_spec = G_PARAM_SPEC_INT (param_spec);
      gint           int_value;

      g_object_get (object, param_spec->name, &int_value, NULL);

      *value = int_value;
      *lower = int_spec->minimum;
      *upper = int_spec->maximum;
    }
  else if (G_IS_PARAM_SPEC_UINT (param_spec))
    {
      GParamSpecUInt *uint_spec = G_PARAM_SPEC_UINT (param_spec);
      guint           uint_value;

      g_object_get (object, param_spec->name, &uint_value, NULL);

      *value = uint_value;
      *lower = uint_spec->minimum;
      *upper = uint_spec->maximum;
    }
  else if (G_IS_PARAM_SPEC_DOUBLE (param_spec))
    {
      GParamSpecDouble *double_spec = G_PARAM_SPEC_DOUBLE (param_spec);

      g_object_get (object, param_spec->name, value, NULL);

      *lower = double_spec->minimum;
      *upper = double_spec->maximum;
    }
  else
    {
      g_warning ("%s: property '%s' of %s is not numeric",
                 strloc,
                 param_spec->name,
                 g_type_name (G_TYPE_FROM_INSTANCE (object)));
      return FALSE;
    }

  return TRUE;
}

static void
connect_notify (GObject     *config,
                const gchar *property_name,
                GCallback    callback,
                gpointer     callback_data)
{
  gchar *notify_name;

  notify_name = g_strconcat ("notify::", property_name, NULL);

  g_signal_connect_object (config, notify_name, callback, callback_data, 0);

  g_free (notify_name);
}
