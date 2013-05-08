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

#ifndef __PICMAN_HISTOGRAM_OPTIONS_H__
#define __PICMAN_HISTOGRAM_OPTIONS_H__


#include "picmancoloroptions.h"


#define PICMAN_TYPE_HISTOGRAM_OPTIONS            (picman_histogram_options_get_type ())
#define PICMAN_HISTOGRAM_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_HISTOGRAM_OPTIONS, PicmanHistogramOptions))
#define PICMAN_HISTOGRAM_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_HISTOGRAM_OPTIONS, PicmanHistogramOptionsClass))
#define PICMAN_IS_HISTOGRAM_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_HISTOGRAM_OPTIONS))
#define PICMAN_IS_HISTOGRAM_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_HISTOGRAM_OPTIONS))
#define PICMAN_HISTOGRAM_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_HISTOGRAM_OPTIONS, PicmanHistogramOptionsClass))


typedef struct _PicmanHistogramOptions  PicmanHistogramOptions;
typedef         PicmanColorOptionsClass PicmanHistogramOptionsClass;

struct _PicmanHistogramOptions
{
  PicmanColorOptions    parent_instance;

  PicmanHistogramScale  scale;
};


GType       picman_histogram_options_get_type     (void) G_GNUC_CONST;

GtkWidget * picman_histogram_options_gui          (PicmanToolOptions      *tool_options);
void        picman_histogram_options_connect_view (PicmanHistogramOptions *options,
                                                 PicmanHistogramView    *view);


#endif /* __PICMAN_HISTOGRAM_OPTIONS_H__ */
