/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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
#include "core/picmandrawable.h"
#include "core/picmandrawable-histogram.h"
#include "core/picmanhistogram.h"
#include "core/picmanimage.h"

#include "picmandocked.h"
#include "picmanhelp-ids.h"
#include "picmanhistogrambox.h"
#include "picmanhistogrameditor.h"
#include "picmanhistogramview.h"
#include "picmansessioninfo-aux.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


static void     picman_histogram_editor_docked_iface_init (PicmanDockedInterface *iface);

static void     picman_histogram_editor_set_aux_info  (PicmanDocked          *docked,
                                                     GList               *aux_info);
static GList  * picman_histogram_editor_get_aux_info  (PicmanDocked          *docked);

static void     picman_histogram_editor_set_image     (PicmanImageEditor     *editor,
                                                     PicmanImage           *image);
static void     picman_histogram_editor_layer_changed (PicmanImage           *image,
                                                     PicmanHistogramEditor *editor);
static void     picman_histogram_editor_frozen_update (PicmanHistogramEditor *editor,
                                                     const GParamSpec    *pspec);
static void     picman_histogram_editor_update        (PicmanHistogramEditor *editor);

static gboolean picman_histogram_editor_idle_update   (PicmanHistogramEditor *editor);
static gboolean picman_histogram_menu_sensitivity     (gint                 value,
                                                     gpointer             data);
static void     picman_histogram_editor_menu_update   (PicmanHistogramEditor *editor);
static void     picman_histogram_editor_name_update   (PicmanHistogramEditor *editor);
static void     picman_histogram_editor_info_update   (PicmanHistogramEditor *editor);

static gboolean picman_histogram_view_expose          (PicmanHistogramEditor *editor);


G_DEFINE_TYPE_WITH_CODE (PicmanHistogramEditor, picman_histogram_editor,
                         PICMAN_TYPE_IMAGE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_histogram_editor_docked_iface_init))

#define parent_class picman_histogram_editor_parent_class

static PicmanDockedInterface *parent_docked_iface = NULL;


static void
picman_histogram_editor_class_init (PicmanHistogramEditorClass *klass)
{
  PicmanImageEditorClass *image_editor_class = PICMAN_IMAGE_EDITOR_CLASS (klass);

  image_editor_class->set_image = picman_histogram_editor_set_image;
}

static void
picman_histogram_editor_init (PicmanHistogramEditor *editor)
{
  PicmanHistogramView *view;
  GtkWidget         *hbox;
  GtkWidget         *label;
  GtkWidget         *menu;
  GtkWidget         *table;
  gint               i;

  const gchar *picman_histogram_editor_labels[] =
    {
      N_("Mean:"),
      N_("Std dev:"),
      N_("Median:"),
      N_("Pixels:"),
      N_("Count:"),
      N_("Percentile:")
    };

  editor->drawable     = NULL;
  editor->histogram    = NULL;
  editor->bg_histogram = NULL;
  editor->valid        = FALSE;
  editor->idle_id      = 0;
  editor->box          = picman_histogram_box_new ();

  picman_editor_set_show_name (PICMAN_EDITOR (editor), TRUE);

  view = PICMAN_HISTOGRAM_BOX (editor->box)->view;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (editor), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Channel:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  editor->menu = menu = picman_prop_enum_combo_box_new (G_OBJECT (view),
                                                      "histogram-channel",
                                                      0, 0);
  picman_enum_combo_box_set_stock_prefix (PICMAN_ENUM_COMBO_BOX (menu),
                                        "picman-channel");
  picman_int_combo_box_set_sensitivity (PICMAN_INT_COMBO_BOX (editor->menu),
                                      picman_histogram_menu_sensitivity,
                                      editor, NULL);
  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (editor->menu),
                                 view->channel);
  gtk_box_pack_start (GTK_BOX (hbox), menu, FALSE, FALSE, 0);
  gtk_widget_show (menu);

  menu = picman_prop_enum_stock_box_new (G_OBJECT (view),
                                       "histogram-scale", "picman-histogram",
                                       0, 0);
  gtk_box_pack_end (GTK_BOX (hbox), menu, FALSE, FALSE, 0);
  gtk_widget_show (menu);

  gtk_box_pack_start (GTK_BOX (editor), editor->box, TRUE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (editor->box));

  g_signal_connect_swapped (view, "range-changed",
                            G_CALLBACK (picman_histogram_editor_info_update),
                            editor);
  g_signal_connect_swapped (view, "notify::histogram-channel",
                            G_CALLBACK (picman_histogram_editor_info_update),
                            editor);

  g_signal_connect_swapped (view, "expose-event",
                            G_CALLBACK (picman_histogram_view_expose),
                            editor);

  table = gtk_table_new (3, 4, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacing (GTK_TABLE (table), 1, 6);
  gtk_box_pack_start (GTK_BOX (editor), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  for (i = 0; i < 6; i++)
    {
      gint x = (i / 3) * 2;
      gint y = (i % 3);

      label = gtk_label_new (gettext (picman_histogram_editor_labels[i]));
      picman_label_set_attributes (GTK_LABEL (label),
                                 PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                                 PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                                 -1);
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_table_attach (GTK_TABLE (table), label, x, x + 1, y, y + 1,
                        GTK_FILL | GTK_EXPAND, GTK_FILL, 2, 2);
      gtk_widget_show (label);

      editor->labels[i] =
        label = g_object_new (GTK_TYPE_LABEL,
                              "xalign",      0.0,
                              "yalign",      0.5,
                              "width-chars", i > 2 ? 9 : 5,
                              NULL);
      picman_label_set_attributes (GTK_LABEL (editor->labels[i]),
                                 PANGO_ATTR_SCALE, PANGO_SCALE_SMALL,
                                 -1);
      gtk_table_attach (GTK_TABLE (table), label, x + 1, x + 2, y, y + 1,
                        GTK_FILL, GTK_FILL, 2, 2);
      gtk_widget_show (label);
    }
}

static void
picman_histogram_editor_docked_iface_init (PicmanDockedInterface *docked_iface)
{
  parent_docked_iface = g_type_interface_peek_parent (docked_iface);

  if (! parent_docked_iface)
    parent_docked_iface = g_type_default_interface_peek (PICMAN_TYPE_DOCKED);

  docked_iface->set_aux_info = picman_histogram_editor_set_aux_info;
  docked_iface->get_aux_info = picman_histogram_editor_get_aux_info;
}

static void
picman_histogram_editor_set_aux_info (PicmanDocked *docked,
                                    GList      *aux_info)
{
  PicmanHistogramEditor *editor = PICMAN_HISTOGRAM_EDITOR (docked);
  PicmanHistogramView   *view   = PICMAN_HISTOGRAM_BOX (editor->box)->view;

  parent_docked_iface->set_aux_info (docked, aux_info);

  picman_session_info_aux_set_props (G_OBJECT (view), aux_info,
                                   "histogram-channel",
                                   "histogram-scale",
                                   NULL);
}

static GList *
picman_histogram_editor_get_aux_info (PicmanDocked *docked)
{
  PicmanHistogramEditor *editor = PICMAN_HISTOGRAM_EDITOR (docked);
  PicmanHistogramView   *view   = PICMAN_HISTOGRAM_BOX (editor->box)->view;
  GList               *aux_info;

  aux_info = parent_docked_iface->get_aux_info (docked);

  return g_list_concat (aux_info,
                        picman_session_info_aux_new_from_props (G_OBJECT (view),
                                                              "histogram-channel",
                                                              "histogram-scale",
                                                              NULL));
}

static void
picman_histogram_editor_set_image (PicmanImageEditor *image_editor,
                                 PicmanImage       *image)
{
  PicmanHistogramEditor *editor = PICMAN_HISTOGRAM_EDITOR (image_editor);
  PicmanHistogramView   *view   = PICMAN_HISTOGRAM_BOX (editor->box)->view;

  if (image_editor->image)
    {
      if (editor->idle_id)
        {
          g_source_remove (editor->idle_id);
          editor->idle_id = 0;
        }

      g_signal_handlers_disconnect_by_func (image_editor->image,
                                            picman_histogram_editor_update,
                                            editor);
      g_signal_handlers_disconnect_by_func (image_editor->image,
                                            picman_histogram_editor_layer_changed,
                                            editor);
      g_signal_handlers_disconnect_by_func (image_editor->image,
                                            picman_histogram_editor_menu_update,
                                            editor);

      if (editor->histogram)
        {
          picman_histogram_unref (editor->histogram);
          editor->histogram = NULL;

          picman_histogram_view_set_histogram (view, NULL);
        }

      if (editor->bg_histogram)
        {
          picman_histogram_unref (editor->bg_histogram);
          editor->bg_histogram = NULL;

          picman_histogram_view_set_background (view, NULL);
        }
    }

  PICMAN_IMAGE_EDITOR_CLASS (parent_class)->set_image (image_editor, image);

  if (image)
    {
      editor->histogram = picman_histogram_new ();

      picman_histogram_view_set_histogram (view, editor->histogram);

      g_signal_connect_object (image, "mode-changed",
                               G_CALLBACK (picman_histogram_editor_menu_update),
                               editor, G_CONNECT_SWAPPED);
      g_signal_connect_object (image, "active-layer-changed",
                               G_CALLBACK (picman_histogram_editor_layer_changed),
                               editor, 0);
      g_signal_connect_object (image, "mask-changed",
                               G_CALLBACK (picman_histogram_editor_update),
                               editor, G_CONNECT_SWAPPED);
    }

  picman_histogram_editor_layer_changed (image, editor);
}

GtkWidget *
picman_histogram_editor_new (void)
{
  return g_object_new (PICMAN_TYPE_HISTOGRAM_EDITOR, NULL);
}

static void
picman_histogram_editor_layer_changed (PicmanImage           *image,
                                     PicmanHistogramEditor *editor)
{
  if (editor->drawable)
    {
      if (editor->bg_histogram)
        {
          PicmanHistogramView *view = PICMAN_HISTOGRAM_BOX (editor->box)->view;

          picman_histogram_unref (editor->bg_histogram);
          editor->bg_histogram = NULL;

          picman_histogram_view_set_background (view, NULL);
        }

      g_signal_handlers_disconnect_by_func (editor->drawable,
                                            picman_histogram_editor_name_update,
                                            editor);
      g_signal_handlers_disconnect_by_func (editor->drawable,
                                            picman_histogram_editor_menu_update,
                                            editor);
      g_signal_handlers_disconnect_by_func (editor->drawable,
                                            picman_histogram_editor_update,
                                            editor);
      g_signal_handlers_disconnect_by_func (editor->drawable,
                                            picman_histogram_editor_frozen_update,
                                            editor);
      editor->drawable = NULL;
    }

  if (image)
    editor->drawable = (PicmanDrawable *) picman_image_get_active_layer (image);

  picman_histogram_editor_menu_update (editor);

  if (editor->drawable)
    {
      g_signal_connect_object (editor->drawable, "notify::frozen",
                               G_CALLBACK (picman_histogram_editor_frozen_update),
                               editor, G_CONNECT_SWAPPED);
      g_signal_connect_object (editor->drawable, "update",
                               G_CALLBACK (picman_histogram_editor_update),
                               editor, G_CONNECT_SWAPPED);
      g_signal_connect_object (editor->drawable, "alpha-changed",
                               G_CALLBACK (picman_histogram_editor_menu_update),
                               editor, G_CONNECT_SWAPPED);
      g_signal_connect_object (editor->drawable, "name-changed",
                               G_CALLBACK (picman_histogram_editor_name_update),
                               editor, G_CONNECT_SWAPPED);

      picman_histogram_editor_update (editor);
    }
  else if (editor->histogram)
    {
      editor->valid = FALSE;
      gtk_widget_queue_draw (GTK_WIDGET (editor->box));
    }

  picman_histogram_editor_info_update (editor);
  picman_histogram_editor_name_update (editor);
}

static gboolean
picman_histogram_editor_validate (PicmanHistogramEditor *editor)
{
  if (! editor->valid && editor->histogram)
    {
      if (editor->drawable)
        picman_drawable_calculate_histogram (editor->drawable, editor->histogram);
      else
        picman_histogram_clear_values (editor->histogram);

      picman_histogram_editor_info_update (editor);

      editor->valid = TRUE;
    }

  return editor->valid;
}

static void
picman_histogram_editor_frozen_update (PicmanHistogramEditor *editor,
                                     const GParamSpec    *pspec)
{
  PicmanHistogramView *view = PICMAN_HISTOGRAM_BOX (editor->box)->view;

  if (picman_viewable_preview_is_frozen (PICMAN_VIEWABLE (editor->drawable)))
    {
      /* Only do the background histogram if the histogram is visible.
       * This is a workaround for the fact that recalculating the
       * histogram is expensive and that it is only validated when it
       * is shown. So don't slow down painting by doing something that
       * is not even seen by the user.
       */
      if (! editor->bg_histogram &&
          gtk_widget_is_drawable (GTK_WIDGET (editor)))
        {
          if (picman_histogram_editor_validate (editor))
            editor->bg_histogram = picman_histogram_duplicate (editor->histogram);

          picman_histogram_view_set_background (view, editor->bg_histogram);
        }
    }
  else if (editor->bg_histogram)
    {
      picman_histogram_unref (editor->bg_histogram);
      editor->bg_histogram = NULL;

      picman_histogram_view_set_background (view, NULL);
    }
}

static void
picman_histogram_editor_update (PicmanHistogramEditor *editor)
{
  if (editor->idle_id)
    g_source_remove (editor->idle_id);

  editor->idle_id =
    g_timeout_add_full (G_PRIORITY_LOW,
                        200,
                        (GSourceFunc) picman_histogram_editor_idle_update,
                        editor,
                        NULL);
}

static gboolean
picman_histogram_editor_idle_update (PicmanHistogramEditor *editor)
{
  editor->idle_id = 0;

  /* Mark the histogram as invalid and queue a redraw.
   * We will then recalculate the histogram when the view is exposed.
   */

  editor->valid = FALSE;
  gtk_widget_queue_draw (GTK_WIDGET (editor->box));

  return FALSE;
}

static gboolean
picman_histogram_editor_channel_valid (PicmanHistogramEditor  *editor,
                                     PicmanHistogramChannel  channel)
{
  if (editor->drawable)
    {
      switch (channel)
        {
        case PICMAN_HISTOGRAM_VALUE:
          return TRUE;

        case PICMAN_HISTOGRAM_RED:
        case PICMAN_HISTOGRAM_GREEN:
        case PICMAN_HISTOGRAM_BLUE:
        case PICMAN_HISTOGRAM_RGB:
          return picman_drawable_is_rgb (editor->drawable);

        case PICMAN_HISTOGRAM_ALPHA:
          return picman_drawable_has_alpha (editor->drawable);
        }
    }

  return TRUE;
}

static gboolean
picman_histogram_menu_sensitivity (gint      value,
                                 gpointer  data)
{
  PicmanHistogramEditor  *editor  = PICMAN_HISTOGRAM_EDITOR (data);
  PicmanHistogramChannel  channel = value;

  if (editor->drawable)
    return picman_histogram_editor_channel_valid (editor, channel);

  return FALSE;
}

static void
picman_histogram_editor_menu_update (PicmanHistogramEditor *editor)
{
  PicmanHistogramView *view = PICMAN_HISTOGRAM_BOX (editor->box)->view;

  gtk_widget_queue_draw (editor->menu);

  if (! picman_histogram_editor_channel_valid (editor, view->channel))
    {
      picman_histogram_view_set_channel (view, PICMAN_HISTOGRAM_VALUE);
    }
}

static void
picman_histogram_editor_name_update (PicmanHistogramEditor *editor)
{
  const gchar *name = NULL;

  if (editor->drawable)
    name = picman_object_get_name (editor->drawable);

  picman_editor_set_name (PICMAN_EDITOR (editor), name);
}

static void
picman_histogram_editor_info_update (PicmanHistogramEditor *editor)
{
  PicmanHistogramView *view = PICMAN_HISTOGRAM_BOX (editor->box)->view;
  PicmanHistogram     *hist = editor->histogram;

  if (hist)
    {
      gdouble pixels;
      gdouble count;
      gchar   text[12];

      pixels = picman_histogram_get_count (hist, view->channel, 0, 255);
      count  = picman_histogram_get_count (hist, view->channel,
                                         view->start, view->end);

      g_snprintf (text, sizeof (text), "%.1f",
                  picman_histogram_get_mean (hist, view->channel,
                                           view->start, view->end));
      gtk_label_set_text (GTK_LABEL (editor->labels[0]), text);

      g_snprintf (text, sizeof (text), "%.1f",
                  picman_histogram_get_std_dev (hist, view->channel,
                                              view->start, view->end));
      gtk_label_set_text (GTK_LABEL (editor->labels[1]), text);

      g_snprintf (text, sizeof (text), "%.1f",
                  (gdouble) picman_histogram_get_median  (hist, view->channel,
                                                        view->start,
                                                        view->end));
      gtk_label_set_text (GTK_LABEL (editor->labels[2]), text);

      g_snprintf (text, sizeof (text), "%d", (gint) pixels);
      gtk_label_set_text (GTK_LABEL (editor->labels[3]), text);

      g_snprintf (text, sizeof (text), "%d", (gint) count);
      gtk_label_set_text (GTK_LABEL (editor->labels[4]), text);

      g_snprintf (text, sizeof (text), "%.1f", (pixels > 0 ?
                                                 (100.0 * count / pixels) :
                                                 0.0));
      gtk_label_set_text (GTK_LABEL (editor->labels[5]), text);
    }
  else
    {
      gint i;

      for (i = 0; i < 6; i++)
        gtk_label_set_text (GTK_LABEL (editor->labels[i]), NULL);
    }
}

static gboolean
picman_histogram_view_expose (PicmanHistogramEditor *editor)
{
  picman_histogram_editor_validate (editor);

  return FALSE;
}
