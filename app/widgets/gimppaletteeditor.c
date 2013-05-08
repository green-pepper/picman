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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmandatafactory.h"
#include "core/picmanpalette.h"

#include "picmandnd.h"
#include "picmandocked.h"
#include "picmanhelp-ids.h"
#include "picmanpaletteeditor.h"
#include "picmanpaletteview.h"
#include "picmansessioninfo-aux.h"
#include "picmanuimanager.h"
#include "picmanviewrendererpalette.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


#define ENTRY_WIDTH  12
#define ENTRY_HEIGHT 10
#define SPACING       1
#define COLUMNS      16
#define ROWS         11

#define PREVIEW_WIDTH  ((ENTRY_WIDTH  + SPACING) * COLUMNS + 1)
#define PREVIEW_HEIGHT ((ENTRY_HEIGHT + SPACING) * ROWS    + 1)


/*  local function prototypes  */

static void   picman_palette_editor_docked_iface_init (PicmanDockedInterface *face);

static void   picman_palette_editor_constructed      (GObject           *object);
static void   picman_palette_editor_dispose          (GObject           *object);

static void   picman_palette_editor_unmap            (GtkWidget         *widget);

static void   picman_palette_editor_set_data         (PicmanDataEditor    *editor,
                                                    PicmanData          *data);

static void   picman_palette_editor_set_context      (PicmanDocked        *docked,
                                                    PicmanContext       *context);
static void   picman_palette_editor_set_aux_info     (PicmanDocked        *docked,
                                                    GList             *aux_info);
static GList *picman_palette_editor_get_aux_info     (PicmanDocked        *docked);

static void   palette_editor_invalidate_preview    (PicmanPalette       *palette,
                                                    PicmanPaletteEditor *editor);

static void   palette_editor_viewport_size_allocate(GtkWidget         *widget,
                                                    GtkAllocation     *allocation,
                                                    PicmanPaletteEditor *editor);

static void   palette_editor_drop_palette          (GtkWidget         *widget,
                                                    gint               x,
                                                    gint               y,
                                                    PicmanViewable      *viewable,
                                                    gpointer           data);

static void   palette_editor_entry_clicked         (PicmanPaletteView   *view,
                                                    PicmanPaletteEntry  *entry,
                                                    GdkModifierType    state,
                                                    PicmanPaletteEditor *editor);
static void   palette_editor_entry_selected        (PicmanPaletteView   *view,
                                                    PicmanPaletteEntry  *entry,
                                                    PicmanPaletteEditor *editor);
static void   palette_editor_entry_activated       (PicmanPaletteView   *view,
                                                    PicmanPaletteEntry  *entry,
                                                    PicmanPaletteEditor *editor);
static void   palette_editor_entry_context         (PicmanPaletteView   *view,
                                                    PicmanPaletteEntry  *entry,
                                                    PicmanPaletteEditor *editor);
static void   palette_editor_color_dropped         (PicmanPaletteView   *view,
                                                    PicmanPaletteEntry  *entry,
                                                    const PicmanRGB     *color,
                                                    PicmanPaletteEditor *editor);

static void   palette_editor_color_name_changed    (GtkWidget         *widget,
                                                    PicmanPaletteEditor *editor);
static void   palette_editor_columns_changed       (GtkAdjustment     *adj,
                                                    PicmanPaletteEditor *editor);

static void   palette_editor_resize                (PicmanPaletteEditor *editor,
                                                    gint               width,
                                                    gdouble            zoom_factor);
static void   palette_editor_scroll_top_left       (PicmanPaletteEditor *editor);


G_DEFINE_TYPE_WITH_CODE (PicmanPaletteEditor, picman_palette_editor,
                         PICMAN_TYPE_DATA_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_palette_editor_docked_iface_init))

#define parent_class picman_palette_editor_parent_class

static PicmanDockedInterface *parent_docked_iface = NULL;


static void
picman_palette_editor_class_init (PicmanPaletteEditorClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass      *widget_class = GTK_WIDGET_CLASS (klass);
  PicmanDataEditorClass *editor_class = PICMAN_DATA_EDITOR_CLASS (klass);

  object_class->constructed = picman_palette_editor_constructed;
  object_class->dispose     = picman_palette_editor_dispose;

  widget_class->unmap       = picman_palette_editor_unmap;

  editor_class->set_data    = picman_palette_editor_set_data;
  editor_class->title       = _("Palette Editor");
}

static void
picman_palette_editor_docked_iface_init (PicmanDockedInterface *iface)
{
  parent_docked_iface = g_type_interface_peek_parent (iface);

  if (! parent_docked_iface)
    parent_docked_iface = g_type_default_interface_peek (PICMAN_TYPE_DOCKED);

  iface->set_context  = picman_palette_editor_set_context;
  iface->set_aux_info = picman_palette_editor_set_aux_info;
  iface->get_aux_info = picman_palette_editor_get_aux_info;
}

static void
picman_palette_editor_init (PicmanPaletteEditor *editor)
{
  PicmanDataEditor *data_editor = PICMAN_DATA_EDITOR (editor);
  GtkWidget      *hbox;
  GtkWidget      *label;
  GtkWidget      *spinbutton;
  GtkObject      *adj;

  editor->zoom_factor = 1.0;
  editor->col_width   = 0;
  editor->last_width  = 0;
  editor->columns     = COLUMNS;

  data_editor->view = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_size_request (data_editor->view, -1, PREVIEW_HEIGHT);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (data_editor->view),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (editor), data_editor->view, TRUE, TRUE, 0);
  gtk_widget_show (data_editor->view);

  editor->view = picman_view_new_full_by_types (NULL,
                                              PICMAN_TYPE_PALETTE_VIEW,
                                              PICMAN_TYPE_PALETTE,
                                              PREVIEW_WIDTH, PREVIEW_HEIGHT, 0,
                                              FALSE, TRUE, FALSE);
  picman_view_renderer_palette_set_cell_size
    (PICMAN_VIEW_RENDERER_PALETTE (PICMAN_VIEW (editor->view)->renderer), -1);
  picman_view_renderer_palette_set_draw_grid
    (PICMAN_VIEW_RENDERER_PALETTE (PICMAN_VIEW (editor->view)->renderer), TRUE);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (data_editor->view),
                                         editor->view);
  gtk_widget_show (editor->view);

  g_signal_connect (gtk_widget_get_parent (editor->view), "size-allocate",
                    G_CALLBACK (palette_editor_viewport_size_allocate),
                    editor);

  g_signal_connect (editor->view, "entry-clicked",
                    G_CALLBACK (palette_editor_entry_clicked),
                    editor);
  g_signal_connect (editor->view, "entry-selected",
                    G_CALLBACK (palette_editor_entry_selected),
                    editor);
  g_signal_connect (editor->view, "entry-activated",
                    G_CALLBACK (palette_editor_entry_activated),
                    editor);
  g_signal_connect (editor->view, "entry-context",
                    G_CALLBACK (palette_editor_entry_context),
                    editor);
  g_signal_connect (editor->view, "color-dropped",
                    G_CALLBACK (palette_editor_color_dropped),
                    editor);

  picman_dnd_viewable_dest_add (editor->view, PICMAN_TYPE_PALETTE,
                              palette_editor_drop_palette,
                              editor);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (editor), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /*  The color name entry  */
  editor->color_name = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), editor->color_name, TRUE, TRUE, 0);
  gtk_entry_set_text (GTK_ENTRY (editor->color_name), _("Undefined"));
  gtk_editable_set_editable (GTK_EDITABLE (editor->color_name), FALSE);
  gtk_widget_show (editor->color_name);

  g_signal_connect (editor->color_name, "changed",
                    G_CALLBACK (palette_editor_color_name_changed),
                    editor);

  label = gtk_label_new (_("Columns:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  spinbutton = picman_spin_button_new (&adj, 0, 0, 64, 1, 4, 0, 1, 0);
  editor->columns_data = GTK_ADJUSTMENT (adj);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);

  g_signal_connect (editor->columns_data, "value-changed",
                    G_CALLBACK (palette_editor_columns_changed),
                    editor);
}

static void
picman_palette_editor_constructed (GObject *object)
{
  PicmanPaletteEditor *editor = PICMAN_PALETTE_EDITOR (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_editor_add_action_button (PICMAN_EDITOR (editor), "palette-editor",
                                 "palette-editor-edit-color", NULL);

  picman_editor_add_action_button (PICMAN_EDITOR (editor), "palette-editor",
                                 "palette-editor-new-color-fg",
                                 "palette-editor-new-color-bg",
                                 picman_get_toggle_behavior_mask (),
                                 NULL);

  picman_editor_add_action_button (PICMAN_EDITOR (editor), "palette-editor",
                                 "palette-editor-delete-color", NULL);

  picman_editor_add_action_button (PICMAN_EDITOR (editor), "palette-editor",
                                 "palette-editor-zoom-out", NULL);

  picman_editor_add_action_button (PICMAN_EDITOR (editor), "palette-editor",
                                 "palette-editor-zoom-in", NULL);

  picman_editor_add_action_button (PICMAN_EDITOR (editor), "palette-editor",
                                 "palette-editor-zoom-all", NULL);
}

static void
picman_palette_editor_dispose (GObject *object)
{
  PicmanPaletteEditor *editor = PICMAN_PALETTE_EDITOR (object);

  if (editor->color_dialog)
    {
      gtk_widget_destroy (editor->color_dialog);
      editor->color_dialog = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_palette_editor_unmap (GtkWidget *widget)
{
  PicmanPaletteEditor *editor = PICMAN_PALETTE_EDITOR (widget);

  if (editor->color_dialog)
    gtk_widget_hide (editor->color_dialog);

  GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

static void
picman_palette_editor_set_data (PicmanDataEditor *editor,
                              PicmanData       *data)
{
  PicmanPaletteEditor *palette_editor = PICMAN_PALETTE_EDITOR (editor);

  g_signal_handlers_block_by_func (palette_editor->columns_data,
                                   palette_editor_columns_changed,
                                   editor);

  if (editor->data)
    {
      if (palette_editor->color_dialog)
        {
          gtk_widget_destroy (palette_editor->color_dialog);
          palette_editor->color_dialog = NULL;
        }

      g_signal_handlers_disconnect_by_func (editor->data,
                                            palette_editor_invalidate_preview,
                                            editor);

      gtk_adjustment_set_value (palette_editor->columns_data, 0);
    }

  PICMAN_DATA_EDITOR_CLASS (parent_class)->set_data (editor, data);

  picman_view_set_viewable (PICMAN_VIEW (palette_editor->view),
                          PICMAN_VIEWABLE (data));

  if (editor->data)
    {
      PicmanPalette *palette = PICMAN_PALETTE (editor->data);

      g_signal_connect (editor->data, "invalidate-preview",
                        G_CALLBACK (palette_editor_invalidate_preview),
                        editor);

      gtk_adjustment_set_value (palette_editor->columns_data,
                                picman_palette_get_columns (palette));

      palette_editor_scroll_top_left (palette_editor);

      palette_editor_invalidate_preview (PICMAN_PALETTE (editor->data),
                                         palette_editor);
    }

  g_signal_handlers_unblock_by_func (palette_editor->columns_data,
                                     palette_editor_columns_changed,
                                     editor);
}

static void
picman_palette_editor_set_context (PicmanDocked  *docked,
                                 PicmanContext *context)
{
  PicmanPaletteEditor *editor = PICMAN_PALETTE_EDITOR (docked);

  parent_docked_iface->set_context (docked, context);

  picman_view_renderer_set_context (PICMAN_VIEW (editor->view)->renderer,
                                  context);
}

#define AUX_INFO_ZOOM_FACTOR "zoom-factor"

static void
picman_palette_editor_set_aux_info (PicmanDocked *docked,
                                  GList      *aux_info)
{
  PicmanPaletteEditor *editor = PICMAN_PALETTE_EDITOR (docked);
  GList             *list;

  parent_docked_iface->set_aux_info (docked, aux_info);

  for (list = aux_info; list; list = g_list_next (list))
    {
      PicmanSessionInfoAux *aux = list->data;

      if (! strcmp (aux->name, AUX_INFO_ZOOM_FACTOR))
        {
          gdouble zoom_factor;

          zoom_factor = g_ascii_strtod (aux->value, NULL);

          editor->zoom_factor = CLAMP (zoom_factor, 0.1, 4.0);
        }
    }
}

static GList *
picman_palette_editor_get_aux_info (PicmanDocked *docked)
{
  PicmanPaletteEditor *editor = PICMAN_PALETTE_EDITOR (docked);
  GList             *aux_info;

  aux_info = parent_docked_iface->get_aux_info (docked);

  if (editor->zoom_factor != 1.0)
    {
      PicmanSessionInfoAux *aux;
      gchar               value[G_ASCII_DTOSTR_BUF_SIZE];

      g_ascii_formatd (value, sizeof (value), "%.2f", editor->zoom_factor);

      aux = picman_session_info_aux_new (AUX_INFO_ZOOM_FACTOR, value);
      aux_info = g_list_append (aux_info, aux);
    }

  return aux_info;
}


/*  public functions  */

GtkWidget *
picman_palette_editor_new (PicmanContext     *context,
                         PicmanMenuFactory *menu_factory)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return g_object_new (PICMAN_TYPE_PALETTE_EDITOR,
                       "menu-factory",    menu_factory,
                       "menu-identifier", "<PaletteEditor>",
                       "ui-path",         "/palette-editor-popup",
                       "data-factory",    context->picman->palette_factory,
                       "context",         context,
                       "data",            picman_context_get_palette (context),
                       NULL);
}

void
picman_palette_editor_pick_color (PicmanPaletteEditor  *editor,
                                const PicmanRGB      *color,
                                PicmanColorPickState  pick_state)
{
  g_return_if_fail (PICMAN_IS_PALETTE_EDITOR (editor));
  g_return_if_fail (color != NULL);

  if (PICMAN_DATA_EDITOR (editor)->data_editable)
    {
      PicmanPaletteEntry *entry;
      PicmanData         *data;
      gint              index = -1;

      data = picman_data_editor_get_data (PICMAN_DATA_EDITOR (editor));

      switch (pick_state)
        {
        case PICMAN_COLOR_PICK_STATE_NEW:
          if (editor->color)
            index = editor->color->position + 1;

          entry = picman_palette_add_entry (PICMAN_PALETTE (data), index,
                                          NULL, color);
          picman_palette_view_select_entry (PICMAN_PALETTE_VIEW (editor->view),
                                          entry);
          break;

        case PICMAN_COLOR_PICK_STATE_UPDATE:
          picman_palette_set_entry_color (PICMAN_PALETTE (data),
                                        editor->color->position,
                                        color);
          break;
        }
    }
}

void
picman_palette_editor_zoom (PicmanPaletteEditor  *editor,
                          PicmanZoomType        zoom_type)
{
  PicmanPalette *palette;
  gdouble      zoom_factor;

  g_return_if_fail (PICMAN_IS_PALETTE_EDITOR (editor));

  palette = PICMAN_PALETTE (PICMAN_DATA_EDITOR (editor)->data);

  if (! palette)
    return;

  zoom_factor = editor->zoom_factor;

  switch (zoom_type)
    {
    case PICMAN_ZOOM_IN_MAX:
    case PICMAN_ZOOM_IN_MORE:
    case PICMAN_ZOOM_IN:
      zoom_factor += 0.1;
      break;

    case PICMAN_ZOOM_OUT_MORE:
    case PICMAN_ZOOM_OUT:
      zoom_factor -= 0.1;
      break;

    case PICMAN_ZOOM_OUT_MAX:
    case PICMAN_ZOOM_TO: /* abused as ZOOM_ALL */
      {
        GtkWidget     *scrolled_win = PICMAN_DATA_EDITOR (editor)->view;
        GtkWidget     *viewport     = gtk_bin_get_child (GTK_BIN (scrolled_win));
        GtkAllocation  allocation;
        gint           columns;
        gint           rows;

        gtk_widget_get_allocation (viewport, &allocation);

        columns = picman_palette_get_columns (palette);
        if (columns == 0)
          columns = COLUMNS;

        rows = picman_palette_get_n_colors (palette) / columns;
        if (picman_palette_get_n_colors (palette) % columns)
          rows += 1;

        rows = MAX (1, rows);

        zoom_factor = (((gdouble) allocation.height - 2 * SPACING) /
                       (gdouble) rows - SPACING) / ENTRY_HEIGHT;
      }
      break;
    }

  zoom_factor = CLAMP (zoom_factor, 0.1, 4.0);

  editor->columns = picman_palette_get_columns (palette);
  if (editor->columns == 0)
    editor->columns = COLUMNS;

  palette_editor_resize (editor, editor->last_width, zoom_factor);

  palette_editor_scroll_top_left (editor);
}

gint
picman_palette_editor_get_index (PicmanPaletteEditor *editor,
                               const PicmanRGB     *search)
{
  PicmanPalette *palette;

  g_return_val_if_fail (PICMAN_IS_PALETTE_EDITOR (editor), -1);
  g_return_val_if_fail (search != NULL, -1);

  palette = PICMAN_PALETTE (PICMAN_DATA_EDITOR (editor)->data);

  if (palette && picman_palette_get_n_colors (palette) > 0)
    {
      PicmanPaletteEntry *entry;

      entry = picman_palette_find_entry (palette, search, editor->color);

      if (entry)
        return entry->position;
    }

  return -1;
}

gboolean
picman_palette_editor_set_index (PicmanPaletteEditor *editor,
                               gint               index,
                               PicmanRGB           *color)
{
  PicmanPalette *palette;

  g_return_val_if_fail (PICMAN_IS_PALETTE_EDITOR (editor), FALSE);

  palette = PICMAN_PALETTE (PICMAN_DATA_EDITOR (editor)->data);

  if (palette && picman_palette_get_n_colors (palette) > 0)
    {
      PicmanPaletteEntry *entry;

      index = CLAMP (index, 0, picman_palette_get_n_colors (palette) - 1);

      entry = picman_palette_get_entry (palette, index);

      picman_palette_view_select_entry (PICMAN_PALETTE_VIEW (editor->view),
                                      entry);

      if (color)
        *color = editor->color->color;

      return TRUE;
    }

  return FALSE;
}

gint
picman_palette_editor_max_index (PicmanPaletteEditor *editor)
{
  PicmanPalette *palette;

  g_return_val_if_fail (PICMAN_IS_PALETTE_EDITOR (editor), -1);

  palette = PICMAN_PALETTE (PICMAN_DATA_EDITOR (editor)->data);

  if (palette && picman_palette_get_n_colors (palette) > 0)
    {
      return picman_palette_get_n_colors (palette) - 1;
    }

  return -1;
}


/*  private functions  */

static void
palette_editor_invalidate_preview (PicmanPalette       *palette,
                                   PicmanPaletteEditor *editor)
{
  editor->columns = picman_palette_get_columns (palette);
  if (editor->columns == 0)
    editor->columns = COLUMNS;

  palette_editor_resize (editor, editor->last_width, editor->zoom_factor);
}

static void
palette_editor_viewport_size_allocate (GtkWidget         *widget,
                                       GtkAllocation     *allocation,
                                       PicmanPaletteEditor *editor)
{
  if (allocation->width != editor->last_width)
    {
      palette_editor_resize (editor, allocation->width,
                             editor->zoom_factor);
    }
}

static void
palette_editor_drop_palette (GtkWidget    *widget,
                             gint          x,
                             gint          y,
                             PicmanViewable *viewable,
                             gpointer      data)
{
  picman_data_editor_set_data (PICMAN_DATA_EDITOR (data), PICMAN_DATA (viewable));
}


/*  palette view callbacks  */

static void
palette_editor_entry_clicked (PicmanPaletteView   *view,
                              PicmanPaletteEntry  *entry,
                              GdkModifierType    state,
                              PicmanPaletteEditor *editor)
{
  if (entry)
    {
      PicmanDataEditor *data_editor = PICMAN_DATA_EDITOR (editor);

      if (state & picman_get_toggle_behavior_mask ())
        picman_context_set_background (data_editor->context, &entry->color);
      else
        picman_context_set_foreground (data_editor->context, &entry->color);
    }
}

static void
palette_editor_entry_selected (PicmanPaletteView   *view,
                               PicmanPaletteEntry  *entry,
                               PicmanPaletteEditor *editor)
{
  PicmanDataEditor *data_editor = PICMAN_DATA_EDITOR (editor);

  if (editor->color != entry)
    {
      editor->color = entry;

      g_signal_handlers_block_by_func (editor->color_name,
                                       palette_editor_color_name_changed,
                                       editor);

      gtk_entry_set_text (GTK_ENTRY (editor->color_name),
                          entry ? entry->name : _("Undefined"));

      g_signal_handlers_unblock_by_func (editor->color_name,
                                         palette_editor_color_name_changed,
                                         editor);

      gtk_editable_set_editable (GTK_EDITABLE (editor->color_name),
                                 entry && data_editor->data_editable);

      picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)),
                              picman_editor_get_popup_data (PICMAN_EDITOR (editor)));
    }
}

static void
palette_editor_entry_activated (PicmanPaletteView   *view,
                                PicmanPaletteEntry  *entry,
                                PicmanPaletteEditor *editor)
{
  if (PICMAN_DATA_EDITOR (editor)->data_editable && entry == editor->color)
    {
      picman_ui_manager_activate_action (picman_editor_get_ui_manager (PICMAN_EDITOR (editor)),
                                       "palette-editor",
                                       "palette-editor-edit-color");
    }
}

static void
palette_editor_entry_context (PicmanPaletteView   *view,
                              PicmanPaletteEntry  *entry,
                              PicmanPaletteEditor *editor)
{
  picman_editor_popup_menu (PICMAN_EDITOR (editor), NULL, NULL);
}

static void
palette_editor_color_dropped (PicmanPaletteView   *view,
                              PicmanPaletteEntry  *entry,
                              const PicmanRGB     *color,
                              PicmanPaletteEditor *editor)
{
  if (PICMAN_DATA_EDITOR (editor)->data_editable)
    {
      PicmanPalette *palette = PICMAN_PALETTE (PICMAN_DATA_EDITOR (editor)->data);
      gint         pos     = -1;

      if (entry)
        pos = entry->position;

      entry = picman_palette_add_entry (palette, pos, NULL, color);
      picman_palette_view_select_entry (PICMAN_PALETTE_VIEW (editor->view), entry);
    }
}


/*  color name and columns callbacks  */

static void
palette_editor_color_name_changed (GtkWidget         *widget,
                                   PicmanPaletteEditor *editor)
{
  if (PICMAN_DATA_EDITOR (editor)->data)
    {
      PicmanPalette *palette = PICMAN_PALETTE (PICMAN_DATA_EDITOR (editor)->data);
      const gchar *name;

      name = gtk_entry_get_text (GTK_ENTRY (editor->color_name));

      picman_palette_set_entry_name (palette, editor->color->position, name);
    }
}

static void
palette_editor_columns_changed (GtkAdjustment     *adj,
                                PicmanPaletteEditor *editor)
{
  if (PICMAN_DATA_EDITOR (editor)->data)
    {
      PicmanPalette *palette = PICMAN_PALETTE (PICMAN_DATA_EDITOR (editor)->data);

      picman_palette_set_columns (palette,
                                ROUND (gtk_adjustment_get_value (adj)));
    }
}


/*  misc utils  */

static void
palette_editor_resize (PicmanPaletteEditor *editor,
                       gint               width,
                       gdouble            zoom_factor)
{
  PicmanPalette *palette;
  gint         rows;
  gint         preview_width;
  gint         preview_height;

  palette = PICMAN_PALETTE (PICMAN_DATA_EDITOR (editor)->data);

  if (! palette)
    return;

  editor->zoom_factor = zoom_factor;
  editor->last_width  = width;
  editor->col_width   = width / (editor->columns + 1) - SPACING;

  if (editor->col_width < 0)
    editor->col_width = 0;

  rows = picman_palette_get_n_colors (palette) / editor->columns;
  if (picman_palette_get_n_colors (palette) % editor->columns)
    rows += 1;

  preview_width  = (editor->col_width + SPACING) * editor->columns;
  preview_height = (rows *
                    (SPACING + (gint) (ENTRY_HEIGHT * editor->zoom_factor)));

  if (preview_height > PICMAN_VIEWABLE_MAX_PREVIEW_SIZE)
    preview_height = ((PICMAN_VIEWABLE_MAX_PREVIEW_SIZE - SPACING) / rows) * rows;

  picman_view_renderer_set_size_full (PICMAN_VIEW (editor->view)->renderer,
                                    preview_width  + SPACING,
                                    preview_height + SPACING, 0);
}

static void
palette_editor_scroll_top_left (PicmanPaletteEditor *palette_editor)
{
  PicmanDataEditor *data_editor = PICMAN_DATA_EDITOR (palette_editor);
  GtkAdjustment  *hadj;
  GtkAdjustment  *vadj;

  if (! data_editor->view)
    return;

  hadj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (data_editor->view));
  vadj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (data_editor->view));

  if (hadj)
    gtk_adjustment_set_value (hadj, 0.0);
  if (vadj)
    gtk_adjustment_set_value (vadj, 0.0);
}
