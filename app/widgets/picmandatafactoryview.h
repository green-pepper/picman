/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandatafactoryview.h
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

#ifndef __PICMAN_DATA_FACTORY_VIEW_H__
#define __PICMAN_DATA_FACTORY_VIEW_H__


#include "picmancontainereditor.h"


#define PICMAN_TYPE_DATA_FACTORY_VIEW            (picman_data_factory_view_get_type ())
#define PICMAN_DATA_FACTORY_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DATA_FACTORY_VIEW, PicmanDataFactoryView))
#define PICMAN_DATA_FACTORY_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DATA_FACTORY_VIEW, PicmanDataFactoryViewClass))
#define PICMAN_IS_DATA_FACTORY_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DATA_FACTORY_VIEW))
#define PICMAN_IS_DATA_FACTORY_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DATA_FACTORY_VIEW))
#define PICMAN_DATA_FACTORY_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DATA_FACTORY_VIEW, PicmanDataFactoryViewClass))


typedef struct _PicmanDataFactoryViewClass  PicmanDataFactoryViewClass;
typedef struct _PicmanDataFactoryViewPriv   PicmanDataFactoryViewPriv;

struct _PicmanDataFactoryView
{
  PicmanContainerEditor      parent_instance;

  PicmanDataFactoryViewPriv *priv;
};

struct _PicmanDataFactoryViewClass
{
  PicmanContainerEditorClass  parent_class;
};


GType             picman_data_factory_view_get_type             (void) G_GNUC_CONST;

GtkWidget *       picman_data_factory_view_new                  (PicmanViewType      view_type,
                                                               PicmanDataFactory  *factory,
                                                               PicmanContext      *context,
                                                               gint              view_size,
                                                               gint              view_border_width,
                                                               PicmanMenuFactory  *menu_factory,
                                                               const gchar      *menu_identifier,
                                                               const gchar      *ui_path,
                                                               const gchar      *action_group);

GtkWidget       * picman_data_factory_view_get_edit_button      (PicmanDataFactoryView *factory_view);
GtkWidget       * picman_data_factory_view_get_duplicate_button (PicmanDataFactoryView *factory_view);
PicmanDataFactory * picman_data_factory_view_get_data_factory     (PicmanDataFactoryView *factory_view);
GType             picman_data_factory_view_get_children_type    (PicmanDataFactoryView *factory_view);
gboolean          picman_data_factory_view_has_data_new_func    (PicmanDataFactoryView *factory_view);
gboolean          picman_data_factory_view_have                 (PicmanDataFactoryView *factory_view,
                                                               PicmanObject          *object);


#endif  /*  __PICMAN_DATA_FACTORY_VIEW_H__  */
