/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmananchor.h
 * Copyright (C) 2002 Simon Budig  <simon@picman.org>
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

#ifndef __PICMAN_ANCHOR_H__
#define __PICMAN_ANCHOR_H__

#define PICMAN_ANCHOR(anchor)  ((PicmanAnchor *) (anchor))

#define PICMAN_TYPE_ANCHOR               (picman_anchor_get_type ())
#define PICMAN_VALUE_HOLDS_ANCHOR(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_ANCHOR))

GType   picman_anchor_get_type           (void) G_GNUC_CONST;


struct _PicmanAnchor
{
  PicmanCoords        position;

  PicmanAnchorType    type;   /* Interpretation dependent on PicmanStroke type */
  gboolean          selected;
};


PicmanAnchor  * picman_anchor_new  (PicmanAnchorType    type,
                                const PicmanCoords *position);

PicmanAnchor  * picman_anchor_copy (const PicmanAnchor *anchor);
void          picman_anchor_free (PicmanAnchor       *anchor);


#endif /* __PICMAN_ANCHOR_H__ */
