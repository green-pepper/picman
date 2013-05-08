/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 2009 Martin Nordholts <martinn@src.gnome.org>
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

#ifndef  __PICMAN_APP_TEST_UTILS_H__
#define  __PICMAN_APP_TEST_UTILS_H__


void            picman_test_utils_set_env_to_subpath   (const gchar *root_env_var,
                                                      const gchar *subdir,
                                                      const gchar *target_env_var);
void            picman_test_utils_set_picman2_directory  (const gchar *root_env_var,
                                                      const gchar *subdir);
void            picman_test_utils_setup_menus_dir      (void);
void            picman_test_utils_create_image         (Picman        *picman,
                                                      gint         width,
                                                      gint         height);
void            picman_test_utils_synthesize_key_event (GtkWidget   *widget,
                                                      guint        keyval);
PicmanUIManager * picman_test_utils_get_ui_manager       (Picman        *picman);
PicmanImage     * picman_test_utils_create_image_from_dialog
                                                     (Picman        *picman);


#endif /* __PICMAN_APP_TEST_UTILS_H__ */
