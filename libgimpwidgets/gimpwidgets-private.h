/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanwidgets-private.h
 * Copyright (C) 2003 Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_WIDGETS_PRIVATE_H__
#define __PICMAN_WIDGETS_PRIVATE_H__


typedef gboolean (* PicmanGetColorFunc)      (PicmanRGB *color);
typedef void     (* PicmanEnsureModulesFunc) (void);


extern PicmanHelpFunc          _picman_standard_help_func;
extern PicmanGetColorFunc      _picman_get_foreground_func;
extern PicmanGetColorFunc      _picman_get_background_func;
extern PicmanEnsureModulesFunc _picman_ensure_modules_func;


G_BEGIN_DECLS


void  picman_widgets_init (PicmanHelpFunc          standard_help_func,
                         PicmanGetColorFunc      get_foreground_func,
                         PicmanGetColorFunc      get_background_func,
                         PicmanEnsureModulesFunc ensure_modules_func);


G_END_DECLS

#endif /* __PICMAN_WIDGETS_PRIVATE_H__ */
