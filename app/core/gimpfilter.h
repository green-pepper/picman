/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfilter.h
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

#ifndef __PICMAN_FILTER_H__
#define __PICMAN_FILTER_H__


#include "picmanviewable.h"


#define PICMAN_TYPE_FILTER            (picman_filter_get_type ())
#define PICMAN_FILTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FILTER, PicmanFilter))
#define PICMAN_FILTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FILTER, PicmanFilterClass))
#define PICMAN_IS_FILTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FILTER))
#define PICMAN_IS_FILTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FILTER))
#define PICMAN_FILTER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FILTER, PicmanFilterClass))


typedef struct _PicmanFilterClass PicmanFilterClass;

struct _PicmanFilter
{
  PicmanViewable  parent_instance;
};

struct _PicmanFilterClass
{
  PicmanViewableClass  parent_class;

  GeglNode * (* get_node) (PicmanFilter *filter);
};


GType            picman_filter_get_type         (void) G_GNUC_CONST;
PicmanFilter     * picman_filter_new              (const gchar    *name);

GeglNode       * picman_filter_get_node         (PicmanFilter     *filter);
GeglNode       * picman_filter_peek_node        (PicmanFilter     *filter);

void             picman_filter_set_is_last_node (PicmanFilter     *filter,
                                               gboolean        is_last_node);
gboolean         picman_filter_get_is_last_node (PicmanFilter     *filter);

void             picman_filter_set_applicator   (PicmanFilter     *filter,
                                               PicmanApplicator *applicator);
PicmanApplicator * picman_filter_get_applicator   (PicmanFilter     *filter);


#endif /* __PICMAN_FILTER_H__ */
