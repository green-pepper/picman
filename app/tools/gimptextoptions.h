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

#ifndef __PICMAN_TEXT_OPTIONS_H__
#define __PICMAN_TEXT_OPTIONS_H__


#include "core/picmantooloptions.h"


#define PICMAN_TYPE_TEXT_OPTIONS            (picman_text_options_get_type ())
#define PICMAN_TEXT_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEXT_OPTIONS, PicmanTextOptions))
#define PICMAN_TEXT_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TEXT_OPTIONS, PicmanTextOptionsClass))
#define PICMAN_IS_TEXT_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEXT_OPTIONS))
#define PICMAN_IS_TEXT_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TEXT_OPTIONS))
#define PICMAN_TEXT_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TEXT_OPTIONS, PicmanTextOptionsClass))


typedef struct _PicmanTextOptions      PicmanTextOptions;
typedef struct _PicmanToolOptionsClass PicmanTextOptionsClass;

struct _PicmanTextOptions
{
  PicmanToolOptions        tool_options;

  PicmanUnit               unit;
  gdouble                font_size;
  gboolean               antialias;
  PicmanTextHintStyle      hint_style;
  gchar                 *language;
  PicmanTextDirection      base_dir;
  PicmanTextJustification  justify;
  gdouble                indent;
  gdouble                line_spacing;
  gdouble                letter_spacing;
  PicmanTextBoxMode        box_mode;

  PicmanViewType           font_view_type;
  PicmanViewSize           font_view_size;

  gboolean               use_editor;

  /*  options gui  */
  GtkWidget             *size_entry;
};


GType       picman_text_options_get_type     (void) G_GNUC_CONST;

void        picman_text_options_connect_text (PicmanTextOptions *options,
                                            PicmanText        *text);

GtkWidget * picman_text_options_gui          (PicmanToolOptions *tool_options);

GtkWidget * picman_text_options_editor_new   (GtkWindow       *parent,
                                            Picman            *picman,
                                            PicmanTextOptions *options,
                                            PicmanMenuFactory *menu_factory,
                                            const gchar     *title,
                                            PicmanText        *text,
                                            PicmanTextBuffer  *text_buffer,
                                            gdouble          xres,
                                            gdouble          yres);


#endif /* __PICMAN_TEXT_OPTIONS_H__ */
