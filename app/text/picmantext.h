/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanText
 * Copyright (C) 2002-2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_TEXT_H__
#define __PICMAN_TEXT_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_TEXT            (picman_text_get_type ())
#define PICMAN_TEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEXT, PicmanText))
#define PICMAN_TEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TEXT, PicmanTextClass))
#define PICMAN_IS_TEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEXT))
#define PICMAN_IS_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TEXT))
#define PICMAN_TEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TEXT, PicmanTextClass))


typedef struct _PicmanTextClass  PicmanTextClass;

struct _PicmanText
{
  PicmanObject             parent_instance;

  gchar                 *text;
  gchar                 *markup;
  gchar                 *font;
  PicmanUnit               unit;
  gdouble                font_size;
  gboolean               antialias;
  PicmanTextHintStyle      hint_style;
  gboolean               kerning;
  gchar                 *language;
  PicmanTextDirection      base_dir;
  PicmanRGB                color;
  PicmanTextOutline        outline;
  PicmanTextJustification  justify;
  gdouble                indent;
  gdouble                line_spacing;
  gdouble                letter_spacing;
  PicmanTextBoxMode        box_mode;
  gdouble                box_width;
  gdouble                box_height;
  PicmanUnit               box_unit;
  PicmanMatrix2            transformation;
  gdouble                offset_x;
  gdouble                offset_y;

  gdouble                border;
};

struct _PicmanTextClass
{
  PicmanObjectClass        parent_class;

  void (* changed) (PicmanText *text);
};


GType  picman_text_get_type           (void) G_GNUC_CONST;

void   picman_text_get_transformation (PicmanText    *text,
                                     PicmanMatrix3 *matrix);


#endif /* __PICMAN_TEXT_H__ */
