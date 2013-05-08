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

#ifndef __PICMAN_CHANNEL_SELECT_H__
#define __PICMAN_CHANNEL_SELECT_H__


/*  basic selection functions  */

void   picman_channel_select_rectangle    (PicmanChannel         *channel,
                                         gint                 x,
                                         gint                 y,
                                         gint                 w,
                                         gint                 h,
                                         PicmanChannelOps       op,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y,
                                         gboolean             push_undo);
void   picman_channel_select_ellipse      (PicmanChannel         *channel,
                                         gint                 x,
                                         gint                 y,
                                         gint                 w,
                                         gint                 h,
                                         PicmanChannelOps       op,
                                         gboolean             antialias,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y,
                                         gboolean             push_undo);
void   picman_channel_select_round_rect   (PicmanChannel         *channel,
                                         gint                 x,
                                         gint                 y,
                                         gint                 w,
                                         gint                 h,
                                         gdouble              corner_radius_y,
                                         gdouble              corner_radius_x,
                                         PicmanChannelOps       op,
                                         gboolean             antialias,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y,
                                         gboolean             push_undo);

/*  select by PicmanScanConvert functions  */

void   picman_channel_select_scan_convert (PicmanChannel         *channel,
                                         const gchar         *undo_desc,
                                         PicmanScanConvert     *scan_convert,
                                         gint                 offset_x,
                                         gint                 offset_y,
                                         PicmanChannelOps       op,
                                         gboolean             antialias,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y,
                                         gboolean             push_undo);
void   picman_channel_select_polygon      (PicmanChannel         *channel,
                                         const gchar         *undo_desc,
                                         gint                 n_points,
                                         PicmanVector2         *points,
                                         PicmanChannelOps       op,
                                         gboolean             antialias,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y,
                                         gboolean             push_undo);
void   picman_channel_select_vectors      (PicmanChannel         *channel,
                                         const gchar         *undo_desc,
                                         PicmanVectors         *vectors,
                                         PicmanChannelOps       op,
                                         gboolean             antialias,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y,
                                         gboolean             push_undo);
void   picman_channel_select_buffer       (PicmanChannel         *channel,
                                         const gchar         *undo_desc,
                                         GeglBuffer          *add_on,
                                         gint                 offset_x,
                                         gint                 offset_y,
                                         PicmanChannelOps       op,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y);


/*  select by PicmanChannel functions  */

void   picman_channel_select_channel      (PicmanChannel         *channel,
                                         const gchar         *undo_desc,
                                         PicmanChannel         *add_on,
                                         gint                 offset_x,
                                         gint                 offset_y,
                                         PicmanChannelOps       op,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y);
void   picman_channel_select_alpha        (PicmanChannel         *channel,
                                         PicmanDrawable        *drawable,
                                         PicmanChannelOps       op,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y);
void   picman_channel_select_component    (PicmanChannel         *channel,
                                         PicmanChannelType      component,
                                         PicmanChannelOps       op,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y);
void   picman_channel_select_fuzzy        (PicmanChannel         *channel,
                                         PicmanDrawable        *drawable,
                                         gboolean             sample_merged,
                                         gint                 x,
                                         gint                 y,
                                         gfloat               threshold,
                                         gboolean             select_transparent,
                                         PicmanSelectCriterion  select_criterion,
                                         PicmanChannelOps       op,
                                         gboolean             antialias,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y);
void   picman_channel_select_by_color     (PicmanChannel         *channel,
                                         PicmanDrawable        *drawable,
                                         gboolean             sample_merged,
                                         const PicmanRGB       *color,
                                         gfloat               threshold,
                                         gboolean             select_transparent,
                                         PicmanSelectCriterion  select_criterion,
                                         PicmanChannelOps       op,
                                         gboolean             antialias,
                                         gboolean             feather,
                                         gdouble              feather_radius_x,
                                         gdouble              feather_radius_y);


#endif  /*  __PICMAN_CHANNEL_SELECT_H__  */
