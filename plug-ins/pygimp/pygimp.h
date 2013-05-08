/* -*- Mode: C; c-basic-offset: 4 -*-
 * Picman-Python - allows the writing of Picman plugins in Python.
 * Copyright (C) 1997-2002  James Henstridge <james@daa.com.au>
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

#ifndef _PYPICMAN_H_
#define _PYPICMAN_H_

#include <Python.h>

#include <libpicman/picman.h>

#define _INSIDE_PYPICMAN_
#include "pypicman-api.h"

#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#define PyInt_AsSsize_t(o) PyInt_AsLong(o)
#endif

G_BEGIN_DECLS

extern PyObject *pypicman_error;

PyObject *pypicman_param_to_tuple(int nparams, const PicmanParam *params);
PicmanParam *pypicman_param_from_tuple(PyObject *args, const PicmanParamDef *ptype,
                                   int nparams);


extern PyTypeObject PyPicmanPDB_Type;
#define pypicman_pdb_check(v) (PyObject_TypeCheck(v, &PyPicmanPDB_Type))
PyObject *pypicman_pdb_new(void);

extern PyTypeObject PyPicmanPDBFunction_Type;
#define pypicman_pdb_function_check(v) (PyObject_TypeCheck(v, &PyPicmanPDBFunction_Type))
PyObject *pypicman_pdb_function_new(const char *name, const char *blurb,
                                  const char *help, const char *author,
                                  const char *copyright, const char *date,
                                  PicmanPDBProcType proc_type,
                                  int n_params, int n_return_vals,
                                  PicmanParamDef *params,
                                  PicmanParamDef *return_vals);

extern PyTypeObject PyPicmanImage_Type;
#define pypicman_image_check(v) (PyObject_TypeCheck(v, &PyPicmanImage_Type))
PyObject *pypicman_image_new(gint32 ID);

extern PyTypeObject PyPicmanDisplay_Type;
#define pypicman_display_check(v) (PyObject_TypeCheck(v, &PyPicmanDisplay_Type))
PyObject *pypicman_display_new(gint32 ID);

extern PyTypeObject PyPicmanItem_Type;
#define pypicman_item_check(v) (PyObject_TypeCheck(v, &PyPicmanItem_Type))
PyObject *pypicman_item_new(gint32 ID);

extern PyTypeObject PyPicmanDrawable_Type;
#define pypicman_drawable_check(v) (PyObject_TypeCheck(v, &PyPicmanDrawable_Type))
PyObject *pypicman_drawable_new(PicmanDrawable *drawable, gint32 ID);

extern PyTypeObject PyPicmanLayer_Type;
#define pypicman_layer_check(v) (PyObject_TypeCheck(v, &PyPicmanLayer_Type))
PyObject *pypicman_layer_new(gint32 ID);

extern PyTypeObject PyPicmanGroupLayer_Type;
#define pypicman_layer__group_check(v) (PyObject_TypeCheck(v, &PyPicmanGroupLayer_Type))
PyObject *pypicman_group_layer_new(gint32 ID);

extern PyTypeObject PyPicmanChannel_Type;
#define pypicman_channel_check(v) (PyObject_TypeCheck(v, &PyPicmanChannel_Type))
PyObject *pypicman_channel_new(gint32 ID);

typedef struct {
    PyObject_HEAD
    PicmanTile *tile;
    PyPicmanDrawable *drawable; /* we keep a reference to the drawable */
} PyPicmanTile;

extern PyTypeObject PyPicmanTile_Type;
#define pypicman_tile_check(v) (PyObject_TypeCheck(v, &PyPicmanTile_Type))
PyObject *pypicman_tile_new(PicmanTile *tile, PyPicmanDrawable *drw);

typedef struct {
    PyObject_HEAD
    PicmanPixelRgn pr;
    PyPicmanDrawable *drawable; /* keep the drawable around */
} PyPicmanPixelRgn;

extern PyTypeObject PyPicmanPixelRgn_Type;
#define pypicman_pixel_rgn_check(v) (PyObject_TypeCheck(v, &PyPicmanPixelRgn_Type))
PyObject *pypicman_pixel_rgn_new(PyPicmanDrawable *drw, int x, int y,
                               int w, int h, int dirty, int shadow);

typedef struct {
    PyObject_HEAD
    PicmanParasite *para;
} PyPicmanParasite;

extern PyTypeObject PyPicmanParasite_Type;
#define pypicman_parasite_check(v) (PyObject_TypeCheck(v, &PyPicmanParasite_Type))
PyObject *pypicman_parasite_new(PicmanParasite *para);

extern PyTypeObject PyPicmanVectors_Type;
#define pypicman_vectors_check(v) (PyObject_TypeCheck(v, &PyPicmanVectors_Type))
PyObject *pypicman_vectors_new(gint32 vectors_ID);

extern PyTypeObject PyPicmanVectorsStroke_Type;
extern PyTypeObject PyPicmanVectorsBezierStroke_Type;

typedef struct {
    PyObject_HEAD
    PicmanPixelFetcher *pf;
    PyPicmanDrawable *drawable; /* keep the drawable around */
    gboolean shadow;
    PicmanRGB bg_color;
    PicmanPixelFetcherEdgeMode edge_mode;
    int bpp;
} PyPicmanPixelFetcher;

extern PyTypeObject PyPicmanPixelFetcher_Type;
#define pypicman_pixel_fetcher_check(v) (PyObject_TypeCheck(v, &PyPicmanPixelFetcher_Type))

G_END_DECLS

#endif
