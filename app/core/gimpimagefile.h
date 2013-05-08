/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanimagefile.h
 *
 * Thumbnail handling according to the Thumbnail Managing Standard.
 * http://triq.net/~pearl/thumbnail-spec/
 *
 * Copyright (C) 2001-2002  Sven Neumann <sven@picman.org>
 *                          Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_IMAGEFILE_H__
#define __PICMAN_IMAGEFILE_H__


#include "picmanviewable.h"


#define PICMAN_TYPE_IMAGEFILE            (picman_imagefile_get_type ())
#define PICMAN_IMAGEFILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGEFILE, PicmanImagefile))
#define PICMAN_IMAGEFILE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_IMAGEFILE, PicmanImagefileClass))
#define PICMAN_IS_IMAGEFILE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGEFILE))
#define PICMAN_IS_IMAGEFILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_IMAGEFILE))
#define PICMAN_IMAGEFILE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_IMAGEFILE, PicmanImagefileClass))


typedef struct _PicmanImagefileClass PicmanImagefileClass;

struct _PicmanImagefile
{
  PicmanViewable  parent_instance;
};

struct _PicmanImagefileClass
{
  PicmanViewableClass   parent_class;

  void (* info_changed) (PicmanImagefile *imagefile);
};


GType           picman_imagefile_get_type              (void) G_GNUC_CONST;

PicmanImagefile * picman_imagefile_new                   (Picman          *picman,
                                                      const gchar   *uri);

PicmanThumbnail * picman_imagefile_get_thumbnail         (PicmanImagefile *imagefile);
GIcon         * picman_imagefile_get_gicon             (PicmanImagefile *imagefile);

void            picman_imagefile_set_mime_type         (PicmanImagefile *imagefile,
                                                      const gchar   *mime_type);
void            picman_imagefile_update                (PicmanImagefile *imagefile);
void            picman_imagefile_create_thumbnail      (PicmanImagefile *imagefile,
                                                      PicmanContext   *context,
                                                      PicmanProgress  *progress,
                                                      gint           size,
                                                      gboolean       replace);
void            picman_imagefile_create_thumbnail_weak (PicmanImagefile *imagefile,
                                                      PicmanContext   *context,
                                                      PicmanProgress  *progress,
                                                      gint           size,
                                                      gboolean       replace);
gboolean        picman_imagefile_check_thumbnail       (PicmanImagefile *imagefile);
gboolean        picman_imagefile_save_thumbnail        (PicmanImagefile *imagefile,
                                                      const gchar   *mime_type,
                                                      PicmanImage     *image);
const gchar   * picman_imagefile_get_desc_string       (PicmanImagefile *imagefile);


#endif /* __PICMAN_IMAGEFILE_H__ */
