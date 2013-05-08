/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancontrollerinfo.h
 * Copyright (C) 2004-2005 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTROLLER_INFO_H__
#define __PICMAN_CONTROLLER_INFO_H__


#include "core/picmanviewable.h"


typedef gboolean (* PicmanControllerEventSnooper) (PicmanControllerInfo        *info,
                                                 PicmanController            *controller,
                                                 const PicmanControllerEvent *event,
                                                 gpointer                   user_data);


#define PICMAN_TYPE_CONTROLLER_INFO            (picman_controller_info_get_type ())
#define PICMAN_CONTROLLER_INFO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTROLLER_INFO, PicmanControllerInfo))
#define PICMAN_CONTROLLER_INFO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTROLLER_INFO, PicmanControllerInfoClass))
#define PICMAN_IS_CONTROLLER_INFO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTROLLER_INFO))
#define PICMAN_IS_CONTROLLER_INFO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTROLLER_INFO))
#define PICMAN_CONTROLLER_INFO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTROLLER_INFO, PicmanControllerInfoClass))


typedef struct _PicmanControllerInfoClass PicmanControllerInfoClass;

struct _PicmanControllerInfo
{
  PicmanViewable                parent_instance;

  gboolean                    enabled;
  gboolean                    debug_events;

  PicmanController             *controller;
  GHashTable                 *mapping;

  PicmanControllerEventSnooper  snooper;
  gpointer                    snooper_data;
};

struct _PicmanControllerInfoClass
{
  PicmanViewableClass  parent_class;

  gboolean (* event_mapped) (PicmanControllerInfo        *info,
                             PicmanController            *controller,
                             const PicmanControllerEvent *event,
                             const gchar               *action_name);
};


GType    picman_controller_info_get_type          (void) G_GNUC_CONST;

PicmanControllerInfo * picman_controller_info_new   (GType                       type);

void     picman_controller_info_set_enabled       (PicmanControllerInfo         *info,
                                                 gboolean                    enabled);
gboolean picman_controller_info_get_enabled       (PicmanControllerInfo         *info);

void     picman_controller_info_set_event_snooper (PicmanControllerInfo         *info,
                                                 PicmanControllerEventSnooper  snooper,
                                                 gpointer                    snooper_data);


#endif /* __PICMAN_CONTROLLER_INFO_H__ */
