/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanactionfactory.h
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

#ifndef __PICMAN_ACTION_FACTORY_H__
#define __PICMAN_ACTION_FACTORY_H__


#include "core/picmanobject.h"


typedef struct _PicmanActionFactoryEntry PicmanActionFactoryEntry;

struct _PicmanActionFactoryEntry
{
  gchar                     *identifier;
  gchar                     *label;
  gchar                     *stock_id;
  PicmanActionGroupSetupFunc   setup_func;
  PicmanActionGroupUpdateFunc  update_func;
};


#define PICMAN_TYPE_ACTION_FACTORY            (picman_action_factory_get_type ())
#define PICMAN_ACTION_FACTORY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ACTION_FACTORY, PicmanActionFactory))
#define PICMAN_ACTION_FACTORY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ACTION_FACTORY, PicmanActionFactoryClass))
#define PICMAN_IS_ACTION_FACTORY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ACTION_FACTORY))
#define PICMAN_IS_ACTION_FACTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ACTION_FACTORY))
#define PICMAN_ACTION_FACTORY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ACTION_FACTORY, PicmanActionFactoryClass))


typedef struct _PicmanActionFactoryClass  PicmanActionFactoryClass;

struct _PicmanActionFactory
{
  PicmanObject  parent_instance;

  Picman       *picman;
  GList      *registered_groups;
};

struct _PicmanActionFactoryClass
{
  PicmanObjectClass  parent_class;
};


GType               picman_action_factory_get_type (void) G_GNUC_CONST;

PicmanActionFactory * picman_action_factory_new      (Picman              *picman);

void          picman_action_factory_group_register (PicmanActionFactory *factory,
                                                  const gchar       *identifier,
                                                  const gchar       *label,
                                                  const gchar       *stock_id,
                                                  PicmanActionGroupSetupFunc  setup_func,
                                                  PicmanActionGroupUpdateFunc update_func);

PicmanActionGroup * picman_action_factory_group_new  (PicmanActionFactory *factory,
                                                  const gchar       *identifier,
                                                  gpointer           user_data);


#endif  /*  __PICMAN_ACTION_FACTORY_H__  */
