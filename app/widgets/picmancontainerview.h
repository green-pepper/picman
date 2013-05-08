/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainerview.h
 * Copyright (C) 2001-2010 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTAINER_VIEW_H__
#define __PICMAN_CONTAINER_VIEW_H__


typedef enum
{
  PICMAN_CONTAINER_VIEW_PROP_0,
  PICMAN_CONTAINER_VIEW_PROP_CONTAINER,
  PICMAN_CONTAINER_VIEW_PROP_CONTEXT,
  PICMAN_CONTAINER_VIEW_PROP_SELECTION_MODE,
  PICMAN_CONTAINER_VIEW_PROP_REORDERABLE,
  PICMAN_CONTAINER_VIEW_PROP_VIEW_SIZE,
  PICMAN_CONTAINER_VIEW_PROP_VIEW_BORDER_WIDTH,
  PICMAN_CONTAINER_VIEW_PROP_LAST = PICMAN_CONTAINER_VIEW_PROP_VIEW_BORDER_WIDTH
} PicmanContainerViewProp;


#define PICMAN_TYPE_CONTAINER_VIEW               (picman_container_view_interface_get_type ())
#define PICMAN_CONTAINER_VIEW(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTAINER_VIEW, PicmanContainerView))
#define PICMAN_IS_CONTAINER_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTAINER_VIEW))
#define PICMAN_CONTAINER_VIEW_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_CONTAINER_VIEW, PicmanContainerViewInterface))


typedef struct _PicmanContainerViewInterface PicmanContainerViewInterface;

struct _PicmanContainerViewInterface
{
  GTypeInterface base_iface;

  /*  signals  */
  gboolean (* select_item)        (PicmanContainerView *view,
                                   PicmanViewable      *object,
                                   gpointer           insert_data);
  void     (* activate_item)      (PicmanContainerView *view,
                                   PicmanViewable      *object,
                                   gpointer           insert_data);
  void     (* context_item)       (PicmanContainerView *view,
                                   PicmanViewable      *object,
                                   gpointer           insert_data);

  /*  virtual functions  */
  void     (* set_container)      (PicmanContainerView *view,
                                   PicmanContainer     *container);
  void     (* set_context)        (PicmanContainerView *view,
                                   PicmanContext       *context);
  void     (* set_selection_mode) (PicmanContainerView *view,
                                   GtkSelectionMode   mode);

  gpointer (* insert_item)        (PicmanContainerView *view,
                                   PicmanViewable      *object,
                                   gpointer           parent_insert_data,
                                   gint               index);
  void     (* insert_item_after)  (PicmanContainerView *view,
                                   PicmanViewable      *object,
                                   gpointer           insert_data);
  void     (* remove_item)        (PicmanContainerView *view,
                                   PicmanViewable      *object,
                                   gpointer           insert_data);
  void     (* reorder_item)       (PicmanContainerView *view,
                                   PicmanViewable      *object,
                                   gint               new_index,
                                   gpointer           insert_data);
  void     (* rename_item)        (PicmanContainerView *view,
                                   PicmanViewable      *object,
                                   gpointer           insert_data);
  void     (* clear_items)        (PicmanContainerView *view);
  void     (* set_view_size)      (PicmanContainerView *view);
  gint     (* get_selected)       (PicmanContainerView  *view,
                                   GList             **items);


  /*  the destroy notifier for private->hash_table's values  */
  GDestroyNotify  insert_data_free;
  gboolean        model_is_tree;
};


GType     picman_container_view_interface_get_type  (void) G_GNUC_CONST;

PicmanContainer * picman_container_view_get_container (PicmanContainerView *view);
void            picman_container_view_set_container (PicmanContainerView *view,
                                                   PicmanContainer     *container);

PicmanContext   * picman_container_view_get_context   (PicmanContainerView *view);
void            picman_container_view_set_context   (PicmanContainerView *view,
                                                   PicmanContext       *context);

GtkSelectionMode picman_container_view_get_selection_mode (PicmanContainerView *view);
void             picman_container_view_set_selection_mode (PicmanContainerView *view,
                                                         GtkSelectionMode   mode);

gint         picman_container_view_get_view_size (PicmanContainerView *view,
                                                gint              *view_border_width);
void         picman_container_view_set_view_size (PicmanContainerView *view,
                                                gint               view_size,
                                                gint               view_border_width);

gboolean  picman_container_view_get_reorderable  (PicmanContainerView *view);
void      picman_container_view_set_reorderable  (PicmanContainerView *view,
                                                gboolean           reorderable);

GtkWidget * picman_container_view_get_dnd_widget (PicmanContainerView *view);
void        picman_container_view_set_dnd_widget (PicmanContainerView *view,
                                                GtkWidget         *dnd_widget);

void      picman_container_view_enable_dnd       (PicmanContainerView *editor,
                                                GtkButton         *button,
                                                GType              children_type);

gboolean  picman_container_view_select_item      (PicmanContainerView *view,
                                                PicmanViewable      *viewable);
void      picman_container_view_activate_item    (PicmanContainerView *view,
                                                PicmanViewable      *viewable);
void      picman_container_view_context_item     (PicmanContainerView *view,
                                                PicmanViewable      *viewable);
gint      picman_container_view_get_selected     (PicmanContainerView  *view,
                                                GList             **list);

/*  protected  */

gpointer  picman_container_view_lookup           (PicmanContainerView *view,
                                                PicmanViewable      *viewable);

gboolean  picman_container_view_item_selected    (PicmanContainerView *view,
                                                PicmanViewable      *item);
gboolean  picman_container_view_multi_selected   (PicmanContainerView *view,
                                                GList             *items);
void      picman_container_view_item_activated   (PicmanContainerView *view,
                                                PicmanViewable      *item);
void      picman_container_view_item_context     (PicmanContainerView *view,
                                                PicmanViewable      *item);

/*  convenience functions  */

void      picman_container_view_install_properties (GObjectClass *klass);
void      picman_container_view_set_property       (GObject      *object,
                                                  guint         property_id,
                                                  const GValue *value,
                                                  GParamSpec   *pspec);
void      picman_container_view_get_property       (GObject      *object,
                                                  guint         property_id,
                                                  GValue       *value,
                                                  GParamSpec   *pspec);

#endif  /*  __PICMAN_CONTAINER_VIEW_H__  */
