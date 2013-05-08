/* -*- Mode: C; c-basic-offset: 4 -*-
 * Picman-Python - allows the writing of Picman plugins in Python.
 * Copyright (C) 2005-2006  Manish Singh <yosh@picman.org>
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

#ifndef _PYPICMANCOLOR_API_H_
#define _PYPICMANCOLOR_API_H_

#include <Python.h>

#include <libpicmancolor/picmancolor.h>

struct _PyPicmanColor_Functions {
    PyTypeObject *RGB_Type;
    PyObject *(* rgb_new)(const PicmanRGB *rgb);
    PyTypeObject *HSV_Type;
    PyObject *(* hsv_new)(const PicmanHSV *hsv);
    PyTypeObject *HSL_Type;
    PyObject *(* hsl_new)(const PicmanHSL *hsl);
    PyTypeObject *CMYK_Type;
    PyObject *(* cmyk_new)(const PicmanCMYK *cmyk);
    int (* rgb_from_pyobject)(PyObject *object, PicmanRGB *color);
};

#ifndef _INSIDE_PYPICMANCOLOR_

#if defined(NO_IMPORT) || defined(NO_IMPORT_PYPICMANCOLOR)
extern struct _PyPicmanColor_Functions *_PyPicmanColor_API;
#else
struct _PyPicmanColor_Functions *_PyPicmanColor_API;
#endif

#define PyPicmanRGB_Type (_PyPicmanColor_API->RGB_Type)
#define PyPicmanHSV_Type (_PyPicmanColor_API->HSV_Type)
#define PyPicmanHSL_Type (_PyPicmanColor_API->HSL_Type)
#define PyPicmanCMYK_Type (_PyPicmanColor_API->CMYK_Type)

#define pypicman_rgb_check(v) (pyg_boxed_check((v), PICMAN_TYPE_RGB))
#define pypicman_hsv_check(v) (pyg_boxed_check((v), PICMAN_TYPE_HSV))
#define pypicman_hsl_check(v) (pyg_boxed_check((v), PICMAN_TYPE_HSL))
#define pypicman_cmyk_check(v) (pyg_boxed_check((v), PICMAN_TYPE_CMYK))

#define pypicman_rgb_new (_PyPicmanColor_API->rgb_new)
#define pypicman_hsv_new (_PyPicmanColor_API->hsv_new)
#define pypicman_hsl_new (_PyPicmanColor_API->hsl_new)
#define pypicman_cmyk_new (_PyPicmanColor_API->cmyk_new)

#define pypicman_rgb_from_pyobject (_PyPicmanColor_API->rgb_from_pyobject)

#define init_pypicmancolor() G_STMT_START { \
    PyObject *picmancolormodule = PyImport_ImportModule("picmancolor"); \
    if (picmancolormodule != NULL) { \
	PyObject *mdict = PyModule_GetDict(picmancolormodule); \
	PyObject *cobject = PyDict_GetItemString(mdict, "_PyPicmanColor_API"); \
	if (PyCObject_Check(cobject)) \
	    _PyPicmanColor_API = PyCObject_AsVoidPtr(cobject); \
	else { \
	    PyErr_SetString(PyExc_RuntimeError, \
		            "could not find _PyPicmanColor_API object"); \
	    return; \
	} \
    } else { \
	PyErr_SetString(PyExc_ImportError, \
	                "could not import picmancolor"); \
	return; \
    } \
} G_STMT_END

#endif /* ! _INSIDE_PYPICMANCOLOR_ */

#endif /* _PYPICMANCOLOR_API_H_ */
