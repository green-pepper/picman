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
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"

#include "picmanaction.h"
#include "picmancolordialog.h"
#include "picmancolorpanel.h"


/*  local function prototypes  */

static void       picman_color_panel_dispose         (GObject            *object);

static gboolean   picman_color_panel_button_press    (GtkWidget          *widget,
                                                    GdkEventButton     *bevent);

static void       picman_color_panel_clicked         (GtkButton          *button);

static void       picman_color_panel_color_changed   (PicmanColorButton    *button);
static GType      picman_color_panel_get_action_type (PicmanColorButton    *button);

static void       picman_color_panel_dialog_update   (PicmanColorDialog    *dialog,
                                                    const PicmanRGB      *color,
                                                    PicmanColorDialogState state,
                                                    PicmanColorPanel     *panel);


G_DEFINE_TYPE (PicmanColorPanel, picman_color_panel, PICMAN_TYPE_COLOR_BUTTON)

#define parent_class picman_color_panel_parent_class


static void
picman_color_panel_class_init (PicmanColorPanelClass *klass)
{
  GObjectClass         *object_class       = G_OBJECT_CLASS (klass);
  GtkWidgetClass       *widget_class       = GTK_WIDGET_CLASS (klass);
  GtkButtonClass       *button_class       = GTK_BUTTON_CLASS (klass);
  PicmanColorButtonClass *color_button_class = PICMAN_COLOR_BUTTON_CLASS (klass);

  object_class->dispose               = picman_color_panel_dispose;

  widget_class->button_press_event    = picman_color_panel_button_press;

  button_class->clicked               = picman_color_panel_clicked;

  color_button_class->color_changed   = picman_color_panel_color_changed;
  color_button_class->get_action_type = picman_color_panel_get_action_type;
}

static void
picman_color_panel_init (PicmanColorPanel *panel)
{
  panel->context      = NULL;
  panel->color_dialog = NULL;
}

static void
picman_color_panel_dispose (GObject *object)
{
  PicmanColorPanel *panel = PICMAN_COLOR_PANEL (object);

  if (panel->color_dialog)
    {
      gtk_widget_destroy (panel->color_dialog);
      panel->color_dialog = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static gboolean
picman_color_panel_button_press (GtkWidget      *widget,
                               GdkEventButton *bevent)
{
  if (gdk_event_triggers_context_menu ((GdkEvent *) bevent))
    {
      PicmanColorButton *color_button;
      PicmanColorPanel  *color_panel;
      GtkUIManager    *ui_manager;
      GtkActionGroup  *group;
      GtkAction       *action;
      PicmanRGB          color;

      color_button = PICMAN_COLOR_BUTTON (widget);
      color_panel  = PICMAN_COLOR_PANEL (widget);
      ui_manager   = GTK_UI_MANAGER (color_button->popup_menu);

      group = gtk_ui_manager_get_action_groups (ui_manager)->data;

      action = gtk_action_group_get_action (group,
                                            "color-button-use-foreground");
      gtk_action_set_visible (action, color_panel->context != NULL);

      action = gtk_action_group_get_action (group,
                                            "color-button-use-background");
      gtk_action_set_visible (action, color_panel->context != NULL);

      if (color_panel->context)
        {
          action = gtk_action_group_get_action (group,
                                                "color-button-use-foreground");
          picman_context_get_foreground (color_panel->context, &color);
          g_object_set (action, "color", &color, NULL);

          action = gtk_action_group_get_action (group,
                                                "color-button-use-background");
          picman_context_get_background (color_panel->context, &color);
          g_object_set (action, "color", &color, NULL);
        }

      action = gtk_action_group_get_action (group, "color-button-use-black");
      picman_rgba_set (&color, 0.0, 0.0, 0.0, PICMAN_OPACITY_OPAQUE);
      g_object_set (action, "color", &color, NULL);

      action = gtk_action_group_get_action (group, "color-button-use-white");
      picman_rgba_set (&color, 1.0, 1.0, 1.0, PICMAN_OPACITY_OPAQUE);
      g_object_set (action, "color", &color, NULL);
    }

  if (GTK_WIDGET_CLASS (parent_class)->button_press_event)
    return GTK_WIDGET_CLASS (parent_class)->button_press_event (widget, bevent);

  return FALSE;
}

static void
picman_color_panel_clicked (GtkButton *button)
{
  PicmanColorPanel *panel = PICMAN_COLOR_PANEL (button);
  PicmanRGB         color;

  picman_color_button_get_color (PICMAN_COLOR_BUTTON (button), &color);

  if (! panel->color_dialog)
    {
      panel->color_dialog =
        picman_color_dialog_new (NULL, panel->context,
                               PICMAN_COLOR_BUTTON (button)->title,
                               NULL, NULL,
                               GTK_WIDGET (button),
                               NULL, NULL,
                               &color,
                               picman_color_button_get_update (PICMAN_COLOR_BUTTON (button)),
                               picman_color_button_has_alpha (PICMAN_COLOR_BUTTON (button)));

      g_signal_connect (panel->color_dialog, "destroy",
                        G_CALLBACK (gtk_widget_destroyed),
                        &panel->color_dialog);

      g_signal_connect (panel->color_dialog, "update",
                        G_CALLBACK (picman_color_panel_dialog_update),
                        panel);
    }

  gtk_window_present (GTK_WINDOW (panel->color_dialog));
}

static GType
picman_color_panel_get_action_type (PicmanColorButton *button)
{
  return PICMAN_TYPE_ACTION;
}


/*  public functions  */

GtkWidget *
picman_color_panel_new (const gchar       *title,
                      const PicmanRGB     *color,
                      PicmanColorAreaType  type,
                      gint               width,
                      gint               height)
{
  g_return_val_if_fail (title != NULL, NULL);
  g_return_val_if_fail (color != NULL, NULL);
  g_return_val_if_fail (width > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);

  return g_object_new (PICMAN_TYPE_COLOR_PANEL,
                       "title",       title,
                       "type",        type,
                       "color",       color,
                       "area-width",  width,
                       "area-height", height,
                       NULL);
}

static void
picman_color_panel_color_changed (PicmanColorButton *button)
{
  PicmanColorPanel *panel = PICMAN_COLOR_PANEL (button);
  PicmanRGB         color;

  if (panel->color_dialog)
    {
      PicmanRGB dialog_color;

      picman_color_button_get_color (PICMAN_COLOR_BUTTON (button), &color);
      picman_color_dialog_get_color (PICMAN_COLOR_DIALOG (panel->color_dialog),
                                   &dialog_color);

      if (picman_rgba_distance (&color, &dialog_color) > 0.00001 ||
          color.a != dialog_color.a)
        {
          picman_color_dialog_set_color (PICMAN_COLOR_DIALOG (panel->color_dialog),
                                       &color);
        }
    }
}

void
picman_color_panel_set_context (PicmanColorPanel *panel,
                              PicmanContext    *context)
{
  g_return_if_fail (PICMAN_IS_COLOR_PANEL (panel));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  panel->context = context;
}


/*  private functions  */

static void
picman_color_panel_dialog_update (PicmanColorDialog      *dialog,
                                const PicmanRGB        *color,
                                PicmanColorDialogState  state,
                                PicmanColorPanel       *panel)
{
  switch (state)
    {
    case PICMAN_COLOR_DIALOG_UPDATE:
      if (picman_color_button_get_update (PICMAN_COLOR_BUTTON (panel)))
        picman_color_button_set_color (PICMAN_COLOR_BUTTON (panel), color);
      break;

    case PICMAN_COLOR_DIALOG_OK:
      if (! picman_color_button_get_update (PICMAN_COLOR_BUTTON (panel)))
        picman_color_button_set_color (PICMAN_COLOR_BUTTON (panel), color);
      gtk_widget_hide (panel->color_dialog);
      break;

    case PICMAN_COLOR_DIALOG_CANCEL:
      if (picman_color_button_get_update (PICMAN_COLOR_BUTTON (panel)))
        picman_color_button_set_color (PICMAN_COLOR_BUTTON (panel), color);
      gtk_widget_hide (panel->color_dialog);
      break;
    }
}
