/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanmenufactory.h
 * Copyright (C) 2003-2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_MENU_FACTORY_H__
#define __PICMAN_MENU_FACTORY_H__


#include "core/picmanobject.h"


typedef struct _PicmanMenuFactoryEntry PicmanMenuFactoryEntry;

struct _PicmanMenuFactoryEntry
{
  gchar *identifier;
  GList *action_groups;
  GList *managed_uis;
};


#define PICMAN_TYPE_MENU_FACTORY            (picman_menu_factory_get_type ())
#define PICMAN_MENU_FACTORY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_MENU_FACTORY, PicmanMenuFactory))
#define PICMAN_MENU_FACTORY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_MENU_FACTORY, PicmanMenuFactoryClass))
#define PICMAN_IS_MENU_FACTORY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_MENU_FACTORY))
#define PICMAN_IS_MENU_FACTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_MENU_FACTORY))
#define PICMAN_MENU_FACTORY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_MENU_FACTORY, PicmanMenuFactoryClass))


typedef struct _PicmanMenuFactoryPrivate  PicmanMenuFactoryPrivate;
typedef struct _PicmanMenuFactoryClass    PicmanMenuFactoryClass;

struct _PicmanMenuFactory
{
  PicmanObject              parent_instance;

  PicmanMenuFactoryPrivate *p;
};

struct _PicmanMenuFactoryClass
{
  PicmanObjectClass  parent_class;
};


GType             picman_menu_factory_get_type             (void) G_GNUC_CONST;
PicmanMenuFactory * picman_menu_factory_new                  (Picman              *picman,
                                                          PicmanActionFactory *action_factory);
void              picman_menu_factory_manager_register     (PicmanMenuFactory   *factory,
                                                          const gchar       *identifier,
                                                          const gchar       *first_group,
                                                          ...)  G_GNUC_NULL_TERMINATED;
GList           * picman_menu_factory_get_registered_menus (PicmanMenuFactory   *factory);
PicmanUIManager   * picman_menu_factory_manager_new          (PicmanMenuFactory   *factory,
                                                          const gchar       *identifier,
                                                          gpointer           callback_data,
                                                          gboolean           create_tearoff);



#endif  /*  __PICMAN_MENU_FACTORY_H__  */
