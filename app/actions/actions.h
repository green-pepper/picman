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

#ifndef __ACTIONS_H__
#define __ACTIONS_H__


extern PicmanActionFactory *global_action_factory;


void               actions_init            (Picman                 *picman);
void               actions_exit            (Picman                 *picman);

Picman             * action_data_get_picman    (gpointer              data);
PicmanContext      * action_data_get_context (gpointer              data);
PicmanImage        * action_data_get_image   (gpointer              data);
PicmanDisplay      * action_data_get_display (gpointer              data);
PicmanDisplayShell * action_data_get_shell   (gpointer              data);
GtkWidget        * action_data_get_widget  (gpointer              data);
gint               action_data_sel_count   (gpointer              data);

gdouble            action_select_value     (PicmanActionSelectType  select_type,
                                            gdouble               value,
                                            gdouble               min,
                                            gdouble               max,
                                            gdouble               def,
                                            gdouble               small_inc,
                                            gdouble               inc,
                                            gdouble               skip_inc,
                                            gdouble               delta_factor,
                                            gboolean              wrap);
void               action_select_property  (PicmanActionSelectType  select_type,
                                            PicmanDisplay          *display,
                                            GObject              *object,
                                            const gchar          *property_name,
                                            gdouble               small_inc,
                                            gdouble               inc,
                                            gdouble               skip_inc,
                                            gdouble               delta_factor,
                                            gboolean              wrap);
PicmanObject       * action_select_object    (PicmanActionSelectType  select_type,
                                            PicmanContainer        *container,
                                            PicmanObject           *current);
void               action_message          (PicmanDisplay          *display,
                                            GObject              *object,
                                            const gchar          *format,
                                            ...) G_GNUC_PRINTF(3,4);


#define return_if_no_picman(picman,data) \
  picman = action_data_get_picman (data); \
  if (! picman) \
    return

#define return_if_no_context(context,data) \
  context = action_data_get_context (data); \
  if (! context) \
    return

#define return_if_no_image(image,data) \
  image = action_data_get_image (data); \
  if (! image) \
    return

#define return_if_no_display(display,data) \
  display = action_data_get_display (data); \
  if (! display) \
    return

#define return_if_no_shell(shell,data) \
  shell = action_data_get_shell (data); \
  if (! shell) \
    return

#define return_if_no_widget(widget,data) \
  widget = action_data_get_widget (data); \
  if (! widget) \
    return


#define return_if_no_drawable(image,drawable,data) \
  return_if_no_image (image,data); \
  drawable = picman_image_get_active_drawable (image); \
  if (! drawable) \
    return

#define return_if_no_layer(image,layer,data) \
  return_if_no_image (image,data); \
  layer = picman_image_get_active_layer (image); \
  if (! layer) \
    return

#define return_if_no_channel(image,channel,data) \
  return_if_no_image (image,data); \
  channel = picman_image_get_active_channel (image); \
  if (! channel) \
    return

#define return_if_no_vectors(image,vectors,data) \
  return_if_no_image (image,data); \
  vectors = picman_image_get_active_vectors (image); \
  if (! vectors) \
    return


#endif /* __ACTIONS_H__ */
