/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#include "config/picmanconfig-utils.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmandatafactory.h"

#include "picmancontainerentry.h"
#include "picmandialogfactory.h"
#include "picmanpropwidgets.h"
#include "picmanview.h"
#include "picmanviewablebutton.h"
#include "picmanviewablebox.h"
#include "picmanviewrenderergradient.h"
#include "picmanwindowstrategy.h"

#include "picman-intl.h"


/*  local function prototypes  */

static GtkWidget * picman_viewable_box_new       (PicmanContainer *container,
                                                PicmanContext   *context,
                                                const gchar   *label,
                                                gint           spacing,
                                                PicmanViewType   view_type,
                                                PicmanViewType   button_view_size,
                                                PicmanViewSize   view_size,
                                                const gchar   *dialog_identifier,
                                                const gchar   *dialog_stock_id,
                                                const gchar   *dialog_tooltip,
                                                const gchar   *editor_id);
static GtkWidget * view_props_connect          (GtkWidget     *box,
                                                PicmanContext   *context,
                                                const gchar   *view_type_prop,
                                                const gchar   *view_size_prop);
static void   picman_viewable_box_edit_clicked   (GtkWidget          *widget,
                                                PicmanViewableButton *button);
static void   picman_gradient_box_reverse_notify (GObject       *object,
                                                GParamSpec    *pspec,
                                                PicmanView      *view);


/*  brush boxes  */

static GtkWidget *
brush_box_new (PicmanContainer *container,
               PicmanContext   *context,
               const gchar   *label,
               gint           spacing,
               PicmanViewType   view_type,
               PicmanViewSize   view_size,
               const gchar   *editor_id)
{
  if (! container)
    container = picman_data_factory_get_container (context->picman->brush_factory);

  return picman_viewable_box_new (container, context, label, spacing,
                                view_type, PICMAN_VIEW_SIZE_SMALL, view_size,
                                "picman-brush-grid|picman-brush-list",
                                PICMAN_STOCK_BRUSH,
                                _("Open the brush selection dialog"),
                                editor_id);
}

GtkWidget *
picman_brush_box_new (PicmanContainer *container,
                    PicmanContext   *context,
                    const gchar   *label,
                    gint           spacing)
{
  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return brush_box_new (container, context, label, spacing,
                        PICMAN_VIEW_TYPE_GRID, PICMAN_VIEW_SIZE_SMALL,
                        NULL);
}

GtkWidget *
picman_prop_brush_box_new (PicmanContainer *container,
                         PicmanContext   *context,
                         const gchar   *label,
                         gint           spacing,
                         const gchar   *view_type_prop,
                         const gchar   *view_size_prop,
                         const gchar   *editor_id)
{
  PicmanViewType view_type;
  PicmanViewSize view_size;

  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  g_object_get (context,
                view_type_prop, &view_type,
                view_size_prop, &view_size,
                NULL);

  return view_props_connect (brush_box_new (container, context, label, spacing,
                                            view_type, view_size,
                                            editor_id),
                             context,
                             view_type_prop, view_size_prop);
}

/*  dynamics boxes  */

static GtkWidget *
dynamics_box_new (PicmanContainer *container,
                  PicmanContext   *context,
                  const gchar   *label,
                  gint           spacing,
                  PicmanViewSize   view_size,
                  const gchar   *editor_id)
{
  if (! container)
    container = picman_data_factory_get_container (context->picman->dynamics_factory);

  return picman_viewable_box_new (container, context, label, spacing,
                                PICMAN_VIEW_TYPE_LIST, PICMAN_VIEW_SIZE_SMALL, view_size,
                                "picman-dynamics-list",
                                PICMAN_STOCK_DYNAMICS,
                                _("Open the dynamics selection dialog"),
                                editor_id);
}

GtkWidget *
picman_dynamics_box_new (PicmanContainer *container,
                       PicmanContext   *context,
                       const gchar   *label,
                       gint           spacing)
{
  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return dynamics_box_new (container, context, label, spacing,
                           PICMAN_VIEW_SIZE_SMALL,
                           NULL);
}

GtkWidget *
picman_prop_dynamics_box_new (PicmanContainer *container,
                            PicmanContext   *context,
                            const gchar   *label,
                            gint           spacing,
                            const gchar   *view_type_prop,
                            const gchar   *view_size_prop,
                            const gchar   *editor_id)
{
  PicmanViewType view_type;
  PicmanViewSize view_size;

  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  g_object_get (context,
                view_type_prop, &view_type,
                view_size_prop, &view_size,
                NULL);

  return view_props_connect (dynamics_box_new (container, context, label,
                                               spacing, view_size,
                                               editor_id),
                             context,
                             view_type_prop, view_size_prop);
}


/*  pattern boxes  */

static GtkWidget *
pattern_box_new (PicmanContainer *container,
                 PicmanContext   *context,
                 const gchar   *label,
                 gint           spacing,
                 PicmanViewType   view_type,
                 PicmanViewSize   view_size)
{
  if (! container)
    container = picman_data_factory_get_container (context->picman->pattern_factory);

  return picman_viewable_box_new (container, context, label, spacing,
                                view_type, PICMAN_VIEW_SIZE_SMALL, view_size,
                                "picman-pattern-grid|picman-pattern-list",
                                PICMAN_STOCK_PATTERN,
                                _("Open the pattern selection dialog"),
                                NULL);
}

GtkWidget *
picman_pattern_box_new (PicmanContainer *container,
                      PicmanContext   *context,
                      const gchar   *label,
                      gint           spacing)
{
  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return pattern_box_new (container, context, label, spacing,
                          PICMAN_VIEW_TYPE_GRID, PICMAN_VIEW_SIZE_SMALL);
}

GtkWidget *
picman_prop_pattern_box_new (PicmanContainer *container,
                           PicmanContext   *context,
                           const gchar   *label,
                           gint           spacing,
                           const gchar   *view_type_prop,
                           const gchar   *view_size_prop)
{
  PicmanViewType view_type;
  PicmanViewSize view_size;

  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  g_object_get (context,
                view_type_prop, &view_type,
                view_size_prop, &view_size,
                NULL);

  return view_props_connect (pattern_box_new (container, context, label, spacing,
                                              view_type, view_size),
                             context,
                             view_type_prop, view_size_prop);
}


/*  gradient boxes  */

static GtkWidget *
gradient_box_new (PicmanContainer *container,
                  PicmanContext   *context,
                  const gchar   *label,
                  gint           spacing,
                  PicmanViewType   view_type,
                  PicmanViewSize   view_size,
                  const gchar   *reverse_prop,
                  const gchar   *editor_id)
{
  GtkWidget *hbox;
  GtkWidget *button;
  GList     *children;

  if (! container)
    container = picman_data_factory_get_container (context->picman->gradient_factory);

  hbox = picman_viewable_box_new (container, context, label, spacing,
                                view_type, PICMAN_VIEW_SIZE_LARGE, view_size,
                                "picman-gradient-list|picman-gradient-grid",
                                PICMAN_STOCK_GRADIENT,
                                _("Open the gradient selection dialog"),
                                editor_id);

  children = gtk_container_get_children (GTK_CONTAINER (hbox));
  button = children->data;
  g_list_free (children);

  PICMAN_VIEWABLE_BUTTON (button)->button_view_size = PICMAN_VIEW_SIZE_SMALL;

  if (reverse_prop)
    {
      GtkWidget *vbox;
      GtkWidget *toggle;
      GtkWidget *view;
      GtkWidget *image;
      gchar     *signal_name;

      vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
      gtk_widget_show (vbox);

      toggle = picman_prop_check_button_new (G_OBJECT (context), reverse_prop,
                                           NULL);
      gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (toggle), FALSE);
      gtk_box_pack_end (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
      gtk_widget_show (toggle);

      picman_help_set_help_data (toggle, _("Reverse"), NULL);

      image = gtk_image_new_from_stock (PICMAN_STOCK_FLIP_HORIZONTAL,
                                        GTK_ICON_SIZE_MENU);
      gtk_misc_set_alignment (GTK_MISC (image), 0.5, 1.0);
      gtk_container_add (GTK_CONTAINER (toggle), image);
      gtk_widget_show (image);

      view = gtk_bin_get_child (GTK_BIN (button));

      signal_name = g_strconcat ("notify::", reverse_prop, NULL);
      g_signal_connect_object (context, signal_name,
                               G_CALLBACK (picman_gradient_box_reverse_notify),
                               G_OBJECT (view), 0);
      g_free (signal_name);

      picman_gradient_box_reverse_notify (G_OBJECT (context),
                                        NULL,
                                        PICMAN_VIEW (view));
    }

  return hbox;
}

GtkWidget *
picman_gradient_box_new (PicmanContainer *container,
                       PicmanContext   *context,
                       const gchar   *label,
                       gint           spacing,
                       const gchar   *reverse_prop)
{
  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return gradient_box_new (container, context, label, spacing,
                           PICMAN_VIEW_TYPE_LIST, PICMAN_VIEW_SIZE_LARGE,
                           reverse_prop,
                           NULL);
}

GtkWidget *
picman_prop_gradient_box_new (PicmanContainer *container,
                            PicmanContext   *context,
                            const gchar   *label,
                            gint           spacing,
                            const gchar   *view_type_prop,
                            const gchar   *view_size_prop,
                            const gchar   *reverse_prop,
                            const gchar   *editor_id)
{
  PicmanViewType view_type;
  PicmanViewSize view_size;

  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  g_object_get (context,
                view_type_prop, &view_type,
                view_size_prop, &view_size,
                NULL);

  return view_props_connect (gradient_box_new (container, context, label, spacing,
                                               view_type, view_size,
                                               reverse_prop,
                                               editor_id),
                             context,
                             view_type_prop, view_size_prop);
}


/*  palette boxes  */

static GtkWidget *
palette_box_new (PicmanContainer *container,
                 PicmanContext   *context,
                 const gchar   *label,
                 gint           spacing,
                 PicmanViewType   view_type,
                 PicmanViewSize   view_size,
                 const gchar   *editor_id)
{
  if (! container)
    container = picman_data_factory_get_container (context->picman->palette_factory);

  return picman_viewable_box_new (container, context, label, spacing,
                                view_type, PICMAN_VIEW_SIZE_MEDIUM, view_size,
                                "picman-palette-list|picman-palette-grid",
                                PICMAN_STOCK_PALETTE,
                                _("Open the palette selection dialog"),
                                editor_id);
}

GtkWidget *
picman_palette_box_new (PicmanContainer *container,
                      PicmanContext   *context,
                      const gchar   *label,
                      gint           spacing)
{
  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return palette_box_new (container, context, label, spacing,
                          PICMAN_VIEW_TYPE_LIST, PICMAN_VIEW_SIZE_MEDIUM,
                          NULL);
}

GtkWidget *
picman_prop_palette_box_new (PicmanContainer *container,
                           PicmanContext   *context,
                           const gchar   *label,
                           gint           spacing,
                           const gchar   *view_type_prop,
                           const gchar   *view_size_prop,
                           const gchar   *editor_id)
{
  PicmanViewType view_type;
  PicmanViewSize view_size;

  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  g_object_get (context,
                view_type_prop, &view_type,
                view_size_prop, &view_size,
                NULL);

  return view_props_connect (palette_box_new (container, context, label, spacing,
                                              view_type, view_size,
                                              editor_id),
                             context,
                             view_type_prop, view_size_prop);
}


/*  font boxes  */

static GtkWidget *
font_box_new (PicmanContainer *container,
              PicmanContext   *context,
              const gchar   *label,
              gint           spacing,
              PicmanViewType   view_type,
              PicmanViewSize   view_size)
{
  if (! container)
    container = context->picman->fonts;

  return picman_viewable_box_new (container, context, label, spacing,
                                view_type, PICMAN_VIEW_SIZE_SMALL, view_size,
                                "picman-font-list|picman-font-grid",
                                PICMAN_STOCK_FONT,
                                _("Open the font selection dialog"),
                                NULL);
}

GtkWidget *
picman_font_box_new (PicmanContainer *container,
                   PicmanContext   *context,
                   const gchar   *label,
                   gint           spacing)
{
  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return font_box_new (container, context, label, spacing,
                       PICMAN_VIEW_TYPE_LIST, PICMAN_VIEW_SIZE_SMALL);
}

GtkWidget *
picman_prop_font_box_new (PicmanContainer *container,
                        PicmanContext   *context,
                        const gchar   *label,
                        gint           spacing,
                        const gchar   *view_type_prop,
                        const gchar   *view_size_prop)
{
  PicmanViewType view_type;
  PicmanViewSize view_size;

  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  g_object_get (context,
                view_type_prop, &view_type,
                view_size_prop, &view_size,
                NULL);

  return view_props_connect (font_box_new (container, context, label, spacing,
                                           view_type, view_size),
                             context,
                             view_type_prop, view_size_prop);
}


/*  private functions  */

static GtkWidget *
picman_viewable_box_new (PicmanContainer *container,
                       PicmanContext   *context,
                       const gchar   *label,
                       gint           spacing,
                       PicmanViewType   view_type,
                       PicmanViewType   button_view_size,
                       PicmanViewSize   view_size,
                       const gchar   *dialog_identifier,
                       const gchar   *dialog_stock_id,
                       const gchar   *dialog_tooltip,
                       const gchar   *editor_id)
{
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *vbox;
  GtkWidget *l;
  GtkWidget *entry;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, spacing);

  button = picman_viewable_button_new (container, context,
                                     view_type, button_view_size, view_size, 1,
                                     picman_dialog_factory_get_singleton (),
                                     dialog_identifier,
                                     dialog_stock_id,
                                     dialog_tooltip);

  g_object_set_data (G_OBJECT (hbox), "viewable-button", button);

  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  if (label)
    {
      l = gtk_label_new_with_mnemonic (label);
      gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
      gtk_box_pack_start (GTK_BOX (vbox), l, FALSE, FALSE, 0);
      gtk_widget_show (l);
    }

  entry = picman_container_entry_new (container, context, view_size, 1);

  /*  set a silly smally size request on the entry to disable
   *  GtkEntry's minimal width of 150 pixels.
   */
  gtk_entry_set_width_chars (GTK_ENTRY (entry), 4);
  gtk_box_pack_start (GTK_BOX (vbox), entry, label ? FALSE: TRUE, FALSE, 0);
  gtk_widget_show (entry);

  if (editor_id)
    {
      GtkWidget *edit_vbox;
      GtkWidget *edit_button;
      GtkWidget *image;

      edit_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      gtk_box_pack_end (GTK_BOX (hbox), edit_vbox, FALSE, FALSE, 0);
      gtk_widget_show (edit_vbox);

      edit_button = gtk_button_new ();
      gtk_button_set_relief (GTK_BUTTON (edit_button), GTK_RELIEF_NONE);
      gtk_box_pack_end (GTK_BOX (edit_vbox), edit_button, FALSE, FALSE, 0);
      gtk_widget_show (edit_button);

      image = gtk_image_new_from_stock (PICMAN_STOCK_EDIT,
                                        GTK_ICON_SIZE_BUTTON);
      gtk_misc_set_alignment (GTK_MISC (image), 0.5, 1.0);
      gtk_container_add (GTK_CONTAINER (edit_button), image);
      gtk_widget_show (image);

      g_object_set_data_full (G_OBJECT (button),
                              "picman-viewable-box-editor",
                              g_strdup (editor_id),
                              (GDestroyNotify) g_free);

      g_signal_connect (edit_button, "clicked",
                        G_CALLBACK (picman_viewable_box_edit_clicked),
                        button);
    }

  return hbox;
}

static GtkWidget *
view_props_connect (GtkWidget   *box,
                    PicmanContext *context,
                    const gchar *view_type_prop,
                    const gchar *view_size_prop)
{
  GtkWidget *button = g_object_get_data (G_OBJECT (box), "viewable-button");

  picman_config_connect_full (G_OBJECT (context), G_OBJECT (button),
                            view_type_prop, "popup-view-type");
  picman_config_connect_full (G_OBJECT (context), G_OBJECT (button),
                            view_size_prop, "popup-view-size");

  return box;
}

static void
picman_viewable_box_edit_clicked (GtkWidget          *widget,
                                PicmanViewableButton *button)
{
  const gchar *editor_id = g_object_get_data (G_OBJECT (button),
                                              "picman-viewable-box-editor");

  picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (button->context->picman)),
                                             button->context->picman,
                                             picman_dialog_factory_get_singleton (),
                                             gtk_widget_get_screen (widget),
                                             editor_id);
}

static void
picman_gradient_box_reverse_notify (GObject    *object,
                                  GParamSpec *pspec,
                                  PicmanView   *view)
{
  PicmanViewRendererGradient *rendergrad;
  gboolean                  reverse;

  rendergrad = PICMAN_VIEW_RENDERER_GRADIENT (view->renderer);

  g_object_get (object, "gradient-reverse", &reverse, NULL);

  picman_view_renderer_gradient_set_reverse (rendergrad, reverse);
}
