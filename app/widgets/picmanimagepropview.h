/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanImagePropView
 * Copyright (C) 2005  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_IMAGE_PROP_VIEW_H__
#define __PICMAN_IMAGE_PROP_VIEW_H__


#define PICMAN_TYPE_IMAGE_PROP_VIEW            (picman_image_prop_view_get_type ())
#define PICMAN_IMAGE_PROP_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGE_PROP_VIEW, PicmanImagePropView))
#define PICMAN_IMAGE_PROP_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_IMAGE_PROP_VIEW, PicmanImagePropViewClass))
#define PICMAN_IS_IMAGE_PROP_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGE_PROP_VIEW))
#define PICMAN_IS_IMAGE_PROP_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_IMAGE_PROP_VIEW))
#define PICMAN_IMAGE_PROP_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_IMAGE_PROP_VIEW, PicmanImagePropViewClass))


typedef struct _PicmanImagePropViewClass PicmanImagePropViewClass;

struct _PicmanImagePropView
{
  GtkTable   parent_instance;

  PicmanImage *image;

  GtkWidget *pixel_size_label;
  GtkWidget *print_size_label;
  GtkWidget *resolution_label;
  GtkWidget *colorspace_label;
  GtkWidget *precision_label;
  GtkWidget *filename_label;
  GtkWidget *filesize_label;
  GtkWidget *filetype_label;
  GtkWidget *memsize_label;
  GtkWidget *undo_label;
  GtkWidget *redo_label;
  GtkWidget *pixels_label;
  GtkWidget *layers_label;
  GtkWidget *channels_label;
  GtkWidget *vectors_label;
};

struct _PicmanImagePropViewClass
{
  GtkTableClass  parent_class;
};


GType       picman_image_prop_view_get_type (void) G_GNUC_CONST;

GtkWidget * picman_image_prop_view_new      (PicmanImage *image);


#endif /*  __PICMAN_IMAGE_PROP_VIEW_H__  */
