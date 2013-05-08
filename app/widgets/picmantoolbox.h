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

#ifndef __PICMAN_TOOLBOX_H__
#define __PICMAN_TOOLBOX_H__


#include "picmandock.h"


#define PICMAN_TYPE_TOOLBOX            (picman_toolbox_get_type ())
#define PICMAN_TOOLBOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TOOLBOX, PicmanToolbox))
#define PICMAN_TOOLBOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TOOLBOX, PicmanToolboxClass))
#define PICMAN_IS_TOOLBOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TOOLBOX))
#define PICMAN_IS_TOOLBOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TOOLBOX))
#define PICMAN_TOOLBOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TOOLBOX, PicmanToolboxClass))


typedef struct _PicmanToolboxClass   PicmanToolboxClass;
typedef struct _PicmanToolboxPrivate PicmanToolboxPrivate;

struct _PicmanToolbox
{
  PicmanDock parent_instance;

  PicmanToolboxPrivate *p;
};

struct _PicmanToolboxClass
{
  PicmanDockClass parent_class;
};


GType               picman_toolbox_get_type           (void) G_GNUC_CONST;
GtkWidget         * picman_toolbox_new                (PicmanDialogFactory *factory,
                                                     PicmanContext       *context,
                                                     PicmanUIManager     *ui_manager);
PicmanContext       * picman_toolbox_get_context        (PicmanToolbox       *toolbox);
void                picman_toolbox_set_drag_handler   (PicmanToolbox       *toolbox,
                                                     PicmanPanedBox      *drag_handler);



#endif /* __PICMAN_TOOLBOX_H__ */
