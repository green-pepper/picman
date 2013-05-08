/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextBuffer
 * Copyright (C) 2010  Michael Natterer <mitch@picman.org>
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

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib/gstdio.h>

#include <glib.h>

#ifdef G_OS_WIN32
#include "libpicmanbase/picmanwin32-io.h"
#endif

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"

#include "widgets-types.h"

#include "picmantextbuffer.h"
#include "picmantextbuffer-serialize.h"
#include "picmantexttag.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   picman_text_buffer_constructed (GObject           *object);
static void   picman_text_buffer_dispose     (GObject           *object);
static void   picman_text_buffer_finalize    (GObject           *object);

static void   picman_text_buffer_mark_set    (GtkTextBuffer     *buffer,
                                            const GtkTextIter *location,
                                            GtkTextMark       *mark);


G_DEFINE_TYPE (PicmanTextBuffer, picman_text_buffer, GTK_TYPE_TEXT_BUFFER)

#define parent_class picman_text_buffer_parent_class


static void
picman_text_buffer_class_init (PicmanTextBufferClass *klass)
{
  GObjectClass       *object_class = G_OBJECT_CLASS (klass);
  GtkTextBufferClass *buffer_class = GTK_TEXT_BUFFER_CLASS (klass);

  object_class->constructed = picman_text_buffer_constructed;
  object_class->dispose     = picman_text_buffer_dispose;
  object_class->finalize    = picman_text_buffer_finalize;

  buffer_class->mark_set    = picman_text_buffer_mark_set;
}

static void
picman_text_buffer_init (PicmanTextBuffer *buffer)
{
  buffer->markup_atom =
    gtk_text_buffer_register_serialize_format (GTK_TEXT_BUFFER (buffer),
                                               "application/x-picman-pango-markup",
                                               picman_text_buffer_serialize,
                                               NULL, NULL);

  gtk_text_buffer_register_deserialize_format (GTK_TEXT_BUFFER (buffer),
                                               "application/x-picman-pango-markup",
                                               picman_text_buffer_deserialize,
                                               NULL, NULL);
}

static void
picman_text_buffer_constructed (GObject *object)
{
  PicmanTextBuffer *buffer = PICMAN_TEXT_BUFFER (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), "", -1);

  buffer->bold_tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer),
                                                 "bold",
                                                 "weight", PANGO_WEIGHT_BOLD,
                                                 NULL);

  buffer->italic_tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer),
                                                   "italic",
                                                   "style", PANGO_STYLE_ITALIC,
                                                   NULL);

  buffer->underline_tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer),
                                                      "underline",
                                                      "underline", PANGO_UNDERLINE_SINGLE,
                                                      NULL);

  buffer->strikethrough_tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer),
                                                          "strikethrough",
                                                          "strikethrough", TRUE,
                                                          NULL);
}

static void
picman_text_buffer_dispose (GObject *object)
{
  /* PicmanTextBuffer *buffer = PICMAN_TEXT_BUFFER (object); */

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_text_buffer_finalize (GObject *object)
{
  PicmanTextBuffer *buffer = PICMAN_TEXT_BUFFER (object);

  if (buffer->size_tags)
    {
      g_list_free (buffer->size_tags);
      buffer->size_tags = NULL;
    }

  if (buffer->baseline_tags)
    {
      g_list_free (buffer->baseline_tags);
      buffer->baseline_tags = NULL;
    }

  if (buffer->kerning_tags)
    {
      g_list_free (buffer->kerning_tags);
      buffer->kerning_tags = NULL;
    }

  if (buffer->font_tags)
    {
      g_list_free (buffer->font_tags);
      buffer->font_tags = NULL;
    }

  if (buffer->color_tags)
    {
      g_list_free (buffer->color_tags);
      buffer->color_tags = NULL;
    }

  picman_text_buffer_clear_insert_tags (buffer);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_text_buffer_mark_set (GtkTextBuffer     *buffer,
                           const GtkTextIter *location,
                           GtkTextMark       *mark)
{
  picman_text_buffer_clear_insert_tags (PICMAN_TEXT_BUFFER (buffer));

  GTK_TEXT_BUFFER_CLASS (parent_class)->mark_set (buffer, location, mark);
}


/*  public functions  */

PicmanTextBuffer *
picman_text_buffer_new (void)
{
  return g_object_new (PICMAN_TYPE_TEXT_BUFFER, NULL);
}

void
picman_text_buffer_set_text (PicmanTextBuffer *buffer,
                           const gchar    *text)
{
  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));

  if (text == NULL)
    text = "";

  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), text, -1);

  picman_text_buffer_clear_insert_tags (buffer);
}

gchar *
picman_text_buffer_get_text (PicmanTextBuffer *buffer)
{
  GtkTextIter start, end;

  g_return_val_if_fail (PICMAN_IS_TEXT_BUFFER (buffer), NULL);

  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (buffer), &start, &end);

  return gtk_text_buffer_get_text (GTK_TEXT_BUFFER (buffer),
                                   &start, &end, TRUE);
}

void
picman_text_buffer_set_markup (PicmanTextBuffer *buffer,
                             const gchar    *markup)
{
  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));

  picman_text_buffer_set_text (buffer, NULL);

  if (markup)
    {
      GtkTextTagTable *tag_table;
      GtkTextBuffer   *content;
      GtkTextIter      insert;
      GError          *error = NULL;

      tag_table = gtk_text_buffer_get_tag_table (GTK_TEXT_BUFFER (buffer));
      content = gtk_text_buffer_new (tag_table);

      gtk_text_buffer_get_start_iter (content, &insert);

      if (! gtk_text_buffer_deserialize (GTK_TEXT_BUFFER (buffer),
                                         content,
                                         buffer->markup_atom,
                                         &insert,
                                         (const guint8 *) markup, -1,
                                         &error))
        {
          g_printerr ("EEK: %s\n", error->message);
          g_clear_error (&error);
        }
      else
        {
          GtkTextIter start, end;

          picman_text_buffer_post_deserialize (buffer, content);

          gtk_text_buffer_get_bounds (content, &start, &end);
          gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (buffer), &insert);

          gtk_text_buffer_insert_range (GTK_TEXT_BUFFER (buffer),
                                        &insert, &start, &end);
        }

      g_object_unref (content);
    }

  picman_text_buffer_clear_insert_tags (buffer);
}

gchar *
picman_text_buffer_get_markup (PicmanTextBuffer *buffer)
{
  GtkTextTagTable *tag_table;
  GtkTextBuffer   *content;
  GtkTextIter      insert;
  GtkTextIter      start, end;
  gchar           *markup;
  gsize            length;

  g_return_val_if_fail (PICMAN_IS_TEXT_BUFFER (buffer), NULL);

  tag_table = gtk_text_buffer_get_tag_table (GTK_TEXT_BUFFER (buffer));
  content = gtk_text_buffer_new (tag_table);

  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (buffer), &start, &end);
  gtk_text_buffer_get_start_iter (content, &insert);

  gtk_text_buffer_insert_range (content, &insert, &start, &end);

  picman_text_buffer_pre_serialize (buffer, content);

  gtk_text_buffer_get_bounds (content, &start, &end);

  markup = (gchar *) gtk_text_buffer_serialize (GTK_TEXT_BUFFER (buffer),
                                                content,
                                                buffer->markup_atom,
                                                &start, &end,
                                                &length);

  g_object_unref (content);

  return markup;
}

gboolean
picman_text_buffer_has_markup (PicmanTextBuffer *buffer)
{
  GtkTextIter iter;

  g_return_val_if_fail (PICMAN_IS_TEXT_BUFFER (buffer), FALSE);

  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (buffer), &iter);

  do
    {
      GSList *tags = gtk_text_iter_get_tags (&iter);

      if (tags)
        {
          g_slist_free (tags);
          return TRUE;
        }
    }
  while (gtk_text_iter_forward_char (&iter));

  return FALSE;
}

GtkTextTag *
picman_text_buffer_get_iter_size (PicmanTextBuffer    *buffer,
                                const GtkTextIter *iter,
                                gint              *size)
{
  GList *list;

  for (list = buffer->size_tags; list; list = g_list_next (list))
    {
      GtkTextTag *tag = list->data;

      if (gtk_text_iter_has_tag (iter, tag))
        {
          if (size)
            *size = picman_text_tag_get_size (tag);

          return tag;
        }
    }

  if (size)
    *size = 0;

  return NULL;
}

GtkTextTag *
picman_text_buffer_get_size_tag (PicmanTextBuffer *buffer,
                               gint            size)
{
  GList      *list;
  GtkTextTag *tag;
  gchar       name[32];

  for (list = buffer->size_tags; list; list = g_list_next (list))
    {
      tag = list->data;

      if (size == picman_text_tag_get_size (tag))
        return tag;
    }

  g_snprintf (name, sizeof (name), "size-%d", size);

  tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer),
                                    name,
                                    PICMAN_TEXT_PROP_NAME_SIZE, size,
                                    NULL);

  buffer->size_tags = g_list_prepend (buffer->size_tags, tag);

  return tag;
}

void
picman_text_buffer_set_size (PicmanTextBuffer    *buffer,
                           const GtkTextIter *start,
                           const GtkTextIter *end,
                           gint               size)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);

  if (gtk_text_iter_equal (start, end))
    return;

  gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (buffer));

  for (list = buffer->size_tags; list; list = g_list_next (list))
    {
      gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (buffer), list->data,
                                  start, end);
    }

  if (size != 0)
    {
      GtkTextTag *tag;

      tag = picman_text_buffer_get_size_tag (buffer, size);

      gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer), tag,
                                 start, end);
    }

  gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (buffer));
}

void
picman_text_buffer_change_size (PicmanTextBuffer    *buffer,
                              const GtkTextIter *start,
                              const GtkTextIter *end,
                              gint               count)
{
  GtkTextIter  iter;
  GtkTextIter  span_start;
  GtkTextIter  span_end;
  GtkTextTag  *span_tag;
  gint         span_size;

  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);

  if (gtk_text_iter_equal (start, end))
    return;

  iter       = *start;
  span_start = *start;
  span_tag   = picman_text_buffer_get_iter_size (buffer, &iter,
                                               &span_size);

  gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (buffer));

  do
    {
      GtkTextTag *iter_tag;
      gint        iter_size;

      gtk_text_iter_forward_char (&iter);

      iter_tag = picman_text_buffer_get_iter_size (buffer, &iter,
                                                 &iter_size);

      span_end = iter;

      if (iter_size != span_size ||
          gtk_text_iter_compare (&iter, end) >= 0)
        {
          if (span_size != 0)
            {
              gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (buffer), span_tag,
                                          &span_start, &span_end);
            }

          if ((span_size + count) > 0)
            {
              span_tag = picman_text_buffer_get_size_tag (buffer,
                                                        span_size + count);

              gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer), span_tag,
                                         &span_start, &span_end);
            }

          span_start = iter;
          span_size  = iter_size;
          span_tag   = iter_tag;
        }

      /* We might have moved too far */
      if (gtk_text_iter_compare (&iter, end) > 0)
        iter = *end;
    }
  while (! gtk_text_iter_equal (&iter, end));

  gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (buffer));
}

GtkTextTag *
picman_text_buffer_get_iter_baseline (PicmanTextBuffer    *buffer,
                                    const GtkTextIter *iter,
                                    gint              *baseline)
{
  GList *list;

  for (list = buffer->baseline_tags; list; list = g_list_next (list))
    {
      GtkTextTag *tag = list->data;

      if (gtk_text_iter_has_tag (iter, tag))
        {
          if (baseline)
            *baseline = picman_text_tag_get_baseline (tag);

          return tag;
        }
    }

  if (baseline)
    *baseline = 0;

  return NULL;
}

static GtkTextTag *
picman_text_buffer_get_baseline_tag (PicmanTextBuffer *buffer,
                                   gint            baseline)
{
  GList      *list;
  GtkTextTag *tag;
  gchar       name[32];

  for (list = buffer->baseline_tags; list; list = g_list_next (list))
    {
      tag = list->data;

      if (baseline == picman_text_tag_get_baseline (tag))
        return tag;
    }

  g_snprintf (name, sizeof (name), "baseline-%d", baseline);

  tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer),
                                    name,
                                    PICMAN_TEXT_PROP_NAME_BASELINE, baseline,
                                    NULL);

  buffer->baseline_tags = g_list_prepend (buffer->baseline_tags, tag);

  return tag;
}

void
picman_text_buffer_set_baseline (PicmanTextBuffer    *buffer,
                               const GtkTextIter *start,
                               const GtkTextIter *end,
                               gint               baseline)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);

  if (gtk_text_iter_equal (start, end))
    return;

  gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (buffer));

  for (list = buffer->baseline_tags; list; list = g_list_next (list))
    {
      gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (buffer), list->data,
                                  start, end);
    }

  if (baseline != 0)
    {
      GtkTextTag *tag;

      tag = picman_text_buffer_get_baseline_tag (buffer, baseline);

      gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer), tag,
                                 start, end);
    }

  gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (buffer));
}

void
picman_text_buffer_change_baseline (PicmanTextBuffer    *buffer,
                                  const GtkTextIter *start,
                                  const GtkTextIter *end,
                                  gint               count)
{
  GtkTextIter  iter;
  GtkTextIter  span_start;
  GtkTextIter  span_end;
  GtkTextTag  *span_tag;
  gint         span_baseline;

  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);

  if (gtk_text_iter_equal (start, end))
    return;

  iter       = *start;
  span_start = *start;
  span_tag   = picman_text_buffer_get_iter_baseline (buffer, &iter,
                                                   &span_baseline);

  gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (buffer));

  do
    {
      GtkTextTag *iter_tag;
      gint        iter_baseline;

      gtk_text_iter_forward_char (&iter);

      iter_tag = picman_text_buffer_get_iter_baseline (buffer, &iter,
                                                     &iter_baseline);

      span_end = iter;

      if (iter_baseline != span_baseline ||
          gtk_text_iter_compare (&iter, end) >= 0)
        {
          if (span_baseline != 0)
            {
              gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (buffer), span_tag,
                                          &span_start, &span_end);
            }

          if (span_baseline + count != 0)
            {
              span_tag = picman_text_buffer_get_baseline_tag (buffer,
                                                            span_baseline + count);

              gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer), span_tag,
                                         &span_start, &span_end);
            }

          span_start    = iter;
          span_baseline = iter_baseline;
          span_tag      = iter_tag;
        }

      /* We might have moved too far */
      if (gtk_text_iter_compare (&iter, end) > 0)
        iter = *end;
    }
  while (! gtk_text_iter_equal (&iter, end));

  gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (buffer));
}

GtkTextTag *
picman_text_buffer_get_iter_kerning (PicmanTextBuffer    *buffer,
                                   const GtkTextIter *iter,
                                   gint              *kerning)
{
  GList *list;

  for (list = buffer->kerning_tags; list; list = g_list_next (list))
    {
      GtkTextTag *tag = list->data;

      if (gtk_text_iter_has_tag (iter, tag))
        {
          if (kerning)
            *kerning = picman_text_tag_get_kerning (tag);

          return tag;
        }
    }

  if (kerning)
    *kerning = 0;

  return NULL;
}

static GtkTextTag *
picman_text_buffer_get_kerning_tag (PicmanTextBuffer *buffer,
                                  gint            kerning)
{
  GList      *list;
  GtkTextTag *tag;
  gchar       name[32];

  for (list = buffer->kerning_tags; list; list = g_list_next (list))
    {
      tag = list->data;

      if (kerning == picman_text_tag_get_kerning (tag))
        return tag;
    }

  g_snprintf (name, sizeof (name), "kerning-%d", kerning);

  tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer),
                                    name,
                                    PICMAN_TEXT_PROP_NAME_KERNING, kerning,
                                    NULL);

  buffer->kerning_tags = g_list_prepend (buffer->kerning_tags, tag);

  return tag;
}

void
picman_text_buffer_set_kerning (PicmanTextBuffer    *buffer,
                              const GtkTextIter *start,
                              const GtkTextIter *end,
                              gint               kerning)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);

  if (gtk_text_iter_equal (start, end))
    return;

  gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (buffer));

  for (list = buffer->kerning_tags; list; list = g_list_next (list))
    {
      gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (buffer), list->data,
                                  start, end);
    }

  if (kerning != 0)
    {
      GtkTextTag *tag;

      tag = picman_text_buffer_get_kerning_tag (buffer, kerning);

      gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer), tag,
                                 start, end);
    }

  gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (buffer));
}

void
picman_text_buffer_change_kerning (PicmanTextBuffer    *buffer,
                                 const GtkTextIter *start,
                                 const GtkTextIter *end,
                                 gint               count)
{
  GtkTextIter  iter;
  GtkTextIter  span_start;
  GtkTextIter  span_end;
  GtkTextTag  *span_tag;
  gint         span_kerning;

  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);

  if (gtk_text_iter_equal (start, end))
    return;

  iter       = *start;
  span_start = *start;
  span_tag   = picman_text_buffer_get_iter_kerning (buffer, &iter,
                                                  &span_kerning);

  gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (buffer));

  do
    {
      GtkTextTag *iter_tag;
      gint        iter_kerning;

      gtk_text_iter_forward_char (&iter);

      iter_tag = picman_text_buffer_get_iter_kerning (buffer, &iter,
                                                    &iter_kerning);

      span_end = iter;

      if (iter_kerning != span_kerning ||
          gtk_text_iter_compare (&iter, end) >= 0)
        {
          if (span_kerning != 0)
            {
              gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (buffer), span_tag,
                                          &span_start, &span_end);
            }

          if (span_kerning + count != 0)
            {
              span_tag = picman_text_buffer_get_kerning_tag (buffer,
                                                           span_kerning + count);

              gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer), span_tag,
                                         &span_start, &span_end);
            }

          span_start   = iter;
          span_kerning = iter_kerning;
          span_tag     = iter_tag;
        }

      /* We might have moved too far */
      if (gtk_text_iter_compare (&iter, end) > 0)
        iter = *end;
    }
  while (! gtk_text_iter_equal (&iter, end));

  gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (buffer));
}

GtkTextTag *
picman_text_buffer_get_iter_font (PicmanTextBuffer     *buffer,
                                const GtkTextIter  *iter,
                                gchar             **font)
{
  GList *list;

  for (list = buffer->font_tags; list; list = g_list_next (list))
    {
      GtkTextTag *tag = list->data;

      if (gtk_text_iter_has_tag (iter, tag))
        {
          if (font)
            *font = picman_text_tag_get_font (tag);

          return tag;
        }
    }

  if (font)
    *font = NULL;

  return NULL;
}

GtkTextTag *
picman_text_buffer_get_font_tag (PicmanTextBuffer *buffer,
                               const gchar    *font)
{
  GList      *list;
  GtkTextTag *tag;
  gchar       name[256];

  for (list = buffer->font_tags; list; list = g_list_next (list))
    {
      gchar *tag_font;

      tag = list->data;

      tag_font = picman_text_tag_get_font (tag);

      if (! strcmp (font, tag_font))
        {
          g_free (tag_font);
          return tag;
        }

      g_free (tag_font);
    }

  g_snprintf (name, sizeof (name), "font-%s", font);

  tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer),
                                    name,
                                    "font", font,
                                    NULL);

  buffer->font_tags = g_list_prepend (buffer->font_tags, tag);

  return tag;
}

void
picman_text_buffer_set_font (PicmanTextBuffer    *buffer,
                           const GtkTextIter *start,
                           const GtkTextIter *end,
                           const gchar       *font)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);

  if (gtk_text_iter_equal (start, end))
    return;

  gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (buffer));

  for (list = buffer->font_tags; list; list = g_list_next (list))
    {
      gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (buffer), list->data,
                                  start, end);
    }

  if (font)
    {
      GtkTextTag *tag = picman_text_buffer_get_font_tag (buffer, font);

      gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer), tag,
                                 start, end);
    }

  gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (buffer));
}

GtkTextTag *
picman_text_buffer_get_iter_color (PicmanTextBuffer    *buffer,
                                 const GtkTextIter *iter,
                                 PicmanRGB           *color)
{
  GList *list;

  for (list = buffer->color_tags; list; list = g_list_next (list))
    {
      GtkTextTag *tag = list->data;

      if (gtk_text_iter_has_tag (iter, tag))
        {
          if (color)
            picman_text_tag_get_color (tag, color);

          return tag;
        }
    }

  return NULL;
}

GtkTextTag *
picman_text_buffer_get_color_tag (PicmanTextBuffer *buffer,
                                const PicmanRGB  *color)
{
  GList      *list;
  GtkTextTag *tag;
  gchar       name[256];
  GdkColor    gdk_color;
  guchar      r, g, b;

  picman_rgb_get_uchar (color, &r, &g, &b);

  for (list = buffer->color_tags; list; list = g_list_next (list))
    {
      PicmanRGB tag_color;
      guchar  tag_r, tag_g, tag_b;

      tag = list->data;

      picman_text_tag_get_color (tag, &tag_color);

      picman_rgb_get_uchar (&tag_color, &tag_r, &tag_g, &tag_b);

      /* Do not compare the alpha channel, since it's unused */
      if (tag_r == r &&
          tag_g == g &&
          tag_b == b)
        {
          return tag;
        }
    }

  g_snprintf (name, sizeof (name), "color-#%02x%02x%02x",
              r, g, b);

  picman_rgb_get_gdk_color (color, &gdk_color);

  tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer),
                                    name,
                                    "foreground-gdk", &gdk_color,
                                    "foreground-set", TRUE,
                                    NULL);

  buffer->color_tags = g_list_prepend (buffer->color_tags, tag);

  return tag;
}

void
picman_text_buffer_set_color (PicmanTextBuffer    *buffer,
                            const GtkTextIter *start,
                            const GtkTextIter *end,
                            const PicmanRGB     *color)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);

  if (gtk_text_iter_equal (start, end))
    return;

  gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (buffer));

  for (list = buffer->color_tags; list; list = g_list_next (list))
    {
      gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (buffer), list->data,
                                  start, end);
    }

  if (color)
    {
      GtkTextTag *tag = picman_text_buffer_get_color_tag (buffer, color);

      gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer), tag,
                                 start, end);
    }

  gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (buffer));
}

/*  Pango markup attribute names  */

#define PICMAN_TEXT_ATTR_NAME_SIZE     "size"
#define PICMAN_TEXT_ATTR_NAME_BASELINE "rise"
#define PICMAN_TEXT_ATTR_NAME_KERNING  "letter_spacing"
#define PICMAN_TEXT_ATTR_NAME_FONT     "font"
#define PICMAN_TEXT_ATTR_NAME_COLOR    "foreground"

const gchar *
picman_text_buffer_tag_to_name (PicmanTextBuffer  *buffer,
                              GtkTextTag      *tag,
                              const gchar    **attribute,
                              gchar          **value)
{
  g_return_val_if_fail (PICMAN_IS_TEXT_BUFFER (buffer), NULL);
  g_return_val_if_fail (GTK_IS_TEXT_TAG (tag), NULL);

  if (attribute)
    *attribute = NULL;

  if (value)
    *value = NULL;

  if (tag == buffer->bold_tag)
    {
      return "b";
    }
  else if (tag == buffer->italic_tag)
    {
      return "i";
    }
  else if (tag == buffer->underline_tag)
    {
      return "u";
    }
  else if (tag == buffer->strikethrough_tag)
    {
      return "s";
    }
  else if (g_list_find (buffer->size_tags, tag))
    {
      if (attribute)
        *attribute = PICMAN_TEXT_ATTR_NAME_SIZE;

      if (value)
        *value = g_strdup_printf ("%d", picman_text_tag_get_size (tag));

      return "span";
    }
  else if (g_list_find (buffer->baseline_tags, tag))
    {
      if (attribute)
        *attribute = PICMAN_TEXT_ATTR_NAME_BASELINE;

      if (value)
        *value = g_strdup_printf ("%d", picman_text_tag_get_baseline (tag));

      return "span";
    }
  else if (g_list_find (buffer->kerning_tags, tag))
    {
      if (attribute)
        *attribute = PICMAN_TEXT_ATTR_NAME_KERNING;

      if (value)
        *value = g_strdup_printf ("%d", picman_text_tag_get_kerning (tag));

      return "span";
    }
  else if (g_list_find (buffer->font_tags, tag))
    {
      if (attribute)
        *attribute = PICMAN_TEXT_ATTR_NAME_FONT;

      if (value)
        *value = picman_text_tag_get_font (tag);

      return "span";
    }
  else if (g_list_find (buffer->color_tags, tag))
    {
      if (attribute)
        *attribute = PICMAN_TEXT_ATTR_NAME_COLOR;

      if (value)
        {
          PicmanRGB color;
          guchar  r, g, b;

          picman_text_tag_get_color (tag, &color);
          picman_rgb_get_uchar (&color, &r, &g, &b);

          *value = g_strdup_printf ("#%02x%02x%02x", r, g, b);
        }

      return "span";
    }

  return NULL;
}

GtkTextTag *
picman_text_buffer_name_to_tag (PicmanTextBuffer *buffer,
                              const gchar    *name,
                              const gchar    *attribute,
                              const gchar    *value)
{
  g_return_val_if_fail (PICMAN_IS_TEXT_BUFFER (buffer), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  if (! strcmp (name, "b"))
    {
      return buffer->bold_tag;
    }
  else if (! strcmp (name, "i"))
    {
      return buffer->italic_tag;
    }
  else if (! strcmp (name, "u"))
    {
      return buffer->underline_tag;
    }
  else if (! strcmp (name, "s"))
    {
      return buffer->strikethrough_tag;
    }
  else if (! strcmp (name, "span") &&
           attribute != NULL       &&
           value     != NULL)
    {
      if (! strcmp (attribute, PICMAN_TEXT_ATTR_NAME_SIZE))
        {
          return picman_text_buffer_get_size_tag (buffer, atoi (value));
        }
      else if (! strcmp (attribute, PICMAN_TEXT_ATTR_NAME_BASELINE))
        {
          return picman_text_buffer_get_baseline_tag (buffer, atoi (value));
        }
      else if (! strcmp (attribute, PICMAN_TEXT_ATTR_NAME_KERNING))
        {
          return picman_text_buffer_get_kerning_tag (buffer, atoi (value));
        }
      else if (! strcmp (attribute, PICMAN_TEXT_ATTR_NAME_FONT))
        {
          return picman_text_buffer_get_font_tag (buffer, value);
        }
      else if (! strcmp (attribute, PICMAN_TEXT_ATTR_NAME_COLOR))
        {
          PicmanRGB color;
          guint   r, g, b;

          sscanf (value, "#%02x%02x%02x", &r, &g, &b);

          picman_rgb_set_uchar (&color, r, g, b);

          return picman_text_buffer_get_color_tag (buffer, &color);
        }
    }

  return NULL;
}

void
picman_text_buffer_set_insert_tags (PicmanTextBuffer *buffer,
                                  GList          *insert_tags,
                                  GList          *remove_tags)
{
  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));

  buffer->insert_tags_set = TRUE;

  g_list_free (buffer->insert_tags);
  g_list_free (buffer->remove_tags);
  buffer->insert_tags = insert_tags;
  buffer->remove_tags = remove_tags;
}

void
picman_text_buffer_clear_insert_tags (PicmanTextBuffer *buffer)
{
  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));

  buffer->insert_tags_set = FALSE;

  g_list_free (buffer->insert_tags);
  g_list_free (buffer->remove_tags);
  buffer->insert_tags = NULL;
  buffer->remove_tags = NULL;
}

void
picman_text_buffer_insert (PicmanTextBuffer *buffer,
                         const gchar    *text)
{
  GtkTextIter  iter, start;
  gint         start_offset;
  gboolean     insert_tags_set;
  GList       *insert_tags;
  GList       *remove_tags;
  GSList      *tags_off = NULL;

  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));

  gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer), &iter,
                                    gtk_text_buffer_get_insert (GTK_TEXT_BUFFER (buffer)));

  start_offset = gtk_text_iter_get_offset (&iter);

  insert_tags_set = buffer->insert_tags_set;
  insert_tags     = buffer->insert_tags;
  remove_tags     = buffer->remove_tags;

  buffer->insert_tags_set = FALSE;
  buffer->insert_tags     = NULL;
  buffer->remove_tags     = NULL;

  tags_off = gtk_text_iter_get_toggled_tags (&iter, FALSE);

  gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (buffer));

  gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &iter, text, -1);

  gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (buffer), &start,
                                      start_offset);

  if (insert_tags_set)
    {
      GList *list;

      for (list = remove_tags; list; list = g_list_next (list))
        {
          GtkTextTag *tag = list->data;

          gtk_text_buffer_remove_tag (GTK_TEXT_BUFFER (buffer), tag,
                                      &start, &iter);
        }

      for (list = insert_tags; list; list = g_list_next (list))
        {
          GtkTextTag *tag = list->data;

          gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer), tag,
                                     &start, &iter);
        }
    }

  if (tags_off)
    {
      GSList *slist;

      for (slist = tags_off; slist; slist = g_slist_next (slist))
        {
          GtkTextTag *tag = slist->data;

          if (! g_list_find (remove_tags, tag) &&
              ! g_list_find (buffer->kerning_tags, tag))
            {
              gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer), tag,
                                         &start, &iter);
            }
        }

      g_slist_free (tags_off);
    }

  g_list_free (remove_tags);
  g_list_free (insert_tags);

  gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (buffer));
}

gint
picman_text_buffer_get_iter_index (PicmanTextBuffer *buffer,
                                 GtkTextIter    *iter,
                                 gboolean        layout_index)
{
  GtkTextIter  start;
  gchar       *string;
  gint         index;

  g_return_val_if_fail (PICMAN_IS_TEXT_BUFFER (buffer), 0);

  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (buffer), &start);

  string = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (buffer),
                                     &start, iter, TRUE);
  index = strlen (string);
  g_free (string);

  if (layout_index)
    {
      do
        {
          GSList *tags = gtk_text_iter_get_tags (&start);
          GSList *list;

          for (list = tags; list; list = g_slist_next (list))
            {
              GtkTextTag *tag = list->data;

              if (g_list_find (buffer->kerning_tags, tag))
                {
                  index += WORD_JOINER_LENGTH;

                  break;
                }
            }

          g_slist_free (tags);

          gtk_text_iter_forward_char (&start);

          /* We might have moved too far */
          if (gtk_text_iter_compare (&start, iter) > 0)
            start = *iter;
        }
      while (! gtk_text_iter_equal (&start, iter));
    }

  return index;
}

void
picman_text_buffer_get_iter_at_index (PicmanTextBuffer *buffer,
                                    GtkTextIter    *iter,
                                    gint            index,
                                    gboolean        layout_index)
{
  GtkTextIter  start;
  GtkTextIter  end;
  gchar       *string;

  g_return_if_fail (PICMAN_IS_TEXT_BUFFER (buffer));

  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (buffer), &start, &end);

  string = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (buffer),
                                     &start, &end, TRUE);

  if (layout_index)
    {
      gchar *my_string = string;
      gint   my_index  = 0;
      gchar *tmp;

      do
        {
          GSList *tags = gtk_text_iter_get_tags (&start);
          GSList *list;

          tmp = g_utf8_next_char (my_string);
          my_index += (tmp - my_string);
          my_string = tmp;

          for (list = tags; list; list = g_slist_next (list))
            {
              GtkTextTag *tag = list->data;

              if (g_list_find (buffer->kerning_tags, tag))
                {
                  index = MAX (0, index - WORD_JOINER_LENGTH);

                  break;
                }
            }

          g_slist_free (tags);

          gtk_text_iter_forward_char (&start);

          /* We might have moved too far */
          if (gtk_text_iter_compare (&start, &end) > 0)
            start = end;
        }
      while (my_index < index &&
             ! gtk_text_iter_equal (&start, &end));
    }

  string[index] = '\0';

  gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (buffer), iter,
                                      g_utf8_strlen (string, -1));

  g_free (string);
}

gboolean
picman_text_buffer_load (PicmanTextBuffer *buffer,
                       const gchar    *filename,
                       GError        **error)
{
  FILE        *file;
  gchar        buf[2048];
  gint         remaining = 0;
  GtkTextIter  iter;

  g_return_val_if_fail (PICMAN_IS_TEXT_BUFFER (buffer), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  file = g_fopen (filename, "r");

  if (! file)
    {
      g_set_error_literal (error, G_FILE_ERROR,
                           g_file_error_from_errno (errno),
                           g_strerror (errno));
      return FALSE;
    }

  gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (buffer));

  picman_text_buffer_set_text (buffer, NULL);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (buffer), &iter);

  while (! feof (file))
    {
      const char *leftover;
      gint        count;
      gint        to_read = sizeof (buf) - remaining - 1;

      count = fread (buf + remaining, 1, to_read, file);
      buf[count + remaining] = '\0';

      g_utf8_validate (buf, count + remaining, &leftover);

      gtk_text_buffer_insert (GTK_TEXT_BUFFER (buffer), &iter,
                              buf, leftover - buf);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (buffer), &iter);

      remaining = (buf + remaining + count) - leftover;
      g_memmove (buf, leftover, remaining);

      if (remaining > 6 || count < to_read)
        break;
    }

  if (remaining)
    g_message (_("Invalid UTF-8 data in file '%s'."),
               picman_filename_to_utf8 (filename));

  fclose (file);

  gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (buffer));

  return TRUE;
}

gboolean
picman_text_buffer_save (PicmanTextBuffer *buffer,
                       const gchar    *filename,
                       gboolean        selection_only,
                       GError        **error)
{
  GtkTextIter  start_iter;
  GtkTextIter  end_iter;
  gint         fd;
  gchar       *text_contents;

  g_return_val_if_fail (PICMAN_IS_TEXT_BUFFER (buffer), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  fd = g_open (filename, O_WRONLY | O_CREAT | O_APPEND, 0666);

  if (fd == -1)
    {
      g_set_error_literal (error, G_FILE_ERROR,
                           g_file_error_from_errno (errno),
                           g_strerror (errno));
      return FALSE;
    }

  if (selection_only)
    gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (buffer),
                                          &start_iter, &end_iter);
  else
    gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (buffer),
                                &start_iter, &end_iter);

  text_contents = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (buffer),
                                            &start_iter, &end_iter, TRUE);

  if (text_contents)
    {
      gint text_length = strlen (text_contents);

      if (text_length > 0)
        {
          gint bytes_written;

          bytes_written = write (fd, text_contents, text_length);

          if (bytes_written != text_length)
            {
              g_free (text_contents);
              close (fd);
              g_set_error_literal (error, G_FILE_ERROR,
                                   g_file_error_from_errno (errno),
                                   g_strerror (errno));
              return FALSE;
            }
        }

      g_free (text_contents);
    }

  close (fd);

  return TRUE;
}
