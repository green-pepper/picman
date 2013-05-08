/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpatternclipboard.h
 * Copyright (C) 2006 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_PATTERN_CLIPBOARD_H__
#define __PICMAN_PATTERN_CLIPBOARD_H__


#include "picmanpattern.h"


#define PICMAN_TYPE_PATTERN_CLIPBOARD            (picman_pattern_clipboard_get_type ())
#define PICMAN_PATTERN_CLIPBOARD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PATTERN_CLIPBOARD, PicmanPatternClipboard))
#define PICMAN_PATTERN_CLIPBOARD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PATTERN_CLIPBOARD, PicmanPatternClipboardClass))
#define PICMAN_IS_PATTERN_CLIPBOARD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PATTERN_CLIPBOARD))
#define PICMAN_IS_PATTERN_CLIPBOARD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PATTERN_CLIPBOARD))
#define PICMAN_PATTERN_CLIPBOARD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PATTERN_CLIPBOARD, PicmanPatternClipboardClass))


typedef struct _PicmanPatternClipboardClass PicmanPatternClipboardClass;

struct _PicmanPatternClipboard
{
  PicmanPattern  parent_instance;

  Picman        *picman;
};

struct _PicmanPatternClipboardClass
{
  PicmanPatternClass  parent_class;
};


GType      picman_pattern_clipboard_get_type (void) G_GNUC_CONST;

PicmanData * picman_pattern_clipboard_new      (Picman *picman);


#endif  /*  __PICMAN_PATTERN_CLIPBOARD_H__  */
