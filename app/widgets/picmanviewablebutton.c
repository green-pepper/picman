/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewablebutton.c
 * Copyright (C) 2003-2005 Michael Natterer <mitch@picman.org>
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

#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanviewable.h"

#include "picmancontainerpopup.h"
#include "picmandialogfactory.h"
#include "picmanpropwidgets.h"
#include "picmanviewrenderer.h"
#include "picmanviewablebutton.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_POPUP_VIEW_TYPE,
  PROP_POPUP_VIEW_SIZE
};


static void     picman_viewable_button_finalize     (GObject            *object);
static void     picman_viewable_button_set_property (GObject            *object,
                                                   guint               property_id,
                                                   const GValue       *value,
                                                   GParamSpec         *pspec);
static void     picman_viewable_button_get_property (GObject            *object,
                                                   guint               property_id,
                                                   GValue             *value,
                                                   GParamSpec         *pspec);
static gboolean picman_viewable_button_scroll_event (GtkWidget          *widget,
                                                   GdkEventScroll     *sevent);
static void     picman_viewable_button_clicked      (GtkButton          *button);

static void     picman_viewable_button_popup_closed (PicmanContainerPopup *popup,
                                                   PicmanViewableButton *button);


G_DEFINE_TYPE (PicmanViewableButton, picman_viewable_button, PICMAN_TYPE_BUTTON)

#define parent_class picman_viewable_button_parent_class


static void
picman_viewable_button_class_init (PicmanViewableButtonClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkButtonClass *button_class = GTK_BUTTON_CLASS (klass);

  object_class->finalize     = picman_viewable_button_finalize;
  object_class->get_property = picman_viewable_button_get_property;
  object_class->set_property = picman_viewable_button_set_property;

  widget_class->scroll_event = picman_viewable_button_scroll_event;

  button_class->clicked      = picman_viewable_button_clicked;

  g_object_class_install_property (object_class, PROP_POPUP_VIEW_TYPE,
                                   g_param_spec_enum ("popup-view-type",
                                                      NULL, NULL,
                                                      PICMAN_TYPE_VIEW_TYPE,
                                                      PICMAN_VIEW_TYPE_LIST,
                                                      PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_POPUP_VIEW_SIZE,
                                   g_param_spec_int ("popup-view-size",
                                                     NULL, NULL,
                                                     PICMAN_VIEW_SIZE_TINY,
                                                     PICMAN_VIEW_SIZE_GIGANTIC,
                                                     PICMAN_VIEW_SIZE_SMALL,
                                                     PICMAN_PARAM_READWRITE));
}

static void
picman_viewable_button_init (PicmanViewableButton *button)
{
  button->popup_view_type   = PICMAN_VIEW_TYPE_LIST;
  button->popup_view_size   = PICMAN_VIEW_SIZE_SMALL;

  button->button_view_size  = PICMAN_VIEW_SIZE_SMALL;
  button->view_border_width = 1;
}

static void
picman_viewable_button_finalize (GObject *object)
{
  PicmanViewableButton *button = PICMAN_VIEWABLE_BUTTON (object);

  if (button->dialog_identifier)
    {
      g_free (button->dialog_identifier);
      button->dialog_identifier = NULL;
    }

  if (button->dialog_stock_id)
    {
      g_free (button->dialog_stock_id);
      button->dialog_stock_id = NULL;
    }

  if (button->dialog_tooltip)
    {
      g_free (button->dialog_tooltip);
      button->dialog_tooltip = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_viewable_button_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanViewableButton *button = PICMAN_VIEWABLE_BUTTON (object);

  switch (property_id)
    {
    case PROP_POPUP_VIEW_TYPE:
      picman_viewable_button_set_view_type (button, g_value_get_enum (value));
      break;
    case PROP_POPUP_VIEW_SIZE:
      picman_viewable_button_set_view_size (button, g_value_get_int (value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_viewable_button_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanViewableButton *button = PICMAN_VIEWABLE_BUTTON (object);

  switch (property_id)
    {
    case PROP_POPUP_VIEW_TYPE:
      g_value_set_enum (value, button->popup_view_type);
      break;
    case PROP_POPUP_VIEW_SIZE:
      g_value_set_int (value, button->popup_view_size);
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_viewable_button_scroll_event (GtkWidget      *widget,
                                   GdkEventScroll *sevent)
{
  PicmanViewableButton *button = PICMAN_VIEWABLE_BUTTON (widget);
  PicmanObject         *object;
  gint                index;

  object = picman_context_get_by_type (button->context,
                                     picman_container_get_children_type (button->container));

  index = picman_container_get_child_index (button->container, object);

  if (index != -1)
    {
      gint n_children;
      gint new_index = index;

      n_children = picman_container_get_n_children (button->container);

      if (sevent->direction == GDK_SCROLL_UP)
        {
          if (index > 0)
            new_index--;
          else
            new_index = n_children - 1;
        }
      else if (sevent->direction == GDK_SCROLL_DOWN)
        {
          if (index == (n_children - 1))
            new_index = 0;
          else
            new_index++;
        }

      if (new_index != index)
        {
          object = picman_container_get_child_by_index (button->container,
                                                      new_index);

          if (object)
            picman_context_set_by_type (button->context,
                                      picman_container_get_children_type (button->container),
                                      object);
        }
    }

  return TRUE;
}

static void
picman_viewable_button_clicked (GtkButton *button)
{
  PicmanViewableButton *viewable_button = PICMAN_VIEWABLE_BUTTON (button);
  GtkWidget          *popup;

  popup = picman_container_popup_new (viewable_button->container,
                                    viewable_button->context,
                                    viewable_button->popup_view_type,
                                    viewable_button->button_view_size,
                                    viewable_button->popup_view_size,
                                    viewable_button->view_border_width,
                                    viewable_button->dialog_factory,
                                    viewable_button->dialog_identifier,
                                    viewable_button->dialog_stock_id,
                                    viewable_button->dialog_tooltip);

  g_signal_connect (popup, "cancel",
                    G_CALLBACK (picman_viewable_button_popup_closed),
                    button);
  g_signal_connect (popup, "confirm",
                    G_CALLBACK (picman_viewable_button_popup_closed),
                    button);

  picman_container_popup_show (PICMAN_CONTAINER_POPUP (popup), GTK_WIDGET (button));
}

static void
picman_viewable_button_popup_closed (PicmanContainerPopup *popup,
                                   PicmanViewableButton *button)
{
  picman_viewable_button_set_view_type (button,
                                      picman_container_popup_get_view_type (popup));
  picman_viewable_button_set_view_size (button,
                                      picman_container_popup_get_view_size (popup));
}


/*  public functions  */

GtkWidget *
picman_viewable_button_new (PicmanContainer     *container,
                          PicmanContext       *context,
                          PicmanViewType       view_type,
                          gint               button_view_size,
                          gint               view_size,
                          gint               view_border_width,
                          PicmanDialogFactory *dialog_factory,
                          const gchar       *dialog_identifier,
                          const gchar       *dialog_stock_id,
                          const gchar       *dialog_tooltip)
{
  PicmanViewableButton *button;
  const gchar        *prop_name;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (view_size >  0 &&
                        view_size <= PICMAN_VIEWABLE_MAX_BUTTON_SIZE, NULL);
  g_return_val_if_fail (view_border_width >= 0 &&
                        view_border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH,
                        NULL);
  g_return_val_if_fail (dialog_factory == NULL ||
                        PICMAN_IS_DIALOG_FACTORY (dialog_factory), NULL);
  if (dialog_factory)
    {
      g_return_val_if_fail (dialog_identifier != NULL, NULL);
      g_return_val_if_fail (dialog_stock_id != NULL, NULL);
      g_return_val_if_fail (dialog_tooltip != NULL, NULL);
    }

  button = g_object_new (PICMAN_TYPE_VIEWABLE_BUTTON,
                         "popup-view-type", view_type,
                         "popup-view-size", view_size,
                         NULL);

  button->container = container;
  button->context   = context;

  button->button_view_size  = button_view_size;
  button->view_border_width = view_border_width;

  if (dialog_factory)
    {
      button->dialog_factory    = dialog_factory;
      button->dialog_identifier = g_strdup (dialog_identifier);
      button->dialog_stock_id   = g_strdup (dialog_stock_id);
      button->dialog_tooltip    = g_strdup (dialog_tooltip);
    }

  prop_name = picman_context_type_to_prop_name (picman_container_get_children_type (container));

  button->view = picman_prop_view_new (G_OBJECT (context), prop_name,
                                     context, button->button_view_size);
  gtk_container_add (GTK_CONTAINER (button), button->view);
  gtk_widget_show (button->view);

  return GTK_WIDGET (button);
}

PicmanViewType
picman_viewable_button_get_view_type (PicmanViewableButton *button)
{
  g_return_val_if_fail (PICMAN_IS_VIEWABLE_BUTTON (button), PICMAN_VIEW_TYPE_LIST);

  return button->popup_view_type;
}

void
picman_viewable_button_set_view_type (PicmanViewableButton *button,
                                    PicmanViewType        view_type)
{
  g_return_if_fail (PICMAN_IS_VIEWABLE_BUTTON (button));

  if (view_type != button->popup_view_type)
    {
      button->popup_view_type = view_type;

      g_object_notify (G_OBJECT (button), "popup-view-type");
    }
}

gint
picman_viewable_button_get_view_size (PicmanViewableButton *button)
{
  g_return_val_if_fail (PICMAN_IS_VIEWABLE_BUTTON (button), PICMAN_VIEW_SIZE_SMALL);

  return button->popup_view_size;
}

void
picman_viewable_button_set_view_size (PicmanViewableButton *button,
                                    gint                view_size)
{
  g_return_if_fail (PICMAN_IS_VIEWABLE_BUTTON (button));

  if (view_size != button->popup_view_size)
    {
      button->popup_view_size = view_size;

      g_object_notify (G_OBJECT (button), "popup-view-size");
    }
}
