/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanImageParasiteView
 * Copyright (C) 2006  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_IMAGE_PARASITE_VIEW_H__
#define __PICMAN_IMAGE_PARASITE_VIEW_H__


#define PICMAN_TYPE_IMAGE_PARASITE_VIEW            (picman_image_parasite_view_get_type ())
#define PICMAN_IMAGE_PARASITE_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGE_PARASITE_VIEW, PicmanImageParasiteView))
#define PICMAN_IMAGE_PARASITE_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_IMAGE_PARASITE_VIEW, PicmanImageParasiteViewClass))
#define PICMAN_IS_IMAGE_PARASITE_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGE_PARASITE_VIEW))
#define PICMAN_IS_IMAGE_PARASITE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_IMAGE_PARASITE_VIEW))
#define PICMAN_IMAGE_PARASITE_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_IMAGE_PARASITE_VIEW, PicmanImageParasiteViewClass))


typedef struct _PicmanImageParasiteViewClass PicmanImageParasiteViewClass;

struct _PicmanImageParasiteView
{
  GtkBox     parent_instance;

  PicmanImage *image;
  gchar     *parasite;
};

struct _PicmanImageParasiteViewClass
{
  GtkBoxClass  parent_class;

  /*  signals  */
  void (* update) (PicmanImageParasiteView *view);
};


GType                picman_image_parasite_view_get_type     (void) G_GNUC_CONST;

GtkWidget          * picman_image_parasite_view_new          (PicmanImage   *image,
                                                            const gchar *parasite);
PicmanImage          * picman_image_parasite_view_get_image    (PicmanImageParasiteView *view);
const PicmanParasite * picman_image_parasite_view_get_parasite (PicmanImageParasiteView *view);


#endif /*  __PICMAN_IMAGE_PARASITE_VIEW_H__  */
