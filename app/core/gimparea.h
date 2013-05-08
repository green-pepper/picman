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

#ifndef __PICMAN_AREA_H__
#define __PICMAN_AREA_H__


struct _PicmanArea
{
  gint x1, y1, x2, y2;   /*  area bounds  */
};


PicmanArea * picman_area_new          (gint      x1,
                                   gint      y1,
                                   gint      x2,
                                   gint      y2);
void       picman_area_free         (PicmanArea *area);

GSList   * picman_area_list_process (GSList   *list,
                                   PicmanArea *area);
void       picman_area_list_free    (GSList   *list);


#endif /*  __PICMAN_AREA_H__  */
