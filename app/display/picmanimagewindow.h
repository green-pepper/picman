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

#ifndef __PICMAN_IMAGE_WINDOW_H__
#define __PICMAN_IMAGE_WINDOW_H__


#include "widgets/picmanwindow.h"


#define PICMAN_TYPE_IMAGE_WINDOW            (picman_image_window_get_type ())
#define PICMAN_IMAGE_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGE_WINDOW, PicmanImageWindow))
#define PICMAN_IMAGE_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_IMAGE_WINDOW, PicmanImageWindowClass))
#define PICMAN_IS_IMAGE_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGE_WINDOW))
#define PICMAN_IS_IMAGE_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_IMAGE_WINDOW))
#define PICMAN_IMAGE_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_IMAGE_WINDOW, PicmanImageWindowClass))


typedef struct _PicmanImageWindowClass  PicmanImageWindowClass;

struct _PicmanImageWindow
{
  PicmanWindow  parent_instance;
};

struct _PicmanImageWindowClass
{
  PicmanWindowClass  parent_class;
};


GType              picman_image_window_get_type             (void) G_GNUC_CONST;

PicmanImageWindow  * picman_image_window_new                  (Picman              *picman,
                                                           PicmanImage         *image,
                                                           PicmanMenuFactory   *menu_factory,
                                                           PicmanDialogFactory *dialog_factory);
void               picman_image_window_destroy              (PicmanImageWindow   *window);

PicmanUIManager    * picman_image_window_get_ui_manager       (PicmanImageWindow  *window);
PicmanDockColumns  * picman_image_window_get_left_docks       (PicmanImageWindow  *window);
PicmanDockColumns  * picman_image_window_get_right_docks      (PicmanImageWindow  *window);

void               picman_image_window_add_shell            (PicmanImageWindow  *window,
                                                           PicmanDisplayShell *shell);
PicmanDisplayShell * picman_image_window_get_shell            (PicmanImageWindow  *window,
                                                           gint              index);
void               picman_image_window_remove_shell         (PicmanImageWindow  *window,
                                                           PicmanDisplayShell *shell);

gint               picman_image_window_get_n_shells         (PicmanImageWindow  *window);

void               picman_image_window_set_active_shell     (PicmanImageWindow  *window,
                                                           PicmanDisplayShell *shell);
PicmanDisplayShell * picman_image_window_get_active_shell     (PicmanImageWindow  *window);

void               picman_image_window_set_fullscreen       (PicmanImageWindow  *window,
                                                           gboolean          fullscreen);
gboolean           picman_image_window_get_fullscreen       (PicmanImageWindow  *window);

void               picman_image_window_set_show_menubar     (PicmanImageWindow  *window,
                                                           gboolean          show);
gboolean           picman_image_window_get_show_menubar     (PicmanImageWindow  *window);

void               picman_image_window_set_show_statusbar   (PicmanImageWindow  *window,
                                                           gboolean          show);
gboolean           picman_image_window_get_show_statusbar   (PicmanImageWindow  *window);

gboolean           picman_image_window_is_iconified         (PicmanImageWindow  *window);
gboolean           picman_image_window_is_maximized         (PicmanImageWindow  *window);

gboolean           picman_image_window_has_toolbox          (PicmanImageWindow  *window);

void               picman_image_window_shrink_wrap          (PicmanImageWindow  *window,
                                                           gboolean          grow_only);

GtkWidget        * picman_image_window_get_default_dockbook (PicmanImageWindow  *window);

void               picman_image_window_keep_canvas_pos      (PicmanImageWindow  *window);

#endif /* __PICMAN_IMAGE_WINDOW_H__ */
