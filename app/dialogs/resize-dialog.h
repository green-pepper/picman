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

#ifndef __RESIZE_DIALOG_H__
#define __RESIZE_DIALOG_H__


typedef void (* PicmanResizeCallback) (GtkWidget    *dialog,
                                     PicmanViewable *viewable,
                                     gint          width,
                                     gint          height,
                                     PicmanUnit      unit,
                                     gint          offset_x,
                                     gint          offset_y,
                                     PicmanItemSet   layer_set,
                                     gboolean      resize_text_layers,
                                     gpointer      user_data);


GtkWidget * resize_dialog_new (PicmanViewable       *viewable,
                               PicmanContext        *context,
                               const gchar        *title,
                               const gchar        *role,
                               GtkWidget          *parent,
                               PicmanHelpFunc        help_func,
                               const gchar        *help_id,
                               PicmanUnit            unit,
                               PicmanResizeCallback  callback,
                               gpointer            user_data);


#endif  /*  __RESIZE_DIALOG_H__  */
