/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoverlaydialog.h
 * Copyright (C) 2009-2010  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_OVERLAY_DIALOG_H__
#define __PICMAN_OVERLAY_DIALOG_H__


#include "picmanoverlayframe.h"


#define PICMAN_TYPE_OVERLAY_DIALOG            (picman_overlay_dialog_get_type ())
#define PICMAN_OVERLAY_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OVERLAY_DIALOG, PicmanOverlayDialog))
#define PICMAN_OVERLAY_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_OVERLAY_DIALOG, PicmanOverlayDialogClass))
#define PICMAN_IS_OVERLAY_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OVERLAY_DIALOG))
#define PICMAN_IS_OVERLAY_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_OVERLAY_DIALOG))
#define PICMAN_OVERLAY_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_OVERLAY_DIALOG, PicmanOverlayDialogClass))


typedef struct _PicmanOverlayDialog      PicmanOverlayDialog;
typedef struct _PicmanOverlayDialogClass PicmanOverlayDialogClass;

struct _PicmanOverlayDialog
{
  PicmanOverlayFrame  parent_instance;

  GtkWidget        *action_area;
};

struct _PicmanOverlayDialogClass
{
  PicmanOverlayFrameClass  parent_class;

  void (* response) (PicmanOverlayDialog *overlay,
                     gint               response_id);

  void (* close)    (PicmanOverlayDialog *overlay);
};


GType       picman_overlay_dialog_get_type           (void) G_GNUC_CONST;

GtkWidget * picman_overlay_dialog_new                (PicmanToolInfo    *tool_info,
                                                    const gchar     *desc,
                                                    ...) G_GNUC_NULL_TERMINATED;

void        picman_overlay_dialog_response           (PicmanOverlayDialog *overlay,
                                                    gint             response_id);
void        picman_overlay_dialog_add_buttons_valist (PicmanOverlayDialog *overlay,
                                                    va_list          args);
GtkWidget * picman_overlay_dialog_add_button         (PicmanOverlayDialog *overlay,
                                                    const gchar     *button_text,
                                                    gint             response_id);


#endif /* __PICMAN_OVERLAY_DIALOG_H__ */
