/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasproxygroup.h
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CANVAS_PROXY_GROUP_H__
#define __PICMAN_CANVAS_PROXY_GROUP_H__


#include "picmancanvasgroup.h"


#define PICMAN_TYPE_CANVAS_PROXY_GROUP            (picman_canvas_proxy_group_get_type ())
#define PICMAN_CANVAS_PROXY_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_PROXY_GROUP, PicmanCanvasProxyGroup))
#define PICMAN_CANVAS_PROXY_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_PROXY_GROUP, PicmanCanvasProxyGroupClass))
#define PICMAN_IS_CANVAS_PROXY_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_PROXY_GROUP))
#define PICMAN_IS_CANVAS_PROXY_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_PROXY_GROUP))
#define PICMAN_CANVAS_PROXY_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_PROXY_GROUP, PicmanCanvasProxyGroupClass))


typedef struct _PicmanCanvasProxyGroup      PicmanCanvasProxyGroup;
typedef struct _PicmanCanvasProxyGroupClass PicmanCanvasProxyGroupClass;

struct _PicmanCanvasProxyGroup
{
  PicmanCanvasGroup  parent_instance;
};

struct _PicmanCanvasProxyGroupClass
{
  PicmanCanvasGroupClass  parent_class;
};


GType            picman_canvas_proxy_group_get_type    (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_proxy_group_new         (PicmanDisplayShell     *shell);

void             picman_canvas_proxy_group_add_item    (PicmanCanvasProxyGroup *group,
                                                      gpointer              object,
                                                      PicmanCanvasItem       *proxy_item);
void             picman_canvas_proxy_group_remove_item (PicmanCanvasProxyGroup *group,
                                                      gpointer              object);
PicmanCanvasItem * picman_canvas_proxy_group_get_item    (PicmanCanvasProxyGroup *group,
                                                      gpointer              object);


#endif /* __PICMAN_CANVAS_PROXY_GROUP_H__ */
