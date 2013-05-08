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

#ifndef __PICMAN_BUFFER_H__
#define __PICMAN_BUFFER_H__


#include "picmanviewable.h"


#define PICMAN_TYPE_BUFFER            (picman_buffer_get_type ())
#define PICMAN_BUFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BUFFER, PicmanBuffer))
#define PICMAN_BUFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BUFFER, PicmanBufferClass))
#define PICMAN_IS_BUFFER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BUFFER))
#define PICMAN_IS_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BUFFER))
#define PICMAN_BUFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BUFFER, PicmanBufferClass))


typedef struct _PicmanBufferClass PicmanBufferClass;

struct _PicmanBuffer
{
  PicmanViewable   parent_instance;

  GeglBuffer    *buffer;
  gint           offset_x;
  gint           offset_y;
};

struct _PicmanBufferClass
{
  PicmanViewableClass  parent_class;
};


GType           picman_buffer_get_type        (void) G_GNUC_CONST;

PicmanBuffer    * picman_buffer_new             (GeglBuffer       *buffer,
                                             const gchar      *name,
                                             gint              offset_x,
                                             gint              offset_y,
                                             gboolean          copy_pixels);
PicmanBuffer    * picman_buffer_new_from_pixbuf (GdkPixbuf        *pixbuf,
                                             const gchar      *name,
                                             gint              offset_x,
                                             gint              offset_y);

gint            picman_buffer_get_width       (const PicmanBuffer *buffer);
gint            picman_buffer_get_height      (const PicmanBuffer *buffer);
const Babl    * picman_buffer_get_format      (const PicmanBuffer *buffer);

GeglBuffer    * picman_buffer_get_buffer      (const PicmanBuffer *buffer);


#endif /* __PICMAN_BUFFER_H__ */
