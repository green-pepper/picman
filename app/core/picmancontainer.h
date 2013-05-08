/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmancontainer.h
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTAINER_H__
#define __PICMAN_CONTAINER_H__


#include "picmanobject.h"


#define PICMAN_TYPE_CONTAINER            (picman_container_get_type ())
#define PICMAN_CONTAINER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTAINER, PicmanContainer))
#define PICMAN_CONTAINER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTAINER, PicmanContainerClass))
#define PICMAN_IS_CONTAINER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTAINER))
#define PICMAN_IS_CONTAINER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTAINER))
#define PICMAN_CONTAINER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTAINER, PicmanContainerClass))


typedef struct _PicmanContainerClass PicmanContainerClass;
typedef struct _PicmanContainerPriv  PicmanContainerPriv;

struct _PicmanContainer
{
  PicmanObject         parent_instance;

  PicmanContainerPriv *priv;
};

struct _PicmanContainerClass
{
  PicmanObjectClass  parent_class;

  /*  signals  */
  void         (* add)                (PicmanContainer       *container,
                                       PicmanObject          *object);
  void         (* remove)             (PicmanContainer       *container,
                                       PicmanObject          *object);
  void         (* reorder)            (PicmanContainer       *container,
                                       PicmanObject          *object,
                                       gint                 new_index);
  void         (* freeze)             (PicmanContainer       *container);
  void         (* thaw)               (PicmanContainer       *container);

  /*  virtual functions  */
  void         (* clear)              (PicmanContainer       *container);
  gboolean     (* have)               (const PicmanContainer *container,
                                       const PicmanObject    *object);
  void         (* foreach)            (const PicmanContainer *container,
                                       GFunc                func,
                                       gpointer             user_data);
  PicmanObject * (* get_child_by_name)  (const PicmanContainer *container,
                                       const gchar         *name);
  PicmanObject * (* get_child_by_index) (const PicmanContainer *container,
                                       gint                 index);
  gint         (* get_child_index)    (const PicmanContainer *container,
                                       const PicmanObject    *object);
};


GType        picman_container_get_type           (void) G_GNUC_CONST;

GType        picman_container_get_children_type  (const PicmanContainer *container);
PicmanContainerPolicy picman_container_get_policy  (const PicmanContainer *container);
gint         picman_container_get_n_children     (const PicmanContainer *container);

gboolean     picman_container_add                (PicmanContainer       *container,
                                                PicmanObject          *object);
gboolean     picman_container_remove             (PicmanContainer       *container,
                                                PicmanObject          *object);
gboolean     picman_container_insert             (PicmanContainer       *container,
                                                PicmanObject          *object,
                                                gint                 new_index);
gboolean     picman_container_reorder            (PicmanContainer       *container,
                                                PicmanObject          *object,
                                                gint                 new_index);

void         picman_container_freeze             (PicmanContainer       *container);
void         picman_container_thaw               (PicmanContainer       *container);
gboolean     picman_container_frozen             (PicmanContainer       *container);

void         picman_container_clear              (PicmanContainer       *container);
gboolean     picman_container_is_empty           (const PicmanContainer *container);
gboolean     picman_container_have               (const PicmanContainer *container,
                                                PicmanObject          *object);
void         picman_container_foreach            (const PicmanContainer *container,
                                                GFunc                func,
                                                gpointer             user_data);

PicmanObject * picman_container_get_child_by_name  (const PicmanContainer *container,
                                                const gchar         *name);
PicmanObject * picman_container_get_child_by_index (const PicmanContainer *container,
                                                gint                 index);
PicmanObject * picman_container_get_first_child    (const PicmanContainer *container);
PicmanObject * picman_container_get_last_child     (const PicmanContainer *container);
gint         picman_container_get_child_index    (const PicmanContainer *container,
                                                const PicmanObject    *object);

PicmanObject * picman_container_get_neighbor_of    (const PicmanContainer *container,
                                                const PicmanObject    *object);

gchar     ** picman_container_get_name_array     (const PicmanContainer *container,
                                                gint                *length);

GQuark       picman_container_add_handler        (PicmanContainer       *container,
                                                const gchar         *signame,
                                                GCallback            callback,
                                                gpointer             callback_data);
void         picman_container_remove_handler     (PicmanContainer       *container,
                                                GQuark               id);


#endif  /* __PICMAN_CONTAINER_H__ */
