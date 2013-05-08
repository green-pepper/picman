/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanaction.h
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_ACTION_H__
#define __PICMAN_ACTION_H__


#define PICMAN_TYPE_ACTION            (picman_action_get_type ())
#define PICMAN_ACTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ACTION, PicmanAction))
#define PICMAN_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ACTION, PicmanActionClass))
#define PICMAN_IS_ACTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ACTION))
#define PICMAN_IS_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), PICMAN_TYPE_ACTION))
#define PICMAN_ACTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PICMAN_TYPE_ACTION, PicmanActionClass))


typedef struct _PicmanActionClass PicmanActionClass;

struct _PicmanAction
{
  GtkAction           parent_instance;

  PicmanContext        *context;

  PicmanRGB            *color;
  PicmanViewable       *viewable;
  PangoEllipsizeMode  ellipsize;
  gint                max_width_chars;
};

struct _PicmanActionClass
{
  GtkActionClass parent_class;
};


GType        picman_action_get_type     (void) G_GNUC_CONST;

PicmanAction * picman_action_new          (const gchar *name,
                                       const gchar *label,
                                       const gchar *tooltip,
                                       const gchar *stock_id);

gint         picman_action_name_compare (PicmanAction  *action1,
                                       PicmanAction  *action2);


#endif  /* __PICMAN_ACTION_H__ */
