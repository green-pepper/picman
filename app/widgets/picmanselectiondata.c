/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmanbrush.h"
#include "core/picmancontainer.h"
#include "core/picmancurve.h"
#include "core/picmandatafactory.h"
#include "core/picmangradient.h"
#include "core/picmanimage.h"
#include "core/picmanimagefile.h"
#include "core/picmanitem.h"
#include "core/picmanpalette.h"
#include "core/picmanpattern.h"
#include "core/picmantoolinfo.h"

#include "text/picmanfont.h"

#include "picmanselectiondata.h"

#include "picman-log.h"
#include "picman-intl.h"


/*  local function prototypes  */

static const gchar * picman_selection_data_get_name   (GtkSelectionData *selection,
                                                     const gchar      *strfunc);
static PicmanObject  * picman_selection_data_get_object (GtkSelectionData *selection,
                                                     PicmanContainer    *container,
                                                     PicmanObject       *additional);
static gchar       * picman_unescape_uri_string       (const char       *escaped,
                                                     int               len,
                                                     const char       *illegal_escaped_characters,
                                                     gboolean          ascii_must_not_be_escaped);


/*  public functions  */

void
picman_selection_data_set_uri_list (GtkSelectionData *selection,
                                  GList            *uri_list)
{
  GList *list;
  gchar *vals = NULL;

  g_return_if_fail (selection != NULL);
  g_return_if_fail (uri_list != NULL);

  for (list = uri_list; list; list = g_list_next (list))
    {
      if (vals)
        {
          gchar *tmp = g_strconcat (vals,
                                    list->data,
                                    list->next ? "\n" : NULL,
                                    NULL);
          g_free (vals);
          vals = tmp;
        }
      else
        {
          vals = g_strconcat (list->data,
                              list->next ? "\n" : NULL,
                              NULL);
        }
    }

  gtk_selection_data_set (selection,
                          gtk_selection_data_get_target (selection),
                          8, (guchar *) vals, strlen (vals));

  g_free (vals);
}

GList *
picman_selection_data_get_uri_list (GtkSelectionData *selection)
{
  GList       *crap_list = NULL;
  GList       *uri_list  = NULL;
  GList       *list;
  gint         length;
  const gchar *data;
  const gchar *buffer;

  g_return_val_if_fail (selection != NULL, NULL);

  length = gtk_selection_data_get_length (selection);

  if (gtk_selection_data_get_format (selection) != 8 || length < 1)
    {
      g_warning ("Received invalid file data!");
      return NULL;
    }

  data = buffer = (const gchar *) gtk_selection_data_get_data (selection);

  PICMAN_LOG (DND, "raw buffer >>%s<<", buffer);

  {
    gchar name_buffer[1024];

    while (*buffer && (buffer - data < length))
      {
        gchar *name = name_buffer;
        gint   len  = 0;

        while (len < sizeof (name_buffer) && *buffer && *buffer != '\n')
          {
            *name++ = *buffer++;
            len++;
          }
        if (len == 0)
          break;

        if (*(name - 1) == 0xd)   /* gmc uses RETURN+NEWLINE as delimiter */
          len--;

        if (len > 2)
          crap_list = g_list_prepend (crap_list, g_strndup (name_buffer, len));

        if (*buffer)
          buffer++;
      }
  }

  if (! crap_list)
    return NULL;

  /*  do various checks because file drag sources send all kinds of
   *  arbitrary crap...
   */
  for (list = crap_list; list; list = g_list_next (list))
    {
      const gchar *dnd_crap = list->data;
      gchar       *filename;
      gchar       *hostname;
      gchar       *uri   = NULL;
      GError      *error = NULL;

      PICMAN_LOG (DND, "trying to convert \"%s\" to an uri", dnd_crap);

      filename = g_filename_from_uri (dnd_crap, &hostname, NULL);

      if (filename)
        {
          /*  if we got a correctly encoded "file:" uri...
           *
           *  (for GLib < 2.4.4, this is escaped UTF-8,
           *   for GLib > 2.4.4, this is escaped local filename encoding)
           */

          uri = g_filename_to_uri (filename, hostname, NULL);

          g_free (hostname);
          g_free (filename);
        }
      else if (g_file_test (dnd_crap, G_FILE_TEST_EXISTS))
        {
          /*  ...else if we got a valid local filename...  */

          uri = g_filename_to_uri (dnd_crap, NULL, NULL);
        }
      else
        {
          /*  ...otherwise do evil things...  */

          const gchar *start = dnd_crap;

          if (g_str_has_prefix (dnd_crap, "file://"))
            {
              start += strlen ("file://");
            }
          else if (g_str_has_prefix (dnd_crap, "file:"))
            {
              start += strlen ("file:");
            }

          if (start != dnd_crap)
            {
              /*  try if we got a "file:" uri in the wrong encoding...
               *
               *  (for GLib < 2.4.4, this is escaped local filename encoding,
               *   for GLib > 2.4.4, this is escaped UTF-8)
               */
              gchar *unescaped_filename;

              if (strstr (dnd_crap, "%"))
                {
                  gchar *local_filename;

                  unescaped_filename = picman_unescape_uri_string (start, -1,
                                                                 "/", FALSE);

                  /*  check if we got a drop from an application that
                   *  encodes file: URIs as UTF-8 (apps linked against
                   *  GLib < 2.4.4)
                   */
                  local_filename = g_filename_from_utf8 (unescaped_filename,
                                                         -1, NULL, NULL,
                                                         NULL);

                  if (local_filename)
                    {
                      g_free (unescaped_filename);
                      unescaped_filename = local_filename;
                    }
                }
              else
                {
                  unescaped_filename = g_strdup (start);
                }

              uri = g_filename_to_uri (unescaped_filename, NULL, &error);

              if (! uri)
                {
                  gchar *escaped_filename = g_strescape (unescaped_filename,
                                                         NULL);

                  g_message (_("The filename '%s' couldn't be converted to a "
                               "valid URI:\n\n%s"),
                             escaped_filename,
                             error->message ?
                             error->message : _("Invalid UTF-8"));
                  g_free (escaped_filename);
                  g_clear_error (&error);

                  g_free (unescaped_filename);
                  continue;
                }

              g_free (unescaped_filename);
            }
          else
            {
              /*  otherwise try the crap passed anyway, in case it's
               *  a "http:" or whatever uri a plug-in might handle
               */
              uri = g_strdup (dnd_crap);
            }
        }

      uri_list = g_list_prepend (uri_list, uri);
    }

  g_list_free_full (crap_list, (GDestroyNotify) g_free);

  return uri_list;
}

void
picman_selection_data_set_color (GtkSelectionData *selection,
                               const PicmanRGB    *color)
{
  guint16  vals[4];
  guchar   r, g, b, a;

  g_return_if_fail (selection != NULL);
  g_return_if_fail (color != NULL);

  picman_rgba_get_uchar (color, &r, &g, &b, &a);

  vals[0] = r + (r << 8);
  vals[1] = g + (g << 8);
  vals[2] = b + (b << 8);
  vals[3] = a + (a << 8);

  gtk_selection_data_set (selection,
                          gtk_selection_data_get_target (selection),
                          16, (const guchar *) vals, 8);
}

gboolean
picman_selection_data_get_color (GtkSelectionData *selection,
                               PicmanRGB          *color)
{
  const guint16 *color_vals;

  g_return_val_if_fail (selection != NULL, FALSE);
  g_return_val_if_fail (color != NULL, FALSE);

  if (gtk_selection_data_get_format (selection) != 16 ||
      gtk_selection_data_get_length (selection) != 8)
    {
      g_warning ("Received invalid color data!");
      return FALSE;
    }

  color_vals = (const guint16 *) gtk_selection_data_get_data (selection);

  picman_rgba_set_uchar (color,
                       (guchar) (color_vals[0] >> 8),
                       (guchar) (color_vals[1] >> 8),
                       (guchar) (color_vals[2] >> 8),
                       (guchar) (color_vals[3] >> 8));

  return TRUE;
}

void
picman_selection_data_set_stream (GtkSelectionData *selection,
                                const guchar     *stream,
                                gsize             stream_length)
{
  g_return_if_fail (selection != NULL);
  g_return_if_fail (stream != NULL);
  g_return_if_fail (stream_length > 0);

  gtk_selection_data_set (selection,
                          gtk_selection_data_get_target (selection),
                          8, (guchar *) stream, stream_length);
}

const guchar *
picman_selection_data_get_stream (GtkSelectionData *selection,
                                gsize            *stream_length)
{
  gint length;

  g_return_val_if_fail (selection != NULL, NULL);
  g_return_val_if_fail (stream_length != NULL, NULL);

  length = gtk_selection_data_get_length (selection);

  if (gtk_selection_data_get_format (selection) != 8 || length < 1)
    {
      g_warning ("Received invalid data stream!");
      return NULL;
    }

  *stream_length = length;

  return (const guchar *) gtk_selection_data_get_data (selection);
}

void
picman_selection_data_set_curve (GtkSelectionData *selection,
                               PicmanCurve        *curve)
{
  gchar *str;

  g_return_if_fail (selection != NULL);
  g_return_if_fail (PICMAN_IS_CURVE (curve));

  str = picman_config_serialize_to_string (PICMAN_CONFIG (curve), NULL);

  gtk_selection_data_set (selection,
                          gtk_selection_data_get_target (selection),
                          8, (guchar *) str, strlen (str));

  g_free (str);
}

PicmanCurve *
picman_selection_data_get_curve (GtkSelectionData *selection)
{
  PicmanCurve *curve;
  gint       length;
  GError    *error = NULL;

  g_return_val_if_fail (selection != NULL, NULL);

  length = gtk_selection_data_get_length (selection);

  if (gtk_selection_data_get_format (selection) != 8 || length < 1)
    {
      g_warning ("Received invalid curve data!");
      return NULL;
    }

  curve = PICMAN_CURVE (picman_curve_new ("pasted curve"));

  if (! picman_config_deserialize_string (PICMAN_CONFIG (curve),
                                        (const gchar *)
                                        gtk_selection_data_get_data (selection),
                                        length,
                                        NULL,
                                        &error))
    {
      g_warning ("Received invalid curve data: %s", error->message);
      g_clear_error (&error);
      g_object_unref (curve);
      return NULL;
    }

  return curve;
}

void
picman_selection_data_set_image (GtkSelectionData *selection,
                               PicmanImage        *image)
{
  gchar *str;

  g_return_if_fail (selection != NULL);
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  str = g_strdup_printf ("%d:%d", picman_get_pid (), picman_image_get_ID (image));

  gtk_selection_data_set (selection,
                          gtk_selection_data_get_target (selection),
                          8, (guchar *) str, strlen (str));

  g_free (str);
}

PicmanImage *
picman_selection_data_get_image (GtkSelectionData *selection,
                               Picman             *picman)
{
  const gchar *str;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  str = picman_selection_data_get_name (selection, G_STRFUNC);

  if (str)
    {
      gint pid;
      gint ID;

      if (sscanf (str, "%i:%i", &pid, &ID) == 2 &&
          pid == picman_get_pid ())
        {
          return picman_image_get_by_ID (picman, ID);
        }
    }

  return NULL;
}

void
picman_selection_data_set_component (GtkSelectionData *selection,
                                   PicmanImage        *image,
                                   PicmanChannelType   channel)
{
  gchar *str;

  g_return_if_fail (selection != NULL);
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  str = g_strdup_printf ("%d:%d:%d", picman_get_pid (), picman_image_get_ID (image),
                         (gint) channel);

  gtk_selection_data_set (selection,
                          gtk_selection_data_get_target (selection),
                          8, (guchar *) str, strlen (str));

  g_free (str);
}

PicmanImage *
picman_selection_data_get_component (GtkSelectionData *selection,
                                   Picman             *picman,
                                   PicmanChannelType  *channel)
{
  const gchar *str;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  if (channel)
    *channel = 0;

  str = picman_selection_data_get_name (selection, G_STRFUNC);

  if (str)
    {
      gint pid;
      gint ID;
      gint ch;

      if (sscanf (str, "%i:%i:%i", &pid, &ID, &ch) == 3 &&
          pid == picman_get_pid ())
        {
          PicmanImage *image = picman_image_get_by_ID (picman, ID);

          if (image && channel)
            *channel = ch;

          return image;
        }
    }

  return NULL;
}

void
picman_selection_data_set_item (GtkSelectionData *selection,
                              PicmanItem         *item)
{
  gchar *str;

  g_return_if_fail (selection != NULL);
  g_return_if_fail (PICMAN_IS_ITEM (item));

  str = g_strdup_printf ("%d:%d", picman_get_pid (), picman_item_get_ID (item));

  gtk_selection_data_set (selection,
                          gtk_selection_data_get_target (selection),
                          8, (guchar *) str, strlen (str));

  g_free (str);
}

PicmanItem *
picman_selection_data_get_item (GtkSelectionData *selection,
                              Picman             *picman)
{
  const gchar *str;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  str = picman_selection_data_get_name (selection, G_STRFUNC);

  if (str)
    {
      gint pid;
      gint ID;

      if (sscanf (str, "%i:%i", &pid, &ID) == 2 &&
          pid == picman_get_pid ())
        {
          return picman_item_get_by_ID (picman, ID);
        }
    }

  return NULL;
}

void
picman_selection_data_set_object (GtkSelectionData *selection,
                                PicmanObject       *object)
{
  const gchar *name;

  g_return_if_fail (selection != NULL);
  g_return_if_fail (PICMAN_IS_OBJECT (object));

  name = picman_object_get_name (object);

  if (name)
    {
      gchar *str;

      str = g_strdup_printf ("%d:%p:%s", picman_get_pid (), object, name);

      gtk_selection_data_set (selection,
                              gtk_selection_data_get_target (selection),
                              8, (guchar *) str, strlen (str));

      g_free (str);
    }
}

PicmanBrush *
picman_selection_data_get_brush (GtkSelectionData *selection,
                               Picman             *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  return (PicmanBrush *)
    picman_selection_data_get_object (selection,
                                    picman_data_factory_get_container (picman->brush_factory),
                                    PICMAN_OBJECT (picman_brush_get_standard (picman_get_user_context (picman))));
}

PicmanPattern *
picman_selection_data_get_pattern (GtkSelectionData *selection,
                                 Picman             *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  return (PicmanPattern *)
    picman_selection_data_get_object (selection,
                                    picman_data_factory_get_container (picman->pattern_factory),
                                    PICMAN_OBJECT (picman_pattern_get_standard (picman_get_user_context (picman))));
}

PicmanGradient *
picman_selection_data_get_gradient (GtkSelectionData *selection,
                                  Picman             *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  return (PicmanGradient *)
    picman_selection_data_get_object (selection,
                                    picman_data_factory_get_container (picman->gradient_factory),
                                    PICMAN_OBJECT (picman_gradient_get_standard (picman_get_user_context (picman))));
}

PicmanPalette *
picman_selection_data_get_palette (GtkSelectionData *selection,
                                 Picman             *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  return (PicmanPalette *)
    picman_selection_data_get_object (selection,
                                    picman_data_factory_get_container (picman->palette_factory),
                                    PICMAN_OBJECT (picman_palette_get_standard (picman_get_user_context (picman))));
}

PicmanFont *
picman_selection_data_get_font (GtkSelectionData *selection,
                              Picman             *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  return (PicmanFont *)
    picman_selection_data_get_object (selection,
                                    picman->fonts,
                                    PICMAN_OBJECT (picman_font_get_standard ()));
}

PicmanBuffer *
picman_selection_data_get_buffer (GtkSelectionData *selection,
                                Picman             *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  return (PicmanBuffer *)
    picman_selection_data_get_object (selection,
                                    picman->named_buffers,
                                    PICMAN_OBJECT (picman->global_buffer));
}

PicmanImagefile *
picman_selection_data_get_imagefile (GtkSelectionData *selection,
                                   Picman             *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  return (PicmanImagefile *) picman_selection_data_get_object (selection,
                                                           picman->documents,
                                                           NULL);
}

PicmanTemplate *
picman_selection_data_get_template (GtkSelectionData *selection,
                                  Picman             *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  return (PicmanTemplate *) picman_selection_data_get_object (selection,
                                                          picman->templates,
                                                          NULL);
}

PicmanToolInfo *
picman_selection_data_get_tool_info (GtkSelectionData *selection,
                                   Picman             *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  return (PicmanToolInfo *)
    picman_selection_data_get_object (selection,
                                    picman->tool_info_list,
                                    PICMAN_OBJECT (picman_tool_info_get_standard (picman)));
}


/*  private functions  */

static const gchar *
picman_selection_data_get_name (GtkSelectionData *selection,
                              const gchar      *strfunc)
{
  const gchar *name;

  if (gtk_selection_data_get_format (selection) != 8 ||
      gtk_selection_data_get_length (selection) < 1)
    {
      g_warning ("%s: received invalid selection data", strfunc);
      return NULL;
    }

  name = (const gchar *) gtk_selection_data_get_data (selection);

  if (! g_utf8_validate (name, -1, NULL))
    {
      g_warning ("%s: received invalid selection data "
                 "(doesn't validate as UTF-8)", strfunc);
      return NULL;
    }

  PICMAN_LOG (DND, "name = '%s'", name);

  return name;
}

static PicmanObject *
picman_selection_data_get_object (GtkSelectionData *selection,
                                PicmanContainer    *container,
                                PicmanObject       *additional)
{
  const gchar *str;

  str = picman_selection_data_get_name (selection, G_STRFUNC);

  if (str)
    {
      gint     pid;
      gpointer object_addr;
      gint     name_offset = 0;

      if (sscanf (str, "%i:%p:%n", &pid, &object_addr, &name_offset) >= 2 &&
          pid == picman_get_pid () && name_offset > 0)
        {
          const gchar *name = str + name_offset;

          PICMAN_LOG (DND, "pid = %d, addr = %p, name = '%s'",
                    pid, object_addr, name);

          if (additional &&
              strcmp (name, picman_object_get_name (additional)) == 0 &&
              object_addr == (gpointer) additional)
            {
              return additional;
            }
          else
            {
              PicmanObject *object;

              object = picman_container_get_child_by_name (container, name);

              if (object_addr == (gpointer) object)
                return object;
            }
        }
    }

  return NULL;
}

/*  the next two functions are straight cut'n'paste from glib/glib/gconvert.c,
 *  except that picman_unescape_uri_string() does not try to UTF-8 validate
 *  the unescaped result.
 */
static int
unescape_character (const char *scanner)
{
  int first_digit;
  int second_digit;

  first_digit = g_ascii_xdigit_value (scanner[0]);
  if (first_digit < 0)
    return -1;

  second_digit = g_ascii_xdigit_value (scanner[1]);
  if (second_digit < 0)
    return -1;

  return (first_digit << 4) | second_digit;
}

static gchar *
picman_unescape_uri_string (const char *escaped,
                          int         len,
                          const char *illegal_escaped_characters,
                          gboolean    ascii_must_not_be_escaped)
{
  const gchar *in, *in_end;
  gchar *out, *result;
  int c;

  if (escaped == NULL)
    return NULL;

  if (len < 0)
    len = strlen (escaped);

  result = g_malloc (len + 1);

  out = result;
  for (in = escaped, in_end = escaped + len; in < in_end; in++)
    {
      c = *in;

      if (c == '%')
        {
          /* catch partial escape sequences past the end of the substring */
          if (in + 3 > in_end)
            break;

          c = unescape_character (in + 1);

          /* catch bad escape sequences and NUL characters */
          if (c <= 0)
            break;

          /* catch escaped ASCII */
          if (ascii_must_not_be_escaped && c <= 0x7F)
            break;

          /* catch other illegal escaped characters */
          if (strchr (illegal_escaped_characters, c) != NULL)
            break;

          in += 2;
        }

      *out++ = c;
    }

  g_assert (out - result <= len);
  *out = '\0';

  if (in != in_end)
    {
      g_free (result);
      return NULL;
    }

  return result;
}
