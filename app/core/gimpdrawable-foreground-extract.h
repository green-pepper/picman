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

#if 0

#ifndef  __PICMAN_DRAWABLE_FOREGROUND_EXTRACT_H__
#define  __PICMAN_DRAWABLE_FOREGROUND_EXTRACT_H__


/*  general API (as seen from the PDB)  */

void       picman_drawable_foreground_extract (PicmanDrawable              *drawable,
                                             PicmanForegroundExtractMode  mode,
                                             PicmanDrawable              *mask,
                                             PicmanProgress              *progress);

/*  SIOX specific API  */

SioxState * picman_drawable_foreground_extract_siox_init   (PicmanDrawable *drawable,
                                                          gint          x,
                                                          gint          y,
                                                          gint          width,
                                                          gint          height);
void        picman_drawable_foreground_extract_siox  (PicmanDrawable       *mask,
                                                    SioxState          *state,
                                                    SioxRefinementType  refinemane,
                                                    gint                smoothness,
                                                    const gdouble       sensitivity[3],
                                                    gboolean            multiblob,
                                                    PicmanProgress       *progress);
void        picman_drawable_foreground_extract_siox_done (SioxState      *state);


#endif  /*  __PICMAN_DRAWABLE_FOREGROUND_EXTRACT_H__  */

#endif
