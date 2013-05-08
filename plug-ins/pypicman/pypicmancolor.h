/* -*- Mode: C; c-basic-offset: 4 -*-
 * Picman-Python - allows the writing of Picman plugins in Python.
 * Copyright (C) 2003  Manish Singh <yosh@picman.org>
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

#ifndef _PYPICMANCOLOR_H_
#define _PYPICMANCOLOR_H_

#include <Python.h>

#include <pygobject.h>

#include <libpicman/picman.h>

G_BEGIN_DECLS

extern PyTypeObject PyPicmanRGB_Type;
#define pypicman_rgb_check(v) (pyg_boxed_check((v), PICMAN_TYPE_RGB))
PyObject *pypicman_rgb_new(const PicmanRGB *rgb);

extern PyTypeObject PyPicmanHSV_Type;
#define pypicman_hsv_check(v) (pyg_boxed_check((v), PICMAN_TYPE_HSV))
PyObject *pypicman_hsv_new(const PicmanHSV *hsv);

extern PyTypeObject PyPicmanHSL_Type;
#define pypicman_hsl_check(v) (pyg_boxed_check((v), PICMAN_TYPE_HSL))
PyObject *pypicman_hsl_new(const PicmanHSL *hsl);

extern PyTypeObject PyPicmanCMYK_Type;
#define pypicman_cmyk_check(v) (pyg_boxed_check((v), PICMAN_TYPE_CMYK))
PyObject *pypicman_cmyk_new(const PicmanCMYK *cmyk);

int pypicman_rgb_from_pyobject(PyObject *object, PicmanRGB *color);

G_END_DECLS

#endif
