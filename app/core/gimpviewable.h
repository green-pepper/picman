/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanviewable.h
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

#ifndef __PICMAN_VIEWABLE_H__
#define __PICMAN_VIEWABLE_H__


#include "picmanobject.h"

#include <gdk-pixbuf/gdk-pixbuf.h>


#define PICMAN_VIEWABLE_MAX_PREVIEW_SIZE 2048
#define PICMAN_VIEWABLE_MAX_POPUP_SIZE    256
#define PICMAN_VIEWABLE_MAX_BUTTON_SIZE    64
#define PICMAN_VIEWABLE_MAX_MENU_SIZE      48

#define PICMAN_VIEWABLE_PRIORITY_IDLE    G_PRIORITY_LOW


#define PICMAN_TYPE_VIEWABLE            (picman_viewable_get_type ())
#define PICMAN_VIEWABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VIEWABLE, PicmanViewable))
#define PICMAN_VIEWABLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_VIEWABLE, PicmanViewableClass))
#define PICMAN_IS_VIEWABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_VIEWABLE))
#define PICMAN_IS_VIEWABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_VIEWABLE))
#define PICMAN_VIEWABLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_VIEWABLE, PicmanViewableClass))


typedef struct _PicmanViewableClass PicmanViewableClass;

struct _PicmanViewable
{
  PicmanObject  parent_instance;
};

struct _PicmanViewableClass
{
  PicmanObjectClass  parent_class;

  const gchar     *default_stock_id;
  const gchar     *name_changed_signal;

  /*  signals  */
  void            (* invalidate_preview) (PicmanViewable  *viewable);
  void            (* size_changed)       (PicmanViewable  *viewable);

  /*  virtual functions  */
  gboolean        (* get_size)           (PicmanViewable  *viewable,
                                          gint          *width,
                                          gint          *height);
  void            (* get_preview_size)   (PicmanViewable  *viewable,
                                          gint           size,
                                          gboolean       is_popup,
                                          gboolean       dot_for_dot,
                                          gint          *width,
                                          gint          *height);
  gboolean        (* get_popup_size)     (PicmanViewable  *viewable,
                                          gint           width,
                                          gint           height,
                                          gboolean       dot_for_dot,
                                          gint          *popup_width,
                                          gint          *popup_height);
  PicmanTempBuf   * (* get_preview)        (PicmanViewable  *viewable,
                                          PicmanContext   *context,
                                          gint           width,
                                          gint           height);
  PicmanTempBuf   * (* get_new_preview)    (PicmanViewable  *viewable,
                                          PicmanContext   *context,
                                          gint           width,
                                          gint           height);
  GdkPixbuf     * (* get_pixbuf)         (PicmanViewable  *viewable,
                                          PicmanContext   *context,
                                          gint           width,
                                          gint           height);
  GdkPixbuf     * (* get_new_pixbuf)     (PicmanViewable  *viewable,
                                          PicmanContext   *context,
                                          gint           width,
                                          gint           height);
  gchar         * (* get_description)    (PicmanViewable  *viewable,
                                          gchar        **tooltip);

  PicmanContainer * (* get_children)       (PicmanViewable  *viewable);

  void            (* set_expanded)       (PicmanViewable  *viewable,
                                          gboolean       expand);
  gboolean        (* get_expanded)       (PicmanViewable  *viewable);
};


GType           picman_viewable_get_type           (void) G_GNUC_CONST;

void            picman_viewable_invalidate_preview (PicmanViewable  *viewable);
void            picman_viewable_size_changed       (PicmanViewable  *viewable);

void            picman_viewable_calc_preview_size  (gint           aspect_width,
                                                  gint           aspect_height,
                                                  gint           width,
                                                  gint           height,
                                                  gboolean       dot_for_dot,
                                                  gdouble        xresolution,
                                                  gdouble        yresolution,
                                                  gint          *return_width,
                                                  gint          *return_height,
                                                  gboolean      *scaling_up);

gboolean        picman_viewable_get_size           (PicmanViewable  *viewable,
                                                  gint          *width,
                                                  gint          *height);
void            picman_viewable_get_preview_size   (PicmanViewable  *viewable,
                                                  gint           size,
                                                  gboolean       popup,
                                                  gboolean       dot_for_dot,
                                                  gint          *width,
                                                  gint          *height);
gboolean        picman_viewable_get_popup_size     (PicmanViewable  *viewable,
                                                  gint           width,
                                                  gint           height,
                                                  gboolean       dot_for_dot,
                                                  gint          *popup_width,
                                                  gint          *popup_height);

PicmanTempBuf   * picman_viewable_get_preview        (PicmanViewable  *viewable,
                                                  PicmanContext   *context,
                                                  gint           width,
                                                  gint           height);
PicmanTempBuf   * picman_viewable_get_new_preview    (PicmanViewable  *viewable,
                                                  PicmanContext   *context,
                                                  gint           width,
                                                  gint           height);

PicmanTempBuf   * picman_viewable_get_dummy_preview  (PicmanViewable  *viewable,
                                                  gint           width,
                                                  gint           height,
                                                  const Babl    *format);

GdkPixbuf     * picman_viewable_get_pixbuf         (PicmanViewable  *viewable,
                                                  PicmanContext   *context,
                                                  gint           width,
                                                  gint           height);
GdkPixbuf     * picman_viewable_get_new_pixbuf     (PicmanViewable  *viewable,
                                                  PicmanContext   *context,
                                                  gint           width,
                                                  gint           height);

GdkPixbuf     * picman_viewable_get_dummy_pixbuf   (PicmanViewable  *viewable,
                                                  gint           width,
                                                  gint           height,
                                                  gboolean       with_alpha);

gchar         * picman_viewable_get_description    (PicmanViewable  *viewable,
                                                  gchar        **tooltip);

const gchar   * picman_viewable_get_stock_id       (PicmanViewable  *viewable);
void            picman_viewable_set_stock_id       (PicmanViewable  *viewable,
                                                  const gchar   *stock_id);

void            picman_viewable_preview_freeze     (PicmanViewable  *viewable);
void            picman_viewable_preview_thaw       (PicmanViewable  *viewable);
gboolean        picman_viewable_preview_is_frozen  (PicmanViewable  *viewable);

PicmanViewable  * picman_viewable_get_parent         (PicmanViewable  *viewable);
void            picman_viewable_set_parent         (PicmanViewable  *viewable,
                                                  PicmanViewable  *parent);

PicmanContainer * picman_viewable_get_children       (PicmanViewable  *viewable);
gboolean        picman_viewable_get_expanded       (PicmanViewable  *viewable);
void            picman_viewable_set_expanded       (PicmanViewable  *viewable,
                                                  gboolean       expanded);

gboolean        picman_viewable_is_ancestor        (PicmanViewable  *ancestor,
                                                  PicmanViewable  *descendant);


#endif  /* __PICMAN_VIEWABLE_H__ */
