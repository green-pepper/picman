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

#ifndef __PICMAN_RECTANGLE_OPTIONS_H__
#define __PICMAN_RECTANGLE_OPTIONS_H__


typedef enum
{
  PICMAN_RECTANGLE_OPTIONS_PROP_0,

  PICMAN_RECTANGLE_OPTIONS_PROP_AUTO_SHRINK,
  PICMAN_RECTANGLE_OPTIONS_PROP_SHRINK_MERGED,
  PICMAN_RECTANGLE_OPTIONS_PROP_HIGHLIGHT,
  PICMAN_RECTANGLE_OPTIONS_PROP_GUIDE,

  PICMAN_RECTANGLE_OPTIONS_PROP_X,
  PICMAN_RECTANGLE_OPTIONS_PROP_Y,
  PICMAN_RECTANGLE_OPTIONS_PROP_WIDTH,
  PICMAN_RECTANGLE_OPTIONS_PROP_HEIGHT,
  PICMAN_RECTANGLE_OPTIONS_PROP_POSITION_UNIT,
  PICMAN_RECTANGLE_OPTIONS_PROP_SIZE_UNIT,

  PICMAN_RECTANGLE_OPTIONS_PROP_FIXED_RULE_ACTIVE,
  PICMAN_RECTANGLE_OPTIONS_PROP_FIXED_RULE,
  PICMAN_RECTANGLE_OPTIONS_PROP_DESIRED_FIXED_WIDTH,
  PICMAN_RECTANGLE_OPTIONS_PROP_DESIRED_FIXED_HEIGHT,
  PICMAN_RECTANGLE_OPTIONS_PROP_DESIRED_FIXED_SIZE_WIDTH,
  PICMAN_RECTANGLE_OPTIONS_PROP_DESIRED_FIXED_SIZE_HEIGHT,
  PICMAN_RECTANGLE_OPTIONS_PROP_DEFAULT_FIXED_SIZE_WIDTH,
  PICMAN_RECTANGLE_OPTIONS_PROP_DEFAULT_FIXED_SIZE_HEIGHT,
  PICMAN_RECTANGLE_OPTIONS_PROP_OVERRIDDEN_FIXED_SIZE,
  PICMAN_RECTANGLE_OPTIONS_PROP_ASPECT_NUMERATOR,
  PICMAN_RECTANGLE_OPTIONS_PROP_ASPECT_DENOMINATOR,
  PICMAN_RECTANGLE_OPTIONS_PROP_DEFAULT_ASPECT_NUMERATOR,
  PICMAN_RECTANGLE_OPTIONS_PROP_DEFAULT_ASPECT_DENOMINATOR,
  PICMAN_RECTANGLE_OPTIONS_PROP_OVERRIDDEN_FIXED_ASPECT,
  PICMAN_RECTANGLE_OPTIONS_PROP_USE_STRING_CURRENT,
  PICMAN_RECTANGLE_OPTIONS_PROP_FIXED_UNIT,
  PICMAN_RECTANGLE_OPTIONS_PROP_FIXED_CENTER,

  PICMAN_RECTANGLE_OPTIONS_PROP_LAST = PICMAN_RECTANGLE_OPTIONS_PROP_FIXED_CENTER
} PicmanRectangleOptionsProp;


#define PICMAN_TYPE_RECTANGLE_OPTIONS               (picman_rectangle_options_interface_get_type ())
#define PICMAN_IS_RECTANGLE_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_RECTANGLE_OPTIONS))
#define PICMAN_RECTANGLE_OPTIONS(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_RECTANGLE_OPTIONS, PicmanRectangleOptions))
#define PICMAN_RECTANGLE_OPTIONS_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_RECTANGLE_OPTIONS, PicmanRectangleOptionsInterface))

#define PICMAN_RECTANGLE_OPTIONS_GET_PRIVATE(obj)   (picman_rectangle_options_get_private (PICMAN_RECTANGLE_OPTIONS (obj)))


typedef struct _PicmanRectangleOptions          PicmanRectangleOptions;
typedef struct _PicmanRectangleOptionsInterface PicmanRectangleOptionsInterface;
typedef struct _PicmanRectangleOptionsPrivate   PicmanRectangleOptionsPrivate;

struct _PicmanRectangleOptionsInterface
{
  GTypeInterface base_iface;
};

struct _PicmanRectangleOptionsPrivate
{
  gboolean                    auto_shrink;
  gboolean                    shrink_merged;

  gboolean                    highlight;
  PicmanGuidesType              guide;

  gdouble                     x;
  gdouble                     y;
  gdouble                     width;
  gdouble                     height;

  PicmanUnit                    position_unit;
  PicmanUnit                    size_unit;

  gboolean                    fixed_rule_active;
  PicmanRectangleToolFixedRule  fixed_rule;

  gdouble                     desired_fixed_width;
  gdouble                     desired_fixed_height;

  gdouble                     desired_fixed_size_width;
  gdouble                     desired_fixed_size_height;

  gdouble                     default_fixed_size_width;
  gdouble                     default_fixed_size_height;
  gboolean                    overridden_fixed_size;

  gdouble                     aspect_numerator;
  gdouble                     aspect_denominator;

  gdouble                     default_aspect_numerator;
  gdouble                     default_aspect_denominator;
  gboolean                    overridden_fixed_aspect;

  gboolean                    fixed_center;

  /* This gboolean is not part of the actual rectangle tool options,
   * and should be refactored out along with the pointers to widgets.
   */
  gboolean                    use_string_current;

  PicmanUnit                    fixed_unit;

  /* options gui */

  GtkWidget                  *auto_shrink_button;

  GtkWidget                  *fixed_width_entry;
  GtkWidget                  *fixed_height_entry;

  GtkWidget                  *fixed_aspect_hbox;
  GtkWidget                  *aspect_button_box;
  GtkListStore               *aspect_history;

  GtkWidget                  *fixed_size_hbox;
  GtkWidget                  *size_button_box;
  GtkListStore               *size_history;

  GtkWidget                  *x_entry;
  GtkWidget                  *y_entry;
  GtkWidget                  *width_entry;
  GtkWidget                  *height_entry;
};


GType       picman_rectangle_options_interface_get_type (void) G_GNUC_CONST;

GtkWidget * picman_rectangle_options_gui                (PicmanToolOptions      *tool_options);

gboolean    picman_rectangle_options_fixed_rule_active  (PicmanRectangleOptions *rectangle_options,
                                                       PicmanRectangleToolFixedRule fixed_rule);

PicmanRectangleOptionsPrivate *
            picman_rectangle_options_get_private        (PicmanRectangleOptions *options);


/*  convenience functions  */

void        picman_rectangle_options_install_properties (GObjectClass *klass);
void        picman_rectangle_options_set_property       (GObject      *object,
                                                       guint         property_id,
                                                       const GValue *value,
                                                       GParamSpec   *pspec);
void        picman_rectangle_options_get_property       (GObject      *object,
                                                       guint         property_id,
                                                       GValue       *value,
                                                       GParamSpec   *pspec);


/*  testing helper functions  */

GtkWidget * picman_rectangle_options_get_width_entry    (PicmanRectangleOptions *rectangle_options);


#endif  /* __PICMAN_RECTANGLE_OPTIONS_H__ */
