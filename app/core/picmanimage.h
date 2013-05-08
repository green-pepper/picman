/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattisbvf
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

#ifndef __PICMAN_IMAGE_H__
#define __PICMAN_IMAGE_H__


#include "picmanviewable.h"


#define PICMAN_IMAGE_ACTIVE_PARENT ((gpointer) 1)


#define PICMAN_TYPE_IMAGE            (picman_image_get_type ())
#define PICMAN_IMAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGE, PicmanImage))
#define PICMAN_IMAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_IMAGE, PicmanImageClass))
#define PICMAN_IS_IMAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGE))
#define PICMAN_IS_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_IMAGE))
#define PICMAN_IMAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_IMAGE, PicmanImageClass))


typedef struct _PicmanImageClass PicmanImageClass;

struct _PicmanImage
{
  PicmanViewable  parent_instance;

  Picman         *picman;  /*  the PICMAN the image belongs to  */
};

struct _PicmanImageClass
{
  PicmanViewableClass  parent_class;

  /*  signals  */
  void (* mode_changed)                 (PicmanImage            *image);
  void (* precision_changed)            (PicmanImage            *image);
  void (* alpha_changed)                (PicmanImage            *image);
  void (* floating_selection_changed)   (PicmanImage            *image);
  void (* active_layer_changed)         (PicmanImage            *image);
  void (* active_channel_changed)       (PicmanImage            *image);
  void (* active_vectors_changed)       (PicmanImage            *image);
  void (* component_visibility_changed) (PicmanImage            *image,
                                         PicmanChannelType       channel);
  void (* component_active_changed)     (PicmanImage            *image,
                                         PicmanChannelType       channel);
  void (* mask_changed)                 (PicmanImage            *image);
  void (* resolution_changed)           (PicmanImage            *image);
  void (* size_changed_detailed)        (PicmanImage            *image,
                                         gint                  previous_origin_x,
                                         gint                  previous_origin_y,
                                         gint                  previous_width,
                                         gint                  previous_height);
  void (* unit_changed)                 (PicmanImage            *image);
  void (* quick_mask_changed)           (PicmanImage            *image);
  void (* selection_invalidate)         (PicmanImage            *image);

  void (* clean)                        (PicmanImage            *image,
                                         PicmanDirtyMask         dirty_mask);
  void (* dirty)                        (PicmanImage            *image,
                                         PicmanDirtyMask         dirty_mask);
  void (* saved)                        (PicmanImage            *image,
                                         const gchar          *uri);
  void (* exported)                     (PicmanImage            *image,
                                         const gchar          *uri);

  void (* guide_added)                  (PicmanImage            *image,
                                         PicmanGuide            *guide);
  void (* guide_removed)                (PicmanImage            *image,
                                         PicmanGuide            *guide);
  void (* guide_moved)                  (PicmanImage            *image,
                                         PicmanGuide            *guide);
  void (* sample_point_added)           (PicmanImage            *image,
                                         PicmanSamplePoint      *sample_point);
  void (* sample_point_removed)         (PicmanImage            *image,
                                         PicmanSamplePoint      *sample_point);
  void (* sample_point_moved)           (PicmanImage            *image,
                                         PicmanSamplePoint      *sample_point);
  void (* parasite_attached)            (PicmanImage            *image,
                                         const gchar          *name);
  void (* parasite_detached)            (PicmanImage            *image,
                                         const gchar          *name);
  void (* colormap_changed)             (PicmanImage            *image,
                                         gint                  color_index);
  void (* undo_event)                   (PicmanImage            *image,
                                         PicmanUndoEvent         event,
                                         PicmanUndo             *undo);
};


GType           picman_image_get_type              (void) G_GNUC_CONST;

PicmanImage     * picman_image_new                   (Picman               *picman,
                                                  gint                width,
                                                  gint                height,
                                                  PicmanImageBaseType   base_type,
                                                  PicmanPrecision       precision);

PicmanImageBaseType  picman_image_get_base_type      (const PicmanImage    *image);
PicmanPrecision      picman_image_get_precision      (const PicmanImage    *image);

const Babl    * picman_image_get_format            (const PicmanImage    *image,
                                                  PicmanImageBaseType   base_type,
                                                  PicmanPrecision       precision,
                                                  gboolean            with_alpha);
const Babl    * picman_image_get_layer_format      (const PicmanImage    *image,
                                                  gboolean            with_alpha);
const Babl    * picman_image_get_channel_format    (const PicmanImage    *image);
const Babl    * picman_image_get_mask_format       (const PicmanImage    *image);

gint            picman_image_get_ID                (const PicmanImage    *image);
PicmanImage     * picman_image_get_by_ID             (Picman               *picman,
                                                  gint                id);

const gchar   * picman_image_get_uri               (const PicmanImage    *image);
const gchar   * picman_image_get_uri_or_untitled   (const PicmanImage    *image);
const gchar   * picman_image_get_imported_uri      (const PicmanImage    *image);
const gchar   * picman_image_get_exported_uri      (const PicmanImage    *image);
const gchar   * picman_image_get_save_a_copy_uri   (const PicmanImage    *image);
const gchar   * picman_image_get_any_uri           (const PicmanImage    *image);

void            picman_image_set_uri               (PicmanImage          *image,
                                                  const gchar        *uri);
void            picman_image_set_imported_uri      (PicmanImage          *image,
                                                  const gchar        *uri);
void            picman_image_set_exported_uri      (PicmanImage          *image,
                                                  const gchar        *uri);
void            picman_image_set_save_a_copy_uri   (PicmanImage          *image,
                                                  const gchar        *uri);

void            picman_image_set_filename          (PicmanImage          *image,
                                                  const gchar        *filename);
gchar         * picman_image_get_filename          (const PicmanImage    *image);

const gchar   * picman_image_get_display_name      (PicmanImage          *image);
const gchar   * picman_image_get_display_path      (PicmanImage          *image);

void            picman_image_set_load_proc         (PicmanImage          *image,
                                                  PicmanPlugInProcedure *proc);
PicmanPlugInProcedure * picman_image_get_load_proc   (const PicmanImage    *image);
void            picman_image_set_save_proc         (PicmanImage          *image,
                                                  PicmanPlugInProcedure *proc);
PicmanPlugInProcedure * picman_image_get_save_proc   (const PicmanImage    *image);
void            picman_image_saved                 (PicmanImage          *image,
                                                  const gchar        *uri);
void            picman_image_exported              (PicmanImage          *image,
                                                  const gchar        *uri);

void            picman_image_set_resolution        (PicmanImage          *image,
                                                  gdouble             xres,
                                                  gdouble             yres);
void            picman_image_get_resolution        (const PicmanImage    *image,
                                                  gdouble            *xres,
                                                  gdouble            *yres);
void            picman_image_resolution_changed    (PicmanImage          *image);

void            picman_image_set_unit              (PicmanImage          *image,
                                                  PicmanUnit            unit);
PicmanUnit        picman_image_get_unit              (const PicmanImage    *image);
void            picman_image_unit_changed          (PicmanImage          *image);

gint            picman_image_get_width             (const PicmanImage    *image);
gint            picman_image_get_height            (const PicmanImage    *image);

gboolean        picman_image_has_alpha             (const PicmanImage    *image);
gboolean        picman_image_is_empty              (const PicmanImage    *image);

void           picman_image_set_floating_selection (PicmanImage          *image,
                                                  PicmanLayer          *floating_sel);
PicmanLayer    * picman_image_get_floating_selection (const PicmanImage    *image);
void       picman_image_floating_selection_changed (PicmanImage          *image);

PicmanChannel   * picman_image_get_mask              (const PicmanImage    *image);
void            picman_image_mask_changed          (PicmanImage          *image);


/*  image components  */

const Babl    * picman_image_get_component_format  (const PicmanImage    *image,
                                                  PicmanChannelType     channel);
gint            picman_image_get_component_index   (const PicmanImage    *image,
                                                  PicmanChannelType     channel);

void            picman_image_set_component_active  (PicmanImage          *image,
                                                  PicmanChannelType     type,
                                                  gboolean            active);
gboolean        picman_image_get_component_active  (const PicmanImage    *image,
                                                  PicmanChannelType     type);
void            picman_image_get_active_array      (const PicmanImage    *image,
                                                  gboolean           *components);
PicmanComponentMask picman_image_get_active_mask     (const PicmanImage    *image);

void            picman_image_set_component_visible (PicmanImage          *image,
                                                  PicmanChannelType     type,
                                                  gboolean            visible);
gboolean        picman_image_get_component_visible (const PicmanImage    *image,
                                                  PicmanChannelType     type);
void            picman_image_get_visible_array     (const PicmanImage    *image,
                                                  gboolean           *components);
PicmanComponentMask picman_image_get_visible_mask    (const PicmanImage    *image);


/*  emitting image signals  */

void            picman_image_mode_changed          (PicmanImage          *image);
void            picman_image_precision_changed     (PicmanImage          *image);
void            picman_image_alpha_changed         (PicmanImage          *image);
void            picman_image_invalidate            (PicmanImage          *image,
                                                  gint                x,
                                                  gint                y,
                                                  gint                width,
                                                  gint                height);
void            picman_image_guide_added           (PicmanImage          *image,
                                                  PicmanGuide          *guide);
void            picman_image_guide_removed         (PicmanImage          *image,
                                                  PicmanGuide          *guide);
void            picman_image_guide_moved           (PicmanImage          *image,
                                                  PicmanGuide          *guide);

void            picman_image_sample_point_added    (PicmanImage          *image,
                                                  PicmanSamplePoint    *sample_point);
void            picman_image_sample_point_removed  (PicmanImage          *image,
                                                  PicmanSamplePoint    *sample_point);
void            picman_image_sample_point_moved    (PicmanImage          *image,
                                                  PicmanSamplePoint    *sample_point);
void            picman_image_colormap_changed      (PicmanImage          *image,
                                                  gint                col);
void            picman_image_selection_invalidate  (PicmanImage          *image);
void            picman_image_quick_mask_changed    (PicmanImage          *image);
void            picman_image_size_changed_detailed (PicmanImage          *image,
                                                  gint                previous_origin_x,
                                                  gint                previous_origin_y,
                                                  gint                previous_width,
                                                  gint                previous_height);
void            picman_image_undo_event            (PicmanImage          *image,
                                                  PicmanUndoEvent       event,
                                                  PicmanUndo           *undo);


/*  dirty counters  */

gint            picman_image_dirty                 (PicmanImage          *image,
                                                  PicmanDirtyMask       dirty_mask);
gint            picman_image_clean                 (PicmanImage          *image,
                                                  PicmanDirtyMask       dirty_mask);
void            picman_image_clean_all             (PicmanImage          *image);
void            picman_image_export_clean_all      (PicmanImage          *image);
gint            picman_image_is_dirty              (const PicmanImage    *image);
gboolean        picman_image_is_export_dirty       (const PicmanImage    *image);
gint            picman_image_get_dirty_time        (const PicmanImage    *image);


/*  flush this image's displays  */

void            picman_image_flush                 (PicmanImage          *image);


/*  display / instance counters  */

gint            picman_image_get_display_count     (const PicmanImage    *image);
void            picman_image_inc_display_count     (PicmanImage          *image);
void            picman_image_dec_display_count     (PicmanImage          *image);

gint            picman_image_get_instance_count    (const PicmanImage    *image);
void            picman_image_inc_instance_count    (PicmanImage          *image);


/*  parasites  */

const PicmanParasite * picman_image_parasite_find    (const PicmanImage    *image,
                                                  const gchar        *name);
gchar        ** picman_image_parasite_list         (const PicmanImage    *image,
                                                  gint               *count);
void            picman_image_parasite_attach       (PicmanImage          *image,
                                                  const PicmanParasite *parasite);
void            picman_image_parasite_detach       (PicmanImage          *image,
                                                  const gchar        *name);


/*  tattoos  */

PicmanTattoo      picman_image_get_new_tattoo        (PicmanImage          *image);
gboolean        picman_image_set_tattoo_state      (PicmanImage          *image,
                                                  PicmanTattoo          val);
PicmanTattoo      picman_image_get_tattoo_state      (PicmanImage          *image);


/*  projection  */

PicmanProjection * picman_image_get_projection       (const PicmanImage    *image);


/*  layers / channels / vectors  */

PicmanItemTree  * picman_image_get_layer_tree        (const PicmanImage    *image);
PicmanItemTree  * picman_image_get_channel_tree      (const PicmanImage    *image);
PicmanItemTree  * picman_image_get_vectors_tree      (const PicmanImage    *image);

PicmanContainer * picman_image_get_layers            (const PicmanImage    *image);
PicmanContainer * picman_image_get_channels          (const PicmanImage    *image);
PicmanContainer * picman_image_get_vectors           (const PicmanImage    *image);

gint            picman_image_get_n_layers          (const PicmanImage    *image);
gint            picman_image_get_n_channels        (const PicmanImage    *image);
gint            picman_image_get_n_vectors         (const PicmanImage    *image);

GList         * picman_image_get_layer_iter        (const PicmanImage    *image);
GList         * picman_image_get_channel_iter      (const PicmanImage    *image);
GList         * picman_image_get_vectors_iter      (const PicmanImage    *image);

GList         * picman_image_get_layer_list        (const PicmanImage    *image);
GList         * picman_image_get_channel_list      (const PicmanImage    *image);
GList         * picman_image_get_vectors_list      (const PicmanImage    *image);

PicmanDrawable  * picman_image_get_active_drawable   (const PicmanImage    *image);
PicmanLayer     * picman_image_get_active_layer      (const PicmanImage    *image);
PicmanChannel   * picman_image_get_active_channel    (const PicmanImage    *image);
PicmanVectors   * picman_image_get_active_vectors    (const PicmanImage    *image);

PicmanLayer     * picman_image_set_active_layer      (PicmanImage          *image,
                                                  PicmanLayer          *layer);
PicmanChannel   * picman_image_set_active_channel    (PicmanImage          *image,
                                                  PicmanChannel        *channel);
PicmanChannel   * picman_image_unset_active_channel  (PicmanImage          *image);
PicmanVectors   * picman_image_set_active_vectors    (PicmanImage          *image,
                                                  PicmanVectors        *vectors);

PicmanLayer     * picman_image_get_layer_by_tattoo   (const PicmanImage    *image,
                                                  PicmanTattoo          tattoo);
PicmanChannel   * picman_image_get_channel_by_tattoo (const PicmanImage    *image,
                                                  PicmanTattoo          tattoo);
PicmanVectors   * picman_image_get_vectors_by_tattoo (const PicmanImage    *image,
                                                  PicmanTattoo          tattoo);

PicmanLayer     * picman_image_get_layer_by_name     (const PicmanImage    *image,
                                                  const gchar        *name);
PicmanChannel   * picman_image_get_channel_by_name   (const PicmanImage    *image,
                                                  const gchar        *name);
PicmanVectors   * picman_image_get_vectors_by_name   (const PicmanImage    *image,
                                                  const gchar        *name);

gboolean        picman_image_reorder_item          (PicmanImage          *image,
                                                  PicmanItem           *item,
                                                  PicmanItem           *new_parent,
                                                  gint                new_index,
                                                  gboolean            push_undo,
                                                  const gchar        *undo_desc);
gboolean        picman_image_raise_item            (PicmanImage          *image,
                                                  PicmanItem           *item,
                                                  GError            **error);
gboolean        picman_image_raise_item_to_top     (PicmanImage          *image,
                                                  PicmanItem           *item);
gboolean        picman_image_lower_item            (PicmanImage          *image,
                                                  PicmanItem           *item,
                                                  GError            **error);
gboolean        picman_image_lower_item_to_bottom  (PicmanImage          *image,
                                                  PicmanItem           *item);

gboolean        picman_image_add_layer             (PicmanImage          *image,
                                                  PicmanLayer          *layer,
                                                  PicmanLayer          *parent,
                                                  gint                position,
                                                  gboolean            push_undo);
void            picman_image_remove_layer          (PicmanImage          *image,
                                                  PicmanLayer          *layer,
                                                  gboolean            push_undo,
                                                  PicmanLayer          *new_active);

void            picman_image_add_layers            (PicmanImage          *image,
                                                  GList              *layers,
                                                  PicmanLayer          *parent,
                                                  gint                position,
                                                  gint                x,
                                                  gint                y,
                                                  gint                width,
                                                  gint                height,
                                                  const gchar        *undo_desc);

gboolean        picman_image_add_channel           (PicmanImage          *image,
                                                  PicmanChannel        *channel,
                                                  PicmanChannel        *parent,
                                                  gint                position,
                                                  gboolean            push_undo);
void            picman_image_remove_channel        (PicmanImage          *image,
                                                  PicmanChannel        *channel,
                                                  gboolean            push_undo,
                                                  PicmanChannel        *new_active);

gboolean        picman_image_add_vectors           (PicmanImage          *image,
                                                  PicmanVectors        *vectors,
                                                  PicmanVectors        *parent,
                                                  gint                position,
                                                  gboolean            push_undo);
void            picman_image_remove_vectors        (PicmanImage          *image,
                                                  PicmanVectors        *vectors,
                                                  gboolean            push_undo,
                                                  PicmanVectors        *new_active);

gboolean    picman_image_coords_in_active_pickable (PicmanImage          *image,
                                                  const PicmanCoords   *coords,
                                                  gboolean            sample_merged,
                                                  gboolean            selected_only);

void            picman_image_invalidate_previews   (PicmanImage          *image);

const gchar   * picman_image_get_string_untitled   (void);


#endif /* __PICMAN_IMAGE_H__ */
