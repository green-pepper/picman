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

#define RADIUS  60
#define MARGIN  4
#define SUM     (2*RADIUS + 2*MARGIN)
#define CENTER  (SUM/2)

#define GRAY_RADIUS  60
#define GRAY_MARGIN  3
#define GRAY_SUM     (2*GRAY_RADIUS + 2*GRAY_MARGIN)
#define GRAY_CENTER  (GRAY_SUM/2)

#define LITTLE_RADIUS 3
#define EACH_OR_BOTH  0.3


void color_rotate_draw_little_circle (cairo_t   *cr,
                                      gfloat     hue,
                                      gfloat     satur);
void color_rotate_draw_large_circle  (cairo_t   *cr,
                                      gfloat     gray_sat);
void color_rotate_draw_arrows        (cairo_t   *cr,
                                      RcmAngle  *angle);
