/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanitemcombobox.c
 * Copyright (C) 2004 Sven Neumann <sven@picman.org>
 * Copyright (C) 2006 Simon Budig <simon@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "picman.h"

#include "picmanuitypes.h"
#include "picmanitemcombobox.h"
#include "picmanpixbuf.h"


/**
 * SECTION: picmanitemcombobox
 * @title: PicmanItemComboBox
 * @short_description: Widgets providing popup menus of items.
 *
 * Widgets providing popup menus of items (layers, channels,
 * drawables, vectors).
 **/


#define THUMBNAIL_SIZE  24
#define WIDTH_REQUEST  200


#define GET_PRIVATE(obj) (g_object_get_data (G_OBJECT (obj), "picman-item-combo-box-private"))


typedef struct _PicmanItemComboBoxPrivate   PicmanItemComboBoxPrivate;

struct _PicmanItemComboBoxPrivate
{
  PicmanItemConstraintFunc  constraint;
  gpointer                data;
};

typedef struct _PicmanDrawableComboBoxClass PicmanDrawableComboBoxClass;
typedef struct _PicmanChannelComboBoxClass  PicmanChannelComboBoxClass;
typedef struct _PicmanLayerComboBoxClass    PicmanLayerComboBoxClass;
typedef struct _PicmanVectorsComboBoxClass  PicmanVectorsComboBoxClass;

struct _PicmanDrawableComboBox
{
  PicmanIntComboBox  parent_instance;
};

struct _PicmanDrawableComboBoxClass
{
  PicmanIntComboBoxClass  parent_class;
};

struct _PicmanChannelComboBox
{
  PicmanIntComboBox  parent_instance;
};

struct _PicmanChannelComboBoxClass
{
  PicmanIntComboBoxClass  parent_class;
};

struct _PicmanLayerComboBox
{
  PicmanIntComboBox  parent_instance;
};

struct _PicmanLayerComboBoxClass
{
  PicmanIntComboBoxClass  parent_class;
};

struct _PicmanVectorsComboBox
{
  PicmanIntComboBox  parent_instance;
};

struct _PicmanVectorsComboBoxClass
{
  PicmanIntComboBoxClass  parent_class;
};


static GtkWidget * picman_item_combo_box_new (GType                       type,
                                            PicmanItemConstraintFunc      constraint,
                                            gpointer                    data);

static void  picman_item_combo_box_populate  (PicmanIntComboBox            *combo_box);
static void  picman_item_combo_box_model_add (PicmanIntComboBox            *combo_box,
                                            GtkListStore               *store,
                                            gint32                      image,
                                            gint                        num_items,
                                            gint32                     *items,
                                            gint                        tree_level);

static void  picman_item_combo_box_drag_data_received (GtkWidget         *widget,
                                                     GdkDragContext    *context,
                                                     gint               x,
                                                     gint               y,
                                                     GtkSelectionData  *selection,
                                                     guint              info,
                                                     guint              time);

static void  picman_item_combo_box_changed   (PicmanIntComboBox *combo_box);


static const GtkTargetEntry targets[] =
{
  { "application/x-picman-channel-id", 0 },
  { "application/x-picman-layer-id",   0 },
  { "application/x-picman-vectors-id", 0 }
};


G_DEFINE_TYPE (PicmanDrawableComboBox, picman_drawable_combo_box,
               PICMAN_TYPE_INT_COMBO_BOX)

static void
picman_drawable_combo_box_class_init (PicmanDrawableComboBoxClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->drag_data_received = picman_item_combo_box_drag_data_received;
}

static void
picman_drawable_combo_box_init (PicmanDrawableComboBox *combo_box)
{
  gtk_drag_dest_set (GTK_WIDGET (combo_box),
                     GTK_DEST_DEFAULT_HIGHLIGHT |
                     GTK_DEST_DEFAULT_MOTION |
                     GTK_DEST_DEFAULT_DROP,
                     targets, 2,
                     GDK_ACTION_COPY);

  g_object_set_data_full (G_OBJECT (combo_box), "picman-item-combo-box-private",
                          g_new0 (PicmanItemComboBoxPrivate, 1),
                          (GDestroyNotify) g_free);
}

/**
 * picman_drawable_combo_box_new:
 * @constraint: a #PicmanDrawableConstraintFunc or %NULL
 * @data:       a pointer that is passed to @constraint
 *
 * Creates a new #PicmanIntComboBox filled with all currently opened
 * drawables. If a @constraint function is specified, it is called for
 * each drawable and only if the function returns %TRUE, the drawable
 * is added to the combobox.
 *
 * You should use picman_int_combo_box_connect() to initialize and connect
 * the combo.  Use picman_int_combo_box_set_active() to get the active
 * drawable ID and picman_int_combo_box_get_active() to retrieve the ID
 * of the selected drawable.
 *
 * Return value: a new #PicmanIntComboBox.
 *
 * Since: PICMAN 2.2
 **/
GtkWidget *
picman_drawable_combo_box_new (PicmanDrawableConstraintFunc constraint,
                             gpointer                   data)
{
  return picman_item_combo_box_new (PICMAN_TYPE_DRAWABLE_COMBO_BOX,
                                  constraint, data);
}


G_DEFINE_TYPE (PicmanChannelComboBox, picman_channel_combo_box,
               PICMAN_TYPE_INT_COMBO_BOX)

static void
picman_channel_combo_box_class_init (PicmanChannelComboBoxClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->drag_data_received = picman_item_combo_box_drag_data_received;
}

static void
picman_channel_combo_box_init (PicmanChannelComboBox *combo_box)
{
  gtk_drag_dest_set (GTK_WIDGET (combo_box),
                     GTK_DEST_DEFAULT_HIGHLIGHT |
                     GTK_DEST_DEFAULT_MOTION |
                     GTK_DEST_DEFAULT_DROP,
                     targets, 1,
                     GDK_ACTION_COPY);

  g_object_set_data_full (G_OBJECT (combo_box), "picman-item-combo-box-private",
                          g_new0 (PicmanItemComboBoxPrivate, 1),
                          (GDestroyNotify) g_free);
}

/**
 * picman_channel_combo_box_new:
 * @constraint: a #PicmanDrawableConstraintFunc or %NULL
 * @data:       a pointer that is passed to @constraint
 *
 * Creates a new #PicmanIntComboBox filled with all currently opened
 * channels. See picman_drawable_combo_box_new() for more information.
 *
 * Return value: a new #PicmanIntComboBox.
 *
 * Since: PICMAN 2.2
 **/
GtkWidget *
picman_channel_combo_box_new (PicmanDrawableConstraintFunc constraint,
                            gpointer                   data)
{
  return picman_item_combo_box_new (PICMAN_TYPE_CHANNEL_COMBO_BOX,
                                  constraint, data);
}


G_DEFINE_TYPE (PicmanLayerComboBox, picman_layer_combo_box,
               PICMAN_TYPE_INT_COMBO_BOX)

static void
picman_layer_combo_box_class_init (PicmanLayerComboBoxClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->drag_data_received = picman_item_combo_box_drag_data_received;
}

static void
picman_layer_combo_box_init (PicmanLayerComboBox *combo_box)
{
  gtk_drag_dest_set (GTK_WIDGET (combo_box),
                     GTK_DEST_DEFAULT_HIGHLIGHT |
                     GTK_DEST_DEFAULT_MOTION |
                     GTK_DEST_DEFAULT_DROP,
                     targets + 1, 1,
                     GDK_ACTION_COPY);

  g_object_set_data_full (G_OBJECT (combo_box), "picman-item-combo-box-private",
                          g_new0 (PicmanItemComboBoxPrivate, 1),
                          (GDestroyNotify) g_free);
}

/**
 * picman_layer_combo_box_new:
 * @constraint: a #PicmanDrawableConstraintFunc or %NULL
 * @data:       a pointer that is passed to @constraint
 *
 * Creates a new #PicmanIntComboBox filled with all currently opened
 * layers. See picman_drawable_combo_box_new() for more information.
 *
 * Return value: a new #PicmanIntComboBox.
 *
 * Since: PICMAN 2.2
 **/
GtkWidget *
picman_layer_combo_box_new (PicmanDrawableConstraintFunc constraint,
                          gpointer                   data)
{
  return picman_item_combo_box_new (PICMAN_TYPE_LAYER_COMBO_BOX,
                                  constraint, data);
}


G_DEFINE_TYPE (PicmanVectorsComboBox, picman_vectors_combo_box,
               PICMAN_TYPE_INT_COMBO_BOX)

static void
picman_vectors_combo_box_class_init (PicmanVectorsComboBoxClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->drag_data_received = picman_item_combo_box_drag_data_received;
}

static void
picman_vectors_combo_box_init (PicmanVectorsComboBox *combo_box)
{
  gtk_drag_dest_set (GTK_WIDGET (combo_box),
                     GTK_DEST_DEFAULT_HIGHLIGHT |
                     GTK_DEST_DEFAULT_MOTION |
                     GTK_DEST_DEFAULT_DROP,
                     targets + 2, 1,
                     GDK_ACTION_COPY);

  g_object_set_data_full (G_OBJECT (combo_box), "picman-item-combo-box-private",
                          g_new0 (PicmanItemComboBoxPrivate, 1),
                          (GDestroyNotify) g_free);
}


/**
 * picman_vectors_combo_box_new:
 * @constraint: a #PicmanVectorsConstraintFunc or %NULL
 * @data:       a pointer that is passed to @constraint
 *
 * Creates a new #PicmanIntComboBox filled with all currently opened
 * vectors objects. If a @constraint function is specified, it is called for
 * each vectors object and only if the function returns %TRUE, the vectors
 * object is added to the combobox.
 *
 * You should use picman_int_combo_box_connect() to initialize and connect
 * the combo.  Use picman_int_combo_box_set_active() to set the active
 * vectors ID and picman_int_combo_box_get_active() to retrieve the ID
 * of the selected vectors object.
 *
 * Return value: a new #PicmanIntComboBox.
 *
 * Since: PICMAN 2.4
 **/
GtkWidget *
picman_vectors_combo_box_new (PicmanVectorsConstraintFunc constraint,
                            gpointer                  data)
{
  return picman_item_combo_box_new (PICMAN_TYPE_VECTORS_COMBO_BOX,
                                  constraint, data);
}


static GtkWidget *
picman_item_combo_box_new (GType                  type,
                         PicmanItemConstraintFunc constraint,
                         gpointer               data)
{
  PicmanIntComboBox         *combo_box;
  PicmanItemComboBoxPrivate *private;

  combo_box = g_object_new (type,
                            "width-request", WIDTH_REQUEST,
                            "ellipsize",     PANGO_ELLIPSIZE_MIDDLE,
                            NULL);

  private = GET_PRIVATE (combo_box);

  private->constraint = constraint;
  private->data       = data;

  picman_item_combo_box_populate (combo_box);

  g_signal_connect (combo_box, "changed",
                    G_CALLBACK (picman_item_combo_box_changed),
                    NULL);

  return GTK_WIDGET (combo_box);
}

static void
picman_item_combo_box_populate (PicmanIntComboBox *combo_box)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gint32       *images;
  gint          num_images;
  gint          i;

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));

  images = picman_image_list (&num_images);

  for (i = 0; i < num_images; i++)
    {
      gint32 *items;
      gint    num_items;

      if (PICMAN_IS_DRAWABLE_COMBO_BOX (combo_box) ||
          PICMAN_IS_LAYER_COMBO_BOX (combo_box))
        {
          items = picman_image_get_layers (images[i], &num_items);
          picman_item_combo_box_model_add (combo_box, GTK_LIST_STORE (model),
                                         images[i],
                                         num_items, items, 0);
          g_free (items);
        }

      if (PICMAN_IS_DRAWABLE_COMBO_BOX (combo_box) ||
          PICMAN_IS_CHANNEL_COMBO_BOX (combo_box))
        {
          items = picman_image_get_channels (images[i], &num_items);
          picman_item_combo_box_model_add (combo_box, GTK_LIST_STORE (model),
                                         images[i],
                                         num_items, items, 0);
          g_free (items);
        }

      if (PICMAN_IS_VECTORS_COMBO_BOX (combo_box))
        {
          items = picman_image_get_vectors (images[i], &num_items);
          picman_item_combo_box_model_add (combo_box, GTK_LIST_STORE (model),
                                         images[i],
                                         num_items, items, 0);
          g_free (items);
        }
    }

  g_free (images);

  if (gtk_tree_model_get_iter_first (model, &iter))
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo_box), &iter);
}

static void
picman_item_combo_box_model_add (PicmanIntComboBox *combo_box,
                               GtkListStore    *store,
                               gint32           image,
                               gint             num_items,
                               gint32          *items,
                               gint             tree_level)
{
  PicmanItemComboBoxPrivate *private = GET_PRIVATE (combo_box);
  GtkTreeIter              iter;
  gint                     i;
  gchar                   *indent;

  if (tree_level > 0)
    {
      indent = g_new (gchar, tree_level + 2);
      memset (indent, '-', tree_level);
      indent[tree_level] = ' ';
      indent[tree_level + 1] = '\0';
    }
  else
    {
      indent = g_strdup ("");
    }

  for (i = 0; i < num_items; i++)
    {
      if (! private->constraint ||
          (* private->constraint) (image, items[i], private->data))
        {
          gchar     *image_name = picman_image_get_name (image);
          gchar     *item_name  = picman_item_get_name (items[i]);
          gchar     *label;
          GdkPixbuf *thumb;

          label = g_strdup_printf ("%s%s-%d / %s-%d",
                                   indent, image_name, image,
                                   item_name, items[i]);

          g_free (item_name);
          g_free (image_name);

          if (PICMAN_IS_VECTORS_COMBO_BOX (combo_box))
            thumb = NULL;
          else
            thumb = picman_drawable_get_thumbnail (items[i],
                                                 THUMBNAIL_SIZE, THUMBNAIL_SIZE,
                                                 PICMAN_PIXBUF_SMALL_CHECKS);

          gtk_list_store_append (store, &iter);
          gtk_list_store_set (store, &iter,
                              PICMAN_INT_STORE_VALUE,  items[i],
                              PICMAN_INT_STORE_LABEL,  label,
                              PICMAN_INT_STORE_PIXBUF, thumb,
                              -1);

          if (thumb)
            g_object_unref (thumb);

          g_free (label);

          if (picman_item_is_group (items[i]))
            {
              gint32 *children;
              gint    n_children;

              children = picman_item_get_children (items[i], &n_children);
              picman_item_combo_box_model_add (combo_box, store,
                                             image,
                                             n_children, children,
                                             tree_level + 1);
              g_free (children);
            }
        }
    }

  g_free (indent);
}

static void
picman_item_combo_box_drag_data_received (GtkWidget        *widget,
                                        GdkDragContext   *context,
                                        gint              x,
                                        gint              y,
                                        GtkSelectionData *selection,
                                        guint             info,
                                        guint             time)
{
  gint   length = gtk_selection_data_get_length (selection);
  gchar *str;

  if (gtk_selection_data_get_format (selection) != 8 || length < 1)
    {
      g_warning ("%s: received invalid item ID data", G_STRFUNC);
      return;
    }

  str = g_strndup ((const gchar *) gtk_selection_data_get_data (selection),
                   length);

  if (g_utf8_validate (str, -1, NULL))
    {
      gint pid;
      gint ID;

      if (sscanf (str, "%i:%i", &pid, &ID) == 2 &&
          pid == picman_getpid ())
        {
          picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (widget), ID);
        }
    }

  g_free (str);
}

static gboolean
picman_item_combo_box_remove_items (GtkTreeModel *model,
                                  GtkTreePath  *path,
                                  GtkTreeIter  *iter,
                                  gpointer      data)
{
  gint    item_ID;
  GList **remove = data;

  gtk_tree_model_get (model, iter,
                      PICMAN_INT_STORE_VALUE, &item_ID,
                      -1);

  if (item_ID > 0)
    *remove = g_list_prepend (*remove, g_memdup (iter, sizeof (GtkTreeIter)));

  return FALSE;
}

static void
picman_item_combo_box_changed (PicmanIntComboBox *combo_box)
{
  gint item_ID;

  if (picman_int_combo_box_get_active (combo_box, &item_ID))
    {
      if (item_ID > 0 && ! picman_item_is_valid (item_ID))
        {
          GtkTreeModel *model;
          GList        *remove = NULL;
          GList        *list;

          model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));

          g_signal_stop_emission_by_name (combo_box, "changed");

          gtk_tree_model_foreach (model,
                                  picman_item_combo_box_remove_items,
                                  &remove);

          for (list = remove; list; list = g_list_next (list))
            gtk_list_store_remove (GTK_LIST_STORE (model), list->data);

          g_list_free_full (remove, (GDestroyNotify) g_free);

          picman_item_combo_box_populate (combo_box);
        }
    }
}
