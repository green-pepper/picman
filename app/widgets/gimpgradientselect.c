/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmangradientselect.c
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmangradient.h"
#include "core/picmanparamspecs.h"

#include "pdb/picmanpdb.h"

#include "picmancontainerbox.h"
#include "picmandatafactoryview.h"
#include "picmangradientselect.h"


enum
{
  PROP_0,
  PROP_SAMPLE_SIZE
};


static void             picman_gradient_select_constructed  (GObject        *object);
static void             picman_gradient_select_set_property (GObject        *object,
                                                           guint           property_id,
                                                           const GValue   *value,
                                                           GParamSpec     *pspec);

static PicmanValueArray * picman_gradient_select_run_callback (PicmanPdbDialog  *dialog,
                                                           PicmanObject     *object,
                                                           gboolean        closing,
                                                           GError        **error);


G_DEFINE_TYPE (PicmanGradientSelect, picman_gradient_select,
               PICMAN_TYPE_PDB_DIALOG)

#define parent_class picman_gradient_select_parent_class


static void
picman_gradient_select_class_init (PicmanGradientSelectClass *klass)
{
  GObjectClass       *object_class = G_OBJECT_CLASS (klass);
  PicmanPdbDialogClass *pdb_class    = PICMAN_PDB_DIALOG_CLASS (klass);

  object_class->constructed  = picman_gradient_select_constructed;
  object_class->set_property = picman_gradient_select_set_property;

  pdb_class->run_callback    = picman_gradient_select_run_callback;

  g_object_class_install_property (object_class, PROP_SAMPLE_SIZE,
                                   g_param_spec_int ("sample-size", NULL, NULL,
                                                     0, 10000, 84,
                                                     PICMAN_PARAM_WRITABLE |
                                                     G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_gradient_select_init (PicmanGradientSelect *select)
{
}

static void
picman_gradient_select_constructed (GObject *object)
{
  PicmanPdbDialog *dialog = PICMAN_PDB_DIALOG (object);
  GtkWidget     *content_area;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  dialog->view =
    picman_data_factory_view_new (PICMAN_VIEW_TYPE_LIST,
                                dialog->context->picman->gradient_factory,
                                dialog->context,
                                PICMAN_VIEW_SIZE_MEDIUM, 1,
                                dialog->menu_factory, "<Gradients>",
                                "/gradients-popup",
                                "gradients");

  picman_container_box_set_size_request (PICMAN_CONTAINER_BOX (PICMAN_CONTAINER_EDITOR (dialog->view)->view),
                                       6 * (PICMAN_VIEW_SIZE_MEDIUM + 2),
                                       6 * (PICMAN_VIEW_SIZE_MEDIUM + 2));

  gtk_container_set_border_width (GTK_CONTAINER (dialog->view), 12);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_box_pack_start (GTK_BOX (content_area), dialog->view, TRUE, TRUE, 0);
  gtk_widget_show (dialog->view);
}

static void
picman_gradient_select_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanGradientSelect *select = PICMAN_GRADIENT_SELECT (object);

  switch (property_id)
    {
    case PROP_SAMPLE_SIZE:
      select->sample_size = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static PicmanValueArray *
picman_gradient_select_run_callback (PicmanPdbDialog  *dialog,
                                   PicmanObject     *object,
                                   gboolean        closing,
                                   GError        **error)
{
  PicmanGradient        *gradient = PICMAN_GRADIENT (object);
  PicmanGradientSegment *seg      = NULL;
  gdouble             *values, *pv;
  gdouble              pos, delta;
  PicmanRGB              color;
  gint                 i;
  PicmanArray           *array;
  PicmanValueArray      *return_vals;

  i      = PICMAN_GRADIENT_SELECT (dialog)->sample_size;
  pos    = 0.0;
  delta  = 1.0 / (i - 1);

  values = g_new (gdouble, 4 * i);
  pv     = values;

  while (i--)
    {
      seg = picman_gradient_get_color_at (gradient, dialog->caller_context,
                                        seg, pos, FALSE, &color);

      *pv++ = color.r;
      *pv++ = color.g;
      *pv++ = color.b;
      *pv++ = color.a;

      pos += delta;
    }

  array = picman_array_new ((guint8 *) values,
                          PICMAN_GRADIENT_SELECT (dialog)->sample_size * 4 *
                          sizeof (gdouble),
                          TRUE);
  array->static_data = FALSE;

  return_vals =
    picman_pdb_execute_procedure_by_name (dialog->pdb,
                                        dialog->caller_context,
                                        NULL, error,
                                        dialog->callback_name,
                                        G_TYPE_STRING,         picman_object_get_name (object),
                                        PICMAN_TYPE_INT32,       array->length / sizeof (gdouble),
                                        PICMAN_TYPE_FLOAT_ARRAY, array,
                                        PICMAN_TYPE_INT32,       closing,
                                        G_TYPE_NONE);

  picman_array_free (array);

  return return_vals;
}
