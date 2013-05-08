/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanimagecombobox.c
 * Copyright (C) 2004 Sven Neumann <sven@picman.org>
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

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "picman.h"

#include "picmanuitypes.h"
#include "picmanimagecombobox.h"
#include "picmanpixbuf.h"


/**
 * SECTION: picmanimagecombobox
 * @title: PicmanImageComboBox
 * @short_description: A widget providing a popup menu of images.
 *
 * A widget providing a popup menu of images.
 **/


#define THUMBNAIL_SIZE   24
#define WIDTH_REQUEST   200


typedef struct _PicmanImageComboBoxClass PicmanImageComboBoxClass;

struct _PicmanImageComboBox
{
  PicmanIntComboBox          parent_instance;

  PicmanImageConstraintFunc  constraint;
  gpointer                 data;
};

struct _PicmanImageComboBoxClass
{
  PicmanIntComboBoxClass  parent_class;
};


static void  picman_image_combo_box_populate  (PicmanImageComboBox       *combo_box);
static void  picman_image_combo_box_model_add (GtkListStore            *store,
                                             gint                     num_images,
                                             gint32                  *images,
                                             PicmanImageConstraintFunc  constraint,
                                             gpointer                 data);

static void  picman_image_combo_box_drag_data_received (GtkWidget        *widget,
                                                      GdkDragContext   *context,
                                                      gint              x,
                                                      gint              y,
                                                      GtkSelectionData *selection,
                                                      guint             info,
                                                      guint             time);

static void  picman_image_combo_box_changed   (PicmanImageComboBox *combo_box);


static const GtkTargetEntry target = { "application/x-picman-image-id", 0 };


G_DEFINE_TYPE (PicmanImageComboBox, picman_image_combo_box, PICMAN_TYPE_INT_COMBO_BOX)


static void
picman_image_combo_box_class_init (PicmanImageComboBoxClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->drag_data_received = picman_image_combo_box_drag_data_received;
}

static void
picman_image_combo_box_init (PicmanImageComboBox *combo_box)
{
  gtk_drag_dest_set (GTK_WIDGET (combo_box),
                     GTK_DEST_DEFAULT_HIGHLIGHT |
                     GTK_DEST_DEFAULT_MOTION |
                     GTK_DEST_DEFAULT_DROP,
                     &target, 1,
                     GDK_ACTION_COPY);
}

/**
 * picman_image_combo_box_new:
 * @constraint: a #PicmanImageConstraintFunc or %NULL
 * @data:       a pointer that is passed to @constraint
 *
 * Creates a new #PicmanIntComboBox filled with all currently opened
 * images. If a @constraint function is specified, it is called for
 * each image and only if the function returns %TRUE, the image is
 * added to the combobox.
 *
 * You should use picman_int_combo_box_connect() to initialize and
 * connect the combo. Use picman_int_combo_box_set_active() to get the
 * active image ID and picman_int_combo_box_get_active() to retrieve the
 * ID of the selected image.
 *
 * Return value: a new #PicmanIntComboBox.
 *
 * Since: PICMAN 2.2
 **/
GtkWidget *
picman_image_combo_box_new (PicmanImageConstraintFunc constraint,
                          gpointer                data)
{
  PicmanImageComboBox *combo_box;

  combo_box = g_object_new (PICMAN_TYPE_IMAGE_COMBO_BOX,
                            "width-request", WIDTH_REQUEST,
                            "ellipsize",     PANGO_ELLIPSIZE_MIDDLE,
                            NULL);

  combo_box->constraint = constraint;
  combo_box->data       = data;

  picman_image_combo_box_populate (combo_box);

  g_signal_connect (combo_box, "changed",
                    G_CALLBACK (picman_image_combo_box_changed),
                    NULL);

  return GTK_WIDGET (combo_box);
}

static void
picman_image_combo_box_populate (PicmanImageComboBox *combo_box)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gint32       *images;
  gint          num_images;

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));

  images = picman_image_list (&num_images);

  picman_image_combo_box_model_add (GTK_LIST_STORE (model),
                                  num_images, images,
                                  combo_box->constraint,
                                  combo_box->data);

  g_free (images);

  if (gtk_tree_model_get_iter_first (model, &iter))
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo_box), &iter);
}

static void
picman_image_combo_box_model_add (GtkListStore            *store,
                                gint                     num_images,
                                gint32                  *images,
                                PicmanImageConstraintFunc  constraint,
                                gpointer                 data)
{
  GtkTreeIter  iter;
  gint         i;

  for (i = 0; i < num_images; i++)
    {
      if (! constraint || (* constraint) (images[i], data))
        {
          gchar     *image_name = picman_image_get_name (images[i]);
          gchar     *label;
          GdkPixbuf *thumb;

          label = g_strdup_printf ("%s-%d", image_name, images[i]);

          g_free (image_name);

          thumb = picman_image_get_thumbnail (images[i],
                                            THUMBNAIL_SIZE, THUMBNAIL_SIZE,
                                            PICMAN_PIXBUF_SMALL_CHECKS);

          gtk_list_store_append (store, &iter);
          gtk_list_store_set (store, &iter,
                              PICMAN_INT_STORE_VALUE,  images[i],
                              PICMAN_INT_STORE_LABEL,  label,
                              PICMAN_INT_STORE_PIXBUF, thumb,
                              -1);

          if (thumb)
            g_object_unref (thumb);

          g_free (label);
        }
    }
}

static void
picman_image_combo_box_drag_data_received (GtkWidget        *widget,
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
      g_warning ("%s: received invalid image ID data", G_STRFUNC);
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

static void
picman_image_combo_box_changed (PicmanImageComboBox *combo_box)
{
  gint image_ID;

  if (picman_int_combo_box_get_active (PICMAN_INT_COMBO_BOX (combo_box),
                                     &image_ID))
    {
      if (! picman_image_is_valid (image_ID))
        {
          GtkTreeModel *model;

          model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));

          g_signal_stop_emission_by_name (combo_box, "changed");

          gtk_list_store_clear (GTK_LIST_STORE (model));
          picman_image_combo_box_populate (combo_box);
        }
    }
}
