/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * tips-parser.h - Parse the picman-tips.xml file.
 * Copyright (C) 2002, 2008  Sven Neumann <sven@picman.org>
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

#ifndef __TIPS_PARSER_H__
#define __TIPS_PARSER_H__


typedef struct _PicmanTip PicmanTip;

struct _PicmanTip
{
  gchar *text;
  gchar *help_id;
};


PicmanTip * picman_tip_new        (const gchar  *title,
                               const gchar  *format,
                               ...) G_GNUC_PRINTF(2, 3);
void      picman_tip_free       (PicmanTip      *tip);

GList   * picman_tips_from_file (const gchar  *filename,
                               GError      **error);
void      picman_tips_free      (GList        *tips);


#endif /* __TIPS_PARSER_H__ */
