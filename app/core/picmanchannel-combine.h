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

#ifndef __PICMAN_CHANNEL_COMBINE_H__
#define __PICMAN_CHANNEL_COMBINE_H__


void   picman_channel_combine_rect         (PicmanChannel    *mask,
                                          PicmanChannelOps  op,
                                          gint            x,
                                          gint            y,
                                          gint            w,
                                          gint            h);
void   picman_channel_combine_ellipse      (PicmanChannel    *mask,
                                          PicmanChannelOps  op,
                                          gint            x,
                                          gint            y,
                                          gint            w,
                                          gint            h,
                                          gboolean        antialias);
void   picman_channel_combine_ellipse_rect (PicmanChannel    *mask,
                                          PicmanChannelOps  op,
                                          gint            x,
                                          gint            y,
                                          gint            w,
                                          gint            h,
                                          gdouble         a,
                                          gdouble         b,
                                          gboolean        antialias);
void   picman_channel_combine_mask         (PicmanChannel    *mask,
                                          PicmanChannel    *add_on,
                                          PicmanChannelOps  op,
                                          gint            off_x,
                                          gint            off_y);
void   picman_channel_combine_buffer       (PicmanChannel    *mask,
                                          GeglBuffer     *add_on_buffer,
                                          PicmanChannelOps  op,
                                          gint            off_x,
                                          gint            off_y);


#endif /* __PICMAN_CHANNEL_COMBINE_H__ */
