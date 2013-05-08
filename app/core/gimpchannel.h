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

#ifndef __PICMAN_CHANNEL_H__
#define __PICMAN_CHANNEL_H__

#include "picmandrawable.h"


#define PICMAN_TYPE_CHANNEL            (picman_channel_get_type ())
#define PICMAN_CHANNEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CHANNEL, PicmanChannel))
#define PICMAN_CHANNEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CHANNEL, PicmanChannelClass))
#define PICMAN_IS_CHANNEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CHANNEL))
#define PICMAN_IS_CHANNEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CHANNEL))
#define PICMAN_CHANNEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CHANNEL, PicmanChannelClass))


typedef struct _PicmanChannelClass PicmanChannelClass;

struct _PicmanChannel
{
  PicmanDrawable  parent_instance;

  PicmanRGB       color;             /*  Also stores the opacity        */
  gboolean      show_masked;       /*  Show masked areas--as          */
                                   /*  opposed to selected areas      */

  GeglNode     *color_node;
  GeglNode     *invert_node;
  GeglNode     *mask_node;

  /*  Selection mask variables  */
  gboolean      boundary_known;    /*  is the current boundary valid  */
  PicmanBoundSeg *segs_in;           /*  outline of selected region     */
  PicmanBoundSeg *segs_out;          /*  outline of selected region     */
  gint          num_segs_in;       /*  number of lines in boundary    */
  gint          num_segs_out;      /*  number of lines in boundary    */
  gboolean      empty;             /*  is the region empty?           */
  gboolean      bounds_known;      /*  recalculate the bounds?        */
  gint          x1, y1;            /*  coordinates for bounding box   */
  gint          x2, y2;            /*  lower right hand coordinate    */
};

struct _PicmanChannelClass
{
  PicmanDrawableClass  parent_class;

  /*  signals  */
  void     (* color_changed) (PicmanChannel         *channel);

  /*  virtual functions  */
  gboolean (* boundary)      (PicmanChannel         *channel,
                              const PicmanBoundSeg **segs_in,
                              const PicmanBoundSeg **segs_out,
                              gint                *num_segs_in,
                              gint                *num_segs_out,
                              gint                 x1,
                              gint                 y1,
                              gint                 x2,
                              gint                 y2);
  gboolean (* bounds)        (PicmanChannel         *channel,
                              gint                *x1,
                              gint                *y1,
                              gint                *x2,
                              gint                *y2);
  gboolean (* is_empty)      (PicmanChannel         *channel);

  void     (* feather)       (PicmanChannel         *channel,
                              gdouble              radius_x,
                              gdouble              radius_y,
                              gboolean             push_undo);
  void     (* sharpen)       (PicmanChannel         *channel,
                              gboolean             push_undo);
  void     (* clear)         (PicmanChannel         *channel,
                              const gchar         *undo_desc,
                              gboolean             push_undo);
  void     (* all)           (PicmanChannel         *channel,
                              gboolean             push_undo);
  void     (* invert)        (PicmanChannel         *channel,
                              gboolean             push_undo);
  void     (* border)        (PicmanChannel         *channel,
                              gint                 radius_x,
                              gint                 radius_y,
                              gboolean             feather,
                              gboolean             edge_lock,
                              gboolean             push_undo);
  void     (* grow)          (PicmanChannel         *channel,
                              gint                 radius_x,
                              gint                 radius_y,
                              gboolean             push_undo);
  void     (* shrink)        (PicmanChannel         *channel,
                              gint                 radius_x,
                              gint                 radius_y,
                              gboolean             edge_lock,
                              gboolean             push_undo);

  const gchar *feather_desc;
  const gchar *sharpen_desc;
  const gchar *clear_desc;
  const gchar *all_desc;
  const gchar *invert_desc;
  const gchar *border_desc;
  const gchar *grow_desc;
  const gchar *shrink_desc;
};


/*  function declarations  */

GType         picman_channel_get_type           (void) G_GNUC_CONST;

PicmanChannel * picman_channel_new                (PicmanImage         *image,
                                               gint               width,
                                               gint               height,
                                               const gchar       *name,
                                               const PicmanRGB     *color);

PicmanChannel * picman_channel_new_from_alpha     (PicmanImage         *image,
                                               PicmanDrawable      *drawable,
                                               const gchar       *name,
                                               const PicmanRGB     *color);
PicmanChannel * picman_channel_new_from_component (PicmanImage         *image,
                                               PicmanChannelType    type,
                                               const gchar       *name,
                                               const PicmanRGB     *color);

PicmanChannel * picman_channel_get_parent         (PicmanChannel       *channel);

gdouble       picman_channel_get_opacity        (const PicmanChannel *channel);
void          picman_channel_set_opacity        (PicmanChannel       *channel,
                                               gdouble            opacity,
                                               gboolean           push_undo);

void          picman_channel_get_color          (const PicmanChannel *channel,
                                               PicmanRGB           *color);
void          picman_channel_set_color          (PicmanChannel       *channel,
                                               const PicmanRGB     *color,
                                               gboolean           push_undo);

gboolean      picman_channel_get_show_masked    (PicmanChannel       *channel);
void          picman_channel_set_show_masked    (PicmanChannel       *channel,
                                               gboolean           show_masked);

void          picman_channel_push_undo          (PicmanChannel       *mask,
                                               const gchar       *undo_desc);


/*  selection mask functions  */

PicmanChannel * picman_channel_new_mask           (PicmanImage           *image,
                                               gint                 width,
                                               gint                 height);

gboolean      picman_channel_boundary           (PicmanChannel         *mask,
                                               const PicmanBoundSeg **segs_in,
                                               const PicmanBoundSeg **segs_out,
                                               gint                *num_segs_in,
                                               gint                *num_segs_out,
                                               gint                 x1,
                                               gint                 y1,
                                               gint                 x2,
                                               gint                 y2);
gboolean      picman_channel_bounds             (PicmanChannel         *mask,
                                               gint                *x1,
                                               gint                *y1,
                                               gint                *x2,
                                               gint                *y2);
gboolean      picman_channel_is_empty           (PicmanChannel         *mask);

void          picman_channel_feather            (PicmanChannel         *mask,
                                               gdouble              radius_x,
                                               gdouble              radius_y,
                                               gboolean             push_undo);
void          picman_channel_sharpen            (PicmanChannel         *mask,
                                               gboolean             push_undo);

void          picman_channel_clear              (PicmanChannel         *mask,
                                               const gchar         *undo_desc,
                                               gboolean             push_undo);
void          picman_channel_all                (PicmanChannel         *mask,
                                               gboolean             push_undo);
void          picman_channel_invert             (PicmanChannel         *mask,
                                               gboolean             push_undo);

void          picman_channel_border             (PicmanChannel         *mask,
                                               gint                 radius_x,
                                               gint                 radius_y,
                                               gboolean             feather,
                                               gboolean             edge_lock,
                                               gboolean             push_undo);
void          picman_channel_grow               (PicmanChannel         *mask,
                                               gint                 radius_x,
                                               gint                 radius_y,
                                               gboolean             push_undo);
void          picman_channel_shrink             (PicmanChannel         *mask,
                                               gint                 radius_x,
                                               gint                 radius_y,
                                               gboolean             edge_lock,
                                               gboolean             push_undo);


#endif /* __PICMAN_CHANNEL_H__ */
