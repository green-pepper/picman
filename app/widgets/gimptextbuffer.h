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

#ifndef __PICMAN_TEXT_BUFFER_H__
#define __PICMAN_TEXT_BUFFER_H__


#define PICMAN_TYPE_TEXT_BUFFER            (picman_text_buffer_get_type ())
#define PICMAN_TEXT_BUFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEXT_BUFFER, PicmanTextBuffer))
#define PICMAN_IS_TEXT_BUFFER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEXT_BUFFER))
#define PICMAN_TEXT_BUFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TEXT_BUFFER, PicmanTextBufferClass))
#define PICMAN_IS_TEXT_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TEXT_BUFFER))


typedef struct _PicmanTextBufferClass  PicmanTextBufferClass;

struct _PicmanTextBuffer
{
  GtkTextBuffer  parent_instance;

  GtkTextTag    *bold_tag;
  GtkTextTag    *italic_tag;
  GtkTextTag    *underline_tag;
  GtkTextTag    *strikethrough_tag;

  GList         *size_tags;
  GList         *baseline_tags;
  GList         *kerning_tags;
  GList         *font_tags;
  GList         *color_tags;

  gboolean       insert_tags_set;
  GList         *insert_tags;
  GList         *remove_tags;

  GdkAtom        markup_atom;
};

struct _PicmanTextBufferClass
{
  GtkTextBufferClass  parent_class;
};


GType            picman_text_buffer_get_type          (void) G_GNUC_CONST;

PicmanTextBuffer * picman_text_buffer_new               (void);

void             picman_text_buffer_set_text          (PicmanTextBuffer    *buffer,
                                                     const gchar       *text);
gchar          * picman_text_buffer_get_text          (PicmanTextBuffer    *buffer);

void             picman_text_buffer_set_markup        (PicmanTextBuffer    *buffer,
                                                     const gchar       *markup);
gchar          * picman_text_buffer_get_markup        (PicmanTextBuffer    *buffer);

gboolean         picman_text_buffer_has_markup        (PicmanTextBuffer    *buffer);

GtkTextTag     * picman_text_buffer_get_iter_size     (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *iter,
                                                     gint              *size);
GtkTextTag     * picman_text_buffer_get_size_tag      (PicmanTextBuffer    *buffer,
                                                     gint               size);
void             picman_text_buffer_set_size          (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *start,
                                                     const GtkTextIter *end,
                                                     gint               size);
void             picman_text_buffer_change_size       (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *start,
                                                     const GtkTextIter *end,
                                                     gint               amount);

GtkTextTag     * picman_text_buffer_get_iter_baseline (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *iter,
                                                     gint              *baseline);
void             picman_text_buffer_set_baseline      (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *start,
                                                     const GtkTextIter *end,
                                                     gint               count);
void             picman_text_buffer_change_baseline   (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *start,
                                                     const GtkTextIter *end,
                                                     gint               count);

GtkTextTag     * picman_text_buffer_get_iter_kerning  (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *iter,
                                                     gint              *kerning);
void             picman_text_buffer_set_kerning       (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *start,
                                                     const GtkTextIter *end,
                                                     gint               count);
void             picman_text_buffer_change_kerning    (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *start,
                                                     const GtkTextIter *end,
                                                     gint               count);

GtkTextTag     * picman_text_buffer_get_iter_font     (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *iter,
                                                     gchar            **font);
GtkTextTag     * picman_text_buffer_get_font_tag      (PicmanTextBuffer    *buffer,
                                                     const gchar       *font);
void             picman_text_buffer_set_font          (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *start,
                                                     const GtkTextIter *end,
                                                     const gchar       *font);

GtkTextTag     * picman_text_buffer_get_iter_color    (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *iter,
                                                     PicmanRGB           *color);
GtkTextTag     * picman_text_buffer_get_color_tag     (PicmanTextBuffer    *buffer,
                                                     const PicmanRGB     *color);
void             picman_text_buffer_set_color         (PicmanTextBuffer    *buffer,
                                                     const GtkTextIter *start,
                                                     const GtkTextIter *end,
                                                     const PicmanRGB     *color);

const gchar    * picman_text_buffer_tag_to_name       (PicmanTextBuffer    *buffer,
                                                     GtkTextTag        *tag,
                                                     const gchar      **attribute,
                                                     gchar            **value);
GtkTextTag     * picman_text_buffer_name_to_tag       (PicmanTextBuffer    *buffer,
                                                     const gchar       *name,
                                                     const gchar       *attribute,
                                                     const gchar       *value);

void             picman_text_buffer_set_insert_tags   (PicmanTextBuffer    *buffer,
                                                     GList             *insert_tags,
                                                     GList             *remove_tags);
void             picman_text_buffer_clear_insert_tags (PicmanTextBuffer    *buffer);
void             picman_text_buffer_insert            (PicmanTextBuffer    *buffer,
                                                     const gchar       *text);

gint             picman_text_buffer_get_iter_index    (PicmanTextBuffer    *buffer,
                                                     GtkTextIter       *iter,
                                                     gboolean           layout_index);
void             picman_text_buffer_get_iter_at_index (PicmanTextBuffer    *buffer,
                                                     GtkTextIter       *iter,
                                                     gint               index,
                                                     gboolean           layout_index);

gboolean         picman_text_buffer_load              (PicmanTextBuffer    *buffer,
                                                     const gchar       *filename,
                                                     GError           **error);
gboolean         picman_text_buffer_save              (PicmanTextBuffer    *buffer,
                                                     const gchar       *filename,
                                                     gboolean           selection_only,
                                                     GError           **error);


#endif /* __PICMAN_TEXT_BUFFER_H__ */
