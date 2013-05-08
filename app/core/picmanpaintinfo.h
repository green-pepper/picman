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

#ifndef __PICMAN_PAINT_INFO_H__
#define __PICMAN_PAINT_INFO_H__


#include "picmanviewable.h"


#define PICMAN_TYPE_PAINT_INFO            (picman_paint_info_get_type ())
#define PICMAN_PAINT_INFO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PAINT_INFO, PicmanPaintInfo))
#define PICMAN_PAINT_INFO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PAINT_INFO, PicmanPaintInfoClass))
#define PICMAN_IS_PAINT_INFO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PAINT_INFO))
#define PICMAN_IS_PAINT_INFO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PAINT_INFO))
#define PICMAN_PAINT_INFO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PAINT_INFO, PicmanPaintInfoClass))


typedef struct _PicmanPaintInfoClass PicmanPaintInfoClass;

struct _PicmanPaintInfo
{
  PicmanViewable      parent_instance;

  Picman             *picman;

  GType             paint_type;
  GType             paint_options_type;

  gchar            *blurb;

  PicmanPaintOptions *paint_options;
};

struct _PicmanPaintInfoClass
{
  PicmanViewableClass  parent_class;
};


GType           picman_paint_info_get_type     (void) G_GNUC_CONST;

PicmanPaintInfo * picman_paint_info_new          (Picman          *picman,
                                              GType          paint_type,
                                              GType          paint_options_type,
                                              const gchar   *identifier,
                                              const gchar   *blurb,
                                              const gchar   *stock_id);

void            picman_paint_info_set_standard (Picman          *picman,
                                              PicmanPaintInfo *paint_info);
PicmanPaintInfo * picman_paint_info_get_standard (Picman          *picman);


#endif  /*  __PICMAN_PAINT_INFO_H__  */
