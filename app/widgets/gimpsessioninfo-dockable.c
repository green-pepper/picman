/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansessioninfo-dockable.c
 * Copyright (C) 2001-2007 Michael Natterer <mitch@picman.org>
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

#include <gtk/gtk.h>

#include "libpicmanconfig/picmanconfig.h"

#include "widgets-types.h"

#include "picmancontainerview-utils.h"
#include "picmancontainerview.h"
#include "picmandialogfactory.h"
#include "picmandock.h"
#include "picmandockable.h"
#include "picmansessioninfo-aux.h"
#include "picmansessioninfo-dockable.h"
#include "picmansessionmanaged.h"
#include "picmantoolbox.h"


enum
{
  SESSION_INFO_DOCKABLE_LOCKED,
  SESSION_INFO_DOCKABLE_TAB_STYLE,
  SESSION_INFO_DOCKABLE_VIEW_SIZE,
  SESSION_INFO_DOCKABLE_AUX
};


/*  public functions  */

PicmanSessionInfoDockable *
picman_session_info_dockable_new (void)
{
  return g_slice_new0 (PicmanSessionInfoDockable);
}

void
picman_session_info_dockable_free (PicmanSessionInfoDockable *info)
{
  g_return_if_fail (info != NULL);

  if (info->identifier)
    {
      g_free (info->identifier);
      info->identifier = NULL;
    }

  if (info->aux_info)
    {
      g_list_free_full (info->aux_info,
                        (GDestroyNotify) picman_session_info_aux_free);
      info->aux_info = NULL;
    }

  g_slice_free (PicmanSessionInfoDockable, info);
}

void
picman_session_info_dockable_serialize (PicmanConfigWriter        *writer,
                                      PicmanSessionInfoDockable *info)
{
  GEnumClass  *enum_class;
  GEnumValue  *enum_value;
  const gchar *tab_style = "icon";

  g_return_if_fail (writer != NULL);
  g_return_if_fail (info != NULL);

  enum_class = g_type_class_ref (PICMAN_TYPE_TAB_STYLE);

  picman_config_writer_open (writer, "dockable");
  picman_config_writer_string (writer, info->identifier);

  if (info->locked)
    {
      picman_config_writer_open (writer, "locked");
      picman_config_writer_close (writer);
    }

  enum_value = g_enum_get_value (enum_class, info->tab_style);

  if (enum_value)
    tab_style = enum_value->value_nick;

  picman_config_writer_open (writer, "tab-style");
  picman_config_writer_print (writer, tab_style, -1);
  picman_config_writer_close (writer);

  if (info->view_size > 0)
    {
      picman_config_writer_open (writer, "preview-size");
      picman_config_writer_printf (writer, "%d", info->view_size);
      picman_config_writer_close (writer);
    }

  if (info->aux_info)
    picman_session_info_aux_serialize (writer, info->aux_info);

  picman_config_writer_close (writer);

  g_type_class_unref (enum_class);
}

GTokenType
picman_session_info_dockable_deserialize (GScanner                 *scanner,
                                        gint                      scope,
                                        PicmanSessionInfoDockable **dockable)
{
  PicmanSessionInfoDockable *info;
  GEnumClass              *enum_class;
  GEnumValue              *enum_value;
  GTokenType               token;

  g_return_val_if_fail (scanner != NULL, G_TOKEN_LEFT_PAREN);
  g_return_val_if_fail (dockable != NULL, G_TOKEN_LEFT_PAREN);

  g_scanner_scope_add_symbol (scanner, scope, "locked",
                              GINT_TO_POINTER (SESSION_INFO_DOCKABLE_LOCKED));
  g_scanner_scope_add_symbol (scanner, scope, "tab-style",
                              GINT_TO_POINTER (SESSION_INFO_DOCKABLE_TAB_STYLE));
  g_scanner_scope_add_symbol (scanner, scope, "preview-size",
                              GINT_TO_POINTER (SESSION_INFO_DOCKABLE_VIEW_SIZE));
  g_scanner_scope_add_symbol (scanner, scope, "aux-info",
                              GINT_TO_POINTER (SESSION_INFO_DOCKABLE_AUX));

  info = picman_session_info_dockable_new ();

  enum_class = g_type_class_ref (PICMAN_TYPE_TAB_STYLE);

  token = G_TOKEN_STRING;
  if (! picman_scanner_parse_string (scanner, &info->identifier))
    goto error;

  token = G_TOKEN_LEFT_PAREN;

  while (g_scanner_peek_next_token (scanner) == token)
    {
      token = g_scanner_get_next_token (scanner);

      switch (token)
        {
        case G_TOKEN_LEFT_PAREN:
          token = G_TOKEN_SYMBOL;
          break;

        case G_TOKEN_SYMBOL:
          switch (GPOINTER_TO_INT (scanner->value.v_symbol))
            {
            case SESSION_INFO_DOCKABLE_LOCKED:
              info->locked = TRUE;
              break;

            case SESSION_INFO_DOCKABLE_TAB_STYLE:
              token = G_TOKEN_IDENTIFIER;
              if (g_scanner_peek_next_token (scanner) != token)
                goto error;

              g_scanner_get_next_token (scanner);

              enum_value = g_enum_get_value_by_nick (enum_class,
                                                     scanner->value.v_identifier);

              if (! enum_value)
                enum_value = g_enum_get_value_by_name (enum_class,
                                                       scanner->value.v_identifier);

              if (enum_value)
                info->tab_style = enum_value->value;
              break;

            case SESSION_INFO_DOCKABLE_VIEW_SIZE:
              token = G_TOKEN_INT;
              if (! picman_scanner_parse_int (scanner, &info->view_size))
                goto error;
              break;

            case SESSION_INFO_DOCKABLE_AUX:
              token = picman_session_info_aux_deserialize (scanner,
                                                         &info->aux_info);
              if (token != G_TOKEN_LEFT_PAREN)
                goto error;
              break;

            default:
              goto error;
            }
          token = G_TOKEN_RIGHT_PAREN;
          break;

        case G_TOKEN_RIGHT_PAREN:
          token = G_TOKEN_LEFT_PAREN;
          break;

        default:
          break;
        }
    }

  *dockable = info;

  g_type_class_unref (enum_class);

  g_scanner_scope_remove_symbol (scanner, scope, "locked");
  g_scanner_scope_remove_symbol (scanner, scope, "tab-style");
  g_scanner_scope_remove_symbol (scanner, scope, "preview-size");
  g_scanner_scope_remove_symbol (scanner, scope, "aux-info");

  return token;

 error:
  *dockable = NULL;

  picman_session_info_dockable_free (info);
  g_type_class_unref (enum_class);

  return token;
}

PicmanSessionInfoDockable *
picman_session_info_dockable_from_widget (PicmanDockable *dockable)
{
  PicmanSessionInfoDockable *info;
  PicmanDialogFactoryEntry  *entry;
  PicmanContainerView       *view;
  gint                     view_size = -1;

  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), NULL);

  picman_dialog_factory_from_widget (GTK_WIDGET (dockable), &entry);

  g_return_val_if_fail (entry != NULL, NULL);

  info = picman_session_info_dockable_new ();

  info->locked     = picman_dockable_get_locked (dockable);
  info->identifier = g_strdup (entry->identifier);
  info->tab_style  = picman_dockable_get_tab_style (dockable);
  info->view_size  = -1;

  view = picman_container_view_get_by_dockable (dockable);

  if (view)
    view_size = picman_container_view_get_view_size (view, NULL);

  if (view_size > 0 && view_size != entry->view_size)
    info->view_size = view_size;

  if (PICMAN_IS_SESSION_MANAGED (dockable))
    info->aux_info =
      picman_session_managed_get_aux_info (PICMAN_SESSION_MANAGED (dockable));

  return info;
}

PicmanDockable *
picman_session_info_dockable_restore (PicmanSessionInfoDockable *info,
                                    PicmanDock                *dock)
{
  GtkWidget *dockable;

  g_return_val_if_fail (info != NULL, NULL);
  g_return_val_if_fail (PICMAN_IS_DOCK (dock), NULL);

  if (info->view_size < PICMAN_VIEW_SIZE_TINY ||
      info->view_size > PICMAN_VIEW_SIZE_GIGANTIC)
    info->view_size = -1;

  dockable =
    picman_dialog_factory_dockable_new (picman_dock_get_dialog_factory (dock),
                                      dock,
                                      info->identifier,
                                      info->view_size);

  if (dockable)
    {
      /*  picman_dialog_factory_dockable_new() might return an already
       *  existing singleton dockable, return NULL so our caller won't
       *  try to add it to another dockbook
       */
      if (picman_dockable_get_dockbook (PICMAN_DOCKABLE (dockable)))
        return NULL;

      picman_dockable_set_locked    (PICMAN_DOCKABLE (dockable), info->locked);
      picman_dockable_set_tab_style (PICMAN_DOCKABLE (dockable), info->tab_style);

      if (info->aux_info)
        picman_session_managed_set_aux_info (PICMAN_SESSION_MANAGED (dockable),
                                           info->aux_info);
    }

  return PICMAN_DOCKABLE (dockable);
}
