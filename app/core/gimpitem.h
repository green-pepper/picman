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

#ifndef __PICMAN_ITEM_H__
#define __PICMAN_ITEM_H__


#include "picmanfilter.h"


#define PICMAN_TYPE_ITEM            (picman_item_get_type ())
#define PICMAN_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ITEM, PicmanItem))
#define PICMAN_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ITEM, PicmanItemClass))
#define PICMAN_IS_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ITEM))
#define PICMAN_IS_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ITEM))
#define PICMAN_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ITEM, PicmanItemClass))


typedef struct _PicmanItemClass PicmanItemClass;

struct _PicmanItem
{
  PicmanFilter  parent_instance;
};

struct _PicmanItemClass
{
  PicmanFilterClass  parent_class;

  /*  signals  */
  void            (* removed)               (PicmanItem            *item);
  void            (* visibility_changed)    (PicmanItem            *item);
  void            (* linked_changed)        (PicmanItem            *item);
  void            (* lock_content_changed)  (PicmanItem            *item);
  void            (* lock_position_changed) (PicmanItem            *item);

  /*  virtual functions  */
  void            (* unset_removed)      (PicmanItem               *item);
  gboolean        (* is_attached)        (const PicmanItem         *item);
  gboolean        (* is_content_locked)  (const PicmanItem         *item);
  gboolean        (* is_position_locked) (const PicmanItem         *item);
  PicmanItemTree  * (* get_tree)           (PicmanItem               *item);
  PicmanItem      * (* duplicate)          (PicmanItem               *item,
                                          GType                   new_type);
  void            (* convert)            (PicmanItem               *item,
                                          PicmanImage              *dest_image);
  gboolean        (* rename)             (PicmanItem               *item,
                                          const gchar            *new_name,
                                          const gchar            *undo_desc,
                                          GError                **error);
  void            (* translate)          (PicmanItem               *item,
                                          gint                    offset_x,
                                          gint                    offset_y,
                                          gboolean                push_undo);
  void            (* scale)              (PicmanItem               *item,
                                          gint                    new_width,
                                          gint                    new_height,
                                          gint                    new_offset_x,
                                          gint                    new_offset_y,
                                          PicmanInterpolationType   interpolation_type,
                                          PicmanProgress           *progress);
  void            (* resize)             (PicmanItem               *item,
                                          PicmanContext            *context,
                                          gint                    new_width,
                                          gint                    new_height,
                                          gint                    offset_x,
                                          gint                    offset_y);
  void            (* flip)               (PicmanItem               *item,
                                          PicmanContext            *context,
                                          PicmanOrientationType     flip_type,
                                          gdouble                 axis,
                                          gboolean                clip_result);
  void            (* rotate)             (PicmanItem               *item,
                                          PicmanContext            *context,
                                          PicmanRotationType        rotate_type,
                                          gdouble                 center_x,
                                          gdouble                 center_y,
                                          gboolean                clip_result);
  void            (* transform)          (PicmanItem               *item,
                                          PicmanContext            *context,
                                          const PicmanMatrix3      *matrix,
                                          PicmanTransformDirection  direction,
                                          PicmanInterpolationType   interpolation_type,
                                          gint                    recursion_level,
                                          PicmanTransformResize     clip_result,
                                          PicmanProgress           *progress);
  gboolean        (* stroke)             (PicmanItem               *item,
                                          PicmanDrawable           *drawable,
                                          PicmanStrokeOptions      *stroke_options,
                                          gboolean                push_undo,
                                          PicmanProgress           *progress,
                                          GError                **error);
  void            (* to_selection)       (PicmanItem               *item,
                                          PicmanChannelOps          op,
                                          gboolean                antialias,
                                          gboolean                feather,
                                          gdouble                 feather_radius_x,
                                          gdouble                 feather_radius_y);

  const gchar *default_name;
  const gchar *rename_desc;
  const gchar *translate_desc;
  const gchar *scale_desc;
  const gchar *resize_desc;
  const gchar *flip_desc;
  const gchar *rotate_desc;
  const gchar *transform_desc;
  const gchar *to_selection_desc;
  const gchar *stroke_desc;

  const gchar *reorder_desc;
  const gchar *raise_desc;
  const gchar *raise_to_top_desc;
  const gchar *lower_desc;
  const gchar *lower_to_bottom_desc;

  const gchar *raise_failed;
  const gchar *lower_failed;
};


GType           picman_item_get_type           (void) G_GNUC_CONST;

PicmanItem      * picman_item_new                (GType               type,
                                              PicmanImage          *image,
                                              const gchar        *name,
                                              gint                offset_x,
                                              gint                offset_y,
                                              gint                width,
                                              gint                height);

void            picman_item_removed            (PicmanItem           *item);
gboolean        picman_item_is_removed         (const PicmanItem     *item);
void            picman_item_unset_removed      (PicmanItem           *item);

gboolean        picman_item_is_attached        (const PicmanItem     *item);

PicmanItem      * picman_item_get_parent         (const PicmanItem     *item);

PicmanItemTree  * picman_item_get_tree           (PicmanItem           *item);
PicmanContainer * picman_item_get_container      (PicmanItem           *item);
GList         * picman_item_get_container_iter (PicmanItem           *item);
gint            picman_item_get_index          (PicmanItem           *item);
GList         * picman_item_get_path           (PicmanItem           *item);

PicmanItem      * picman_item_duplicate          (PicmanItem           *item,
                                              GType               new_type);
PicmanItem      * picman_item_convert            (PicmanItem           *item,
                                              PicmanImage          *dest_image,
                                              GType               new_type);

gboolean        picman_item_rename             (PicmanItem           *item,
                                              const gchar        *new_name,
                                              GError            **error);

gint            picman_item_get_width          (const PicmanItem     *item);
gint            picman_item_get_height         (const PicmanItem     *item);
void            picman_item_set_size           (PicmanItem           *item,
                                              gint                width,
                                              gint                height);

void            picman_item_get_offset         (const PicmanItem     *item,
                                              gint               *offset_x,
                                              gint               *offset_y);
void            picman_item_set_offset         (PicmanItem           *item,
                                              gint                offset_x,
                                              gint                offset_y);
gint            picman_item_get_offset_x       (PicmanItem           *item);
gint            picman_item_get_offset_y       (PicmanItem           *item);

void            picman_item_translate          (PicmanItem           *item,
                                              gint                offset_x,
                                              gint                offset_y,
                                              gboolean            push_undo);

gboolean        picman_item_check_scaling      (const PicmanItem     *item,
                                              gint                new_width,
                                              gint                new_height);
void            picman_item_scale              (PicmanItem           *item,
                                              gint                new_width,
                                              gint                new_height,
                                              gint                new_offset_x,
                                              gint                new_offset_y,
                                              PicmanInterpolationType interpolation,
                                              PicmanProgress       *progress);
gboolean        picman_item_scale_by_factors   (PicmanItem           *item,
                                              gdouble             w_factor,
                                              gdouble             h_factor,
                                              PicmanInterpolationType interpolation,
                                              PicmanProgress       *progress);
void            picman_item_scale_by_origin    (PicmanItem           *item,
                                              gint                new_width,
                                              gint                new_height,
                                              PicmanInterpolationType interpolation,
                                              PicmanProgress       *progress,
                                              gboolean            local_origin);
void            picman_item_resize             (PicmanItem           *item,
                                              PicmanContext        *context,
                                              gint                new_width,
                                              gint                new_height,
                                              gint                offset_x,
                                              gint                offset_y);
void            picman_item_resize_to_image    (PicmanItem           *item);

void            picman_item_flip               (PicmanItem           *item,
                                              PicmanContext        *context,
                                              PicmanOrientationType flip_type,
                                              gdouble             axis,
                                              gboolean            flip_result);
void            picman_item_rotate             (PicmanItem           *item,
                                              PicmanContext        *context,
                                              PicmanRotationType    rotate_type,
                                              gdouble             center_x,
                                              gdouble             center_y,
                                              gboolean            flip_result);
void            picman_item_transform          (PicmanItem           *item,
                                              PicmanContext        *context,
                                              const PicmanMatrix3  *matrix,
                                              PicmanTransformDirection direction,
                                              PicmanInterpolationType interpolation_type,
                                              gint                recursion_level,
                                              PicmanTransformResize clip_result,
                                              PicmanProgress       *progress);

gboolean        picman_item_stroke             (PicmanItem           *item,
                                              PicmanDrawable       *drawable,
                                              PicmanContext        *context,
                                              PicmanStrokeOptions  *stroke_options,
                                              gboolean            use_default_values,
                                              gboolean            push_undo,
                                              PicmanProgress       *progress,
                                              GError            **error);

void            picman_item_to_selection       (PicmanItem           *item,
                                              PicmanChannelOps      op,
                                              gboolean            antialias,
                                              gboolean            feather,
                                              gdouble             feather_radius_x,
                                              gdouble             feather_radius_y);

void            picman_item_add_offset_node    (PicmanItem           *item,
                                              GeglNode           *node);
void            picman_item_remove_offset_node (PicmanItem           *item,
                                              GeglNode           *node);

gint            picman_item_get_ID             (PicmanItem           *item);
PicmanItem      * picman_item_get_by_ID          (Picman               *picman,
                                              gint                id);

PicmanTattoo      picman_item_get_tattoo         (const PicmanItem     *item);
void            picman_item_set_tattoo         (PicmanItem           *item,
                                              PicmanTattoo          tattoo);

PicmanImage     * picman_item_get_image          (const PicmanItem     *item);
void            picman_item_set_image          (PicmanItem           *item,
                                              PicmanImage          *image);

void            picman_item_replace_item       (PicmanItem           *item,
                                              PicmanItem           *replace);

void               picman_item_set_parasites   (PicmanItem           *item,
                                              PicmanParasiteList   *parasites);
PicmanParasiteList * picman_item_get_parasites   (const PicmanItem     *item);

void            picman_item_parasite_attach    (PicmanItem           *item,
                                              const PicmanParasite *parasite,
                                              gboolean            push_undo);
void            picman_item_parasite_detach    (PicmanItem           *item,
                                              const gchar        *name,
                                              gboolean            push_undo);
const PicmanParasite * picman_item_parasite_find (const PicmanItem     *item,
                                              const gchar        *name);
gchar        ** picman_item_parasite_list      (const PicmanItem     *item,
                                              gint               *count);

void            picman_item_set_visible        (PicmanItem           *item,
                                              gboolean            visible,
                                              gboolean            push_undo);
gboolean        picman_item_get_visible        (const PicmanItem     *item);
gboolean        picman_item_is_visible         (const PicmanItem     *item);

void            picman_item_set_linked         (PicmanItem           *item,
                                              gboolean            linked,
                                              gboolean            push_undo);
gboolean        picman_item_get_linked         (const PicmanItem     *item);

void            picman_item_set_lock_content   (PicmanItem           *item,
                                              gboolean            lock_content,
                                              gboolean            push_undo);
gboolean        picman_item_get_lock_content   (const PicmanItem     *item);
gboolean        picman_item_can_lock_content   (const PicmanItem     *item);
gboolean        picman_item_is_content_locked  (const PicmanItem     *item);

void            picman_item_set_lock_position  (PicmanItem          *item,
                                              gboolean           lock_position,
                                              gboolean           push_undo);
gboolean        picman_item_get_lock_position  (const PicmanItem    *item);
gboolean        picman_item_can_lock_position  (const PicmanItem    *item);
gboolean        picman_item_is_position_locked (const PicmanItem    *item);

gboolean        picman_item_mask_bounds        (PicmanItem           *item,
                                              gint               *x1,
                                              gint               *y1,
                                              gint               *x2,
                                              gint               *y2);
gboolean        picman_item_mask_intersect     (PicmanItem           *item,
                                              gint               *x,
                                              gint               *y,
                                              gint               *width,
                                              gint               *height);

gboolean        picman_item_is_in_set          (PicmanItem           *item,
                                              PicmanItemSet         set);


#endif /* __PICMAN_ITEM_H__ */
