/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpropwidgets.h
 * Copyright (C) 2002 Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_PROP_WIDGETS_H__
#define __PICMAN_PROP_WIDGETS_H__

G_BEGIN_DECLS


/*  GParamBoolean  */

GtkWidget     * picman_prop_check_button_new        (GObject      *config,
                                                   const gchar  *property_name,
                                                   const gchar  *label);
GtkWidget     * picman_prop_boolean_combo_box_new   (GObject      *config,
                                                   const gchar  *property_name,
                                                   const gchar  *true_text,
                                                   const gchar  *false_text);
GtkWidget     * picman_prop_boolean_radio_frame_new (GObject      *config,
                                                   const gchar  *property_name,
                                                   const gchar  *title,
                                                   const gchar  *true_text,
                                                   const gchar  *false_text);

GtkWidget     * picman_prop_expander_new            (GObject      *config,
                                                   const gchar  *property_name,
                                                   const gchar  *label);


/*  GParamInt  */

GtkWidget     * picman_prop_int_combo_box_new       (GObject      *config,
                                                   const gchar  *property_name,
                                                   PicmanIntStore *store);


/*  GParamEnum  */

GtkWidget     * picman_prop_enum_combo_box_new      (GObject      *config,
                                                   const gchar  *property_name,
                                                   gint          minimum,
                                                   gint          maximum);

GtkWidget     * picman_prop_enum_check_button_new   (GObject      *config,
                                                   const gchar  *property_name,
                                                   const gchar  *label,
                                                   gint          false_value,
                                                   gint          true_value);

GtkWidget     * picman_prop_enum_radio_frame_new    (GObject      *config,
                                                   const gchar  *property_name,
                                                   const gchar  *title,
                                                   gint          minimum,
                                                   gint          maximum);
GtkWidget     * picman_prop_enum_radio_box_new      (GObject      *config,
                                                   const gchar  *property_name,
                                                   gint          minimum,
                                                   gint          maximum);
GtkWidget     * picman_prop_enum_stock_box_new      (GObject      *config,
                                                   const gchar  *property_name,
                                                   const gchar  *stock_prefix,
                                                   gint          minimum,
                                                   gint          maximum);

GtkWidget     * picman_prop_enum_label_new          (GObject      *config,
                                                   const gchar  *property_name);


/*  GParamInt, GParamUInt, GParamLong, GParamULong, GParamDouble  */

GtkWidget     * picman_prop_spin_button_new         (GObject      *config,
                                                   const gchar  *property_name,
                                                   gdouble       step_increment,
                                                   gdouble       page_increment,
                                                   gint          digits);

GtkWidget     * picman_prop_hscale_new              (GObject      *config,
                                                   const gchar  *property_name,
                                                   gdouble       step_increment,
                                                   gdouble       page_increment,
                                                   gint          digits);

GtkObject     * picman_prop_scale_entry_new         (GObject      *config,
                                                   const gchar  *property_name,
                                                   GtkTable     *table,
                                                   gint          column,
                                                   gint          row,
                                                   const gchar  *label,
                                                   gdouble       step_increment,
                                                   gdouble       page_increment,
                                                   gint          digits,
                                                   gboolean      limit_scale,
                                                   gdouble       lower_limit,
                                                   gdouble       upper_limit);

/*  special form of picman_prop_scale_entry_new() for GParamDouble  */

GtkObject     * picman_prop_opacity_entry_new       (GObject       *config,
                                                   const gchar   *property_name,
                                                   GtkTable      *table,
                                                   gint           column,
                                                   gint           row,
                                                   const gchar   *label);


/*  PicmanParamMemsize  */

GtkWidget     * picman_prop_memsize_entry_new       (GObject       *config,
                                                   const gchar   *property_name);


/*  GParamString  */

GtkWidget     * picman_prop_label_new               (GObject       *config,
                                                   const gchar   *property_name);
GtkWidget     * picman_prop_entry_new               (GObject       *config,
                                                   const gchar   *property_name,
                                                   gint           max_len);
GtkTextBuffer * picman_prop_text_buffer_new         (GObject       *config,
                                                   const gchar   *property_name,
                                                   gint           max_len);
GtkWidget     * picman_prop_string_combo_box_new    (GObject       *config,
                                                   const gchar   *property_name,
                                                   GtkTreeModel  *model,
                                                   gint           id_column,
                                                   gint           label_column);


/*  PicmanParamPath  */

GtkWidget     * picman_prop_file_chooser_button_new (GObject              *config,
                                                   const gchar          *property_name,
                                                   const gchar          *title,
                                                   GtkFileChooserAction  action);
GtkWidget     * picman_prop_file_chooser_button_new_with_dialog (GObject     *config,
                                                               const gchar *property_name,

                                                               GtkWidget   *dialog);
GtkWidget     * picman_prop_path_editor_new         (GObject       *config,
                                                   const gchar   *path_property_name,
                                                   const gchar   *writable_property_name,
                                                   const gchar   *filesel_title);


/*  GParamInt, GParamUInt, GParamDouble   unit: PicmanParamUnit  */

GtkWidget     * picman_prop_size_entry_new          (GObject       *config,
                                                   const gchar   *property_name,
                                                   gboolean       property_is_pixel,
                                                   const gchar   *unit_property_name,
                                                   const gchar   *unit_format,
                                                   PicmanSizeEntryUpdatePolicy  update_policy,
                                                   gdouble        resolution);


/*  x,y: GParamInt, GParamDouble   unit: PicmanParamUnit  */

GtkWidget     * picman_prop_coordinates_new         (GObject       *config,
                                                   const gchar   *x_property_name,
                                                   const gchar   *y_property_name,
                                                   const gchar   *unit_property_name,
                                                   const gchar   *unit_format,
                                                   PicmanSizeEntryUpdatePolicy  update_policy,
                                                   gdouble        xresolution,
                                                   gdouble        yresolution,
                                                   gboolean       has_chainbutton);
gboolean        picman_prop_coordinates_connect     (GObject       *config,
                                                   const gchar   *x_property_name,
                                                   const gchar   *y_property_name,
                                                   const gchar   *unit_property_name,
                                                   GtkWidget     *sizeentry,
                                                   GtkWidget     *chainbutton,
                                                   gdouble        xresolution,
                                                   gdouble        yresolution);


/*  PicmanParamColor  */

GtkWidget     * picman_prop_color_area_new          (GObject       *config,
                                                   const gchar   *property_name,
                                                   gint           width,
                                                   gint           height,
                                                   PicmanColorAreaType  type);

/*  PicmanParamUnit  */

GtkWidget     * picman_prop_unit_combo_box_new      (GObject       *config,
                                                   const gchar   *property_name);
PICMAN_DEPRECATED_FOR(picman_prop_unit_combo_box_new)
GtkWidget     * picman_prop_unit_menu_new           (GObject       *config,
                                                   const gchar   *property_name,
                                                   const gchar   *unit_format);


/*  GParamString (stock_id)  */

GtkWidget     * picman_prop_stock_image_new         (GObject       *config,
                                                   const gchar   *property_name,
                                                   GtkIconSize    icon_size);


G_END_DECLS

#endif /* __PICMAN_PROP_WIDGETS_H__ */
