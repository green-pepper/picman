/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbrusheditor.c
 * Copyright 1998 Jay Cox <jaycox@earthlink.net>
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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanbrushgenerated.h"
#include "core/picmancontext.h"

#include "picmanbrusheditor.h"
#include "picmandocked.h"
#include "picmanspinscale.h"
#include "picmanview.h"
#include "picmanviewrenderer.h"

#include "picman-intl.h"


#define BRUSH_VIEW_SIZE 96


/*  local function prototypes  */

static void   picman_brush_editor_docked_iface_init (PicmanDockedInterface *face);

static void   picman_brush_editor_constructed    (GObject            *object);

static void   picman_brush_editor_set_data       (PicmanDataEditor     *editor,
                                                PicmanData           *data);

static void   picman_brush_editor_set_context    (PicmanDocked         *docked,
                                                PicmanContext        *context);

static void   picman_brush_editor_update_brush   (GtkAdjustment      *adjustment,
                                                PicmanBrushEditor    *editor);
static void   picman_brush_editor_update_shape   (GtkWidget          *widget,
                                                PicmanBrushEditor    *editor);
static void   picman_brush_editor_notify_brush   (PicmanBrushGenerated *brush,
                                                GParamSpec         *pspec,
                                                PicmanBrushEditor    *editor);


G_DEFINE_TYPE_WITH_CODE (PicmanBrushEditor, picman_brush_editor,
                         PICMAN_TYPE_DATA_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_brush_editor_docked_iface_init))

#define parent_class picman_brush_editor_parent_class

static PicmanDockedInterface *parent_docked_iface = NULL;


static void
picman_brush_editor_class_init (PicmanBrushEditorClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanDataEditorClass *editor_class = PICMAN_DATA_EDITOR_CLASS (klass);

  object_class->constructed = picman_brush_editor_constructed;

  editor_class->set_data    = picman_brush_editor_set_data;
  editor_class->title       = _("Brush Editor");
}

static void
picman_brush_editor_docked_iface_init (PicmanDockedInterface *iface)
{
  parent_docked_iface = g_type_interface_peek_parent (iface);

  if (! parent_docked_iface)
    parent_docked_iface = g_type_default_interface_peek (PICMAN_TYPE_DOCKED);

  iface->set_context = picman_brush_editor_set_context;
}

static void
picman_brush_editor_init (PicmanBrushEditor *editor)
{
  PicmanDataEditor *data_editor = PICMAN_DATA_EDITOR (editor);
  GtkWidget      *frame;
  GtkWidget      *hbox;
  GtkWidget      *label;
  GtkWidget      *box;
  GtkWidget      *scale;

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (editor), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  data_editor->view = picman_view_new_full_by_types (NULL,
                                                   PICMAN_TYPE_VIEW,
                                                   PICMAN_TYPE_BRUSH,
                                                   BRUSH_VIEW_SIZE,
                                                   BRUSH_VIEW_SIZE, 0,
                                                   FALSE, FALSE, TRUE);
  gtk_widget_set_size_request (data_editor->view, -1, BRUSH_VIEW_SIZE);
  picman_view_set_expand (PICMAN_VIEW (data_editor->view), TRUE);
  gtk_container_add (GTK_CONTAINER (frame), data_editor->view);
  gtk_widget_show (data_editor->view);

  editor->shape_group = NULL;

  editor->options_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_box_pack_start (GTK_BOX (editor), editor->options_box, FALSE, FALSE, 0);
  gtk_widget_show (editor->options_box);

  /* Stock Box for the brush shape */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (editor->options_box), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Shape:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  box = picman_enum_stock_box_new (PICMAN_TYPE_BRUSH_GENERATED_SHAPE,
                                 "picman-shape",
                                 GTK_ICON_SIZE_MENU,
                                 G_CALLBACK (picman_brush_editor_update_shape),
                                 editor,
                                 &editor->shape_group);
  gtk_box_pack_start (GTK_BOX (hbox), box, FALSE, FALSE, 0);
  gtk_widget_show (box);

  /*  brush radius scale  */
  editor->radius_data =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.1, 1000.0, 0.1, 1.0, 0.0));
  scale = picman_spin_scale_new (editor->radius_data, _("Radius"), 1);
  gtk_box_pack_start (GTK_BOX (editor->options_box), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  g_signal_connect (editor->radius_data, "value-changed",
                    G_CALLBACK (picman_brush_editor_update_brush),
                    editor);

  /*  number of spikes  */
  editor->spikes_data =
    GTK_ADJUSTMENT (gtk_adjustment_new (2.0, 2.0, 20.0, 1.0, 1.0, 0.0));
  scale = picman_spin_scale_new (editor->spikes_data, _("Spikes"), 0);
  gtk_box_pack_start (GTK_BOX (editor->options_box), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  g_signal_connect (editor->spikes_data, "value-changed",
                    G_CALLBACK (picman_brush_editor_update_brush),
                    editor);

  /*  brush hardness scale  */
  editor->hardness_data =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 1.0, 0.01, 0.1, 0.0));
  scale = picman_spin_scale_new (editor->hardness_data, _("Hardness"), 2);
  gtk_box_pack_start (GTK_BOX (editor->options_box), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  g_signal_connect (editor->hardness_data, "value-changed",
                    G_CALLBACK (picman_brush_editor_update_brush),
                    editor);

  /*  brush aspect ratio scale  */
  editor->aspect_ratio_data =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 1.0, 20.0, 0.1, 1.0, 0.0));
  scale = picman_spin_scale_new (editor->aspect_ratio_data, _("Aspect ratio"), 1);
  gtk_box_pack_start (GTK_BOX (editor->options_box), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  g_signal_connect (editor->aspect_ratio_data,"value-changed",
                    G_CALLBACK (picman_brush_editor_update_brush),
                    editor);

  /*  brush angle scale  */
  editor->angle_data =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 180.0, 0.1, 1.0, 0.0));
  scale = picman_spin_scale_new (editor->angle_data, _("Angle"), 1);
  gtk_box_pack_start (GTK_BOX (editor->options_box), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  g_signal_connect (editor->angle_data, "value-changed",
                    G_CALLBACK (picman_brush_editor_update_brush),
                    editor);

  /*  brush spacing  */
  editor->spacing_data =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 1.0, 5000.0, 1.0, 10.0, 0.0));
  scale = picman_spin_scale_new (editor->spacing_data, _("Spacing"), 1);
  picman_spin_scale_set_scale_limits (PICMAN_SPIN_SCALE (scale), 1.0, 200.0);
  gtk_box_pack_start (GTK_BOX (editor->options_box), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  picman_help_set_help_data (scale, _("Percentage of width of brush"), NULL);

  g_signal_connect (editor->spacing_data, "value-changed",
                    G_CALLBACK (picman_brush_editor_update_brush),
                    editor);
}

static void
picman_brush_editor_constructed (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_docked_set_show_button_bar (PICMAN_DOCKED (object), FALSE);
}

static void
picman_brush_editor_set_data (PicmanDataEditor *editor,
                            PicmanData       *data)
{
  PicmanBrushEditor         *brush_editor = PICMAN_BRUSH_EDITOR (editor);
  PicmanBrushGeneratedShape  shape        = PICMAN_BRUSH_GENERATED_CIRCLE;
  gdouble                  radius       = 0.0;
  gint                     spikes       = 2;
  gdouble                  hardness     = 0.0;
  gdouble                  ratio        = 0.0;
  gdouble                  angle        = 0.0;
  gdouble                  spacing      = 0.0;

  if (editor->data)
    g_signal_handlers_disconnect_by_func (editor->data,
                                          picman_brush_editor_notify_brush,
                                          editor);

  PICMAN_DATA_EDITOR_CLASS (parent_class)->set_data (editor, data);

  if (editor->data)
    g_signal_connect (editor->data, "notify",
                      G_CALLBACK (picman_brush_editor_notify_brush),
                      editor);

  picman_view_set_viewable (PICMAN_VIEW (editor->view), PICMAN_VIEWABLE (data));

  if (editor->data)
    {
      spacing = picman_brush_get_spacing (PICMAN_BRUSH (editor->data));

      if (PICMAN_IS_BRUSH_GENERATED (editor->data))
        {
          PicmanBrushGenerated *brush = PICMAN_BRUSH_GENERATED (editor->data);

          shape    = picman_brush_generated_get_shape        (brush);
          radius   = picman_brush_generated_get_radius       (brush);
          spikes   = picman_brush_generated_get_spikes       (brush);
          hardness = picman_brush_generated_get_hardness     (brush);
          ratio    = picman_brush_generated_get_aspect_ratio (brush);
          angle    = picman_brush_generated_get_angle        (brush);
        }
    }

  gtk_widget_set_sensitive (brush_editor->options_box,
                            editor->data_editable);

  picman_int_radio_group_set_active (GTK_RADIO_BUTTON (brush_editor->shape_group),
                                   shape);

  gtk_adjustment_set_value (brush_editor->radius_data,       radius);
  gtk_adjustment_set_value (brush_editor->spikes_data,       spikes);
  gtk_adjustment_set_value (brush_editor->hardness_data,     hardness);
  gtk_adjustment_set_value (brush_editor->aspect_ratio_data, ratio);
  gtk_adjustment_set_value (brush_editor->angle_data,        angle);
  gtk_adjustment_set_value (brush_editor->spacing_data,      spacing);
}

static void
picman_brush_editor_set_context (PicmanDocked  *docked,
                               PicmanContext *context)
{
  PicmanDataEditor *data_editor = PICMAN_DATA_EDITOR (docked);

  parent_docked_iface->set_context (docked, context);

  picman_view_renderer_set_context (PICMAN_VIEW (data_editor->view)->renderer,
                                  context);
}


/*  public functions  */

GtkWidget *
picman_brush_editor_new (PicmanContext     *context,
                       PicmanMenuFactory *menu_factory)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return g_object_new (PICMAN_TYPE_BRUSH_EDITOR,
                       "menu-factory",    menu_factory,
                       "menu-identifier", "<BrushEditor>",
                       "ui-path",         "/brush-editor-popup",
                       "data-factory",    context->picman->brush_factory,
                       "context",         context,
                       "data",            picman_context_get_brush (context),
                       NULL);
}


/*  private functions  */

static void
picman_brush_editor_update_brush (GtkAdjustment   *adjustment,
                                PicmanBrushEditor *editor)
{
  PicmanBrushGenerated *brush;
  gdouble             radius;
  gint                spikes;
  gdouble             hardness;
  gdouble             ratio;
  gdouble             angle;
  gdouble             spacing;

  if (! PICMAN_IS_BRUSH_GENERATED (PICMAN_DATA_EDITOR (editor)->data))
    return;

  brush = PICMAN_BRUSH_GENERATED (PICMAN_DATA_EDITOR (editor)->data);

  radius   = gtk_adjustment_get_value (editor->radius_data);
  spikes   = ROUND (gtk_adjustment_get_value (editor->spikes_data));
  hardness = gtk_adjustment_get_value (editor->hardness_data);
  ratio    = gtk_adjustment_get_value (editor->aspect_ratio_data);
  angle    = gtk_adjustment_get_value (editor->angle_data);
  spacing  = gtk_adjustment_get_value (editor->spacing_data);

  if (radius   != picman_brush_generated_get_radius       (brush) ||
      spikes   != picman_brush_generated_get_spikes       (brush) ||
      hardness != picman_brush_generated_get_hardness     (brush) ||
      ratio    != picman_brush_generated_get_aspect_ratio (brush) ||
      angle    != picman_brush_generated_get_angle        (brush) ||
      spacing  != picman_brush_get_spacing                (PICMAN_BRUSH (brush)))
    {
      g_signal_handlers_block_by_func (brush,
                                       picman_brush_editor_notify_brush,
                                       editor);

      picman_data_freeze (PICMAN_DATA (brush));
      g_object_freeze_notify (G_OBJECT (brush));

      picman_brush_generated_set_radius       (brush, radius);
      picman_brush_generated_set_spikes       (brush, spikes);
      picman_brush_generated_set_hardness     (brush, hardness);
      picman_brush_generated_set_aspect_ratio (brush, ratio);
      picman_brush_generated_set_angle        (brush, angle);
      picman_brush_set_spacing                (PICMAN_BRUSH (brush), spacing);

      g_object_thaw_notify (G_OBJECT (brush));
      picman_data_thaw (PICMAN_DATA (brush));

      g_signal_handlers_unblock_by_func (brush,
                                         picman_brush_editor_notify_brush,
                                         editor);
    }
}

static void
picman_brush_editor_update_shape (GtkWidget       *widget,
                                PicmanBrushEditor *editor)
{
  PicmanBrushGenerated *brush;

  if (! PICMAN_IS_BRUSH_GENERATED (PICMAN_DATA_EDITOR (editor)->data))
    return;

  brush = PICMAN_BRUSH_GENERATED (PICMAN_DATA_EDITOR (editor)->data);

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    {
      PicmanBrushGeneratedShape shape;

      shape = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
                                                  "picman-item-data"));

      if (picman_brush_generated_get_shape (brush) != shape)
        picman_brush_generated_set_shape (brush, shape);
    }
}

static void
picman_brush_editor_notify_brush (PicmanBrushGenerated   *brush,
                                GParamSpec           *pspec,
                                PicmanBrushEditor      *editor)
{
  GtkAdjustment *adj   = NULL;
  gdouble        value = 0.0;

  if (! strcmp (pspec->name, "shape"))
    {
      g_signal_handlers_block_by_func (editor->shape_group,
                                       picman_brush_editor_update_shape,
                                       editor);

      picman_int_radio_group_set_active (GTK_RADIO_BUTTON (editor->shape_group),
                                       brush->shape);

      g_signal_handlers_unblock_by_func (editor->shape_group,
                                         picman_brush_editor_update_shape,
                                         editor);

      adj   = editor->radius_data;
      value = brush->radius;
    }
  else if (! strcmp (pspec->name, "radius"))
    {
      adj   = editor->radius_data;
      value = brush->radius;
    }
  else if (! strcmp (pspec->name, "spikes"))
    {
      adj   = editor->spikes_data;
      value = brush->spikes;
    }
  else if (! strcmp (pspec->name, "hardness"))
    {
      adj   = editor->hardness_data;
      value = brush->hardness;
    }
  else if (! strcmp (pspec->name, "angle"))
    {
      adj   = editor->angle_data;
      value = brush->angle;
    }
  else if (! strcmp (pspec->name, "aspect-ratio"))
    {
      adj   = editor->aspect_ratio_data;
      value = brush->aspect_ratio;
    }
  else if (! strcmp (pspec->name, "spacing"))
    {
      adj   = editor->spacing_data;
      value = PICMAN_BRUSH (brush)->spacing;
    }

  if (adj)
    {
      g_signal_handlers_block_by_func (adj,
                                       picman_brush_editor_update_brush,
                                       editor);

      gtk_adjustment_set_value (adj, value);

      g_signal_handlers_unblock_by_func (adj,
                                         picman_brush_editor_update_brush,
                                         editor);
    }
}
