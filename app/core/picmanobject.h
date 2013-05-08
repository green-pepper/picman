/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_OBJECT_H__
#define __PICMAN_OBJECT_H__


#define PICMAN_TYPE_OBJECT            (picman_object_get_type ())
#define PICMAN_OBJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OBJECT, PicmanObject))
#define PICMAN_OBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_OBJECT, PicmanObjectClass))
#define PICMAN_IS_OBJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OBJECT))
#define PICMAN_IS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_OBJECT))
#define PICMAN_OBJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_OBJECT, PicmanObjectClass))


typedef struct _PicmanObjectPrivate  PicmanObjectPrivate;
typedef struct _PicmanObjectClass    PicmanObjectClass;

struct _PicmanObject
{
  GObject            parent_instance;

  PicmanObjectPrivate *p;
};

struct _PicmanObjectClass
{
  GObjectClass  parent_class;

  /*  signals  */
  void    (* disconnect)   (PicmanObject *object);
  void    (* name_changed) (PicmanObject *object);

  /*  virtual functions  */
  gint64  (* get_memsize)  (PicmanObject *object,
                            gint64     *gui_size);
};


GType         picman_object_get_type        (void) G_GNUC_CONST;

void          picman_object_set_name        (PicmanObject       *object,
                                           const gchar      *name);
void          picman_object_set_name_safe   (PicmanObject       *object,
                                           const gchar      *name);
void          picman_object_set_static_name (PicmanObject       *object,
                                           const gchar      *name);
void          picman_object_take_name       (PicmanObject       *object,
                                           gchar            *name);
const gchar * picman_object_get_name        (gconstpointer     object);
void          picman_object_name_changed    (PicmanObject       *object);
void          picman_object_name_free       (PicmanObject       *object);

gint          picman_object_name_collate    (PicmanObject       *object1,
                                           PicmanObject       *object2);
gint64        picman_object_get_memsize     (PicmanObject       *object,
                                           gint64           *gui_size);


#endif  /* __PICMAN_OBJECT_H__ */
