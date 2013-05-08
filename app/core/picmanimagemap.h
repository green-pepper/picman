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

#ifndef __PICMAN_IMAGE_MAP_H__
#define __PICMAN_IMAGE_MAP_H__


#include "picmanobject.h"


#define PICMAN_TYPE_IMAGE_MAP            (picman_image_map_get_type ())
#define PICMAN_IMAGE_MAP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGE_MAP, PicmanImageMap))
#define PICMAN_IMAGE_MAP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_IMAGE_MAP, PicmanImageMapClass))
#define PICMAN_IS_IMAGE_MAP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGE_MAP))
#define PICMAN_IS_IMAGE_MAP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_IMAGE_MAP))
#define PICMAN_IMAGE_MAP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_IMAGE_MAP, PicmanImageMapClass))


typedef struct _PicmanImageMapClass  PicmanImageMapClass;

struct _PicmanImageMapClass
{
  PicmanObjectClass  parent_class;

  void (* flush) (PicmanImageMap *image_map);
};


/*  Image Map functions  */

/*  Successive image_map_apply functions can be called, but eventually
 *  MUST be followed with an image_map_commit or an image_map_abort call
 *  The image map is no longer valid after a call to commit or abort.
 */

GType          picman_image_map_get_type (void) G_GNUC_CONST;

PicmanImageMap * picman_image_map_new      (PicmanDrawable *drawable,
                                        const gchar  *undo_desc,
                                        GeglNode     *operation,
                                        const gchar  *stock_id);

void           picman_image_map_apply    (PicmanImageMap *image_map);

void           picman_image_map_commit   (PicmanImageMap *image_map,
                                        PicmanProgress *progress);
void           picman_image_map_abort    (PicmanImageMap *image_map);


#endif /* __PICMAN_IMAGE_MAP_H__ */
