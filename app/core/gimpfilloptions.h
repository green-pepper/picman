/* The PICMAN -- an image manipulation program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmanfilloptions.h
 * Copyright (C) 2003 Simon Budig
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

#ifndef __PICMAN_FILL_OPTIONS_H__
#define __PICMAN_FILL_OPTIONS_H__


#include "picmancontext.h"


#define PICMAN_TYPE_FILL_OPTIONS            (picman_fill_options_get_type ())
#define PICMAN_FILL_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FILL_OPTIONS, PicmanFillOptions))
#define PICMAN_FILL_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FILL_OPTIONS, PicmanFillOptionsClass))
#define PICMAN_IS_FILL_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FILL_OPTIONS))
#define PICMAN_IS_FILL_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FILL_OPTIONS))
#define PICMAN_FILL_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FILL_OPTIONS, PicmanFillOptionsClass))


typedef struct _PicmanFillOptionsClass PicmanFillOptionsClass;

struct _PicmanFillOptions
{
  PicmanContext  parent_instance;
};

struct _PicmanFillOptionsClass
{
  PicmanContextClass  parent_class;
};


GType             picman_fill_options_get_type      (void) G_GNUC_CONST;

PicmanFillOptions * picman_fill_options_new           (Picman *picman);

PicmanFillStyle     picman_fill_options_get_style     (PicmanFillOptions *options);
gboolean          picman_fill_options_get_antialias (PicmanFillOptions *options);


#endif /* __PICMAN_FILL_OPTIONS_H__ */
