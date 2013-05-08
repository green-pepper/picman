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

#ifndef __CHANNELS_COMMANDS_H__
#define __CHANNELS_COMMANDS_H__


void   channels_edit_attributes_cmd_callback (GtkAction   *action,
                                              gpointer     data);
void   channels_new_cmd_callback             (GtkAction   *action,
                                              gpointer     data);
void   channels_new_last_vals_cmd_callback   (GtkAction   *action,
                                              gpointer     data);

void   channels_raise_cmd_callback           (GtkAction   *action,
                                              gpointer     data);
void   channels_raise_to_top_cmd_callback    (GtkAction   *action,
                                              gpointer     data);
void   channels_lower_cmd_callback           (GtkAction   *action,
                                              gpointer     data);
void   channels_lower_to_bottom_cmd_callback (GtkAction   *action,
                                              gpointer     data);

void   channels_duplicate_cmd_callback       (GtkAction   *action,
                                              gpointer     data);
void   channels_delete_cmd_callback          (GtkAction   *action,
                                              gpointer     data);
void   channels_to_selection_cmd_callback    (GtkAction   *action,
                                              gint         value,
                                              gpointer     data);


#endif /* __CHANNELS_COMMANDS_H__ */
