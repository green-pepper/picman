/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontrollerlist.h
 * Copyright (C) 2005 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTROLLER_LIST_H__
#define __PICMAN_CONTROLLER_LIST_H__


#define PICMAN_TYPE_CONTROLLER_LIST            (picman_controller_list_get_type ())
#define PICMAN_CONTROLLER_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTROLLER_LIST, PicmanControllerList))
#define PICMAN_CONTROLLER_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTROLLER_LIST, PicmanControllerListClass))
#define PICMAN_IS_CONTROLLER_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTROLLER_LIST))
#define PICMAN_IS_CONTROLLER_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTROLLER_LIST))
#define PICMAN_CONTROLLER_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTROLLER_LIST, PicmanControllerListClass))


typedef struct _PicmanControllerListClass PicmanControllerListClass;

struct _PicmanControllerList
{
  GtkBox              parent_instance;

  Picman               *picman;

  GtkWidget          *hbox;

  GtkListStore       *src;
  GtkTreeSelection   *src_sel;
  GType               src_gtype;

  GtkWidget          *dest;
  PicmanControllerInfo *dest_info;

  GtkWidget          *add_button;
  GtkWidget          *remove_button;
  GtkWidget          *edit_button;
  GtkWidget          *up_button;
  GtkWidget          *down_button;
};

struct _PicmanControllerListClass
{
  GtkBoxClass   parent_class;
};


GType       picman_controller_list_get_type (void) G_GNUC_CONST;

GtkWidget * picman_controller_list_new      (Picman *picman);


#endif  /*  __PICMAN_CONTROLLER_LIST_H__  */
