/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansessioninfo.h
 * Copyright (C) 2001-2008 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_SESSION_INFO_H__
#define __PICMAN_SESSION_INFO_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_SESSION_INFO            (picman_session_info_get_type ())
#define PICMAN_SESSION_INFO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SESSION_INFO, PicmanSessionInfo))
#define PICMAN_SESSION_INFO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SESSION_INFO, PicmanSessionInfoClass))
#define PICMAN_IS_SESSION_INFO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SESSION_INFO))
#define PICMAN_IS_SESSION_INFO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SESSION_INFO))
#define PICMAN_SESSION_INFO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SESSION_INFO, PicmanSessionInfoClass))


typedef struct _PicmanSessionInfoPrivate  PicmanSessionInfoPrivate;
typedef struct _PicmanSessionInfoClass    PicmanSessionInfoClass;

/**
 * PicmanSessionInfo:
 *
 * Contains session info for one toplevel window in the interface such
 * as a dock, the empty-image-window, or the open/save dialog.
 */
struct _PicmanSessionInfo
{
  PicmanObject  parent_instance;

  PicmanSessionInfoPrivate *p;
};

struct _PicmanSessionInfoClass
{
  PicmanObjectClass  parent_class;

  gint             position_accuracy;
};


GType                    picman_session_info_get_type                      (void) G_GNUC_CONST;
PicmanSessionInfo *        picman_session_info_new                           (void);
void                     picman_session_info_restore                       (PicmanSessionInfo        *info,
                                                                          PicmanDialogFactory      *factory);
void                     picman_session_info_apply_geometry                (PicmanSessionInfo        *info);
void                     picman_session_info_read_geometry                 (PicmanSessionInfo        *info,
                                                                          GdkEventConfigure      *cevent);
void                     picman_session_info_get_info                      (PicmanSessionInfo        *info);
void                     picman_session_info_get_info_with_widget          (PicmanSessionInfo        *info,
                                                                          GtkWidget              *widget);
void                     picman_session_info_clear_info                    (PicmanSessionInfo        *info);
gboolean                 picman_session_info_is_singleton                  (PicmanSessionInfo        *info);
gboolean                 picman_session_info_is_session_managed            (PicmanSessionInfo        *info);
gboolean                 picman_session_info_get_remember_size             (PicmanSessionInfo        *info);
gboolean                 picman_session_info_get_remember_if_open          (PicmanSessionInfo        *info);
GtkWidget              * picman_session_info_get_widget                    (PicmanSessionInfo        *info);
void                     picman_session_info_set_widget                    (PicmanSessionInfo        *info,
                                                                          GtkWidget              *widget);
PicmanDialogFactoryEntry * picman_session_info_get_factory_entry             (PicmanSessionInfo        *info);
void                     picman_session_info_set_factory_entry             (PicmanSessionInfo        *info,
                                                                          PicmanDialogFactoryEntry *entry);
gboolean                 picman_session_info_get_open                      (PicmanSessionInfo        *info);
void                     picman_session_info_append_book                   (PicmanSessionInfo        *info,
                                                                          PicmanSessionInfoBook    *book);
gint                     picman_session_info_get_x                         (PicmanSessionInfo        *info);
gint                     picman_session_info_get_y                         (PicmanSessionInfo        *info);
gint                     picman_session_info_get_width                     (PicmanSessionInfo        *info);
gint                     picman_session_info_get_height                    (PicmanSessionInfo        *info);
void                     picman_session_info_class_set_position_accuracy   (PicmanSessionInfoClass   *klass,
                                                                          gint                    accuracy);
gint                     picman_session_info_class_apply_position_accuracy (PicmanSessionInfoClass   *klass,
                                                                          gint                    position);


#endif  /* __PICMAN_SESSION_INFO_H__ */
