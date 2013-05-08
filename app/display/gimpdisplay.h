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

#ifndef __PICMAN_DISPLAY_H__
#define __PICMAN_DISPLAY_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_DISPLAY            (picman_display_get_type ())
#define PICMAN_DISPLAY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DISPLAY, PicmanDisplay))
#define PICMAN_DISPLAY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DISPLAY, PicmanDisplayClass))
#define PICMAN_IS_DISPLAY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DISPLAY))
#define PICMAN_IS_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DISPLAY))
#define PICMAN_DISPLAY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DISPLAY, PicmanDisplayClass))


typedef struct _PicmanDisplayClass PicmanDisplayClass;

struct _PicmanDisplay
{
  PicmanObject         parent_instance;

  Picman              *picman;
  PicmanDisplayConfig *config;

};

struct _PicmanDisplayClass
{
  PicmanObjectClass  parent_class;
};


GType              picman_display_get_type        (void) G_GNUC_CONST;

PicmanDisplay      * picman_display_new             (Picman              *picman,
                                                 PicmanImage         *image,
                                                 PicmanUnit           unit,
                                                 gdouble            scale,
                                                 PicmanMenuFactory   *menu_factory,
                                                 PicmanUIManager     *popup_manager,
                                                 PicmanDialogFactory *dialog_factory);
void               picman_display_delete          (PicmanDisplay       *display);
void               picman_display_close           (PicmanDisplay       *display);

gint               picman_display_get_ID          (PicmanDisplay       *display);
PicmanDisplay      * picman_display_get_by_ID       (Picman              *picman,
                                                 gint               ID);

gchar            * picman_display_get_action_name (PicmanDisplay       *display);

Picman             * picman_display_get_picman        (PicmanDisplay       *display);

PicmanImage        * picman_display_get_image       (PicmanDisplay       *display);
void               picman_display_set_image       (PicmanDisplay       *display,
                                                 PicmanImage         *image);

gint               picman_display_get_instance    (PicmanDisplay       *display);

PicmanDisplayShell * picman_display_get_shell       (PicmanDisplay       *display);

void               picman_display_empty           (PicmanDisplay       *display);
void               picman_display_fill            (PicmanDisplay       *display,
                                                 PicmanImage         *image,
                                                 PicmanUnit           unit,
                                                 gdouble            scale);

void               picman_display_update_area     (PicmanDisplay       *display,
                                                 gboolean           now,
                                                 gint               x,
                                                 gint               y,
                                                 gint               w,
                                                 gint               h);

void               picman_display_flush           (PicmanDisplay       *display);
void               picman_display_flush_now       (PicmanDisplay       *display);


#endif /*  __PICMAN_DISPLAY_H__  */
