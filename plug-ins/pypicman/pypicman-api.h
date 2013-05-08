/* -*- Mode: C; c-basic-offset: 4 -*-
 * Picman-Python - allows the writing of Picman plugins in Python.
 * Copyright (C) 2005  Manish Singh <yosh@picman.org>
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

#ifndef _PYPICMAN_API_H_
#define _PYPICMAN_API_H_

#include <Python.h>

#include <libpicman/picman.h>

typedef struct {
    PyObject_HEAD
    gint32 ID;
} PyPicmanImage, PyPicmanItem;

typedef struct {
    PyObject_HEAD
    gint32 ID;
} PyPicmanDisplay;

typedef struct {
    PyObject_HEAD
    gint32 ID;
    PicmanDrawable *drawable;
} PyPicmanDrawable, PyPicmanLayer, PyPicmanGroupLayer, PyPicmanChannel;

typedef struct {
    PyObject_HEAD
    gint32 ID;
} PyPicmanVectors;

struct _PyPicman_Functions {
    PyTypeObject *Image_Type;
    PyObject *(* image_new)(gint32 ID);

    PyTypeObject *Display_Type;
    PyObject *(* display_new)(gint32 ID);

    PyTypeObject *Item_Type;
    PyObject *(* item_new)(gint32 ID);

    PyTypeObject *Drawable_Type;
    PyObject *(* drawable_new)(PicmanDrawable *drawable, gint32 ID);

    PyTypeObject *Layer_Type;
    PyObject *(* layer_new)(gint32 ID);

    PyTypeObject *GroupLayer_Type;
    PyObject *(* group_layer_new)(gint32 ID);

    PyTypeObject *Channel_Type;
    PyObject *(* channel_new)(gint32 ID);

    PyTypeObject *Vectors_Type;
    PyObject *(* vectors_new)(gint32 ID);

    PyObject *pypicman_error;
};

#ifndef _INSIDE_PYPICMAN_

#if defined(NO_IMPORT) || defined(NO_IMPORT_PYPICMAN)
extern struct _PyPicman_Functions *_PyPicman_API;
#else
struct _PyPicman_Functions *_PyPicman_API;
#endif

#define PyPicmanImage_Type        (_PyPicman_API->Image_Type)
#define pypicman_image_new        (_PyPicman_API->image_new)
#define PyPicmanDisplay_Type      (_PyPicman_API->Display_Type)
#define pypicman_display_new      (_PyPicman_API->display_new)
#define PyPicmanItem_Type         (_PyPicman_API->Item_Type)
#define pypicman_item_new         (_PyPicman_API->item_new)
#define PyPicmanDrawable_Type     (_PyPicman_API->Drawable_Type)
#define pypicman_drawable_new     (_PyPicman_API->drawable_new)
#define PyPicmanLayer_Type        (_PyPicman_API->Layer_Type)
#define pypicman_layer_new        (_PyPicman_API->layer_new)
#define PyPicmanGroupLayer_Type   (_PyPicman_API->GroupLayer_Type)
#define pypicman_group_layer_new  (_PyPicman_API->group_layer_new)
#define PyPicmanChannel_Type      (_PyPicman_API->Channel_Type)
#define pypicman_channel_new      (_PyPicman_API->channel_new)
#define PyPicmanVectors_Type      (_PyPicman_API->Vectors_Type)
#define pypicman_vectors_new      (_PyPicman_API->vectors_new)
#define pypicman_error            (_PyPicman_API->pypicman_error)

#define init_pypicman() G_STMT_START { \
    PyObject *picmanmodule = PyImport_ImportModule("picman"); \
    if (picmanmodule != NULL) { \
        PyObject *mdict = PyModule_GetDict(picmanmodule); \
        PyObject *cobject = PyDict_GetItemString(mdict, "_PyPicman_API"); \
        if (PyCObject_Check(cobject)) \
            _PyPicman_API = PyCObject_AsVoidPtr(cobject); \
        else { \
            PyErr_SetString(PyExc_RuntimeError, \
                            "could not find _PyPicman_API object"); \
            return; \
        } \
    } else { \
        PyErr_SetString(PyExc_ImportError, \
                        "could not import picman"); \
        return; \
    } \
} G_STMT_END

#endif /* ! _INSIDE_PYPICMAN_ */

#endif /* _PYPICMAN_API_H_ */
