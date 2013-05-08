/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanpropwidgets.h
 * Copyright (C) 2002 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_APP_PROP_WIDGETS_H__
#define __PICMAN_APP_PROP_WIDGETS_H__


/*  GParamBoolean  */

GtkWidget * picman_prop_expanding_frame_new (GObject     *config,
                                           const gchar *property_name,
                                           const gchar *button_label,
                                           GtkWidget   *child,
                                           GtkWidget  **button);


/*  GParamEnum  */

GtkWidget * picman_prop_paint_mode_menu_new (GObject     *config,
                                           const gchar *property_name,
                                           gboolean     with_behind_mode,
                                           gboolean     with_replace_modes);


/*  PicmanParamColor  */

GtkWidget * picman_prop_color_button_new    (GObject     *config,
                                           const gchar *property_name,
                                           const gchar *title,
                                           gint         width,
                                           gint         height,
                                           PicmanColorAreaType  type);


/*  GParamDouble  */

GtkWidget * picman_prop_scale_button_new    (GObject     *config,
                                           const gchar *property_name);
GtkWidget * picman_prop_spin_scale_new      (GObject     *config,
                                           const gchar *property_name,
                                           const gchar *label,
                                           gdouble      step_increment,
                                           gdouble      page_increment,
                                           gint         digits);
GtkWidget * picman_prop_opacity_spin_scale_new (GObject     *config,
                                              const gchar *property_name,
                                              const gchar *label);


/*  GParamObject (PicmanViewable)  */

GtkWidget * picman_prop_view_new            (GObject     *config,
                                           const gchar *property_name,
                                           PicmanContext *context,
                                           gint         size);


/*  GParamDouble, GParamDouble, GParamDouble, GParamDouble, GParamBoolean  */

GtkWidget * picman_prop_number_pair_entry_new
                                          (GObject     *config,
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
                                           gdouble      max_valid_value);

/*  GParamString  */

GtkWidget * picman_prop_language_combo_box_new (GObject      *config,
                                              const gchar  *property_name);
GtkWidget * picman_prop_language_entry_new     (GObject      *config,
                                              const gchar  *property_name);

GtkWidget * picman_prop_icon_picker_new        (PicmanViewable *viewable,
                                              Picman         *picman);

/*  A view on all of an object's properties  */

typedef GtkWidget * (* PicmanCreatePickerFunc) (gpointer     creator,
                                              const gchar *property_name,
                                              const gchar *stock_id,
                                              const gchar *help_id);

GtkWidget * picman_prop_table_new (GObject              *config,
                                 GType                 owner_type,
                                 PicmanContext          *context,
                                 PicmanCreatePickerFunc  create_picker_fnc,
                                 gpointer              picker_creator);


#endif /* __PICMAN_APP_PROP_WIDGETS_H__ */
