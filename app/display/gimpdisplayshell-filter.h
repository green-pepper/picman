/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1999 Manish Singh
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

#ifndef __PICMAN_DISPLAY_SHELL_FILTER_H__
#define __PICMAN_DISPLAY_SHELL_FILTER_H__


void   picman_display_shell_filter_set (PicmanDisplayShell      *shell,
                                      PicmanColorDisplayStack *stack);

PicmanColorDisplayStack * picman_display_shell_filter_new (PicmanDisplayShell *shell,
                                                       PicmanColorConfig  *config);


#endif /* __PICMAN_DISPLAY_SHELL_FILTER_H__ */
