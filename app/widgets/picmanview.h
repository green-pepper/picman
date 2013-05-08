/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanview.h
 * Copyright (C) 2001-2006 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_VIEW_H__
#define __PICMAN_VIEW_H__


#define PICMAN_TYPE_VIEW            (picman_view_get_type ())
#define PICMAN_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VIEW, PicmanView))
#define PICMAN_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_VIEW, PicmanViewClass))
#define PICMAN_IS_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, PICMAN_TYPE_VIEW))
#define PICMAN_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_VIEW))
#define PICMAN_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_VIEW, PicmanViewClass))


typedef struct _PicmanViewClass  PicmanViewClass;

struct _PicmanView
{
  GtkWidget         parent_instance;

  GdkWindow        *event_window;

  PicmanViewable     *viewable;
  PicmanViewRenderer *renderer;

  guint             clickable : 1;
  guint             eat_button_events : 1;
  guint             show_popup : 1;
  guint             expand : 1;

  /*< private >*/
  guint             in_button : 1;
  guint             has_grab : 1;
  GdkModifierType   press_state;
};

struct _PicmanViewClass
{
  GtkWidgetClass  parent_class;

  /*  signals  */
  void        (* set_viewable)   (PicmanView        *view,
                                  PicmanViewable    *old_viewable,
                                  PicmanViewable    *new_viewable);
  void        (* clicked)        (PicmanView        *view,
                                  GdkModifierType  modifier_state);
  void        (* double_clicked) (PicmanView        *view);
  void        (* context)        (PicmanView        *view);
};


GType          picman_view_get_type          (void) G_GNUC_CONST;

GtkWidget    * picman_view_new               (PicmanContext   *context,
                                            PicmanViewable  *viewable,
                                            gint           size,
                                            gint           border_width,
                                            gboolean       is_popup);
GtkWidget    * picman_view_new_full          (PicmanContext   *context,
                                            PicmanViewable  *viewable,
                                            gint           width,
                                            gint           height,
                                            gint           border_width,
                                            gboolean       is_popup,
                                            gboolean       clickable,
                                            gboolean       show_popup);
GtkWidget    * picman_view_new_by_types      (PicmanContext   *context,
                                            GType          view_type,
                                            GType          viewable_type,
                                            gint           size,
                                            gint           border_width,
                                            gboolean       is_popup);
GtkWidget    * picman_view_new_full_by_types (PicmanContext   *context,
                                            GType          view_type,
                                            GType          viewable_type,
                                            gint           width,
                                            gint           height,
                                            gint           border_width,
                                            gboolean       is_popup,
                                            gboolean       clickable,
                                            gboolean       show_popup);

PicmanViewable * picman_view_get_viewable      (PicmanView      *view);
void           picman_view_set_viewable      (PicmanView      *view,
                                            PicmanViewable  *viewable);
void           picman_view_set_expand        (PicmanView      *view,
                                            gboolean       expand);


#endif /* __PICMAN_VIEW_H__ */
