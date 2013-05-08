/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainercombobox.h
 * Copyright (C) 2004  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_CONTAINER_COMBO_BOX_H__
#define __PICMAN_CONTAINER_COMBO_BOX_H__


#define PICMAN_TYPE_CONTAINER_COMBO_BOX            (picman_container_combo_box_get_type ())
#define PICMAN_CONTAINER_COMBO_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTAINER_COMBO_BOX, PicmanContainerComboBox))
#define PICMAN_CONTAINER_COMBO_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTAINER_COMBO_BOX, PicmanContainerComboBoxClass))
#define PICMAN_IS_CONTAINER_COMBO_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTAINER_COMBO_BOX))
#define PICMAN_IS_CONTAINER_COMBO_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTAINER_COMBO_BOX))
#define PICMAN_CONTAINER_COMBO_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTAINER_COMBO_BOX, PicmanContainerComboBoxClass))


typedef struct _PicmanContainerComboBoxClass  PicmanContainerComboBoxClass;

struct _PicmanContainerComboBox
{
  GtkComboBox      parent_instance;

  GtkCellRenderer *text_renderer;
  GtkCellRenderer *viewable_renderer;
};

struct _PicmanContainerComboBoxClass
{
  GtkComboBoxClass  parent_class;
};


GType       picman_container_combo_box_get_type (void) G_GNUC_CONST;

GtkWidget * picman_container_combo_box_new      (PicmanContainer *container,
                                               PicmanContext   *context,
                                               gint           view_size,
                                               gint           view_border_width);


#endif  /*  __PICMAN_CONTAINER_COMBO_BOX_H__  */
