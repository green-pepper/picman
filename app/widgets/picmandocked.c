/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandocked.c
 * Copyright (C) 2003  Michael Natterer <mitch@picman.org>
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

#include "widgets-types.h"

#include "core/picmancontext.h"
#include "core/picmanmarshal.h"

#include "picmandocked.h"
#include "picmansessioninfo-aux.h"


enum
{
  TITLE_CHANGED,
  LAST_SIGNAL
};


static void    picman_docked_iface_base_init    (PicmanDockedInterface *docked_iface);

static void    picman_docked_iface_set_aux_info (PicmanDocked          *docked,
                                               GList               *aux_info);
static GList * picman_docked_iface_get_aux_info (PicmanDocked          *docked);



static guint docked_signals[LAST_SIGNAL] = { 0 };


GType
picman_docked_interface_get_type (void)
{
  static GType docked_iface_type = 0;

  if (!docked_iface_type)
    {
      const GTypeInfo docked_iface_info =
      {
        sizeof (PicmanDockedInterface),
        (GBaseInitFunc)     picman_docked_iface_base_init,
        (GBaseFinalizeFunc) NULL,
      };

      docked_iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                                  "PicmanDockedInterface",
                                                  &docked_iface_info,
                                                  0);

      g_type_interface_add_prerequisite (docked_iface_type, GTK_TYPE_WIDGET);
    }

  return docked_iface_type;
}

static void
picman_docked_iface_base_init (PicmanDockedInterface *docked_iface)
{
  static gboolean initialized = FALSE;

  if (! docked_iface->get_aux_info)
    {
      docked_iface->get_aux_info = picman_docked_iface_get_aux_info;
      docked_iface->set_aux_info = picman_docked_iface_set_aux_info;
    }

  if (! initialized)
    {
      docked_signals[TITLE_CHANGED] =
        g_signal_new ("title-changed",
                      PICMAN_TYPE_DOCKED,
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (PicmanDockedInterface, title_changed),
                      NULL, NULL,
                      picman_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

      initialized = TRUE;
    }
}

#define AUX_INFO_SHOW_BUTTON_BAR "show-button-bar"

static void
picman_docked_iface_set_aux_info (PicmanDocked *docked,
                                GList      *aux_info)
{
  GList *list;

  for (list = aux_info; list; list = g_list_next (list))
    {
      PicmanSessionInfoAux *aux = list->data;

      if (strcmp (aux->name, AUX_INFO_SHOW_BUTTON_BAR) == 0)
        {
          gboolean show = g_ascii_strcasecmp (aux->value, "false");

          picman_docked_set_show_button_bar (docked, show);
        }
    }
}

static GList *
picman_docked_iface_get_aux_info (PicmanDocked *docked)
{
  if (picman_docked_has_button_bar (docked))
    {
      gboolean show = picman_docked_get_show_button_bar (docked);

      return g_list_append (NULL,
                            picman_session_info_aux_new (AUX_INFO_SHOW_BUTTON_BAR,
                                                       show ? "true" : "false"));
    }

  return NULL;
}

void
picman_docked_title_changed (PicmanDocked *docked)
{
  g_return_if_fail (PICMAN_IS_DOCKED (docked));

  g_signal_emit (docked, docked_signals[TITLE_CHANGED], 0);
}

void
picman_docked_set_aux_info (PicmanDocked *docked,
                          GList      *aux_info)
{
  PicmanDockedInterface *docked_iface;

  g_return_if_fail (PICMAN_IS_DOCKED (docked));

  docked_iface = PICMAN_DOCKED_GET_INTERFACE (docked);

  if (docked_iface->set_aux_info)
    docked_iface->set_aux_info (docked, aux_info);
}

GList *
picman_docked_get_aux_info (PicmanDocked *docked)
{
  PicmanDockedInterface *docked_iface;

  g_return_val_if_fail (PICMAN_IS_DOCKED (docked), NULL);

  docked_iface = PICMAN_DOCKED_GET_INTERFACE (docked);

  if (docked_iface->get_aux_info)
    return docked_iface->get_aux_info (docked);

  return NULL;
}

GtkWidget *
picman_docked_get_preview (PicmanDocked  *docked,
                         PicmanContext *context,
                         GtkIconSize  size)
{
  PicmanDockedInterface *docked_iface;

  g_return_val_if_fail (PICMAN_IS_DOCKED (docked), NULL);

  docked_iface = PICMAN_DOCKED_GET_INTERFACE (docked);

  if (docked_iface->get_preview)
    return docked_iface->get_preview (docked, context, size);

  return NULL;
}

gboolean
picman_docked_get_prefer_icon (PicmanDocked *docked)
{
  PicmanDockedInterface *docked_iface;

  g_return_val_if_fail (PICMAN_IS_DOCKED (docked), FALSE);

  docked_iface = PICMAN_DOCKED_GET_INTERFACE (docked);

  if (docked_iface->get_prefer_icon)
    return docked_iface->get_prefer_icon (docked);

  return FALSE;
}

PicmanUIManager *
picman_docked_get_menu (PicmanDocked     *docked,
                      const gchar   **ui_path,
                      gpointer       *popup_data)
{
  PicmanDockedInterface *docked_iface;

  g_return_val_if_fail (PICMAN_IS_DOCKED (docked), NULL);
  g_return_val_if_fail (ui_path != NULL, NULL);
  g_return_val_if_fail (popup_data != NULL, NULL);

  docked_iface = PICMAN_DOCKED_GET_INTERFACE (docked);

  if (docked_iface->get_menu)
    return docked_iface->get_menu (docked, ui_path, popup_data);

  return NULL;
}

gchar *
picman_docked_get_title (PicmanDocked *docked)
{
  PicmanDockedInterface *docked_iface;

  g_return_val_if_fail (PICMAN_IS_DOCKED (docked), NULL);

  docked_iface = PICMAN_DOCKED_GET_INTERFACE (docked);

  if (docked_iface->get_title)
    return docked_iface->get_title (docked);

  return NULL;
}

void
picman_docked_set_context (PicmanDocked  *docked,
                         PicmanContext *context)
{
  PicmanDockedInterface *docked_iface;

  g_return_if_fail (PICMAN_IS_DOCKED (docked));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  docked_iface = PICMAN_DOCKED_GET_INTERFACE (docked);

  if (docked_iface->set_context)
    docked_iface->set_context (docked, context);
}

gboolean
picman_docked_has_button_bar (PicmanDocked *docked)
{
  PicmanDockedInterface *docked_iface;

  g_return_val_if_fail (PICMAN_IS_DOCKED (docked), FALSE);

  docked_iface = PICMAN_DOCKED_GET_INTERFACE (docked);

  if (docked_iface->has_button_bar)
    return docked_iface->has_button_bar (docked);

  return FALSE;
}

void
picman_docked_set_show_button_bar (PicmanDocked *docked,
                                 gboolean    show)
{
  PicmanDockedInterface *docked_iface;

  g_return_if_fail (PICMAN_IS_DOCKED (docked));

  docked_iface = PICMAN_DOCKED_GET_INTERFACE (docked);

  if (docked_iface->set_show_button_bar)
    docked_iface->set_show_button_bar (docked, show ? TRUE : FALSE);
}

gboolean
picman_docked_get_show_button_bar (PicmanDocked *docked)
{
  PicmanDockedInterface *docked_iface;

  g_return_val_if_fail (PICMAN_IS_DOCKED (docked), FALSE);

  docked_iface = PICMAN_DOCKED_GET_INTERFACE (docked);

  if (docked_iface->get_show_button_bar)
    return docked_iface->get_show_button_bar (docked);

  return FALSE;
}
