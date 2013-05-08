/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbrushselect.c
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
#include "core/picmanbrush.h"
#include "core/picmanparamspecs.h"
#include "core/picmantempbuf.h"

#include "pdb/picmanpdb.h"

#include "picmanbrushfactoryview.h"
#include "picmanbrushselect.h"
#include "picmancontainerbox.h"
#include "picmanspinscale.h"
#include "picmanwidgets-constructors.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_OPACITY,
  PROP_PAINT_MODE,
  PROP_SPACING
};


static void             picman_brush_select_constructed  (GObject         *object);
static void             picman_brush_select_set_property (GObject         *object,
                                                        guint            property_id,
                                                        const GValue    *value,
                                                        GParamSpec      *pspec);

static PicmanValueArray * picman_brush_select_run_callback (PicmanPdbDialog   *dialog,
                                                        PicmanObject      *object,
                                                        gboolean         closing,
                                                        GError         **error);

static void          picman_brush_select_opacity_changed (PicmanContext     *context,
                                                        gdouble          opacity,
                                                        PicmanBrushSelect *select);
static void          picman_brush_select_mode_changed    (PicmanContext     *context,
                                                        PicmanLayerModeEffects  paint_mode,
                                                        PicmanBrushSelect *select);

static void          picman_brush_select_opacity_update  (GtkAdjustment   *adj,
                                                        PicmanBrushSelect *select);
static void          picman_brush_select_mode_update     (GtkWidget       *widget,
                                                        PicmanBrushSelect *select);
static void          picman_brush_select_spacing_update  (GtkAdjustment   *adj,
                                                        PicmanBrushSelect *select);


G_DEFINE_TYPE (PicmanBrushSelect, picman_brush_select, PICMAN_TYPE_PDB_DIALOG)

#define parent_class picman_brush_select_parent_class


static void
picman_brush_select_class_init (PicmanBrushSelectClass *klass)
{
  GObjectClass       *object_class = G_OBJECT_CLASS (klass);
  PicmanPdbDialogClass *pdb_class    = PICMAN_PDB_DIALOG_CLASS (klass);

  object_class->constructed  = picman_brush_select_constructed;
  object_class->set_property = picman_brush_select_set_property;

  pdb_class->run_callback    = picman_brush_select_run_callback;

  g_object_class_install_property (object_class, PROP_OPACITY,
                                   g_param_spec_double ("opacity", NULL, NULL,
                                                        PICMAN_OPACITY_TRANSPARENT,
                                                        PICMAN_OPACITY_OPAQUE,
                                                        PICMAN_OPACITY_OPAQUE,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_PAINT_MODE,
                                   g_param_spec_enum ("paint-mode", NULL, NULL,
                                                      PICMAN_TYPE_LAYER_MODE_EFFECTS,
                                                      PICMAN_NORMAL_MODE,
                                                      PICMAN_PARAM_WRITABLE |
                                                      G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_SPACING,
                                   g_param_spec_int ("spacing", NULL, NULL,
                                                     -G_MAXINT, 1000, -1,
                                                     PICMAN_PARAM_WRITABLE |
                                                     G_PARAM_CONSTRUCT));
}

static void
picman_brush_select_init (PicmanBrushSelect *select)
{
}

static void
picman_brush_select_constructed (GObject *object)
{
  PicmanPdbDialog   *dialog = PICMAN_PDB_DIALOG (object);
  PicmanBrushSelect *select = PICMAN_BRUSH_SELECT (object);
  GtkWidget       *content_area;
  GtkWidget       *vbox;
  GtkWidget       *scale;
  GtkWidget       *hbox;
  GtkWidget       *label;
  GtkAdjustment   *spacing_adj;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_context_set_opacity    (dialog->context, select->initial_opacity);
  picman_context_set_paint_mode (dialog->context, select->initial_mode);

  g_signal_connect (dialog->context, "opacity-changed",
                    G_CALLBACK (picman_brush_select_opacity_changed),
                    dialog);
  g_signal_connect (dialog->context, "paint-mode-changed",
                    G_CALLBACK (picman_brush_select_mode_changed),
                    dialog);

  dialog->view =
    picman_brush_factory_view_new (PICMAN_VIEW_TYPE_GRID,
                                 dialog->context->picman->brush_factory,
                                 dialog->context,
                                 FALSE,
                                 PICMAN_VIEW_SIZE_MEDIUM, 1,
                                 dialog->menu_factory);

  picman_container_box_set_size_request (PICMAN_CONTAINER_BOX (PICMAN_CONTAINER_EDITOR (dialog->view)->view),
                                       5 * (PICMAN_VIEW_SIZE_MEDIUM + 2),
                                       5 * (PICMAN_VIEW_SIZE_MEDIUM + 2));

  gtk_container_set_border_width (GTK_CONTAINER (dialog->view), 12);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_box_pack_start (GTK_BOX (content_area), dialog->view, TRUE, TRUE, 0);
  gtk_widget_show (dialog->view);

  vbox = GTK_WIDGET (PICMAN_CONTAINER_EDITOR (dialog->view)->view);

  /*  Create the opacity scale widget  */
  select->opacity_data =
    GTK_ADJUSTMENT (gtk_adjustment_new (picman_context_get_opacity (dialog->context) * 100.0,
                                        0.0, 100.0,
                                        1.0, 10.0, 0.0));

  scale = picman_spin_scale_new (select->opacity_data,
                               _("Opacity"), 1);
  gtk_box_pack_end (GTK_BOX (vbox), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  g_signal_connect (select->opacity_data, "value-changed",
                    G_CALLBACK (picman_brush_select_opacity_update),
                    select);

  /*  Create the paint mode option menu  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Mode:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  select->paint_mode_menu = picman_paint_mode_menu_new (TRUE, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), select->paint_mode_menu, TRUE, TRUE, 0);
  gtk_widget_show (select->paint_mode_menu);

  picman_int_combo_box_connect (PICMAN_INT_COMBO_BOX (select->paint_mode_menu),
                              picman_context_get_paint_mode (dialog->context),
                              G_CALLBACK (picman_brush_select_mode_update),
                              select);

  spacing_adj = PICMAN_BRUSH_FACTORY_VIEW (dialog->view)->spacing_adjustment;

  /*  Use passed spacing instead of brushes default  */
  if (select->spacing >= 0)
    gtk_adjustment_set_value (spacing_adj, select->spacing);

  g_signal_connect (spacing_adj, "value-changed",
                    G_CALLBACK (picman_brush_select_spacing_update),
                    select);
}

static void
picman_brush_select_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanPdbDialog   *dialog = PICMAN_PDB_DIALOG (object);
  PicmanBrushSelect *select = PICMAN_BRUSH_SELECT (object);

  switch (property_id)
    {
    case PROP_OPACITY:
      if (dialog->view)
        picman_context_set_opacity (dialog->context, g_value_get_double (value));
      else
        select->initial_opacity = g_value_get_double (value);
      break;
    case PROP_PAINT_MODE:
      if (dialog->view)
        picman_context_set_paint_mode (dialog->context, g_value_get_enum (value));
      else
        select->initial_mode = g_value_get_enum (value);
      break;
    case PROP_SPACING:
      if (dialog->view)
        {
          if (g_value_get_int (value) >= 0)
            gtk_adjustment_set_value (PICMAN_BRUSH_FACTORY_VIEW (dialog->view)->spacing_adjustment,
                                      g_value_get_int (value));
        }
      else
        {
          select->spacing = g_value_get_int (value);
        }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static PicmanValueArray *
picman_brush_select_run_callback (PicmanPdbDialog  *dialog,
                                PicmanObject     *object,
                                gboolean        closing,
                                GError        **error)
{
  PicmanBrush      *brush = PICMAN_BRUSH (object);
  PicmanArray      *array;
  PicmanValueArray *return_vals;

  array = picman_array_new (picman_temp_buf_get_data (brush->mask),
                          picman_temp_buf_get_data_size (brush->mask),
                          TRUE);

  return_vals =
    picman_pdb_execute_procedure_by_name (dialog->pdb,
                                        dialog->caller_context,
                                        NULL, error,
                                        dialog->callback_name,
                                        G_TYPE_STRING,        picman_object_get_name (object),
                                        G_TYPE_DOUBLE,        picman_context_get_opacity (dialog->context) * 100.0,
                                        PICMAN_TYPE_INT32,      PICMAN_BRUSH_SELECT (dialog)->spacing,
                                        PICMAN_TYPE_INT32,      picman_context_get_paint_mode (dialog->context),
                                        PICMAN_TYPE_INT32,      picman_temp_buf_get_width  (brush->mask),
                                        PICMAN_TYPE_INT32,      picman_temp_buf_get_height (brush->mask),
                                        PICMAN_TYPE_INT32,      array->length,
                                        PICMAN_TYPE_INT8_ARRAY, array,
                                        PICMAN_TYPE_INT32,      closing,
                                        G_TYPE_NONE);

  picman_array_free (array);

  return return_vals;
}

static void
picman_brush_select_opacity_changed (PicmanContext     *context,
                                   gdouble          opacity,
                                   PicmanBrushSelect *select)
{
  g_signal_handlers_block_by_func (select->opacity_data,
                                   picman_brush_select_opacity_update,
                                   select);

  gtk_adjustment_set_value (select->opacity_data, opacity * 100.0);

  g_signal_handlers_unblock_by_func (select->opacity_data,
                                     picman_brush_select_opacity_update,
                                     select);

  picman_pdb_dialog_run_callback (PICMAN_PDB_DIALOG (select), FALSE);
}

static void
picman_brush_select_mode_changed (PicmanContext          *context,
                                PicmanLayerModeEffects  paint_mode,
                                PicmanBrushSelect      *select)
{
  g_signal_handlers_block_by_func (select->paint_mode_menu,
                                   picman_brush_select_mode_update,
                                   select);

  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (select->paint_mode_menu),
                                 paint_mode);

  g_signal_handlers_unblock_by_func (select->paint_mode_menu,
                                     picman_brush_select_mode_update,
                                     select);

  picman_pdb_dialog_run_callback (PICMAN_PDB_DIALOG (select), FALSE);
}

static void
picman_brush_select_opacity_update (GtkAdjustment   *adjustment,
                                  PicmanBrushSelect *select)
{
  picman_context_set_opacity (PICMAN_PDB_DIALOG (select)->context,
                            gtk_adjustment_get_value (adjustment) / 100.0);
}

static void
picman_brush_select_mode_update (GtkWidget       *widget,
                               PicmanBrushSelect *select)
{
  gint paint_mode;

  if (picman_int_combo_box_get_active (PICMAN_INT_COMBO_BOX (widget),
                                     &paint_mode))
    {
      picman_context_set_paint_mode (PICMAN_PDB_DIALOG (select)->context,
                                   (PicmanLayerModeEffects) paint_mode);
    }
}

static void
picman_brush_select_spacing_update (GtkAdjustment   *adjustment,
                                  PicmanBrushSelect *select)
{
  gdouble value = gtk_adjustment_get_value (adjustment);

  if (select->spacing != value)
    {
      select->spacing = value;

      picman_pdb_dialog_run_callback (PICMAN_PDB_DIALOG (select), FALSE);
    }
}
