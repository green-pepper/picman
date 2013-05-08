/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandockable.h
 * Copyright (C) 2001-2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DOCKABLE_H__
#define __PICMAN_DOCKABLE_H__


#define PICMAN_DOCKABLE_DRAG_OFFSET (-6)


#define PICMAN_TYPE_DOCKABLE            (picman_dockable_get_type ())
#define PICMAN_DOCKABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DOCKABLE, PicmanDockable))
#define PICMAN_DOCKABLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DOCKABLE, PicmanDockableClass))
#define PICMAN_IS_DOCKABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DOCKABLE))
#define PICMAN_IS_DOCKABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DOCKABLE))
#define PICMAN_DOCKABLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DOCKABLE, PicmanDockableClass))


typedef struct _PicmanDockablePrivate PicmanDockablePrivate;
typedef struct _PicmanDockableClass   PicmanDockableClass;

/**
 * PicmanDockable:
 *
 * A kind of adpater to make other widgets dockable. The widget to
 * dock is put inside the PicmanDockable, which is put in a
 * PicmanDockbook.
 */
struct _PicmanDockable
{
  GtkBin               parent_instance;

  PicmanDockablePrivate *p;
};

struct _PicmanDockableClass
{
  GtkBinClass  parent_class;
};


GType           picman_dockable_get_type         (void) G_GNUC_CONST;

GtkWidget     * picman_dockable_new              (const gchar    *name,
                                                const gchar    *blurb,
                                                const gchar    *stock_id,
                                                const gchar    *help_id);
void            picman_dockable_set_dockbook     (PicmanDockable   *dockable,
                                                PicmanDockbook   *dockbook);
PicmanDockbook  * picman_dockable_get_dockbook     (PicmanDockable   *dockable);
PicmanTabStyle    picman_dockable_get_tab_style    (PicmanDockable   *dockable);
const gchar   * picman_dockable_get_name         (PicmanDockable   *dockable);
const gchar   * picman_dockable_get_blurb        (PicmanDockable   *dockable);
const gchar   * picman_dockable_get_help_id      (PicmanDockable   *dockable);
const gchar   * picman_dockable_get_stock_id     (PicmanDockable   *dockable);
GtkWidget     * picman_dockable_get_icon         (PicmanDockable   *dockable,
                                                GtkIconSize     size);

gboolean        picman_dockable_get_locked       (PicmanDockable   *dockable);
void            picman_dockable_set_drag_pos     (PicmanDockable   *dockable,
                                                gint            drag_x,
                                                gint            drag_y);
void            picman_dockable_get_drag_pos     (PicmanDockable   *dockable,
                                                gint           *drag_x,
                                                gint           *drag_y);
PicmanPanedBox  * picman_dockable_get_drag_handler (PicmanDockable  *dockable);

void            picman_dockable_set_locked       (PicmanDockable   *dockable,
                                                gboolean        lock);
gboolean        picman_dockable_is_locked        (PicmanDockable   *dockable);

                                                      
void            picman_dockable_set_tab_style           (PicmanDockable   *dockable,
                                                       PicmanTabStyle    tab_style);
gboolean        picman_dockable_set_actual_tab_style    (PicmanDockable   *dockable,
                                                       PicmanTabStyle    tab_style);
PicmanTabStyle    picman_dockable_get_actual_tab_style    (PicmanDockable   *dockable);
GtkWidget     * picman_dockable_create_tab_widget       (PicmanDockable   *dockable,
                                                       PicmanContext    *context,
                                                       PicmanTabStyle    tab_style,
                                                       GtkIconSize     size);
GtkWidget     * picman_dockable_create_drag_widget      (PicmanDockable   *dockable);
void            picman_dockable_set_context             (PicmanDockable   *dockable,
                                                       PicmanContext    *context);
PicmanUIManager * picman_dockable_get_menu                (PicmanDockable   *dockable,
                                                const gchar   **ui_path,
                                                gpointer       *popup_data);
void            picman_dockable_set_drag_handler (PicmanDockable   *dockable,
                                                PicmanPanedBox   *drag_handler);

void            picman_dockable_detach           (PicmanDockable   *dockable);

void            picman_dockable_blink            (PicmanDockable   *dockable);
void            picman_dockable_blink_cancel     (PicmanDockable   *dockable);


#endif /* __PICMAN_DOCKABLE_H__ */
