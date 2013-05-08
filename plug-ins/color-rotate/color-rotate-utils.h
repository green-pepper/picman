/*
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This is a plug-in for PICMAN.
 *
 * Colormap-Rotation plug-in. Exchanges two color ranges.
 *
 * Copyright (C) 1999 Sven Anders (anderss@fmi.uni-passau.de)
 *                    Based on code from Pavel Grinfeld (pavel@ml.com)
 *
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

/*----------------------------------------------------------------------------
 * Change log:
 *
 * Version 2.0, 04 April 1999.
 *  Nearly complete rewrite, made plug-in stable.
 *  (Works with PICMAN 1.1 and GTK+ 1.2)
 *
 * Version 1.0, 27 March 1997.
 *  Initial (unstable) release by Pavel Grinfeld
 *
 *----------------------------------------------------------------------------*/


/* Global defines */

#define SWAP(X,Y) {float t=X; X=Y; Y=t;}


/* used in 'rcm_callback.c' and 'rcm_dialog.c' */

float         arctg              (gfloat        y,
                                  gfloat        x);
float         min_prox           (gfloat        alpha,
                                  gfloat        beta,
                                  gfloat        angle);
float        *closest            (gfloat       *alpha,
                                  gfloat       *beta,
                                  gfloat        angle);
float         angle_mod_2PI      (gfloat        angle);
ReducedImage *rcm_reduce_image   (PicmanDrawable *drawable,
                                  PicmanDrawable *mask,
                                  gint          longer_size,
                                  gint          selection);
void          rcm_render_preview (GtkWidget    *preview);
void          rcm_render_circle  (GtkWidget    *preview,
                                  gint          sum,
                                  gint          margin);


/* only used in 'rcm.c' (or local) */

float rcm_angle_inside_slice (float     angle,
                              RcmAngle *slice);
gint  rcm_is_gray            (float     s);
float rcm_linear             (float,
                              float,
                              float,
                              float,
                              float);
float rcm_left_end           (RcmAngle *angle);
float rcm_right_end          (RcmAngle *angle);
