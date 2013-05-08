/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandatafactory.h
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

#ifndef __PICMAN_DATA_FACTORY_H__
#define __PICMAN_DATA_FACTORY_H__


#include "picmanobject.h"


typedef PicmanData * (* PicmanDataNewFunc)         (PicmanContext  *context,
                                                const gchar  *name);
typedef GList    * (* PicmanDataLoadFunc)        (PicmanContext  *context,
                                                const gchar  *filename,
                                                GError      **error);
typedef PicmanData * (* PicmanDataGetStandardFunc) (PicmanContext  *context);


typedef struct _PicmanDataFactoryLoaderEntry PicmanDataFactoryLoaderEntry;

struct _PicmanDataFactoryLoaderEntry
{
  PicmanDataLoadFunc  load_func;
  const gchar      *extension;
  gboolean          writable;
};


#define PICMAN_TYPE_DATA_FACTORY            (picman_data_factory_get_type ())
#define PICMAN_DATA_FACTORY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DATA_FACTORY, PicmanDataFactory))
#define PICMAN_DATA_FACTORY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DATA_FACTORY, PicmanDataFactoryClass))
#define PICMAN_IS_DATA_FACTORY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DATA_FACTORY))
#define PICMAN_IS_DATA_FACTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DATA_FACTORY))
#define PICMAN_DATA_FACTORY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DATA_FACTORY, PicmanDataFactoryClass))


typedef struct _PicmanDataFactoryClass  PicmanDataFactoryClass;
typedef struct _PicmanDataFactoryPriv   PicmanDataFactoryPriv;

struct _PicmanDataFactory
{
  PicmanObject           parent_instance;

  PicmanDataFactoryPriv *priv;
};

struct _PicmanDataFactoryClass
{
  PicmanObjectClass  parent_class;
};


GType             picman_data_factory_get_type (void) G_GNUC_CONST;

PicmanDataFactory * picman_data_factory_new      (Picman                             *picman,
                                              GType                             data_type,
                                              const gchar                      *path_property_name,
                                              const gchar                      *writable_property_name,
                                              const PicmanDataFactoryLoaderEntry *loader_entries,
                                              gint                              n_loader_entries,
                                              PicmanDataNewFunc                   new_func,
                                              PicmanDataGetStandardFunc           get_standard_func);

void            picman_data_factory_data_init         (PicmanDataFactory  *factory,
                                                     PicmanContext      *context,
                                                     gboolean          no_data);
void            picman_data_factory_data_refresh      (PicmanDataFactory  *factory,
                                                     PicmanContext      *context);
void            picman_data_factory_data_save         (PicmanDataFactory  *factory);
void            picman_data_factory_data_free         (PicmanDataFactory  *factory);

PicmanData      * picman_data_factory_data_new          (PicmanDataFactory  *factory,
                                                     PicmanContext      *context,
                                                     const gchar      *name);
PicmanData      * picman_data_factory_data_duplicate    (PicmanDataFactory  *factory,
                                                     PicmanData         *data);
gboolean        picman_data_factory_data_delete       (PicmanDataFactory  *factory,
                                                     PicmanData         *data,
                                                     gboolean          delete_from_disk,
                                                     GError          **error);
PicmanData      * picman_data_factory_data_get_standard (PicmanDataFactory  *factory,
                                                     PicmanContext      *context);
gboolean        picman_data_factory_data_save_single  (PicmanDataFactory  *factory,
                                                     PicmanData         *data,
                                                     GError          **error);
GType           picman_data_factory_get_data_type     (PicmanDataFactory  *factory);
PicmanContainer * picman_data_factory_get_container     (PicmanDataFactory  *factory);
PicmanContainer * picman_data_factory_get_container_obsolete
                                                    (PicmanDataFactory  *factory);
Picman          * picman_data_factory_get_picman          (PicmanDataFactory  *factory);
gboolean        picman_data_factory_has_data_new_func (PicmanDataFactory  *factory);


#endif  /*  __PICMAN_DATA_FACTORY_H__  */
