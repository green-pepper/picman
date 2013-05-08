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

#ifndef __DATA_COMMANDS_H__
#define __DATA_COMMANDS_H__


void   data_open_as_image_cmd_callback (GtkAction   *action,
                                        gpointer     data);
void   data_new_cmd_callback           (GtkAction   *action,
                                        gpointer     data);
void   data_duplicate_cmd_callback     (GtkAction   *action,
                                        gpointer     data);
void   data_copy_location_cmd_callback (GtkAction   *action,
                                        gpointer     user_data);
void   data_delete_cmd_callback        (GtkAction   *action,
                                        gpointer     data);
void   data_refresh_cmd_callback       (GtkAction   *action,
                                        gpointer     data);
void   data_edit_cmd_callback          (GtkAction   *action,
                                        const gchar *value,
                                        gpointer     data);


#endif /* __DATA_COMMANDS_H__ */
