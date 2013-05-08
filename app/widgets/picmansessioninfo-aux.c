/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansessioninfo-aux.c
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

#include <string.h>

#include <gtk/gtk.h>

#include "libpicmanconfig/picmanconfig.h"

#include "widgets-types.h"

#include "picmandock.h"
#include "picmandockable.h"
#include "picmandockwindow.h"
#include "picmansessioninfo-aux.h"
#include "picmansessionmanaged.h"


/*  public functions  */

PicmanSessionInfoAux *
picman_session_info_aux_new (const gchar *name,
                           const gchar *value)
{
  PicmanSessionInfoAux *aux;

  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (value != NULL, NULL);

  aux = g_slice_new0 (PicmanSessionInfoAux);

  aux->name  = g_strdup (name);
  aux->value = g_strdup (value);

  return aux;
}

void
picman_session_info_aux_free (PicmanSessionInfoAux *aux)
{
  g_return_if_fail (aux != NULL);

  g_free (aux->name);
  g_free (aux->value);

  g_slice_free (PicmanSessionInfoAux, aux);
}

GList *
picman_session_info_aux_new_from_props (GObject *object,
                                      ...)
{
  GList       *list = NULL;
  const gchar *prop_name;
  va_list      args;

  g_return_val_if_fail (G_IS_OBJECT (object), NULL);

  va_start (args, object);

  for (prop_name = va_arg (args, const gchar *);
       prop_name;
       prop_name = va_arg (args, const gchar *))
    {
      GObjectClass *class = G_OBJECT_GET_CLASS (object);
      GParamSpec   *pspec = g_object_class_find_property (class, prop_name);

      if (pspec)
        {
          GString *str   = g_string_new (NULL);
          GValue   value = { 0, };

          g_value_init (&value, pspec->value_type);
          g_object_get_property (object, pspec->name, &value);

          if (! g_param_value_defaults (pspec, &value) &&
              picman_config_serialize_value (&value, str, TRUE))
            {
              list = g_list_prepend (list,
                                     picman_session_info_aux_new (prop_name,
                                                                str->str));
            }

          g_value_unset (&value);
          g_string_free (str, TRUE);
        }
      else
        {
          g_warning ("%s: no property named '%s' for %s",
                     G_STRFUNC,
                     prop_name, G_OBJECT_CLASS_NAME (class));
        }
    }

  va_end (args);

  return g_list_reverse (list);
}

void
picman_session_info_aux_set_props (GObject *object,
                                 GList   *auxs,
                                 ...)
{
  const gchar *prop_name;
  va_list      args;

  g_return_if_fail (G_IS_OBJECT (object));

  va_start (args, auxs);

  for (prop_name = va_arg (args, const gchar *);
       prop_name;
       prop_name = va_arg (args, const gchar *))
    {
      GList *list;

      for (list = auxs; list; list = g_list_next (list))
        {
          PicmanSessionInfoAux *aux = list->data;

          if (strcmp (aux->name, prop_name) == 0)
            {
              GObjectClass *class = G_OBJECT_GET_CLASS (object);
              GParamSpec   *pspec = g_object_class_find_property (class,
                                                                  prop_name);

              if (pspec)
                {
                  GValue  value = { 0, };

                  g_value_init (&value, pspec->value_type);

                  if (G_VALUE_HOLDS_ENUM (&value))
                    {
                      GEnumClass *enum_class;
                      GEnumValue *enum_value;

                      enum_class = g_type_class_peek (pspec->value_type);
                      enum_value = g_enum_get_value_by_nick (enum_class,
                                                             aux->value);

                      if (enum_value)
                        {
                          g_value_set_enum (&value, enum_value->value);
                          g_object_set_property (object, pspec->name, &value);
                        }
                      else
                        {
                        g_warning ("%s: unknown enum value in '%s' for %s",
                                   G_STRFUNC,
                                   prop_name, G_OBJECT_CLASS_NAME (class));
                        }
                    }
                  else
                    {
                      GValue  str_value = { 0, };

                      g_value_init (&str_value, G_TYPE_STRING);
                      g_value_set_static_string (&str_value, aux->value);

                      if (g_value_transform (&str_value, &value))
                        g_object_set_property (object, pspec->name, &value);
                      else
                        g_warning ("%s: cannot convert property '%s' for %s",
                                   G_STRFUNC,
                                   prop_name, G_OBJECT_CLASS_NAME (class));

                      g_value_unset (&str_value);
                    }

                  g_value_unset (&value);
                }
              else
                {
                  g_warning ("%s: no property named '%s' for %s",
                             G_STRFUNC,
                             prop_name, G_OBJECT_CLASS_NAME (class));
                }
            }
        }
    }

  va_end (args);
}

void
picman_session_info_aux_serialize (PicmanConfigWriter *writer,
                                 GList            *aux_info)
{
  GList *list;

  g_return_if_fail (writer != NULL);
  g_return_if_fail (aux_info != NULL);

  picman_config_writer_open (writer, "aux-info");

  for (list = aux_info; list; list = g_list_next (list))
    {
      PicmanSessionInfoAux *aux = list->data;

      picman_config_writer_open (writer, aux->name);
      picman_config_writer_string (writer, aux->value);
      picman_config_writer_close (writer);
    }

  picman_config_writer_close (writer);
}

GTokenType
picman_session_info_aux_deserialize (GScanner  *scanner,
                                   GList    **aux_list)
{
  PicmanSessionInfoAux *aux_info = NULL;
  GTokenType          token    = G_TOKEN_LEFT_PAREN;

  g_return_val_if_fail (scanner != NULL, G_TOKEN_LEFT_PAREN);
  g_return_val_if_fail (aux_list != NULL, G_TOKEN_LEFT_PAREN);

  while (g_scanner_peek_next_token (scanner) == token)
    {
      token = g_scanner_get_next_token (scanner);

      switch (token)
        {
        case G_TOKEN_LEFT_PAREN:
          token = G_TOKEN_IDENTIFIER;
          break;

        case G_TOKEN_IDENTIFIER:
          {
            aux_info = g_slice_new0 (PicmanSessionInfoAux);

            aux_info->name = g_strdup (scanner->value.v_identifier);

            token = G_TOKEN_STRING;
            if (g_scanner_peek_next_token (scanner) != token)
              goto error;

            if (! picman_scanner_parse_string (scanner, &aux_info->value))
              goto error;

            *aux_list = g_list_append (*aux_list, aux_info);
            aux_info = NULL;
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

  return token;

 error:
  if (aux_info)
    picman_session_info_aux_free (aux_info);

  return token;
}
