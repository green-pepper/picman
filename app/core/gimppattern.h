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

#ifndef __PICMAN_PATTERN_H__
#define __PICMAN_PATTERN_H__


#include "picmandata.h"


#define PICMAN_TYPE_PATTERN            (picman_pattern_get_type ())
#define PICMAN_PATTERN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PATTERN, PicmanPattern))
#define PICMAN_PATTERN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PATTERN, PicmanPatternClass))
#define PICMAN_IS_PATTERN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PATTERN))
#define PICMAN_IS_PATTERN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PATTERN))
#define PICMAN_PATTERN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PATTERN, PicmanPatternClass))


typedef struct _PicmanPatternClass PicmanPatternClass;

struct _PicmanPattern
{
  PicmanData     parent_instance;

  PicmanTempBuf *mask;
};

struct _PicmanPatternClass
{
  PicmanDataClass  parent_class;
};


GType         picman_pattern_get_type      (void) G_GNUC_CONST;

PicmanData    * picman_pattern_new           (PicmanContext       *context,
                                          const gchar       *name);
PicmanData    * picman_pattern_get_standard  (PicmanContext       *context);

PicmanTempBuf * picman_pattern_get_mask      (const PicmanPattern *pattern);
GeglBuffer  * picman_pattern_create_buffer (const PicmanPattern *pattern);


#endif /* __PICMAN_PATTERN_H__ */
