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

#ifndef __PICMAN_SELECTION_H__
#define __PICMAN_SELECTION_H__


#include "picmanchannel.h"


#define PICMAN_TYPE_SELECTION            (picman_selection_get_type ())
#define PICMAN_SELECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SELECTION, PicmanSelection))
#define PICMAN_SELECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SELECTION, PicmanSelectionClass))
#define PICMAN_IS_SELECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SELECTION))
#define PICMAN_IS_SELECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SELECTION))
#define PICMAN_SELECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SELECTION, PicmanSelectionClass))


typedef struct _PicmanSelectionClass PicmanSelectionClass;

struct _PicmanSelection
{
  PicmanChannel parent_instance;

  gint        stroking_count;
};

struct _PicmanSelectionClass
{
  PicmanChannelClass parent_class;
};


GType         picman_selection_get_type      (void) G_GNUC_CONST;

PicmanChannel * picman_selection_new           (PicmanImage     *image,
                                            gint           width,
                                            gint           height);

gint          picman_selection_push_stroking (PicmanSelection *selection);
gint          picman_selection_pop_stroking  (PicmanSelection *selection);

void          picman_selection_load          (PicmanSelection *selection,
                                            PicmanChannel   *channel);
PicmanChannel * picman_selection_save          (PicmanSelection *selection);

GeglBuffer  * picman_selection_extract       (PicmanSelection *selection,
                                            PicmanPickable  *pickable,
                                            PicmanContext   *context,
                                            gboolean       cut_image,
                                            gboolean       keep_indexed,
                                            gboolean       add_alpha,
                                            gint          *offset_x,
                                            gint          *offset_y,
                                            GError       **error);

PicmanLayer   * picman_selection_float         (PicmanSelection *selection,
                                            PicmanDrawable  *drawable,
                                            PicmanContext   *context,
                                            gboolean       cut_image,
                                            gint           off_x,
                                            gint           off_y,
                                            GError       **error);


#endif /* __PICMAN_SELECTION_H__ */
