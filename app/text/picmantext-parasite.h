/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanText
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_TEXT_PARASITE_H__
#define __PICMAN_TEXT_PARASITE_H__


const gchar  * picman_text_parasite_name          (void) G_GNUC_CONST;
PicmanParasite * picman_text_to_parasite            (const PicmanText      *text);
PicmanText     * picman_text_from_parasite          (const PicmanParasite  *parasite,
                                                 GError             **error);

const gchar  * picman_text_gdyntext_parasite_name (void) G_GNUC_CONST;
PicmanText     * picman_text_from_gdyntext_parasite (const PicmanParasite  *parasite);


#endif /* __PICMAN_TEXT_PARASITE_H__ */
