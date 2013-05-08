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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmancontainer.h"
#include "core/picmanimage.h"
#include "core/picmanimage-colormap.h"
#include "core/picmanmarshal.h"
#include "core/picmanpalette.h"

#include "picmancolormapeditor.h"
#include "picmandnd.h"
#include "picmandocked.h"
#include "picmanmenufactory.h"
#include "picmanpaletteview.h"
#include "picmanuimanager.h"
#include "picmanviewrendererpalette.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


#define BORDER  6
#define EPSILON 1e-10

#define HAVE_COLORMAP(image) \
        (image != NULL && \
         picman_image_get_base_type (image) == PICMAN_INDEXED && \
         picman_image_get_colormap (image) != NULL)


static void picman_colormap_editor_docked_iface_init (PicmanDockedInterface *face);

static void   picman_colormap_editor_constructed     (GObject            *object);
static void   picman_colormap_editor_dispose         (GObject            *object);
static void   picman_colormap_editor_finalize        (GObject            *object);

static void   picman_colormap_editor_unmap           (GtkWidget          *widget);

static void   picman_colormap_editor_set_image       (PicmanImageEditor    *editor,
                                                    PicmanImage          *image);

static void   picman_colormap_editor_set_context     (PicmanDocked        *docked,
                                                    PicmanContext       *context);

static PangoLayout *
              picman_colormap_editor_create_layout   (GtkWidget          *widget);

static void   picman_colormap_editor_update_entries  (PicmanColormapEditor *editor);

static gboolean picman_colormap_preview_expose       (GtkWidget          *widget,
                                                    GdkEventExpose     *event,
                                                    PicmanColormapEditor *editor);

static void   picman_colormap_editor_entry_clicked   (PicmanPaletteView    *view,
                                                    PicmanPaletteEntry   *entry,
                                                    GdkModifierType    state,
                                                    PicmanColormapEditor *editor);
static void   picman_colormap_editor_entry_selected  (PicmanPaletteView    *view,
                                                    PicmanPaletteEntry   *entry,
                                                    PicmanColormapEditor *editor);
static void   picman_colormap_editor_entry_activated (PicmanPaletteView    *view,
                                                    PicmanPaletteEntry   *entry,
                                                    PicmanColormapEditor *editor);
static void   picman_colormap_editor_entry_context   (PicmanPaletteView    *view,
                                                    PicmanPaletteEntry   *entry,
                                                    PicmanColormapEditor *editor);
static void   picman_colormap_editor_color_dropped   (PicmanPaletteView    *view,
                                                    PicmanPaletteEntry   *entry,
                                                    const PicmanRGB      *color,
                                                    PicmanColormapEditor *editor);

static void   picman_colormap_adjustment_changed     (GtkAdjustment      *adjustment,
                                                    PicmanColormapEditor *editor);
static void   picman_colormap_hex_entry_changed      (PicmanColorHexEntry  *entry,
                                                    PicmanColormapEditor *editor);

static void   picman_colormap_image_mode_changed     (PicmanImage          *image,
                                                    PicmanColormapEditor *editor);
static void   picman_colormap_image_colormap_changed (PicmanImage          *image,
                                                    gint                ncol,
                                                    PicmanColormapEditor *editor);


G_DEFINE_TYPE_WITH_CODE (PicmanColormapEditor, picman_colormap_editor,
                         PICMAN_TYPE_IMAGE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_colormap_editor_docked_iface_init))

#define parent_class picman_colormap_editor_parent_class

static PicmanDockedInterface *parent_docked_iface = NULL;


static void
picman_colormap_editor_class_init (PicmanColormapEditorClass* klass)
{
  GObjectClass         *object_class       = G_OBJECT_CLASS (klass);
  GtkWidgetClass       *widget_class       = GTK_WIDGET_CLASS (klass);
  PicmanImageEditorClass *image_editor_class = PICMAN_IMAGE_EDITOR_CLASS (klass);

  object_class->constructed     = picman_colormap_editor_constructed;
  object_class->dispose         = picman_colormap_editor_dispose;
  object_class->finalize        = picman_colormap_editor_finalize;

  widget_class->unmap           = picman_colormap_editor_unmap;

  image_editor_class->set_image = picman_colormap_editor_set_image;
}

static void
picman_colormap_editor_docked_iface_init (PicmanDockedInterface *iface)
{
  parent_docked_iface = g_type_interface_peek_parent (iface);

  if (! parent_docked_iface)
    parent_docked_iface = g_type_default_interface_peek (PICMAN_TYPE_DOCKED);

  iface->set_context = picman_colormap_editor_set_context;
}

static void
picman_colormap_editor_init (PicmanColormapEditor *editor)
{
  GtkWidget *frame;
  GtkWidget *table;
  GtkObject *adj;

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (editor), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  editor->view = picman_view_new_full_by_types (NULL,
                                              PICMAN_TYPE_PALETTE_VIEW,
                                              PICMAN_TYPE_PALETTE,
                                              1, 1, 0,
                                              FALSE, TRUE, FALSE);
  picman_view_set_expand (PICMAN_VIEW (editor->view), TRUE);
  gtk_container_add (GTK_CONTAINER (frame), editor->view);
  gtk_widget_show (editor->view);

  g_signal_connect (editor->view, "expose-event",
                    G_CALLBACK (picman_colormap_preview_expose),
                    editor);

  g_signal_connect (editor->view, "entry-clicked",
                    G_CALLBACK (picman_colormap_editor_entry_clicked),
                    editor);
  g_signal_connect (editor->view, "entry-selected",
                    G_CALLBACK (picman_colormap_editor_entry_selected),
                    editor);
  g_signal_connect (editor->view, "entry-activated",
                    G_CALLBACK (picman_colormap_editor_entry_activated),
                    editor);
  g_signal_connect (editor->view, "entry-context",
                    G_CALLBACK (picman_colormap_editor_entry_context),
                    editor);
  g_signal_connect (editor->view, "color-dropped",
                    G_CALLBACK (picman_colormap_editor_color_dropped),
                    editor);

  /*  Some helpful hints  */
  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacing (GTK_TABLE (table), 0, 4);
  gtk_table_set_row_spacing (GTK_TABLE (table), 0, 2);
  gtk_box_pack_end (GTK_BOX (editor), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  editor->index_spinbutton = picman_spin_button_new (&adj,
                                                   0, 0, 0, 1, 10, 0, 1.0, 0);
  editor->index_adjustment = GTK_ADJUSTMENT (adj);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("Color index:"), 0.0, 0.5,
                             editor->index_spinbutton, 1, TRUE);

  g_signal_connect (editor->index_adjustment, "value-changed",
                    G_CALLBACK (picman_colormap_adjustment_changed),
                    editor);

  editor->color_entry = picman_color_hex_entry_new ();
  gtk_entry_set_width_chars (GTK_ENTRY (editor->color_entry), 12);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                             _("HTML notation:"), 0.0, 0.5,
                             editor->color_entry, 1, TRUE);

  g_signal_connect (editor->color_entry, "color-changed",
                    G_CALLBACK (picman_colormap_hex_entry_changed),
                    editor);
}

static void
picman_colormap_editor_constructed (GObject *object)
{
  PicmanColormapEditor *editor = PICMAN_COLORMAP_EDITOR (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_editor_add_action_button (PICMAN_EDITOR (editor), "colormap",
                                 "colormap-edit-color",
                                 NULL);

  picman_editor_add_action_button (PICMAN_EDITOR (editor), "colormap",
                                 "colormap-add-color-from-fg",
                                 "colormap-add-color-from-bg",
                                 picman_get_toggle_behavior_mask (),
                                 NULL);
}

static void
picman_colormap_editor_dispose (GObject *object)
{
  PicmanColormapEditor *editor = PICMAN_COLORMAP_EDITOR (object);

  if (editor->color_dialog)
    {
      gtk_widget_destroy (editor->color_dialog);
      editor->color_dialog = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_colormap_editor_finalize (GObject *object)
{
  PicmanColormapEditor *editor = PICMAN_COLORMAP_EDITOR (object);

  if (editor->layout)
    {
      g_object_unref (editor->layout);
      editor->layout = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_colormap_editor_unmap (GtkWidget *widget)
{
  PicmanColormapEditor *editor = PICMAN_COLORMAP_EDITOR (widget);

  if (editor->color_dialog)
    gtk_widget_hide (editor->color_dialog);

  GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

static void
picman_colormap_editor_set_image (PicmanImageEditor *image_editor,
                                PicmanImage       *image)
{
  PicmanColormapEditor *editor = PICMAN_COLORMAP_EDITOR (image_editor);

  if (image_editor->image)
    {
      g_signal_handlers_disconnect_by_func (image_editor->image,
                                            picman_colormap_image_mode_changed,
                                            editor);
      g_signal_handlers_disconnect_by_func (image_editor->image,
                                            picman_colormap_image_colormap_changed,
                                            editor);

      if (editor->color_dialog)
        gtk_widget_hide (editor->color_dialog);

      if (! HAVE_COLORMAP (image))
        {
          gtk_adjustment_set_upper (editor->index_adjustment, 0);

          if (gtk_widget_get_mapped (GTK_WIDGET (editor)))
            picman_view_set_viewable (PICMAN_VIEW (editor->view), NULL);
        }
    }

  PICMAN_IMAGE_EDITOR_CLASS (parent_class)->set_image (image_editor, image);

  editor->col_index = 0;

  if (image)
    {
      g_signal_connect (image, "mode-changed",
                        G_CALLBACK (picman_colormap_image_mode_changed),
                        editor);
      g_signal_connect (image, "colormap-changed",
                        G_CALLBACK (picman_colormap_image_colormap_changed),
                        editor);

      if (HAVE_COLORMAP (image))
        {
          picman_view_set_viewable (PICMAN_VIEW (editor->view),
                                  PICMAN_VIEWABLE (picman_image_get_colormap_palette (image)));

          gtk_adjustment_set_upper (editor->index_adjustment,
                                    picman_image_get_colormap_size (image) - 1);
        }
    }

  picman_colormap_editor_update_entries (editor);
}

static void
picman_colormap_editor_set_context (PicmanDocked  *docked,
                                  PicmanContext *context)
{
  PicmanColormapEditor *editor = PICMAN_COLORMAP_EDITOR (docked);

  parent_docked_iface->set_context (docked, context);

  picman_view_renderer_set_context (PICMAN_VIEW (editor->view)->renderer,
                                  context);
}


/*  public functions  */

GtkWidget *
picman_colormap_editor_new (PicmanMenuFactory *menu_factory)
{
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  return g_object_new (PICMAN_TYPE_COLORMAP_EDITOR,
                       "menu-factory",    menu_factory,
                       "menu-identifier", "<Colormap>",
                       "ui-path",         "/colormap-popup",
                       NULL);
}

gint
picman_colormap_editor_get_index (PicmanColormapEditor *editor,
                                const PicmanRGB      *search)
{
  PicmanImage *image;
  gint       index;

  g_return_val_if_fail (PICMAN_IS_COLORMAP_EDITOR (editor), 01);

  image = PICMAN_IMAGE_EDITOR (editor)->image;

  if (! HAVE_COLORMAP (image))
    return -1;

  index = editor->col_index;

  if (search)
    {
      PicmanRGB temp;

      picman_image_get_colormap_entry (image, index, &temp);

      if (picman_rgb_distance (&temp, search) > EPSILON)
        {
          gint n_colors = picman_image_get_colormap_size (image);
          gint i;

          for (i = 0; i < n_colors; i++)
            {
              picman_image_get_colormap_entry (image, i, &temp);

              if (picman_rgb_distance (&temp, search) < EPSILON)
                {
                  index = i;
                  break;
                }
            }
        }
    }

  return index;
}

gboolean
picman_colormap_editor_set_index (PicmanColormapEditor *editor,
                                gint                index,
                                PicmanRGB            *color)
{
  PicmanImage *image;
  gint       size;

  g_return_val_if_fail (PICMAN_IS_COLORMAP_EDITOR (editor), FALSE);

  image = PICMAN_IMAGE_EDITOR (editor)->image;

  if (! HAVE_COLORMAP (image))
    return FALSE;

  size = picman_image_get_colormap_size (image);

  if (size < 1)
    return FALSE;

  index = CLAMP (index, 0, size - 1);

  if (index != editor->col_index)
    {
      PicmanPalette *palette = picman_image_get_colormap_palette (image);

      editor->col_index = index;

      picman_palette_view_select_entry (PICMAN_PALETTE_VIEW (editor->view),
                                      picman_palette_get_entry (palette, index));

      picman_colormap_editor_update_entries (editor);
    }

  if (color)
    picman_image_get_colormap_entry (PICMAN_IMAGE_EDITOR (editor)->image,
                                   index, color);

  return TRUE;
}

gint
picman_colormap_editor_max_index (PicmanColormapEditor *editor)
{
  PicmanImage *image;

  g_return_val_if_fail (PICMAN_IS_COLORMAP_EDITOR (editor), -1);

  image = PICMAN_IMAGE_EDITOR (editor)->image;

  if (! HAVE_COLORMAP (image))
    return -1;

  return MAX (0, picman_image_get_colormap_size (image) - 1);
}


/*  private functions  */

static PangoLayout *
picman_colormap_editor_create_layout (GtkWidget *widget)
{
  PangoLayout    *layout;
  PangoAttrList  *attrs;
  PangoAttribute *attr;

  layout = gtk_widget_create_pango_layout (widget,
                                           _("Only indexed images have "
                                             "a colormap."));

  pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);

  attrs = pango_attr_list_new ();

  attr = pango_attr_style_new (PANGO_STYLE_ITALIC);
  attr->start_index = 0;
  attr->end_index   = -1;
  pango_attr_list_insert (attrs, attr);

  pango_layout_set_attributes (layout, attrs);
  pango_attr_list_unref (attrs);

  return layout;
}

static gboolean
picman_colormap_preview_expose (GtkWidget          *widget,
                              GdkEventExpose     *event,
                              PicmanColormapEditor *editor)
{
  PicmanImageEditor *image_editor = PICMAN_IMAGE_EDITOR (editor);
  GtkStyle        *style;
  cairo_t         *cr;
  GtkAllocation    allocation;
  gint             width, height;
  gint             y;

  if (image_editor->image == NULL ||
      picman_image_get_base_type (image_editor->image) == PICMAN_INDEXED)
    return FALSE;

  cr = gdk_cairo_create (event->window);
  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);

  style = gtk_widget_get_style (widget);
  gdk_cairo_set_source_color (cr, &style->fg[gtk_widget_get_state (widget)]);

  gtk_widget_get_allocation (widget, &allocation);

  if (! gtk_widget_get_has_window (widget))
    cairo_translate (cr, allocation.x, allocation.y);

  if (! editor->layout)
    editor->layout = picman_colormap_editor_create_layout (editor->view);

  pango_layout_set_width (editor->layout,
                          PANGO_SCALE * (allocation.width - 2 * BORDER));

  pango_layout_get_pixel_size (editor->layout, &width, &height);

  y = (allocation.height - height) / 2;

  cairo_move_to (cr, BORDER, MAX (y, 0));
  pango_cairo_show_layout (cr, editor->layout);

  cairo_destroy (cr);

  return TRUE;
}

static void
picman_colormap_editor_update_entries (PicmanColormapEditor *editor)
{
  PicmanImage *image = PICMAN_IMAGE_EDITOR (editor)->image;

  if (! HAVE_COLORMAP (image) ||
      ! picman_image_get_colormap_size (image))
    {
      gtk_widget_set_sensitive (editor->index_spinbutton, FALSE);
      gtk_widget_set_sensitive (editor->color_entry, FALSE);

      gtk_adjustment_set_value (editor->index_adjustment, 0);
      gtk_entry_set_text (GTK_ENTRY (editor->color_entry), "");
    }
  else
    {
      const guchar *colormap = picman_image_get_colormap (image);
      const guchar *col;
      gchar        *string;

      gtk_adjustment_set_value (editor->index_adjustment, editor->col_index);

      col = colormap + editor->col_index * 3;

      string = g_strdup_printf ("%02x%02x%02x", col[0], col[1], col[2]);
      gtk_entry_set_text (GTK_ENTRY (editor->color_entry), string);
      g_free (string);

      gtk_widget_set_sensitive (editor->index_spinbutton, TRUE);
      gtk_widget_set_sensitive (editor->color_entry, TRUE);
    }
}

static void
picman_colormap_editor_entry_clicked (PicmanPaletteView    *view,
                                    PicmanPaletteEntry   *entry,
                                    GdkModifierType     state,
                                    PicmanColormapEditor *editor)
{
  PicmanImageEditor *image_editor = PICMAN_IMAGE_EDITOR (editor);

  picman_colormap_editor_set_index (editor, entry->position, NULL);

  if (state & picman_get_toggle_behavior_mask ())
    picman_context_set_background (image_editor->context, &entry->color);
  else
    picman_context_set_foreground (image_editor->context, &entry->color);
}

static void
picman_colormap_editor_entry_selected (PicmanPaletteView    *view,
                                     PicmanPaletteEntry   *entry,
                                     PicmanColormapEditor *editor)
{
  gint index = entry ? entry->position : 0;

  picman_colormap_editor_set_index (editor, index, NULL);
}

static void
picman_colormap_editor_entry_activated (PicmanPaletteView    *view,
                                      PicmanPaletteEntry   *entry,
                                      PicmanColormapEditor *editor)
{
  picman_colormap_editor_set_index (editor, entry->position, NULL);

  picman_ui_manager_activate_action (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)),
                                   "colormap",
                                   "colormap-edit-color");
}

static void
picman_colormap_editor_entry_context (PicmanPaletteView    *view,
                                    PicmanPaletteEntry   *entry,
                                    PicmanColormapEditor *editor)
{
  picman_colormap_editor_set_index (editor, entry->position, NULL);

  picman_editor_popup_menu (PICMAN_EDITOR (editor), NULL, NULL);
}

static void
picman_colormap_editor_color_dropped (PicmanPaletteView    *view,
                                    PicmanPaletteEntry   *entry,
                                    const PicmanRGB      *color,
                                    PicmanColormapEditor *editor)
{
}

static void
picman_colormap_adjustment_changed (GtkAdjustment      *adjustment,
                                  PicmanColormapEditor *editor)
{
  PicmanImage *image = PICMAN_IMAGE_EDITOR (editor)->image;

  if (HAVE_COLORMAP (image))
    {
      gint index = ROUND (gtk_adjustment_get_value (adjustment));

      picman_colormap_editor_set_index (editor, index, NULL);

      picman_colormap_editor_update_entries (editor);
    }
}

static void
picman_colormap_hex_entry_changed (PicmanColorHexEntry  *entry,
                                 PicmanColormapEditor *editor)
{
  PicmanImage *image = PICMAN_IMAGE_EDITOR (editor)->image;

  if (image)
    {
      PicmanRGB color;

      picman_color_hex_entry_get_color (entry, &color);

      picman_image_set_colormap_entry (image, editor->col_index, &color, TRUE);
      picman_image_flush (image);
    }
}

static void
picman_colormap_image_mode_changed (PicmanImage          *image,
                                  PicmanColormapEditor *editor)
{
  if (editor->color_dialog)
    gtk_widget_hide (editor->color_dialog);

  picman_colormap_image_colormap_changed (image, -1, editor);
}

static void
picman_colormap_image_colormap_changed (PicmanImage          *image,
                                      gint                ncol,
                                      PicmanColormapEditor *editor)
{
  if (HAVE_COLORMAP (image))
    {
      picman_view_set_viewable (PICMAN_VIEW (editor->view),
                              PICMAN_VIEWABLE (picman_image_get_colormap_palette (image)));

      gtk_adjustment_set_upper (editor->index_adjustment,
                                picman_image_get_colormap_size (image) - 1);
    }
  else
    {
      picman_view_set_viewable (PICMAN_VIEW (editor->view), NULL);
    }

  if (ncol == editor->col_index || ncol == -1)
    picman_colormap_editor_update_entries (editor);
}
