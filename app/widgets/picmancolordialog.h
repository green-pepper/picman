/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancolordialog.h
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

#ifndef __PICMAN_COLOR_DIALOG_H__
#define __PICMAN_COLOR_DIALOG_H__


#include "picmanviewabledialog.h"

#include "gui/color-history.h"


#define PICMAN_TYPE_COLOR_DIALOG            (picman_color_dialog_get_type ())
#define PICMAN_COLOR_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_DIALOG, PicmanColorDialog))
#define PICMAN_COLOR_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_DIALOG, PicmanColorDialogClass))
#define PICMAN_IS_COLOR_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_DIALOG))
#define PICMAN_IS_COLOR_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_DIALOG))
#define PICMAN_COLOR_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_DIALOG, PicmanColorDialogClass))


typedef struct _PicmanColorDialogClass PicmanColorDialogClass;

struct _PicmanColorDialog
{
  PicmanViewableDialog   parent_instance;

  gboolean             wants_updates;

  GtkWidget           *selection;
  GtkWidget           *history[COLOR_HISTORY_SIZE];
};

struct _PicmanColorDialogClass
{
  PicmanViewableDialogClass  parent_class;

  void (* update) (PicmanColorDialog      *dialog,
                   const PicmanRGB        *color,
                   PicmanColorDialogState  state);
};


GType       picman_color_dialog_get_type  (void) G_GNUC_CONST;

GtkWidget * picman_color_dialog_new       (PicmanViewable      *viewable,
                                         PicmanContext       *context,
                                         const gchar       *title,
                                         const gchar       *stock_id,
                                         const gchar       *desc,
                                         GtkWidget         *parent,
                                         PicmanDialogFactory *dialog_factory,
                                         const gchar       *dialog_identifier,
                                         const PicmanRGB     *color,
                                         gboolean           wants_update,
                                         gboolean           show_alpha);

void        picman_color_dialog_set_color (PicmanColorDialog   *dialog,
                                         const PicmanRGB     *color);
void        picman_color_dialog_get_color (PicmanColorDialog   *dialog,
                                         PicmanRGB           *color);


#endif /* __PICMAN_COLOR_DIALOG_H__ */
