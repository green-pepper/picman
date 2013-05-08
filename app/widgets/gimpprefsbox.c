/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanprefsbox.c
 * Copyright (C) 2013 Michael Natterer <mitch@picman.org>
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

#include "config.h"

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "picmanprefsbox.h"

#include "picman-intl.h"


enum
{
  COLUMN_TREE_PIXBUF,
  COLUMN_TREE_LABEL,
  COLUMN_NOTEBOOK_PIXBUF,
  COLUMN_NOTEBOOK_LABEL,
  COLUMN_PAGE_INDEX
};

typedef struct _PicmanPrefsBoxPrivate PicmanPrefsBoxPrivate;

struct _PicmanPrefsBoxPrivate
{
  GtkTreeStore *store;
  GtkWidget    *tree_view;
  GtkWidget    *notebook;
  GtkWidget    *label;
  GtkWidget    *image;

  gint          page_index;
};

#define GET_PRIVATE(item) G_TYPE_INSTANCE_GET_PRIVATE (item, \
                                                       PICMAN_TYPE_PREFS_BOX, \
                                                       PicmanPrefsBoxPrivate)


static void   picman_prefs_box_tree_select_callback   (GtkTreeSelection *sel,
                                                     PicmanPrefsBox     *box);
static void   picman_prefs_box_notebook_page_callback (GtkNotebook      *notebook,
                                                     gpointer          page,
                                                     guint             page_num,
                                                     PicmanPrefsBox     *box);


G_DEFINE_TYPE (PicmanPrefsBox, picman_prefs_box, GTK_TYPE_BOX)

#define parent_class picman_prefs_box_parent_class


static void
picman_prefs_box_class_init (PicmanPrefsBoxClass *klass)
{
  g_type_class_add_private (klass, sizeof (PicmanPrefsBoxPrivate));
}

static void
picman_prefs_box_init (PicmanPrefsBox *box)
{
  PicmanPrefsBoxPrivate *private = GET_PRIVATE (box);
  GtkTreeViewColumn   *column;
  GtkCellRenderer     *cell;
  GtkTreeSelection    *sel;
  GtkWidget           *frame;
  GtkWidget           *hbox;
  GtkWidget           *ebox;
  GtkWidget           *vbox;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (box),
                                  GTK_ORIENTATION_HORIZONTAL);
  gtk_box_set_spacing (GTK_BOX (box), 12);


  /*  the categories tree  */

  frame = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (frame),
                                       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (frame),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  private->store = gtk_tree_store_new (5,
                                       GDK_TYPE_PIXBUF,
                                       G_TYPE_STRING,
                                       GDK_TYPE_PIXBUF,
                                       G_TYPE_STRING,
                                       G_TYPE_INT);
  private->tree_view =
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (private->store));
  g_object_unref (private->store);

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (private->tree_view), FALSE);

  column = gtk_tree_view_column_new ();

  cell = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, cell, FALSE);
  gtk_tree_view_column_set_attributes (column, cell, "pixbuf",
                                       COLUMN_TREE_PIXBUF, NULL);

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell, TRUE);
  gtk_tree_view_column_set_attributes (column, cell, "text",
                                       COLUMN_TREE_LABEL, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (private->tree_view), column);

  gtk_container_add (GTK_CONTAINER (frame), private->tree_view);
  gtk_widget_show (private->tree_view);


  /*  the notebook  */

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_box_pack_start (GTK_BOX (box), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  ebox = gtk_event_box_new ();
  gtk_widget_set_state (ebox, GTK_STATE_SELECTED);
  gtk_box_pack_start (GTK_BOX (vbox), ebox, FALSE, TRUE, 0);
  gtk_widget_show (ebox);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_container_add (GTK_CONTAINER (ebox), hbox);
  gtk_widget_show (hbox);

  private->label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (private->label), 0.0, 0.5);
  picman_label_set_attributes (GTK_LABEL (private->label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_LARGE,
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);
  gtk_box_pack_start (GTK_BOX (hbox), private->label, FALSE, FALSE, 0);
  gtk_widget_show (private->label);

  private->image = gtk_image_new ();
  gtk_box_pack_end (GTK_BOX (hbox), private->image, FALSE, FALSE, 0);
  gtk_widget_show (private->image);

  /* The main preferences notebook */
  private->notebook = gtk_notebook_new ();
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (private->notebook), FALSE);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (private->notebook), FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), private->notebook, TRUE, TRUE, 0);
  gtk_widget_show (private->notebook);

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (private->tree_view));
  g_signal_connect (sel, "changed",
                    G_CALLBACK (picman_prefs_box_tree_select_callback),
                    box);
  g_signal_connect (private->notebook, "switch-page",
                    G_CALLBACK (picman_prefs_box_notebook_page_callback),
                    box);
}

static void
picman_prefs_box_tree_select_callback (GtkTreeSelection *sel,
                                     PicmanPrefsBox     *box)
{
  PicmanPrefsBoxPrivate *private = GET_PRIVATE (box);
  GtkTreeModel        *model;
  GtkTreeIter          iter;
  gchar               *text;
  GdkPixbuf           *pixbuf;
  gint                 index;

  if (! gtk_tree_selection_get_selected (sel, &model, &iter))
    return;

  gtk_tree_model_get (model, &iter,
                      COLUMN_NOTEBOOK_LABEL,  &text,
                      COLUMN_NOTEBOOK_PIXBUF, &pixbuf,
                      COLUMN_PAGE_INDEX,      &index,
                      -1);

  gtk_label_set_text (GTK_LABEL (private->label), text);
  g_free (text);

  gtk_image_set_from_pixbuf (GTK_IMAGE (private->image), pixbuf);
  if (pixbuf)
    g_object_unref (pixbuf);

  g_signal_handlers_block_by_func (private->notebook,
                                   picman_prefs_box_notebook_page_callback,
                                   sel);

  gtk_notebook_set_current_page (GTK_NOTEBOOK (private->notebook), index);

  g_signal_handlers_unblock_by_func (private->notebook,
                                     picman_prefs_box_notebook_page_callback,
                                     sel);
}

static void
picman_prefs_box_notebook_page_callback (GtkNotebook  *notebook,
                                       gpointer      page,
                                       guint         page_num,
                                       PicmanPrefsBox *box)
{
  PicmanPrefsBoxPrivate *private = GET_PRIVATE (box);
  GtkTreeSelection    *sel;
  GtkTreeModel        *model;
  GtkTreeIter          iter;
  gboolean             iter_valid;

  sel   = gtk_tree_view_get_selection (GTK_TREE_VIEW (private->tree_view));
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (private->tree_view));

  for (iter_valid = gtk_tree_model_get_iter_first (model, &iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, &iter))
    {
      gint index;

      gtk_tree_model_get (model, &iter,
                          COLUMN_PAGE_INDEX, &index,
                          -1);

      if (index == page_num)
        {
          gtk_tree_selection_select_iter (sel, &iter);
          return;
        }

      if (gtk_tree_model_iter_has_child (model, &iter))
        {
          gint num_children;
          gint i;

          num_children = gtk_tree_model_iter_n_children (model, &iter);

          for (i = 0; i < num_children; i++)
            {
              GtkTreeIter child_iter;

              gtk_tree_model_iter_nth_child (model, &child_iter, &iter, i);
              gtk_tree_model_get (model, &child_iter,
                                  COLUMN_PAGE_INDEX, &index,
                                  -1);

              if (index == page_num)
                {
                  GtkTreePath *path;

                  path = gtk_tree_model_get_path (model, &child_iter);
                  gtk_tree_view_expand_to_path (GTK_TREE_VIEW (private->tree_view),
                                                path);
                  gtk_tree_selection_select_iter (sel, &child_iter);
                  return;
                }
            }
        }
    }
}


/*  public functions  */

GtkWidget *
picman_prefs_box_new (void)
{
  return g_object_new (PICMAN_TYPE_PREFS_BOX, NULL);
}

GtkWidget *
picman_prefs_box_add_page (PicmanPrefsBox *box,
                         const gchar  *notebook_label,
                         GdkPixbuf    *notebook_icon,
                         const gchar  *tree_label,
                         GdkPixbuf    *tree_icon,
                         const gchar  *help_id,
                         GtkTreeIter  *parent,
                         GtkTreeIter  *iter)
{
  PicmanPrefsBoxPrivate *private;
  GtkWidget           *event_box;
  GtkWidget           *vbox;

  g_return_val_if_fail (PICMAN_IS_PREFS_BOX (box), NULL);

  private = GET_PRIVATE (box);

  event_box = gtk_event_box_new ();
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (event_box), FALSE);
  gtk_notebook_append_page (GTK_NOTEBOOK (private->notebook), event_box, NULL);
  gtk_widget_show (event_box);

  picman_help_set_help_data (event_box, NULL, help_id);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_add (GTK_CONTAINER (event_box), vbox);
  gtk_widget_show (vbox);

  if (! notebook_icon)
    notebook_icon = tree_icon;

  if (! tree_icon)
    tree_icon = notebook_icon;

  if (! tree_label)
    tree_label = notebook_label;

  gtk_tree_store_append (private->store, iter, parent);
  gtk_tree_store_set (private->store, iter,
                      COLUMN_TREE_PIXBUF,     tree_icon,
                      COLUMN_TREE_LABEL,      tree_label,
                      COLUMN_NOTEBOOK_PIXBUF, notebook_icon,
                      COLUMN_NOTEBOOK_LABEL , notebook_label,
                      COLUMN_PAGE_INDEX,      private->page_index++,
                      -1);

  return vbox;
}

GtkWidget *
picman_prefs_box_get_tree_view (PicmanPrefsBox *box)
{
  g_return_val_if_fail (PICMAN_IS_PREFS_BOX (box), NULL);

  return GET_PRIVATE (box)->tree_view;
}

GtkWidget *
picman_prefs_box_get_notebook (PicmanPrefsBox *box)
{
  g_return_val_if_fail (PICMAN_IS_PREFS_BOX (box), NULL);

  return GET_PRIVATE (box)->notebook;
}

GtkWidget *
picman_prefs_box_get_image (PicmanPrefsBox *box)
{
  g_return_val_if_fail (PICMAN_IS_PREFS_BOX (box), NULL);

  return GET_PRIVATE (box)->image;
}
