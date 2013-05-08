/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewabledialog.c
 * Copyright (C) 2000 Michael Natterer <mitch@picman.org>
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanitem.h"

#include "file/file-utils.h"

#include "picmanview.h"
#include "picmanviewabledialog.h"
#include "picmanviewrenderer.h"


enum
{
  PROP_0,
  PROP_VIEWABLE,
  PROP_CONTEXT,
  PROP_STOCK_ID,
  PROP_DESC
};


static void   picman_viewable_dialog_dispose      (GObject            *object);
static void   picman_viewable_dialog_set_property (GObject            *object,
                                                 guint               property_id,
                                                 const GValue       *value,
                                                 GParamSpec         *pspec);
static void   picman_viewable_dialog_get_property (GObject            *object,
                                                 guint               property_id,
                                                 GValue             *value,
                                                 GParamSpec         *pspec);

static void   picman_viewable_dialog_name_changed (PicmanObject         *object,
                                                 PicmanViewableDialog *dialog);
static void   picman_viewable_dialog_close        (PicmanViewableDialog *dialog);


G_DEFINE_TYPE (PicmanViewableDialog, picman_viewable_dialog, PICMAN_TYPE_DIALOG)

#define parent_class picman_viewable_dialog_parent_class


static void
picman_viewable_dialog_class_init (PicmanViewableDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose      = picman_viewable_dialog_dispose;
  object_class->get_property = picman_viewable_dialog_get_property;
  object_class->set_property = picman_viewable_dialog_set_property;

  g_object_class_install_property (object_class, PROP_VIEWABLE,
                                   g_param_spec_object ("viewable", NULL, NULL,
                                                        PICMAN_TYPE_VIEWABLE,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context", NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_STOCK_ID,
                                   g_param_spec_string ("stock-id", NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_DESC,
                                   g_param_spec_string ("description", NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT));
}

static void
picman_viewable_dialog_init (PicmanViewableDialog *dialog)
{
  GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vbox;

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (content_area), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_widget_show (hbox);

  dialog->icon = gtk_image_new ();
  gtk_misc_set_alignment (GTK_MISC (dialog->icon), 0.5, 0.0);
  gtk_box_pack_start (GTK_BOX (hbox), dialog->icon, FALSE, FALSE, 0);
  gtk_widget_show (dialog->icon);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  dialog->desc_label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (dialog->desc_label), 0.0, 0.5);
  picman_label_set_attributes (GTK_LABEL (dialog->desc_label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_LARGE,
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);
  gtk_box_pack_start (GTK_BOX (vbox), dialog->desc_label, FALSE, FALSE, 0);
  gtk_widget_show (dialog->desc_label);

  dialog->viewable_label = g_object_new (GTK_TYPE_LABEL,
                                         "xalign",    0.0,
                                         "yalign",    0.5,
                                         "ellipsize", PANGO_ELLIPSIZE_END,
                                         NULL);
  picman_label_set_attributes (GTK_LABEL (dialog->viewable_label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             -1);
  gtk_box_pack_start (GTK_BOX (vbox), dialog->viewable_label, FALSE, FALSE, 0);
  gtk_widget_show (dialog->viewable_label);
}

static void
picman_viewable_dialog_dispose (GObject *object)
{
  PicmanViewableDialog *dialog = PICMAN_VIEWABLE_DIALOG (object);

  if (dialog->view)
    picman_viewable_dialog_set_viewable (dialog, NULL, NULL);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_viewable_dialog_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanViewableDialog *dialog = PICMAN_VIEWABLE_DIALOG (object);

  switch (property_id)
    {
    case PROP_VIEWABLE:
      picman_viewable_dialog_set_viewable (dialog,
                                         g_value_get_object (value),
                                         dialog->context);
      break;

    case PROP_CONTEXT:
      picman_viewable_dialog_set_viewable (dialog,
                                         dialog->view ?
                                         PICMAN_VIEW (dialog->view)->viewable :
                                         NULL,
                                         g_value_get_object (value));
      break;

    case PROP_STOCK_ID:
      gtk_image_set_from_stock (GTK_IMAGE (dialog->icon),
                                g_value_get_string (value),
                                GTK_ICON_SIZE_LARGE_TOOLBAR);
      break;

    case PROP_DESC:
      gtk_label_set_text (GTK_LABEL (dialog->desc_label),
                          g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_viewable_dialog_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanViewableDialog *dialog = PICMAN_VIEWABLE_DIALOG (object);

  switch (property_id)
    {
    case PROP_VIEWABLE:
      g_value_set_object (value,
                          dialog->view ?
                          PICMAN_VIEW (dialog->view)->viewable : NULL);
      break;

    case PROP_CONTEXT:
      g_value_set_object (value, dialog->context);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_viewable_dialog_new (PicmanViewable *viewable,
                          PicmanContext  *context,
                          const gchar  *title,
                          const gchar  *role,
                          const gchar  *stock_id,
                          const gchar  *desc,
                          GtkWidget    *parent,
                          PicmanHelpFunc  help_func,
                          const gchar  *help_id,
                          ...)
{
  PicmanViewableDialog *dialog;
  va_list             args;

  g_return_val_if_fail (viewable == NULL || PICMAN_IS_VIEWABLE (viewable), NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (title != NULL, NULL);
  g_return_val_if_fail (role != NULL, NULL);
  g_return_val_if_fail (parent == NULL || GTK_IS_WIDGET (parent), NULL);

  if (! viewable)
    g_warning ("Use of PicmanViewableDialog with a NULL viewable is depecrated!");

  dialog = g_object_new (PICMAN_TYPE_VIEWABLE_DIALOG,
                         "viewable",    viewable,
                         "context",     context,
                         "title",       title,
                         "role",        role,
                         "help-func",   help_func,
                         "help-id",     help_id,
                         "stock-id",    stock_id,
                         "description", desc,
                         "parent",      parent,
                         NULL);

  va_start (args, help_id);
  picman_dialog_add_buttons_valist (PICMAN_DIALOG (dialog), args);
  va_end (args);

  return GTK_WIDGET (dialog);
}

void
picman_viewable_dialog_set_viewable (PicmanViewableDialog *dialog,
                                   PicmanViewable       *viewable,
                                   PicmanContext        *context)
{
  g_return_if_fail (PICMAN_IS_VIEWABLE_DIALOG (dialog));
  g_return_if_fail (viewable == NULL || PICMAN_IS_VIEWABLE (viewable));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  dialog->context = context;

  if (dialog->view)
    {
      PicmanViewable *old_viewable = PICMAN_VIEW (dialog->view)->viewable;

      if (viewable == old_viewable)
        {
          picman_view_renderer_set_context (PICMAN_VIEW (dialog->view)->renderer,
                                          context);
          return;
        }

      gtk_widget_destroy (dialog->view);

      if (old_viewable)
        {
          g_signal_handlers_disconnect_by_func (old_viewable,
                                                picman_viewable_dialog_name_changed,
                                                dialog);

          g_signal_handlers_disconnect_by_func (old_viewable,
                                                picman_viewable_dialog_close,
                                                dialog);
        }
    }

  if (viewable)
    {
      GtkWidget *box;

      g_signal_connect_object (viewable,
                               PICMAN_VIEWABLE_GET_CLASS (viewable)->name_changed_signal,
                               G_CALLBACK (picman_viewable_dialog_name_changed),
                               dialog,
                               0);

      box = gtk_widget_get_parent (dialog->icon);

      dialog->view = picman_view_new (context, viewable, 32, 1, TRUE);
      gtk_box_pack_end (GTK_BOX (box), dialog->view, FALSE, FALSE, 2);
      gtk_widget_show (dialog->view);

      g_object_add_weak_pointer (G_OBJECT (dialog->view),
                                 (gpointer) &dialog->view);

      picman_viewable_dialog_name_changed (PICMAN_OBJECT (viewable), dialog);

      if (PICMAN_IS_ITEM (viewable))
        {
          g_signal_connect_object (viewable, "removed",
                                   G_CALLBACK (picman_viewable_dialog_close),
                                   dialog,
                                   G_CONNECT_SWAPPED);
        }
      else
        {
          g_signal_connect_object (viewable, "disconnect",
                                   G_CALLBACK (picman_viewable_dialog_close),
                                   dialog,
                                   G_CONNECT_SWAPPED);
        }
    }
}


/*  private functions  */

static void
picman_viewable_dialog_name_changed (PicmanObject         *object,
                                   PicmanViewableDialog *dialog)
{
  gchar *name;

  name = picman_viewable_get_description (PICMAN_VIEWABLE (object), NULL);

  if (PICMAN_IS_ITEM (object))
    {
      PicmanImage *image = picman_item_get_image (PICMAN_ITEM (object));
      gchar     *tmp;

      tmp = name;
      name = g_strdup_printf ("%s-%d (%s)",
                              tmp,
                              picman_item_get_ID (PICMAN_ITEM (object)),
                              picman_image_get_display_name (image));
      g_free (tmp);
    }

  gtk_label_set_text (GTK_LABEL (dialog->viewable_label), name);
  g_free (name);
}

static void
picman_viewable_dialog_close (PicmanViewableDialog *dialog)
{
  g_signal_emit_by_name (dialog, "close");
}
