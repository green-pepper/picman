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

#ifndef __PRINT_SIZE_DIALOG_H__
#define __PRINT_SIZE_DIALOG_H__


typedef void (* PicmanResolutionCallback) (GtkWidget *dialog,
                                         PicmanImage *image,
                                         gdouble    xresolution,
                                         gdouble    yresolution,
                                         PicmanUnit   resolution_unit,
                                         gpointer   user_data);


GtkWidget * print_size_dialog_new (PicmanImage              *image,
                                   PicmanContext            *context,
                                   const gchar            *title,
                                   const gchar            *role,
                                   GtkWidget              *parent,
                                   PicmanHelpFunc            help_func,
                                   const gchar            *help_id,
                                   PicmanResolutionCallback  callback,
                                   gpointer                user_data);


#endif  /*  __PRINT_SIZE_DIALOG_H__  */
