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

#ifndef __PICMAN_SUB_PROGRESS_H__
#define __PICMAN_SUB_PROGRESS_H__


#define PICMAN_TYPE_SUB_PROGRESS            (picman_sub_progress_get_type ())
#define PICMAN_SUB_PROGRESS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SUB_PROGRESS, PicmanSubProgress))
#define PICMAN_SUB_PROGRESS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SUB_PROGRESS, PicmanSubProgressClass))
#define PICMAN_IS_SUB_PROGRESS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SUB_PROGRESS))
#define PICMAN_IS_SUB_PROGRESS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SUB_PROGRESS))
#define PICMAN_SUB_PROGRESS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SUB_PROGRESS, PicmanSubProgressClass))


typedef struct _PicmanSubProgressClass PicmanSubProgressClass;

struct _PicmanSubProgress
{
  GObject       parent_instance;

  PicmanProgress *progress;
  gdouble       start;
  gdouble       end;
};

struct _PicmanSubProgressClass
{
  GObjectClass  parent_class;
};


GType          picman_sub_progress_get_type (void) G_GNUC_CONST;

PicmanProgress * picman_sub_progress_new       (PicmanProgress    *progress);
void           picman_sub_progress_set_range (PicmanSubProgress *progress,
                                            gdouble          start,
                                            gdouble          end);
void           picman_sub_progress_set_step  (PicmanSubProgress *progress,
                                            gint             index,
                                            gint             num_steps);



#endif /* __PICMAN_SUB_PROGRESS_H__ */
