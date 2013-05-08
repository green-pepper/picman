/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansamplepointeditor.c
 * Copyright (C) 2005 Michael Natterer <mitch@picman.org>
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

#include "core/picman.h"
#include "core/picmanimage.h"
#include "core/picmanimage-pick-color.h"
#include "core/picmanimage-sample-points.h"
#include "core/picmansamplepoint.h"

#include "picmancolorframe.h"
#include "picmanmenufactory.h"
#include "picmansamplepointeditor.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_SAMPLE_MERGED
};


static void   picman_sample_point_editor_constructed    (GObject               *object);
static void   picman_sample_point_editor_dispose        (GObject               *object);
static void   picman_sample_point_editor_set_property   (GObject               *object,
                                                       guint                  property_id,
                                                       const GValue          *value,
                                                       GParamSpec            *pspec);
static void   picman_sample_point_editor_get_property   (GObject               *object,
                                                       guint                  property_id,
                                                       GValue                *value,
                                                       GParamSpec            *pspec);

static void   picman_sample_point_editor_style_set      (GtkWidget             *widget,
                                                       GtkStyle              *prev_style);
static void   picman_sample_point_editor_set_image      (PicmanImageEditor       *editor,
                                                       PicmanImage             *image);

static void   picman_sample_point_editor_point_added    (PicmanImage             *image,
                                                       PicmanSamplePoint       *sample_point,
                                                       PicmanSamplePointEditor *editor);
static void   picman_sample_point_editor_point_removed  (PicmanImage             *image,
                                                       PicmanSamplePoint       *sample_point,
                                                       PicmanSamplePointEditor *editor);
static void   picman_sample_point_editor_point_moved    (PicmanImage             *image,
                                                       PicmanSamplePoint       *sample_point,
                                                       PicmanSamplePointEditor *editor);
static void   picman_sample_point_editor_proj_update    (PicmanImage             *image,
                                                       gboolean               now,
                                                       gint                   x,
                                                       gint                   y,
                                                       gint                   width,
                                                       gint                   height,
                                                       PicmanSamplePointEditor *editor);
static void   picman_sample_point_editor_points_changed (PicmanSamplePointEditor *editor);
static void   picman_sample_point_editor_dirty          (PicmanSamplePointEditor *editor,
                                                       gint                   index);
static gboolean picman_sample_point_editor_update       (PicmanSamplePointEditor *editor);


G_DEFINE_TYPE (PicmanSamplePointEditor, picman_sample_point_editor,
               PICMAN_TYPE_IMAGE_EDITOR)

#define parent_class picman_sample_point_editor_parent_class


static void
picman_sample_point_editor_class_init (PicmanSamplePointEditorClass *klass)
{
  GObjectClass         *object_class       = G_OBJECT_CLASS (klass);
  GtkWidgetClass       *widget_class       = GTK_WIDGET_CLASS (klass);
  PicmanImageEditorClass *image_editor_class = PICMAN_IMAGE_EDITOR_CLASS (klass);

  object_class->constructed     = picman_sample_point_editor_constructed;
  object_class->dispose         = picman_sample_point_editor_dispose;
  object_class->get_property    = picman_sample_point_editor_get_property;
  object_class->set_property    = picman_sample_point_editor_set_property;

  widget_class->style_set       = picman_sample_point_editor_style_set;

  image_editor_class->set_image = picman_sample_point_editor_set_image;

  g_object_class_install_property (object_class, PROP_SAMPLE_MERGED,
                                   g_param_spec_boolean ("sample-merged",
                                                         NULL, NULL,
                                                         TRUE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));
}

static void
picman_sample_point_editor_init (PicmanSamplePointEditor *editor)
{
  gint content_spacing;
  gint i;
  gint row    = 0;
  gint column = 0;

  editor->sample_merged = TRUE;

  gtk_widget_style_get (GTK_WIDGET (editor),
                        "content-spacing", &content_spacing,
                        NULL);

  editor->table = gtk_table_new (2, 2, TRUE);
  gtk_table_set_row_spacing (GTK_TABLE (editor->table), 0, content_spacing);
  gtk_table_set_col_spacing (GTK_TABLE (editor->table), 0, content_spacing);
  gtk_box_pack_start (GTK_BOX (editor), editor->table, FALSE, FALSE, 0);
  gtk_widget_show (editor->table);

  for (i = 0; i < 4; i++)
    {
      GtkWidget *frame;

      frame = g_object_new (PICMAN_TYPE_COLOR_FRAME,
                            "mode",           PICMAN_COLOR_FRAME_MODE_PIXEL,
                            "has-number",     TRUE,
                            "number",         i + 1,
                            "has-color-area", TRUE,
                            "sensitive",      FALSE,
                            NULL);

      gtk_table_attach (GTK_TABLE (editor->table), frame,
                        column, column + 1, row, row + 1,
                        GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
      gtk_widget_show (frame);

      editor->color_frames[i] = frame;

      column++;

      if (column > 1)
        {
          column = 0;
          row++;
        }
    }
}

static void
picman_sample_point_editor_constructed (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->constructed (object);
}

static void
picman_sample_point_editor_dispose (GObject *object)
{
  PicmanSamplePointEditor *editor = PICMAN_SAMPLE_POINT_EDITOR (object);

  if (editor->dirty_idle_id)
    {
      g_source_remove (editor->dirty_idle_id);
      editor->dirty_idle_id = 0;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_sample_point_editor_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PicmanSamplePointEditor *editor = PICMAN_SAMPLE_POINT_EDITOR (object);

  switch (property_id)
    {
    case PROP_SAMPLE_MERGED:
      picman_sample_point_editor_set_sample_merged (editor,
                                                  g_value_get_boolean (value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_sample_point_editor_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PicmanSamplePointEditor *editor = PICMAN_SAMPLE_POINT_EDITOR (object);

  switch (property_id)
    {
    case PROP_SAMPLE_MERGED:
      g_value_set_boolean (value, editor->sample_merged);
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_sample_point_editor_style_set (GtkWidget *widget,
                                    GtkStyle  *prev_style)
{
  PicmanSamplePointEditor *editor = PICMAN_SAMPLE_POINT_EDITOR (widget);
  gint                   content_spacing;

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  gtk_widget_style_get (widget,
                        "content-spacing", &content_spacing,
                        NULL);

  gtk_table_set_row_spacing (GTK_TABLE (editor->table), 0, content_spacing);
  gtk_table_set_col_spacing (GTK_TABLE (editor->table), 0, content_spacing);
}

static void
picman_sample_point_editor_set_image (PicmanImageEditor *image_editor,
                                    PicmanImage       *image)
{
  PicmanSamplePointEditor *editor = PICMAN_SAMPLE_POINT_EDITOR (image_editor);

  if (image_editor->image)
    {
      g_signal_handlers_disconnect_by_func (image_editor->image,
                                            picman_sample_point_editor_point_added,
                                            editor);
      g_signal_handlers_disconnect_by_func (image_editor->image,
                                            picman_sample_point_editor_point_removed,
                                            editor);
      g_signal_handlers_disconnect_by_func (image_editor->image,
                                            picman_sample_point_editor_point_moved,
                                            editor);

      g_signal_handlers_disconnect_by_func (picman_image_get_projection (image_editor->image),
                                            picman_sample_point_editor_proj_update,
                                            editor);
    }

  PICMAN_IMAGE_EDITOR_CLASS (parent_class)->set_image (image_editor, image);

  if (image)
    {
      g_signal_connect (image, "sample-point-added",
                        G_CALLBACK (picman_sample_point_editor_point_added),
                        editor);
      g_signal_connect (image, "sample-point-removed",
                        G_CALLBACK (picman_sample_point_editor_point_removed),
                        editor);
      g_signal_connect (image, "sample-point-moved",
                        G_CALLBACK (picman_sample_point_editor_point_moved),
                        editor);

      g_signal_connect (picman_image_get_projection (image), "update",
                        G_CALLBACK (picman_sample_point_editor_proj_update),
                        editor);
    }

  picman_sample_point_editor_points_changed (editor);
}


/*  public functions  */

GtkWidget *
picman_sample_point_editor_new (PicmanMenuFactory *menu_factory)
{
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  return g_object_new (PICMAN_TYPE_SAMPLE_POINT_EDITOR,
                       "menu-factory",    menu_factory,
                       "menu-identifier", "<SamplePoints>",
                       "ui-path",         "/sample-points-popup",
                       NULL);
}

void
picman_sample_point_editor_set_sample_merged (PicmanSamplePointEditor *editor,
                                            gboolean               sample_merged)
{
  g_return_if_fail (PICMAN_IS_SAMPLE_POINT_EDITOR (editor));

  sample_merged = sample_merged ? TRUE : FALSE;

  if (editor->sample_merged != sample_merged)
    {
      gint i;

      editor->sample_merged = sample_merged;

      for (i = 0; i < 4; i++)
        editor->dirty[i] = TRUE;

      picman_sample_point_editor_dirty (editor, -1);

      g_object_notify (G_OBJECT (editor), "sample-merged");
    }
}

gboolean
picman_sample_point_editor_get_sample_merged (PicmanSamplePointEditor *editor)
{
  g_return_val_if_fail (PICMAN_IS_SAMPLE_POINT_EDITOR (editor), FALSE);

  return editor->sample_merged;
}

/*  private functions  */

static void
picman_sample_point_editor_point_added (PicmanImage             *image,
                                      PicmanSamplePoint       *sample_point,
                                      PicmanSamplePointEditor *editor)
{
  picman_sample_point_editor_points_changed (editor);
}

static void
picman_sample_point_editor_point_removed (PicmanImage             *image,
                                        PicmanSamplePoint       *sample_point,
                                        PicmanSamplePointEditor *editor)
{
  picman_sample_point_editor_points_changed (editor);
}

static void
picman_sample_point_editor_point_moved (PicmanImage             *image,
                                      PicmanSamplePoint       *sample_point,
                                      PicmanSamplePointEditor *editor)
{
  gint i = g_list_index (picman_image_get_sample_points (image), sample_point);

  if (i < 4)
    picman_sample_point_editor_dirty (editor, i);
}

static void
picman_sample_point_editor_proj_update (PicmanImage             *image,
                                      gboolean               now,
                                      gint                   x,
                                      gint                   y,
                                      gint                   width,
                                      gint                   height,
                                      PicmanSamplePointEditor *editor)
{
  PicmanImageEditor *image_editor = PICMAN_IMAGE_EDITOR (editor);
  GList           *sample_points;
  gint             n_points     = 0;
  GList           *list;
  gint             i;

  sample_points = picman_image_get_sample_points (image_editor->image);

  n_points = MIN (4, g_list_length (sample_points));

  for (i = 0, list = sample_points;
       i < n_points;
       i++, list = g_list_next (list))
    {
      PicmanSamplePoint *sample_point = list->data;

      if (sample_point->x >= x && sample_point->x < (x + width) &&
          sample_point->y >= y && sample_point->y < (y + height))
        {
          picman_sample_point_editor_dirty (editor, i);
        }
    }
}

static void
picman_sample_point_editor_points_changed (PicmanSamplePointEditor *editor)
{
  PicmanImageEditor *image_editor = PICMAN_IMAGE_EDITOR (editor);
  GList           *sample_points;
  gint             n_points     = 0;
  gint             i;

  if (image_editor->image)
    {
      sample_points = picman_image_get_sample_points (image_editor->image);
      n_points = MIN (4, g_list_length (sample_points));
    }

  for (i = 0; i < n_points; i++)
    {
      gtk_widget_set_sensitive (editor->color_frames[i], TRUE);
      editor->dirty[i] = TRUE;
    }

  for (i = n_points; i < 4; i++)
    {
      gtk_widget_set_sensitive (editor->color_frames[i], FALSE);
      picman_color_frame_set_invalid (PICMAN_COLOR_FRAME (editor->color_frames[i]));
      editor->dirty[i] = FALSE;
    }

  if (n_points > 0)
    picman_sample_point_editor_dirty (editor, -1);
}

static void
picman_sample_point_editor_dirty (PicmanSamplePointEditor *editor,
                                gint                   index)
{
  if (index >= 0)
    editor->dirty[index] = TRUE;

  if (editor->dirty_idle_id)
    g_source_remove (editor->dirty_idle_id);

  editor->dirty_idle_id =
    g_idle_add ((GSourceFunc) picman_sample_point_editor_update,
                editor);
}

static gboolean
picman_sample_point_editor_update (PicmanSamplePointEditor *editor)
{
  PicmanImageEditor *image_editor = PICMAN_IMAGE_EDITOR (editor);
  GList           *sample_points;
  gint             n_points     = 0;
  GList           *list;
  gint             i;

  editor->dirty_idle_id = 0;

  if (! image_editor->image)
    return FALSE;

  sample_points = picman_image_get_sample_points (image_editor->image);

  n_points = MIN (4, g_list_length (sample_points));

  for (i = 0, list = sample_points;
       i < n_points;
       i++, list = g_list_next (list))
    {
      if (editor->dirty[i])
        {
          PicmanSamplePoint *sample_point = list->data;
          PicmanColorFrame  *color_frame;
          const Babl      *format;
          PicmanRGB          color;
          gint             color_index;

          editor->dirty[i] = FALSE;

          color_frame = PICMAN_COLOR_FRAME (editor->color_frames[i]);

          if (picman_image_pick_color (image_editor->image, NULL,
                                     sample_point->x,
                                     sample_point->y,
                                     editor->sample_merged,
                                     FALSE, 0.0,
                                     &format,
                                     &color,
                                     &color_index))
            {
              picman_color_frame_set_color (color_frame, format,
                                          &color, color_index);
            }
          else
            {
              picman_color_frame_set_invalid (color_frame);
            }
        }
    }

  return FALSE;
}
