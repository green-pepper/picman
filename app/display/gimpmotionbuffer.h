/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanmotionbuffer.h
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

#ifndef __PICMAN_MOTION_BUFFER_H__
#define __PICMAN_MOTION_BUFFER_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_MOTION_BUFFER            (picman_motion_buffer_get_type ())
#define PICMAN_MOTION_BUFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_MOTION_BUFFER, PicmanMotionBuffer))
#define PICMAN_MOTION_BUFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_MOTION_BUFFER, PicmanMotionBufferClass))
#define PICMAN_IS_MOTION_BUFFER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_MOTION_BUFFER))
#define PICMAN_IS_MOTION_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_MOTION_BUFFER))
#define PICMAN_MOTION_BUFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_MOTION_BUFFER, PicmanMotionBufferClass))


typedef struct _PicmanMotionBufferClass PicmanMotionBufferClass;

struct _PicmanMotionBuffer
{
  PicmanObject  parent_instance;

  guint32     last_read_motion_time;

  guint32     last_motion_time; /*  previous time of a forwarded motion event  */
  gdouble     last_motion_delta_time;
  gdouble     last_motion_delta_x;
  gdouble     last_motion_delta_y;
  gdouble     last_motion_distance;

  PicmanCoords  last_coords;      /* last motion event                   */

  GArray     *event_history;
  GArray     *event_queue;
  gboolean    event_delay;      /* TRUE if theres an unsent event in
                                 *  the history buffer
                                 */

  gint               event_delay_timeout;
  GdkModifierType    last_active_state;
};

struct _PicmanMotionBufferClass
{
  PicmanObjectClass  parent_class;

  void (* stroke) (PicmanMotionBuffer *buffer,
                   const PicmanCoords *coords,
                   guint32           time,
                   GdkModifierType   state);
  void (* hover)  (PicmanMotionBuffer *buffer,
                   const PicmanCoords *coords,
                   GdkModifierType   state,
                   gboolean          proximity);
};


GType              picman_motion_buffer_get_type     (void) G_GNUC_CONST;

PicmanMotionBuffer * picman_motion_buffer_new          (void);

void       picman_motion_buffer_begin_stroke         (PicmanMotionBuffer *buffer,
                                                    guint32           time,
                                                    PicmanCoords       *last_motion);
void       picman_motion_buffer_end_stroke           (PicmanMotionBuffer *buffer);

gboolean   picman_motion_buffer_motion_event         (PicmanMotionBuffer *buffer,
                                                    PicmanCoords       *coords,
                                                    guint32           time,
                                                    gdouble           scale_x,
                                                    gdouble           scale_y,
                                                    gboolean          event_fill);
guint32    picman_motion_buffer_get_last_motion_time (PicmanMotionBuffer *buffer);

void       picman_motion_buffer_request_stroke       (PicmanMotionBuffer *buffer,
                                                    GdkModifierType   state,
                                                    guint32           time);
void       picman_motion_buffer_request_hover        (PicmanMotionBuffer *buffer,
                                                    GdkModifierType   state,
                                                    gboolean          proximity);


#endif /* __PICMAN_MOTION_BUFFER_H__ */
