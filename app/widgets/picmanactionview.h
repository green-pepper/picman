/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanactionview.h
 * Copyright (C) 2004-2005  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_ACTION_VIEW_H__
#define __PICMAN_ACTION_VIEW_H__


enum
{
  PICMAN_ACTION_VIEW_COLUMN_VISIBLE,
  PICMAN_ACTION_VIEW_COLUMN_ACTION,
  PICMAN_ACTION_VIEW_COLUMN_STOCK_ID,
  PICMAN_ACTION_VIEW_COLUMN_LABEL,
  PICMAN_ACTION_VIEW_COLUMN_LABEL_CASEFOLD,
  PICMAN_ACTION_VIEW_COLUMN_NAME,
  PICMAN_ACTION_VIEW_COLUMN_ACCEL_KEY,
  PICMAN_ACTION_VIEW_COLUMN_ACCEL_MASK,
  PICMAN_ACTION_VIEW_COLUMN_ACCEL_CLOSURE,
  PICMAN_ACTION_VIEW_N_COLUMNS
};


#define PICMAN_TYPE_ACTION_VIEW            (picman_action_view_get_type ())
#define PICMAN_ACTION_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ACTION_VIEW, PicmanActionView))
#define PICMAN_ACTION_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ACTION_VIEW, PicmanActionViewClass))
#define PICMAN_IS_ACTION_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ACTION_VIEW))
#define PICMAN_IS_ACTION_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ACTION_VIEW))
#define PICMAN_ACTION_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ACTION_VIEW, PicmanActionViewClass))


typedef struct _PicmanActionViewClass PicmanActionViewClass;

struct _PicmanActionView
{
  GtkTreeView    parent_instance;

  PicmanUIManager *manager;
  gboolean       show_shortcuts;

  gchar         *filter;
};

struct _PicmanActionViewClass
{
  GtkTreeViewClass  parent_class;
};


GType       picman_action_view_get_type   (void) G_GNUC_CONST;

GtkWidget * picman_action_view_new        (PicmanUIManager  *manager,
                                         const gchar    *select_action,
                                         gboolean        show_shortcuts);

void        picman_action_view_set_filter (PicmanActionView *view,
                                         const gchar    *filter);


#endif  /*  __PICMAN_ACTION_VIEW_H__  */
