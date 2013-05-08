/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanConfigWriter
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/types.h>

#include <glib-object.h>
#include <glib/gstdio.h>

#ifdef G_OS_WIN32
#include <io.h>
#endif

#include "libpicmanbase/picmanbase.h"

#include "picmanconfigtypes.h"

#include "picmanconfigwriter.h"
#include "picmanconfig-iface.h"
#include "picmanconfig-error.h"
#include "picmanconfig-serialize.h"
#include "picmanconfig-utils.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmanconfigwriter
 * @title: PicmanConfigWriter
 * @short_description: Functions for writing config info to a file for
 *                     libpicmanconfig.
 *
 * Functions for writing config info to a file for libpicmanconfig.
 **/


struct _PicmanConfigWriter
{
  gint      fd;
  gchar    *filename;
  gchar    *tmpname;
  GError   *error;
  GString  *buffer;
  gboolean  comment;
  gint      depth;
  gint      marker;
};


static inline void  picman_config_writer_flush      (PicmanConfigWriter  *writer);
static inline void  picman_config_writer_newline    (PicmanConfigWriter  *writer);
static gboolean     picman_config_writer_close_file (PicmanConfigWriter  *writer,
                                                   GError           **error);

static inline void
picman_config_writer_flush (PicmanConfigWriter *writer)
{
  if (write (writer->fd, writer->buffer->str, writer->buffer->len) < 0)
    g_set_error (&writer->error, PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_WRITE,
                 _("Error writing to '%s': %s"),
                 picman_filename_to_utf8 (writer->filename), g_strerror (errno));

  g_string_truncate (writer->buffer, 0);
}

static inline void
picman_config_writer_newline (PicmanConfigWriter *writer)
{
  gint i;

  g_string_append_c (writer->buffer, '\n');

  if (writer->comment)
    g_string_append_len (writer->buffer, "# ", 2);

  for (i = 0; i < writer->depth; i++)
    g_string_append_len (writer->buffer, "    ", 4);
}

/**
 * picman_config_writer_new_file:
 * @filename: a filename
 * @atomic: if %TRUE the file is written atomically
 * @header: text to include as comment at the top of the file
 * @error: return location for errors
 *
 * Creates a new #PicmanConfigWriter and sets it up to write to
 * @filename. If @atomic is %TRUE, a temporary file is used to avoid
 * possible race conditions. The temporary file is then moved to
 * @filename when the writer is closed.
 *
 * Return value: a new #PicmanConfigWriter or %NULL in case of an error
 *
 * Since: PICMAN 2.4
 **/
PicmanConfigWriter *
picman_config_writer_new_file (const gchar  *filename,
                             gboolean      atomic,
                             const gchar  *header,
                             GError      **error)
{
  PicmanConfigWriter *writer;
  gchar            *tmpname = NULL;
  gint              fd;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (atomic)
    {
      tmpname = g_strconcat (filename, "XXXXXX", NULL);

      fd = g_mkstemp (tmpname);

      if (fd == -1)
        {
          g_set_error (error, PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_WRITE,
                       _("Could not create temporary file for '%s': %s"),
                       picman_filename_to_utf8 (filename), g_strerror (errno));
          g_free (tmpname);
          return NULL;
        }
    }
  else
    {
      fd = g_creat (filename, 0644);

      if (fd == -1)
        {
          g_set_error (error, PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_WRITE,
                       _("Could not open '%s' for writing: %s"),
                       picman_filename_to_utf8 (filename), g_strerror (errno));
          return NULL;
        }
    }

  writer = g_slice_new0 (PicmanConfigWriter);

  writer->fd       = fd;
  writer->filename = g_strdup (filename);
  writer->tmpname  = tmpname;
  writer->buffer   = g_string_new (NULL);

  if (header)
    {
      picman_config_writer_comment (writer, header);
      picman_config_writer_linefeed (writer);
    }

  return writer;
}

/**
 * picman_config_writer_new_fd:
 * @fd:
 *
 * Return value: a new #PicmanConfigWriter or %NULL in case of an error
 *
 * Since: PICMAN 2.4
 **/
PicmanConfigWriter *
picman_config_writer_new_fd (gint fd)
{
  PicmanConfigWriter *writer;

  g_return_val_if_fail (fd > 0, NULL);

  writer = g_slice_new0 (PicmanConfigWriter);

  writer->fd     = fd;
  writer->buffer = g_string_new (NULL);

  return writer;
}

/**
 * picman_config_writer_new_string:
 * @string:
 *
 * Return value: a new #PicmanConfigWriter or %NULL in case of an error
 *
 * Since: PICMAN 2.4
 **/
PicmanConfigWriter *
picman_config_writer_new_string (GString *string)
{
  PicmanConfigWriter *writer;

  g_return_val_if_fail (string != NULL, NULL);

  writer = g_slice_new0 (PicmanConfigWriter);

  writer->buffer = string;

  return writer;
}

/**
 * picman_config_writer_comment_mode:
 * @writer: a #PicmanConfigWriter
 * @enable: %TRUE to enable comment mode, %FALSE to disable it
 *
 * This function toggles whether the @writer should create commented
 * or uncommented output. This feature is used to generate the
 * system-wide installed picmanrc that documents the default settings.
 *
 * Since comments have to start at the beginning of a line, this
 * function will insert a newline if necessary.
 *
 * Since: PICMAN 2.4
 **/
void
picman_config_writer_comment_mode (PicmanConfigWriter *writer,
                                 gboolean          enable)
{
  g_return_if_fail (writer != NULL);

  if (writer->error)
    return;

  enable = (enable ? TRUE : FALSE);

  if (writer->comment == enable)
    return;

  writer->comment = enable;

  if (enable)
    {
     if (writer->buffer->len == 0)
       g_string_append_len (writer->buffer, "# ", 2);
     else
       picman_config_writer_newline (writer);
    }
}


/**
 * picman_config_writer_open:
 * @writer: a #PicmanConfigWriter
 * @name: name of the element to open
 *
 * This function writes the opening parenthese followed by @name.
 * It also increases the indentation level and sets a mark that
 * can be used by picman_config_writer_revert().
 *
 * Since: PICMAN 2.4
 **/
void
picman_config_writer_open (PicmanConfigWriter *writer,
                         const gchar      *name)
{
  g_return_if_fail (writer != NULL);
  g_return_if_fail (name != NULL);

  if (writer->error)
    return;

  /* store the current buffer length so we can revert to this state */
  writer->marker = writer->buffer->len;

  if (writer->depth > 0)
    picman_config_writer_newline (writer);

  writer->depth++;

  g_string_append_printf (writer->buffer, "(%s", name);
}

/**
 * picman_config_writer_print:
 * @writer: a #PicmanConfigWriter
 * @string: a string to write
 * @len: number of bytes from @string or -1 if @string is NUL-terminated.
 *
 * Appends a space followed by @string to the @writer. Note that string
 * must not contain any special characters that might need to be escaped.
 *
 * Since: PICMAN 2.4
 **/
void
picman_config_writer_print (PicmanConfigWriter  *writer,
                          const gchar       *string,
                          gint               len)
{
  g_return_if_fail (writer != NULL);
  g_return_if_fail (len == 0 || string != NULL);

  if (writer->error)
    return;

  if (len < 0)
    len = strlen (string);

  if (len)
    {
      g_string_append_c (writer->buffer, ' ');
      g_string_append_len (writer->buffer, string, len);
    }
}

/**
 * picman_config_writer_printf:
 * @writer: a #PicmanConfigWriter
 * @format: a format string as described for g_strdup_printf().
 * @...: list of arguments according to @format
 *
 * A printf-like function for #PicmanConfigWriter.
 *
 * Since: PICMAN 2.4
 **/
void
picman_config_writer_printf (PicmanConfigWriter *writer,
                           const gchar      *format,
                           ...)
{
  gchar   *buffer;
  va_list  args;

  g_return_if_fail (writer != NULL);
  g_return_if_fail (format != NULL);

  if (writer->error)
    return;

  va_start (args, format);
  buffer = g_strdup_vprintf (format, args);
  va_end (args);

  g_string_append_c (writer->buffer, ' ');
  g_string_append (writer->buffer, buffer);

  g_free (buffer);
}

/**
 * picman_config_writer_string:
 * @writer: a #PicmanConfigWriter
 * @string: a NUL-terminated string
 *
 * Writes a string value to @writer. The @string is quoted and special
 * characters are escaped.
 *
 * Since: PICMAN 2.4
 **/
void
picman_config_writer_string (PicmanConfigWriter *writer,
                           const gchar      *string)
{
  g_return_if_fail (writer != NULL);

  if (writer->error)
    return;

  g_string_append_c (writer->buffer, ' ');
  picman_config_string_append_escaped (writer->buffer, string);
}

/**
 * picman_config_writer_identifier:
 * @writer:     a #PicmanConfigWriter
 * @identifier: a NUL-terminated string
 *
 * Writes an identifier to @writer. The @string is *not* quoted and special
 * characters are *not* escaped.
 *
 * Since: PICMAN 2.4
 **/
void
picman_config_writer_identifier (PicmanConfigWriter *writer,
                               const gchar      *identifier)
{
  g_return_if_fail (writer != NULL);
  g_return_if_fail (identifier != NULL);

  if (writer->error)
    return;

  g_string_append_printf (writer->buffer, " %s", identifier);
}


/**
 * picman_config_writer_data:
 * @writer: a #PicmanConfigWriter
 * @length:
 * @data:
 *
 * Since: PICMAN 2.4
 **/
void
picman_config_writer_data (PicmanConfigWriter *writer,
                         gint              length,
                         const guint8     *data)
{
  gint i;

  g_return_if_fail (writer != NULL);
  g_return_if_fail (length > 0);
  g_return_if_fail (data != NULL);

  if (writer->error)
    return;

  g_string_append (writer->buffer, " \"");

  for (i = 0; i < length; i++)
    {
      if (g_ascii_isalpha (data[i]))
        g_string_append_c (writer->buffer, data[i]);
      else
        g_string_append_printf (writer->buffer, "\\%o", data[i]);
    }

  g_string_append (writer->buffer, "\"");
}

/**
 * picman_config_writer_revert:
 * @writer: a #PicmanConfigWriter
 *
 * Reverts all changes to @writer that were done since the last call
 * to picman_config_writer_open(). This can only work if you didn't call
 * picman_config_writer_close() yet.
 *
 * Since: PICMAN 2.4
 **/
void
picman_config_writer_revert (PicmanConfigWriter *writer)
{
  g_return_if_fail (writer != NULL);

  if (writer->error)
    return;

  g_return_if_fail (writer->depth > 0);
  g_return_if_fail (writer->marker != -1);

  g_string_truncate (writer->buffer, writer->marker);

  writer->depth--;
  writer->marker = -1;
}

/**
 * picman_config_writer_close:
 * @writer: a #PicmanConfigWriter
 *
 * Closes an element opened with picman_config_writer_open().
 *
 * Since: PICMAN 2.4
 **/
void
picman_config_writer_close (PicmanConfigWriter *writer)
{
  g_return_if_fail (writer != NULL);

  if (writer->error)
    return;

  g_return_if_fail (writer->depth > 0);

  g_string_append_c (writer->buffer, ')');

  if (--writer->depth == 0)
    {
      g_string_append_c (writer->buffer, '\n');

      if (writer->fd)
        picman_config_writer_flush (writer);
    }
}

/**
 * picman_config_writer_finish:
 * @writer: a #PicmanConfigWriter
 * @footer: text to include as comment at the bottom of the file
 * @error: return location for possible errors
 *
 * This function finishes the work of @writer and frees it afterwards.
 * It closes all open elements, appends an optional comment and
 * releases all resources allocated by @writer. You must not access
 * the @writer afterwards.
 *
 * Return value: %TRUE if everything could be successfully written,
 *               %FALSE otherwise
 *
 * Since: PICMAN 2.4
 **/
gboolean
picman_config_writer_finish (PicmanConfigWriter  *writer,
                           const gchar       *footer,
                           GError           **error)
{
  gboolean success = TRUE;

  g_return_val_if_fail (writer != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (writer->depth < 0)
    {
      g_warning ("picman_config_writer_finish: depth < 0 !!");
    }
  else
    {
      while (writer->depth)
        picman_config_writer_close (writer);
    }

  if (footer)
    {
      picman_config_writer_linefeed (writer);
      picman_config_writer_comment (writer, footer);
    }

  if (writer->fd)
    {
      success = picman_config_writer_close_file (writer, error);

      g_free (writer->filename);
      g_free (writer->tmpname);

      g_string_free (writer->buffer, TRUE);
    }
  else
    {
      success = TRUE;
    }

  if (writer->error)
    {
      g_propagate_error (error, writer->error);
      success = FALSE;
    }

  g_slice_free (PicmanConfigWriter, writer);

  return success;
}

void
picman_config_writer_linefeed (PicmanConfigWriter *writer)
{
  g_return_if_fail (writer != NULL);

  if (writer->error)
    return;

  if (writer->buffer->len == 0 && !writer->comment)
    {
      if (write (writer->fd, "\n", 1) < 0)
        g_set_error_literal (&writer->error,
			     PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_WRITE,
			     g_strerror (errno));
    }
  else
    {
      picman_config_writer_newline (writer);
    }
}

/**
 * picman_config_writer_comment:
 * @writer: a #PicmanConfigWriter
 * @comment: the comment to write (ASCII only)
 *
 * Appends the @comment to @str and inserts linebreaks and hash-marks to
 * format it as a comment. Note that this function does not handle non-ASCII
 * characters.
 *
 * Since: PICMAN 2.4
 **/
void
picman_config_writer_comment (PicmanConfigWriter *writer,
                            const gchar      *comment)
{
  const gchar *s;
  gboolean     comment_mode;
  gint         i, len, space;

#define LINE_LENGTH 75

  g_return_if_fail (writer != NULL);

  if (writer->error)
    return;

  g_return_if_fail (writer->depth == 0);

  if (!comment)
    return;

  comment_mode = writer->comment;
  picman_config_writer_comment_mode (writer, TRUE);

  len = strlen (comment);

  while (len > 0)
    {
      for (s = comment, i = 0, space = 0;
           *s != '\n' && (i <= LINE_LENGTH || space == 0) && i < len;
           s++, i++)
        {
          if (g_ascii_isspace (*s))
            space = i;
        }

      if (i > LINE_LENGTH && space && *s != '\n')
        i = space;

      g_string_append_len (writer->buffer, comment, i);

      i++;

      comment += i;
      len     -= i;

      if (len > 0)
        picman_config_writer_newline (writer);
    }

  picman_config_writer_comment_mode (writer, comment_mode);
  picman_config_writer_newline (writer);

  if (writer->depth == 0)
    picman_config_writer_flush (writer);

#undef LINE_LENGTH
}

static gboolean
picman_config_writer_close_file (PicmanConfigWriter  *writer,
                               GError           **error)
{
  g_return_val_if_fail (writer->fd != 0, FALSE);

  if (! writer->filename)
    return TRUE;

  if (writer->error)
    {
      close (writer->fd);

      if (writer->tmpname)
        g_unlink (writer->tmpname);

      return FALSE;
    }

#ifdef HAVE_FSYNC
  /* If the final destination exists, we want to sync the newly written
   * file to ensure the data is on disk when we rename over the destination.
   * otherwise if we get a system crash we can lose both the new and the
   * old file on some filesystems. (I.E. those that don't guarantee the
   * data is written to the disk before the metadata.)
   */
  if (writer->tmpname && g_file_test (writer->filename, G_FILE_TEST_EXISTS))
    {
      if (fsync (writer->fd) != 0)
        {
          g_set_error (error, PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_WRITE,
                       _("Error writing to temporary file for '%s': %s\n"
                         "The original file has not been touched."),
                       picman_filename_to_utf8 (writer->filename),
                       g_strerror (errno));

          close (writer->fd);
          g_unlink (writer->tmpname);

          return FALSE;
        }
    }
#endif

  if (close (writer->fd) != 0)
    {
      if (writer->tmpname)
        {
          if (g_file_test (writer->filename, G_FILE_TEST_EXISTS))
            {
              g_set_error (error, PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_WRITE,
                           _("Error writing to temporary file for '%s': %s\n"
                             "The original file has not been touched."),
                           picman_filename_to_utf8 (writer->filename),
                           g_strerror (errno));
            }
          else
            {
              g_set_error (error, PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_WRITE,
                           _("Error writing to temporary file for '%s': %s\n"
                             "No file has been created."),
                           picman_filename_to_utf8 (writer->filename),
                           g_strerror (errno));
            }

          g_unlink (writer->tmpname);
        }
      else
        {
          g_set_error (error, PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_WRITE,
                       _("Error writing to '%s': %s"),
                       picman_filename_to_utf8 (writer->filename),
                       g_strerror (errno));
        }

      return FALSE;
    }

  if (writer->tmpname)
    {
      if (g_rename (writer->tmpname, writer->filename) == -1)
        {
          g_set_error (error, PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_WRITE,
                       _("Could not create '%s': %s"),
                       picman_filename_to_utf8 (writer->filename),
                       g_strerror (errno));

          g_unlink (writer->tmpname);
          return FALSE;
        }
    }

  return TRUE;
}
