/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanImagePropView
 * Copyright (C) 2005  Michael Natterer <mitch@picman.org>
 * Copyright (C) 2006  Sven Neumann <sven@picman.org>
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

#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gegl.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmanimage.h"
#include "core/picmanimage-colormap.h"
#include "core/picmanimage-undo.h"
#include "core/picmanundostack.h"
#include "core/picman-utils.h"

#include "file/file-procedure.h"
#include "file/file-utils.h"

#include "plug-in/picmanpluginmanager.h"
#include "plug-in/picmanpluginprocedure.h"

#include "picmanimagepropview.h"
#include "picmanpropwidgets.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_IMAGE
};


static void        picman_image_prop_view_constructed  (GObject           *object);
static void        picman_image_prop_view_set_property (GObject           *object,
                                                      guint              property_id,
                                                      const GValue      *value,
                                                      GParamSpec        *pspec);
static void        picman_image_prop_view_get_property (GObject           *object,
                                                      guint              property_id,
                                                      GValue            *value,
                                                      GParamSpec        *pspec);

static GtkWidget * picman_image_prop_view_add_label    (GtkTable          *table,
                                                      gint               row,
                                                      const gchar       *text);
static void        picman_image_prop_view_undo_event   (PicmanImage         *image,
                                                      PicmanUndoEvent      event,
                                                      PicmanUndo          *undo,
                                                      PicmanImagePropView *view);
static void        picman_image_prop_view_update       (PicmanImagePropView *view);
static void        picman_image_prop_view_file_update  (PicmanImagePropView *view);


G_DEFINE_TYPE (PicmanImagePropView, picman_image_prop_view, GTK_TYPE_TABLE)

#define parent_class picman_image_prop_view_parent_class


static void
picman_image_prop_view_class_init (PicmanImagePropViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_image_prop_view_constructed;
  object_class->set_property = picman_image_prop_view_set_property;
  object_class->get_property = picman_image_prop_view_get_property;

  g_object_class_install_property (object_class, PROP_IMAGE,
                                   g_param_spec_object ("image", NULL, NULL,
                                                        PICMAN_TYPE_IMAGE,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_image_prop_view_init (PicmanImagePropView *view)
{
  GtkTable *table = GTK_TABLE (view);
  gint      row = 0;

  gtk_table_resize (table, 15, 2);

  gtk_table_set_col_spacings (table, 6);
  gtk_table_set_row_spacings (table, 3);

  view->pixel_size_label =
    picman_image_prop_view_add_label (table, row++, _("Size in pixels:"));

  view->print_size_label =
    picman_image_prop_view_add_label (table, row++, _("Print size:"));

  view->resolution_label =
    picman_image_prop_view_add_label (table, row++, _("Resolution:"));

  view->colorspace_label =
    picman_image_prop_view_add_label (table, row++, _("Color space:"));

  view->precision_label =
    picman_image_prop_view_add_label (table, row, _("Precision:"));

  gtk_table_set_row_spacing (GTK_TABLE (view), row++, 12);

  view->filename_label =
    picman_image_prop_view_add_label (table, row++, _("File Name:"));

  gtk_label_set_ellipsize (GTK_LABEL (view->filename_label),
                           PANGO_ELLIPSIZE_MIDDLE);

  view->filesize_label =
    picman_image_prop_view_add_label (table, row++, _("File Size:"));

  view->filetype_label =
    picman_image_prop_view_add_label (table, row, _("File Type:"));

  gtk_table_set_row_spacing (GTK_TABLE (view), row++, 12);

  view->memsize_label =
    picman_image_prop_view_add_label (table, row++, _("Size in memory:"));

  view->undo_label =
    picman_image_prop_view_add_label (table, row++, _("Undo steps:"));

  view->redo_label =
    picman_image_prop_view_add_label (table, row, _("Redo steps:"));

  gtk_table_set_row_spacing (GTK_TABLE (view), row++, 12);

  view->pixels_label =
    picman_image_prop_view_add_label (table, row++, _("Number of pixels:"));

  view->layers_label =
    picman_image_prop_view_add_label (table, row++, _("Number of layers:"));

  view->channels_label =
    picman_image_prop_view_add_label (table, row++, _("Number of channels:"));

  view->vectors_label =
    picman_image_prop_view_add_label (table, row++, _("Number of paths:"));

}

static void
picman_image_prop_view_constructed (GObject *object)
{
  PicmanImagePropView *view = PICMAN_IMAGE_PROP_VIEW (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (view->image != NULL);

  g_signal_connect_object (view->image, "name-changed",
                           G_CALLBACK (picman_image_prop_view_file_update),
                           G_OBJECT (view),
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (view->image, "size-changed",
                           G_CALLBACK (picman_image_prop_view_update),
                           G_OBJECT (view),
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (view->image, "resolution-changed",
                           G_CALLBACK (picman_image_prop_view_update),
                           G_OBJECT (view),
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (view->image, "unit-changed",
                           G_CALLBACK (picman_image_prop_view_update),
                           G_OBJECT (view),
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (view->image, "mode-changed",
                           G_CALLBACK (picman_image_prop_view_update),
                           G_OBJECT (view),
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (view->image, "undo-event",
                           G_CALLBACK (picman_image_prop_view_undo_event),
                           G_OBJECT (view),
                           0);

  picman_image_prop_view_update (view);
  picman_image_prop_view_file_update (view);
}

static void
picman_image_prop_view_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanImagePropView *view = PICMAN_IMAGE_PROP_VIEW (object);

  switch (property_id)
    {
    case PROP_IMAGE:
      view->image = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_image_prop_view_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanImagePropView *view = PICMAN_IMAGE_PROP_VIEW (object);

  switch (property_id)
    {
    case PROP_IMAGE:
      g_value_set_object (value, view->image);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/*  public functions  */

GtkWidget *
picman_image_prop_view_new (PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return g_object_new (PICMAN_TYPE_IMAGE_PROP_VIEW,
                       "image", image,
                       NULL);
}


/*  private functions  */

static GtkWidget *
picman_image_prop_view_add_label (GtkTable    *table,
                                gint         row,
                                const gchar *text)
{
  GtkWidget *label;
  GtkWidget *desc;

  desc = g_object_new (GTK_TYPE_LABEL,
                       "label",  text,
                       "xalign", 1.0,
                       "yalign", 0.5,
                       NULL);
  picman_label_set_attributes (GTK_LABEL (desc),
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);
  gtk_table_attach (table, desc,
                    0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (desc);

  label = g_object_new (GTK_TYPE_LABEL,
                        "xalign",     0.0,
                        "yalign",     0.5,
                        "selectable", TRUE,
                        NULL);

  gtk_table_attach (table, label,
                    1, 2, row, row + 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);

  gtk_widget_show (label);

  return label;
}

static void
picman_image_prop_view_label_set_memsize (GtkWidget  *label,
                                        PicmanObject *object)
{
  gchar *str = g_format_size (picman_object_get_memsize (object, NULL));
  gtk_label_set_text (GTK_LABEL (label), str);
  g_free (str);
}

static void
picman_image_prop_view_label_set_filename (GtkWidget *label,
                                         PicmanImage *image)
{
  const gchar *uri = picman_image_get_any_uri (image);

  if (uri)
    {
      gchar *name = file_utils_uri_display_name (uri);

      gtk_label_set_text (GTK_LABEL (label), name);
      g_free (name);
    }
  else
    {
      gtk_label_set_text (GTK_LABEL (label), NULL);
      picman_help_set_help_data (gtk_widget_get_parent (label), NULL, NULL);
    }
}

static void
picman_image_prop_view_label_set_filesize (GtkWidget *label,
                                         PicmanImage *image)
{
  const gchar *uri      = picman_image_get_any_uri (image);
  gchar       *filename = NULL;

  if (uri)
    filename = g_filename_from_uri (uri, NULL, NULL);

  if (filename)
    {
      struct stat  buf;

      if (g_stat (filename, &buf) == 0)
        {
          gchar *str = g_format_size (buf.st_size);

          gtk_label_set_text (GTK_LABEL (label), str);
          g_free (str);
        }
      else
        {
          gtk_label_set_text (GTK_LABEL (label), NULL);
        }

      g_free (filename);
    }
  else
    {
      gtk_label_set_text (GTK_LABEL (label), NULL);
    }
}

static void
picman_image_prop_view_label_set_filetype (GtkWidget *label,
                                         PicmanImage *image)
{
  PicmanPlugInManager   *manager = image->picman->plug_in_manager;
  PicmanPlugInProcedure *proc;

  proc = picman_image_get_save_proc (image);

  if (! proc)
    proc = picman_image_get_load_proc (image);

  if (! proc)
    {
      gchar *filename = picman_image_get_filename (image);

      if (filename)
        {
          proc = file_procedure_find (manager->load_procs, filename, NULL);
          g_free (filename);
        }
    }

  gtk_label_set_text (GTK_LABEL (label),
                      proc ? picman_plug_in_procedure_get_label (proc) : NULL);
}

static void
picman_image_prop_view_label_set_undo (GtkWidget     *label,
                                     PicmanUndoStack *stack)
{
  gint steps = picman_undo_stack_get_depth (stack);

  if (steps > 0)
    {
      PicmanObject *object = PICMAN_OBJECT (stack);
      gchar      *str;
      gchar       buf[256];

      str = g_format_size (picman_object_get_memsize (object, NULL));
      g_snprintf (buf, sizeof (buf), "%d (%s)", steps, str);
      g_free (str);

      gtk_label_set_text (GTK_LABEL (label), buf);
    }
  else
    {
      /*  no undo (or redo) steps available  */
      gtk_label_set_text (GTK_LABEL (label), _("None"));
    }
}

static void
picman_image_prop_view_undo_event (PicmanImage         *image,
                                 PicmanUndoEvent      event,
                                 PicmanUndo          *undo,
                                 PicmanImagePropView *view)
{
  picman_image_prop_view_update (view);
}

static void
picman_image_prop_view_update (PicmanImagePropView *view)
{
  PicmanImage         *image = view->image;
  PicmanImageBaseType  type;
  PicmanPrecision      precision;
  PicmanUnit           unit;
  gdouble            unit_factor;
  gint               unit_digits;
  const gchar       *desc;
  gchar              format_buf[32];
  gchar              buf[256];
  gdouble            xres;
  gdouble            yres;

  picman_image_get_resolution (image, &xres, &yres);

  /*  pixel size  */
  g_snprintf (buf, sizeof (buf), ngettext ("%d × %d pixel",
                                           "%d × %d pixels",
                                           picman_image_get_height (image)),
              picman_image_get_width  (image),
              picman_image_get_height (image));
  gtk_label_set_text (GTK_LABEL (view->pixel_size_label), buf);

  /*  print size  */
  unit = picman_get_default_unit ();

  unit_digits = picman_unit_get_digits (unit);

  g_snprintf (format_buf, sizeof (format_buf), "%%.%df × %%.%df %s",
              unit_digits + 1, unit_digits + 1,
              picman_unit_get_plural (unit));
  g_snprintf (buf, sizeof (buf), format_buf,
              picman_pixels_to_units (picman_image_get_width  (image), unit, xres),
              picman_pixels_to_units (picman_image_get_height (image), unit, yres));
  gtk_label_set_text (GTK_LABEL (view->print_size_label), buf);

  /*  resolution  */
  unit        = picman_image_get_unit (image);
  unit_factor = picman_unit_get_factor (unit);

  g_snprintf (format_buf, sizeof (format_buf), _("pixels/%s"),
              picman_unit_get_abbreviation (unit));
  g_snprintf (buf, sizeof (buf), _("%g × %g %s"),
              xres / unit_factor,
              yres / unit_factor,
              unit == PICMAN_UNIT_INCH ? _("ppi") : format_buf);
  gtk_label_set_text (GTK_LABEL (view->resolution_label), buf);

  /*  color type  */
  type = picman_image_get_base_type (image);

  picman_enum_get_value (PICMAN_TYPE_IMAGE_BASE_TYPE, type,
                       NULL, NULL, &desc, NULL);

  switch (type)
    {
    case PICMAN_RGB:
    case PICMAN_GRAY:
      g_snprintf (buf, sizeof (buf), "%s", desc);
      break;
    case PICMAN_INDEXED:
      g_snprintf (buf, sizeof (buf),
                  "%s (%d %s)", desc, picman_image_get_colormap_size (image),
                  _("colors"));
      break;
    }

  gtk_label_set_text (GTK_LABEL (view->colorspace_label), buf);

  /*  precision  */
  precision = picman_image_get_precision (image);

  picman_enum_get_value (PICMAN_TYPE_PRECISION, precision,
                       NULL, NULL, &desc, NULL);

  gtk_label_set_text (GTK_LABEL (view->precision_label), desc);

  /*  size in memory  */
  picman_image_prop_view_label_set_memsize (view->memsize_label,
                                          PICMAN_OBJECT (image));

  /*  undo / redo  */
  picman_image_prop_view_label_set_undo (view->undo_label,
                                       picman_image_get_undo_stack (image));
  picman_image_prop_view_label_set_undo (view->redo_label,
                                       picman_image_get_redo_stack (image));

  /*  number of layers  */
  g_snprintf (buf, sizeof (buf), "%d",
              picman_image_get_width  (image) *
              picman_image_get_height (image));
  gtk_label_set_text (GTK_LABEL (view->pixels_label), buf);

  /*  number of layers  */
  g_snprintf (buf, sizeof (buf), "%d",
              picman_image_get_n_layers (image));
  gtk_label_set_text (GTK_LABEL (view->layers_label), buf);

  /*  number of channels  */
  g_snprintf (buf, sizeof (buf), "%d",
              picman_image_get_n_channels (image));
  gtk_label_set_text (GTK_LABEL (view->channels_label), buf);

  /*  number of vectors  */
  g_snprintf (buf, sizeof (buf), "%d",
              picman_image_get_n_vectors (image));
  gtk_label_set_text (GTK_LABEL (view->vectors_label), buf);
}

static void
picman_image_prop_view_file_update (PicmanImagePropView *view)
{
  PicmanImage *image = view->image;

  /*  filename  */
  picman_image_prop_view_label_set_filename (view->filename_label, image);

  /*  filesize  */
  picman_image_prop_view_label_set_filesize (view->filesize_label, image);

  /*  filetype  */
  picman_image_prop_view_label_set_filetype (view->filetype_label, image);
}
