/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanGrid
 * Copyright (C) 2003  Henrik Brix Andersen <brix@picman.org>
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

#ifndef __PICMAN_GRID_H__
#define __PICMAN_GRID_H__


#include "picmanobject.h"


#define PICMAN_TYPE_GRID            (picman_grid_get_type ())
#define PICMAN_GRID(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_GRID, PicmanGrid))
#define PICMAN_GRID_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_GRID, PicmanGridClass))
#define PICMAN_IS_GRID(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_GRID))
#define PICMAN_IS_GRID_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_GRID))
#define PICMAN_GRID_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_GRID, PicmanGridClass))


typedef struct _PicmanGridClass  PicmanGridClass;

struct _PicmanGrid
{
  PicmanObject     parent_instance;

  PicmanGridStyle  style;
  PicmanRGB        fgcolor;
  PicmanRGB        bgcolor;
  gdouble        xspacing;
  gdouble        yspacing;
  PicmanUnit       spacing_unit;
  gdouble        xoffset;
  gdouble        yoffset;
  PicmanUnit       offset_unit;
};


struct _PicmanGridClass
{
  PicmanObjectClass  parent_class;
};


GType          picman_grid_get_type               (void) G_GNUC_CONST;
const gchar  * picman_grid_parasite_name          (void) G_GNUC_CONST;
PicmanParasite * picman_grid_to_parasite            (const PicmanGrid     *grid);
PicmanGrid     * picman_grid_from_parasite          (const PicmanParasite *parasite);


#endif /* __PICMAN_GRID_H__ */
