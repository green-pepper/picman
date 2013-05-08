/* PICMAN - The GNU Image Manipulation Program
 *
 * picmancageconfig.h
 * Copyright (C) 2010 Michael Muré <batolettre@gmail.com>
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

#ifndef __PICMAN_CAGE_CONFIG_H__
#define __PICMAN_CAGE_CONFIG_H__


#include "core/picmanimagemapconfig.h"


struct _PicmanCagePoint
{
  PicmanVector2 src_point;
  PicmanVector2 dest_point;
  PicmanVector2 edge_normal;
  gdouble     edge_scaling_factor;
  gboolean    selected;
};


#define PICMAN_TYPE_CAGE_CONFIG            (picman_cage_config_get_type ())
#define PICMAN_CAGE_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CAGE_CONFIG, PicmanCageConfig))
#define PICMAN_CAGE_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CAGE_CONFIG, PicmanCageConfigClass))
#define PICMAN_IS_CAGE_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CAGE_CONFIG))
#define PICMAN_IS_CAGE_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CAGE_CONFIG))
#define PICMAN_CAGE_CONFIG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CAGE_CONFIG, PicmanCageConfigClass))


typedef struct _PicmanCageConfigClass PicmanCageConfigClass;

struct _PicmanCageConfig
{
  PicmanImageMapConfig  parent_instance;

  GArray             *cage_points;

  gdouble             displacement_x;
  gdouble             displacement_y;
  PicmanCageMode        cage_mode;  /* Cage mode, used to commit displacement */
};

struct _PicmanCageConfigClass
{
  PicmanImageMapConfigClass  parent_class;
};


GType           picman_cage_config_get_type               (void) G_GNUC_CONST;

guint           picman_cage_config_get_n_points           (PicmanCageConfig  *gcc);
void            picman_cage_config_add_cage_point         (PicmanCageConfig  *gcc,
                                                         gdouble          x,
                                                         gdouble          y);
void            picman_cage_config_insert_cage_point      (PicmanCageConfig  *gcc,
                                                         gint             point_number,
                                                         gdouble          x,
                                                         gdouble          y);
void            picman_cage_config_remove_last_cage_point (PicmanCageConfig  *gcc);
void            picman_cage_config_remove_cage_point      (PicmanCageConfig  *gcc,
                                                         gint             point_number);
void            picman_cage_config_remove_selected_points (PicmanCageConfig  *gcc);
PicmanVector2     picman_cage_config_get_point_coordinate   (PicmanCageConfig  *gcc,
                                                         PicmanCageMode     mode,
                                                         gint             point_number);
void            picman_cage_config_add_displacement       (PicmanCageConfig  *gcc,
                                                         PicmanCageMode     mode,
                                                         gdouble          x,
                                                         gdouble          y);
void            picman_cage_config_commit_displacement    (PicmanCageConfig  *gcc);
void            picman_cage_config_reset_displacement     (PicmanCageConfig  *gcc);
GeglRectangle   picman_cage_config_get_bounding_box       (PicmanCageConfig  *gcc);
void            picman_cage_config_reverse_cage_if_needed (PicmanCageConfig  *gcc);
void            picman_cage_config_reverse_cage           (PicmanCageConfig  *gcc);
gboolean        picman_cage_config_point_inside           (PicmanCageConfig  *gcc,
                                                         gfloat           x,
                                                         gfloat           y);
void            picman_cage_config_select_point           (PicmanCageConfig  *gcc,
                                                         gint             point_number);
void            picman_cage_config_select_area            (PicmanCageConfig  *gcc,
                                                         PicmanCageMode     mode,
                                                         GeglRectangle    area);
void            picman_cage_config_select_add_area        (PicmanCageConfig  *gcc,
                                                         PicmanCageMode     mode,
                                                         GeglRectangle    area);
void            picman_cage_config_toggle_point_selection (PicmanCageConfig  *gcc,
                                                         gint             point_number);
void            picman_cage_config_deselect_points        (PicmanCageConfig  *gcc);
gboolean        picman_cage_config_point_is_selected      (PicmanCageConfig  *gcc,
                                                         gint             point_number);


#endif /* __PICMAN_CAGE_CONFIG_H__ */
