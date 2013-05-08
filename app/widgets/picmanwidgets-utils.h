/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanwidgets-utils.h
 * Copyright (C) 1999-2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_WIDGETS_UTILS_H__
#define __PICMAN_WIDGETS_UTILS_H__


void              picman_menu_position               (GtkMenu              *menu,
                                                    gint                 *x,
                                                    gint                 *y);
void              picman_button_menu_position        (GtkWidget            *button,
                                                    GtkMenu              *menu,
                                                    GtkPositionType       position,
                                                    gint                 *x,
                                                    gint                 *y);
void              picman_table_attach_stock          (GtkTable             *table,
                                                    gint                  row,
                                                    const gchar          *stock_id,
                                                    GtkWidget            *widget,
                                                    gint                  colspan,
                                                    gboolean              left_align);
void              picman_enum_radio_box_add          (GtkBox               *box,
                                                    GtkWidget            *widget,
                                                    gint                  enum_value,
                                                    gboolean              below);
void              picman_enum_radio_frame_add        (GtkFrame             *frame,
                                                    GtkWidget            *widget,
                                                    gint                  enum_value,
                                                    gboolean              below);
GtkIconSize       picman_get_icon_size               (GtkWidget            *widget,
                                                    const gchar          *stock_id,
                                                    GtkIconSize           max_size,
                                                    gint                  width,
                                                    gint                  height);
PicmanTabStyle      picman_preview_tab_style_to_icon   (PicmanTabStyle          tab_style);

const gchar     * picman_get_mod_string              (GdkModifierType       modifiers);
gchar           * picman_suggest_modifiers           (const gchar          *message,
                                                    GdkModifierType       modifiers,
                                                    const gchar          *shift_format,
                                                    const gchar          *control_format,
                                                    const gchar          *alt_format);
PicmanChannelOps    picman_modifiers_to_channel_op     (GdkModifierType       modifiers);
GdkModifierType   picman_replace_virtual_modifiers   (GdkModifierType       modifiers);
GdkModifierType   picman_get_extend_selection_mask   (void);
GdkModifierType   picman_get_modify_selection_mask   (void);
GdkModifierType   picman_get_toggle_behavior_mask    (void);
GdkModifierType   picman_get_constrain_behavior_mask (void);
GdkModifierType   picman_get_all_modifiers_mask      (void);

void              picman_get_screen_resolution       (GdkScreen            *screen,
                                                    gdouble              *xres,
                                                    gdouble              *yres);
void              picman_rgb_get_gdk_color           (const PicmanRGB        *rgb,
                                                    GdkColor             *gdk_color);
void              picman_rgb_set_gdk_color           (PicmanRGB              *rgb,
                                                    const GdkColor       *gdk_color);
void              picman_window_set_hint             (GtkWindow            *window,
                                                    PicmanWindowHint        hint);
guint32           picman_window_get_native_id        (GtkWindow            *window);
void              picman_window_set_transient_for    (GtkWindow            *window,
                                                    guint32               parent_ID);
void              picman_toggle_button_set_visible   (GtkToggleButton      *toggle,
                                                    GtkWidget            *widget);
void              picman_widget_set_accel_help       (GtkWidget            *widget,
                                                    GtkAction            *action);
const gchar     * picman_get_message_stock_id        (PicmanMessageSeverity   severity);
void              picman_pango_layout_set_scale      (PangoLayout          *layout,
                                                    double                scale);
void              picman_pango_layout_set_weight     (PangoLayout          *layout,
                                                    PangoWeight           weight);
void              picman_highlight_widget            (GtkWidget            *widget,
                                                    gboolean              highlight);
GtkWidget       * picman_dock_with_window_new        (PicmanDialogFactory    *factory,
                                                    GdkScreen            *screen,
                                                    gboolean              toolbox);
GtkWidget *       picman_tools_get_tool_options_gui  (PicmanToolOptions      *tool_options);
void              picman_tools_set_tool_options_gui  (PicmanToolOptions      *tool_options,
                                                    GtkWidget            *widget);

void              picman_widget_flush_expose         (GtkWidget            *widget);

const gchar     * picman_print_event                 (const GdkEvent       *event);
void              picman_session_write_position      (PicmanConfigWriter     *writer,
                                                    gint                  position);


#endif /* __PICMAN_WIDGETS_UTILS_H__ */
