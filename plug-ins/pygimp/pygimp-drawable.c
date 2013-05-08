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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define NO_IMPORT_PYGOBJECT
#include <pygobject.h>

#include "pypicman.h"

#define NO_IMPORT_PYPICMANCOLOR
#include "pypicmancolor-api.h"

#include <glib-object.h>

static void
ensure_drawable(PyPicmanDrawable *self)
{
    if (!self->drawable)
	self->drawable = picman_drawable_get(self->ID);
}

static PyObject *
drw_flush(PyPicmanDrawable *self)
{
    ensure_drawable(self);

    picman_drawable_flush(self->drawable);

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
drw_update(PyPicmanDrawable *self, PyObject *args)
{
    int x, y;
    unsigned int w, h;

    if (!PyArg_ParseTuple(args, "iiii:update", &x, &y, &w, &h))
	return NULL;

    if (!picman_drawable_update(self->ID, x, y, w, h)) {
	PyErr_Format(pypicman_error,
		     "could not update drawable (ID %d): "
		     "x=%d, y=%d, w=%d, h=%d",
		     self->ID, x, y, (int)w, (int)h);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
drw_merge_shadow(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    gboolean undo = FALSE;

    static char *kwlist[] = { "undo", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i:merge_shadow", kwlist,
				     &undo))
	return NULL;

    if (!picman_drawable_merge_shadow(self->ID, undo)) {
	PyErr_Format(pypicman_error,
		     "could not merge the shadow buffer on drawable (ID %d)",
		     self->ID);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
drw_free_shadow(PyPicmanDrawable *self)
{
    if (!picman_drawable_free_shadow(self->ID)) {
	PyErr_Format(pypicman_error, "could not free shadow tiles on drawable (ID %d)",
		     self->ID);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
drw_fill(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    int fill = PICMAN_FOREGROUND_FILL;

    static char *kwlist[] = { "fill", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i:fill", kwlist, &fill))
	return NULL;

    if (!picman_drawable_fill(self->ID, fill)) {
	PyErr_Format(pypicman_error,
		     "could not fill drawable (ID %d) with fill mode %d",
		     self->ID, fill);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
drw_get_tile(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    PicmanTile *t;
    int shadow, row, col;

    static char *kwlist[] = { "shadow", "row", "col", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iii:get_tile", kwlist,
				     &shadow, &row, &col))
	return NULL;

    ensure_drawable(self);

    if(row < 0 || row >= self->drawable->ntile_rows ||
       col < 0 || col >= self->drawable->ntile_cols) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    t = picman_drawable_get_tile(self->drawable, shadow, row, col);
    return pypicman_tile_new(t, self);
}

static PyObject *
drw_get_tile2(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    PicmanTile *t;
    int shadow, x, y;

    static char *kwlist[] = { "shadow", "x", "y", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iii:get_tile2", kwlist,
				     &shadow, &x ,&y))
	return NULL;

    ensure_drawable(self);
    if(x < 0 || x >= self->drawable->width ||
       y < 0 || y >= self->drawable->height) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    t = picman_drawable_get_tile2(self->drawable, shadow, x, y);
    return pypicman_tile_new(t, self);
}

static PyObject *
drw_get_pixel_rgn(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    int x, y, width, height, dirty = 1, shadow = 0;

    static char *kwlist[] = { "x", "y", "width", "height", "dirty", "shadow",
			      NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iiii|ii:get_pixel_rgn", kwlist,
				     &x, &y, &width, &height, &dirty, &shadow))
	return NULL;

    ensure_drawable(self);

    return pypicman_pixel_rgn_new(self, x, y, width, height, dirty, shadow);
}

static PyObject *
drw_offset(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    int wrap_around;
    PicmanOffsetType fill_type;
    int offset_x, offset_y;

    static char *kwlist[] = { "wrap_around", "fill_type",
			      "offset_x", "offset_y",
			      NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iiii:offset", kwlist,
				     &wrap_around, &fill_type,
				     &offset_x, &offset_y))
	return NULL;

    if (!picman_drawable_offset(self->ID, wrap_around, fill_type,
			      offset_x, offset_y)) {
	PyErr_Format(pypicman_error,
		     "could not offset drawable (ID %d) by x: %d, y: %d",
		     self->ID, offset_x, offset_y);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
drw_parasite_find(PyPicmanDrawable *self, PyObject *args)
{
    char *name;

    if (!PyArg_ParseTuple(args, "s:parasite_find", &name))
	return NULL;

    return pypicman_parasite_new(picman_item_get_parasite(self->ID, name));
}

static PyObject *
drw_parasite_attach(PyPicmanDrawable *self, PyObject *args)
{
    PyPicmanParasite *parasite;

    if (!PyArg_ParseTuple(args, "O!:parasite_attach", &PyPicmanParasite_Type,
			  &parasite))
	return NULL;

    if (!picman_item_attach_parasite(self->ID, parasite->para)) {
	PyErr_Format(pypicman_error,
		     "could not attach parasite '%s' on drawable (ID %d)",
		     picman_parasite_name(parasite->para), self->ID);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
drw_attach_new_parasite(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    char         *name;
    int           flags, size;
    guint8       *data;
    PicmanParasite *parasite;
    gboolean      success;

    static char *kwlist[] = { "name", "flags", "data", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "sis#:attach_new_parasite", kwlist,
				     &name, &flags, &data, &size))
	return NULL;

    parasite = picman_parasite_new (name,
                                  flags, size + 1, data);
    success = picman_item_attach_parasite (self->ID, parasite);
    picman_parasite_free (parasite);

    if (!success) {
	PyErr_Format(pypicman_error,
		     "could not attach new parasite '%s' to drawable (ID %d)",
		     name, self->ID);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
drw_parasite_detach(PyPicmanDrawable *self, PyObject *args)
{
    char *name;
    if (!PyArg_ParseTuple(args, "s:detach_parasite", &name))
	return NULL;

    if (!picman_item_detach_parasite(self->ID, name)) {
	PyErr_Format(pypicman_error,
		     "could not detach parasite '%s' from drawable (ID %d)",
		     name, self->ID);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
drw_parasite_list(PyPicmanDrawable *self)
{
    gint num_parasites;
    gchar **parasites;

    parasites = picman_item_get_parasite_list(self->ID, &num_parasites);
    if (parasites) {
	PyObject *ret;
	gint i;

	ret = PyTuple_New(num_parasites);

	for (i = 0; i < num_parasites; i++) {
	    PyTuple_SetItem(ret, i, PyString_FromString(parasites[i]));
	}

	g_strfreev(parasites);
	return ret;
    }

    PyErr_Format(pypicman_error, "could not list parasites on drawable (ID %d)",
		 self->ID);
    return NULL;
}

static PyObject *
drw_get_pixel(PyPicmanDrawable *self, PyObject *args)
{
    int x, y;
    int num_channels, i;
    guint8 *pixel;
    PyObject *ret;

    if (!PyArg_ParseTuple(args, "(ii):get_pixel", &x, &y)) {
	PyErr_Clear();
	if (!PyArg_ParseTuple(args, "ii:get_pixel", &x, &y))
	    return NULL;
    }

    pixel = picman_drawable_get_pixel(self->ID, x, y, &num_channels);

    if (!pixel) {
	PyErr_Format(pypicman_error,
		     "could not get pixel (%d, %d) on drawable (ID %d)",
		     x, y, self->ID);
	return NULL;
    }

    ret = PyTuple_New(num_channels);

    for (i = 0; i < num_channels; i++)
	PyTuple_SetItem(ret, i, PyInt_FromLong(pixel[i]));

    g_free(pixel);

    return ret;
}

static PyObject *
drw_set_pixel(PyPicmanDrawable *self, PyObject *args)
{
    int x, y;
    int num_channels, i, val;
    guint8 *pixel;
    PyObject *seq, *item;
    gboolean is_string, error = TRUE;

    if (!PyArg_ParseTuple(args, "(ii)O:set_pixel", &x, &y, &seq)) {
	PyErr_Clear();
	if (!PyArg_ParseTuple(args, "iiO:set_pixel", &x, &y, &seq))
	    return NULL;
    }

    if (!PyString_Check(seq)) {
	if (!PySequence_Check(seq)) {
	    PyErr_SetString(PyExc_TypeError,
			    "pixel values must be a sequence");
	    return NULL;
	}

	is_string = FALSE;

	num_channels = PySequence_Length(seq);
	pixel = g_new(guint8, num_channels);

	for (i = 0; i < num_channels; i++) {
	    item = PySequence_GetItem(seq, i);

	    if (!PyInt_Check(item)) {
		PyErr_SetString(PyExc_TypeError,
				"pixel values must be a sequence of ints");
		goto out;
	    }

	    val = PyInt_AsLong(item);

	    if (val < 0 || val > 255) {
		PyErr_SetString(PyExc_TypeError,
				"pixel values must be between 0 and 255");
		goto out;
	    }

	    pixel[i] = val;
	}
    } else {
	is_string = TRUE;

	num_channels = PyString_Size(seq);
	pixel = (guint8 *)PyString_AsString(seq);
    }

    error = !picman_drawable_set_pixel(self->ID, x, y, num_channels, pixel);

    if (error)
	PyErr_Format(pypicman_error,
		     "could not set %d-element pixel (%d, %d) on "
		     "drawable (ID %d)",
		     num_channels, x, y, self->ID);

out:
    if (!is_string)
	g_free(pixel);

    if (!error) {
	Py_INCREF(Py_None);
	return Py_None;
    } else
	return NULL;
}

static PyObject *
drw_mask_intersect(PyPicmanDrawable *self)
{
    int x, y, width, height;

    if (!picman_drawable_mask_intersect(self->ID, &x, &y, &width, &height)) {
	PyErr_Format(pypicman_error,
		     "could not get selection bounds of drawable (ID %d)",
		     self->ID);
	return NULL;
    }

    return Py_BuildValue("(iiii)", x, y, width, height);
}

static PyObject *
transform_result(PyPicmanDrawable *self, gint32 id, const char *err_desc)
{
    if (id == self->ID) {
	Py_INCREF(self);
	return (PyObject *)self;
    } else if (id != -1) {
	return pypicman_drawable_new(NULL, id);
    } else {
	PyErr_Format(pypicman_error, "could not %s drawable (ID %d)",
		     err_desc, self->ID);
	return NULL;
    }
}

static PyObject *
drw_transform_flip(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double x0, y0, x1, y1;
    int transform_direction, interpolation, recursion_level = 3;
    gboolean supersample = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "x0", "y0", "x1", "y1",
			      "transform_direction", "interpolation",
			      "supersample", "recursion_level",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "ddddii|iii:transform_flip", kwlist,
				     &x0, &y0, &x1, &y1, &transform_direction,
				     &interpolation, &supersample,
				     &recursion_level, &clip_result))
	return NULL;

    picman_context_push ();
    picman_context_set_transform_direction (transform_direction);
    picman_context_set_interpolation (interpolation);
    picman_context_set_transform_recursion (recursion_level);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_flip (self->ID, x0, y0, x1, y1);

    picman_context_pop ();

    return transform_result(self, id, "flip");
}

static PyObject *
drw_transform_flip_simple(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    int flip_type;
    gboolean auto_center, clip_result = FALSE;
    double axis;
    gint32 id;

    static char *kwlist[] = { "flip_type", "auto_center", "axis",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iid|i:transform_flip_simple", kwlist,
				     &flip_type, &auto_center, &axis,
				     &clip_result))
	return NULL;

    picman_context_push ();
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_flip_simple (self->ID, flip_type,
                                          auto_center, axis);

    picman_context_pop ();

    return transform_result(self, id, "flip");
}

static PyObject *
drw_transform_flip_default(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double x0, y0, x1, y1;
    gboolean interpolate = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "x0", "y0", "x1", "y1", "interpolate",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "dddd|ii:transform_flip_default", kwlist,
				     &x0, &y0, &x1, &y1, &interpolate,
				     &clip_result))
	return NULL;

    picman_context_push ();
    if (! interpolate)
        picman_context_set_interpolation (PICMAN_INTERPOLATION_NONE);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_flip (self->ID, x0, y0, x1, y1);

    picman_context_pop ();

    return transform_result(self, id, "flip");
}

static PyObject *
drw_transform_perspective(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double x0, y0, x1, y1, x2, y2, x3, y3;
    int transform_direction, interpolation, recursion_level = 3;
    gboolean supersample = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "x0", "y0", "x1", "y1", "x2", "y2", "x3", "y3",
			      "transform_direction", "interpolation",
			      "supersample", "recursion_level",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "ddddddddii|iii:transform_perspective",
				     kwlist,
				     &x0, &y0, &x1, &y1, &x2, &y2, &x3, &y3,
				     &transform_direction, &interpolation,
				     &supersample, &recursion_level,
				     &clip_result))
	return NULL;

    picman_context_push ();
    picman_context_set_transform_direction (transform_direction);
    picman_context_set_interpolation (interpolation);
    picman_context_set_transform_recursion (recursion_level);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_perspective (self->ID,
                                          x0, y0, x1, y1, x2, y2, x3, y3);

    picman_context_pop ();

    return transform_result(self, id, "apply perspective transform to");
}

static PyObject *
drw_transform_perspective_default(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double x0, y0, x1, y1, x2, y2, x3, y3;
    gboolean interpolate = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "x0", "y0", "x1", "y1", "x2", "y2", "x3", "y3",
			      "interpolate", "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "dddddddd|ii:transform_perspective_default",
				     kwlist,
				     &x0, &y0, &x1, &y1, &x2, &y2, &x3, &y3,
				     &interpolate, &clip_result))
	return NULL;

    picman_context_push ();
    if (! interpolate)
        picman_context_set_interpolation (PICMAN_INTERPOLATION_NONE);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_perspective (self->ID,
                                          x0, y0, x1, y1, x2, y2, x3, y3);

    picman_context_pop ();

    return transform_result(self, id, "apply perspective transform to");
}

static PyObject *
drw_transform_rotate(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double angle;
    gboolean auto_center, supersample = FALSE, clip_result = FALSE;
    int center_x, center_y, transform_direction, interpolation,
	recursion_level = 3;
    gint32 id;

    static char *kwlist[] = { "angle", "auto_center", "center_x", "center_y",
			      "transform_direction", "interpolation",
			      "supersample", "recursion_level",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "diiiii|iii:transform_rotate", kwlist,
				     &angle, &auto_center, &center_x, &center_y,
				     &transform_direction, &interpolation,
				     &supersample, &recursion_level,
				     &clip_result))
	return NULL;

    picman_context_push ();
    picman_context_set_transform_direction (transform_direction);
    picman_context_set_interpolation (interpolation);
    picman_context_set_transform_recursion (recursion_level);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_rotate (self->ID, angle, auto_center,
                                     center_x, center_y);

    picman_context_pop ();

    return transform_result(self, id, "rotate");
}

static PyObject *
drw_transform_rotate_simple(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    int rotate_type, center_x, center_y;
    gboolean auto_center, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "rotate_type", "auto_center",
			      "center_x", "center_y",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iiii|i:transform_rotate_simple", kwlist,
				     &rotate_type, &auto_center,
				     &center_x, &center_y,
				     &clip_result))
	return NULL;

    picman_context_push ();
    picman_context_set_transform_resize (clip_result);

   id = picman_item_transform_rotate_simple (self->ID, rotate_type,
                                           auto_center,
                                           center_x, center_y);

    picman_context_pop ();

    return transform_result(self, id, "rotate");
}

static PyObject *
drw_transform_rotate_default(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double angle;
    gboolean auto_center, interpolate = FALSE, clip_result = FALSE;
    int center_x, center_y;
    gint32 id;

    static char *kwlist[] = { "angle", "auto_center", "center_x", "center_y",
			      "interpolate", "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "dddd|ii:transform_rotate_default", kwlist,
				     &angle, &auto_center, &center_x, &center_y,
				     &interpolate, &clip_result))
	return NULL;

    picman_context_push ();
    if (! interpolate)
        picman_context_set_interpolation (PICMAN_INTERPOLATION_NONE);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_rotate (self->ID, angle, auto_center,
                                     center_x, center_y);

    picman_context_pop ();

    return transform_result(self, id, "rotate");
}

static PyObject *
drw_transform_scale(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double x0, y0, x1, y1;
    int transform_direction, interpolation, recursion_level = 3;
    gboolean supersample = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "x0", "y0", "x1", "y1",
			      "transform_direction", "interpolation",
			      "supersample", "recursion_level",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "ddddii|iii:transform_scale", kwlist,
				     &x0, &y0, &x1, &y1, &transform_direction,
				     &interpolation, &supersample,
				     &recursion_level, &clip_result))
	return NULL;

    picman_context_push ();
    picman_context_set_transform_direction (transform_direction);
    picman_context_set_interpolation (interpolation);
    picman_context_set_transform_recursion (recursion_level);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_scale (self->ID, x0, y0, x1, y1);

    picman_context_pop ();

    return transform_result(self, id, "scale");
}

static PyObject *
drw_transform_scale_default(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double x0, y0, x1, y1;
    gboolean interpolate = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "x0", "y0", "x1", "y1", "interpolate",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "dddd|ii:transform_scale_default", kwlist,
				     &x0, &y0, &x1, &y1, &interpolate,
				     &clip_result))
	return NULL;

    picman_context_push ();
    if (! interpolate)
        picman_context_set_interpolation (PICMAN_INTERPOLATION_NONE);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_scale (self->ID, x0, y0, x1, y1);

    picman_context_pop ();

    return transform_result(self, id, "scale");
}

static PyObject *
drw_transform_shear(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    int shear_type, transform_direction, interpolation, recursion_level = 3;
    double magnitude;
    gboolean supersample = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "shear_type", "magnitude",
			      "transform_direction", "interpolation",
			      "supersample", "recursion_level",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "idii|iii:transform_shear", kwlist,
				     &shear_type, &magnitude,
				     &transform_direction, &interpolation,
				     &supersample, &recursion_level,
				     &clip_result))
	return NULL;

    picman_context_push ();
    picman_context_set_transform_direction (transform_direction);
    picman_context_set_interpolation (interpolation);
    picman_context_set_transform_recursion (recursion_level);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_shear (self->ID, shear_type, magnitude);

    picman_context_pop ();

    return transform_result(self, id, "shear");
}

static PyObject *
drw_transform_shear_default(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    int shear_type;
    double magnitude;
    gboolean interpolate = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "shear_type", "magnitude", "interpolate",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "id|ii:transform_shear_default", kwlist,
				     &shear_type, &magnitude, &interpolate,
				     &clip_result))
	return NULL;

    picman_context_push ();
    if (! interpolate)
        picman_context_set_interpolation (PICMAN_INTERPOLATION_NONE);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_shear (self->ID, shear_type, magnitude);

    picman_context_pop ();

    return transform_result(self, id, "shear");
}

static PyObject *
drw_transform_2d(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double source_x, source_y, scale_x, scale_y, angle, dest_x, dest_y;
    int transform_direction, interpolation, recursion_level = 3;
    gboolean supersample = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "source_x", "source_y", "scale_x", "scale_y",
			      "angle", "dest_x", "dest_y",
			      "transform_direction", "interpolation",
			      "supersample", "recursion_level",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "dddddddii|iii:transform_2d", kwlist,
				     &source_x, &source_y, &scale_x, &scale_y,
				     &angle, &dest_x, &dest_y,
				     &transform_direction, &interpolation,
				     &supersample, &recursion_level,
				     &clip_result))
	return NULL;

    picman_context_push ();
    picman_context_set_transform_direction (transform_direction);
    picman_context_set_interpolation (interpolation);
    picman_context_set_transform_recursion (recursion_level);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_2d (self->ID, source_x, source_y,
                                 scale_x, scale_y, angle, dest_x, dest_y);

    picman_context_pop ();

    return transform_result(self, id, "apply 2d transform to");
}

static PyObject *
drw_transform_2d_default(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double source_x, source_y, scale_x, scale_y, angle, dest_x, dest_y;
    gboolean interpolate = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "source_x", "source_y", "scale_x", "scale_y",
			      "angle", "dest_x", "dest_y", "interpolate",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "ddddddd|ii:transform_2d_default", kwlist,
				     &source_x, &source_y, &scale_x, &scale_y,
				     &angle, &dest_x, &dest_y, &interpolate,
				     &clip_result))
	return NULL;

    picman_context_push ();
    if (! interpolate)
        picman_context_set_interpolation (PICMAN_INTERPOLATION_NONE);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_2d (self->ID, source_x, source_y,
                                 scale_x, scale_y, angle, dest_x, dest_y);

    picman_context_pop ();

    return transform_result(self, id, "apply 2d transform to");
}

static PyObject *
drw_transform_matrix(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double coeff_0_0, coeff_0_1, coeff_0_2,
	   coeff_1_0, coeff_1_1, coeff_1_2,
	   coeff_2_0, coeff_2_1, coeff_2_2;
    int transform_direction, interpolation, recursion_level = 3;
    gboolean supersample = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "coeff_0_0", "coeff_0_1", "coeff_0_2",
			      "coeff_1_0", "coeff_1_1", "coeff_1_2",
			      "coeff_2_0", "coeff_2_1", "coeff_2_2",
			      "transform_direction", "interpolation",
			      "supersample", "recursion_level",
			      "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "dddddddddii|iii:transform_matrix", kwlist,
				     &coeff_0_0, &coeff_0_1, &coeff_0_2,
				     &coeff_1_0, &coeff_1_1, &coeff_1_2,
				     &coeff_2_0, &coeff_2_1, &coeff_2_2,
				     &transform_direction, &interpolation,
				     &supersample, &recursion_level,
				     &clip_result))
	return NULL;

    picman_context_push ();
    picman_context_set_transform_direction (transform_direction);
    picman_context_set_interpolation (interpolation);
    picman_context_set_transform_recursion (recursion_level);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_matrix (self->ID,
                                     coeff_0_0, coeff_0_1, coeff_0_2,
                                     coeff_1_0, coeff_1_1, coeff_1_2,
                                     coeff_2_0, coeff_2_1, coeff_2_2);

    picman_context_pop ();

    return transform_result(self, id, "apply 2d matrix transform to");
}

static PyObject *
drw_transform_matrix_default(PyPicmanDrawable *self, PyObject *args, PyObject *kwargs)
{
    double coeff_0_0, coeff_0_1, coeff_0_2,
	   coeff_1_0, coeff_1_1, coeff_1_2,
	   coeff_2_0, coeff_2_1, coeff_2_2;
    gboolean interpolate = FALSE, clip_result = FALSE;
    gint32 id;

    static char *kwlist[] = { "coeff_0_0", "coeff_0_1", "coeff_0_2",
			      "coeff_1_0", "coeff_1_1", "coeff_1_2",
			      "coeff_2_0", "coeff_2_1", "coeff_2_2",
			      "interpolate", "clip_result", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "ddddddddd|ii:transform_matrix_default",
				     kwlist,
				     &coeff_0_0, &coeff_0_1, &coeff_0_2,
				     &coeff_1_0, &coeff_1_1, &coeff_1_2,
				     &coeff_2_0, &coeff_2_1, &coeff_2_2,
				     &interpolate, &clip_result))
	return NULL;

    picman_context_push ();
    if (! interpolate)
        picman_context_set_interpolation (PICMAN_INTERPOLATION_NONE);
    picman_context_set_transform_resize (clip_result);

    id = picman_item_transform_matrix (self->ID,
                                     coeff_0_0, coeff_0_1, coeff_0_2,
                                     coeff_1_0, coeff_1_1, coeff_1_2,
                                     coeff_2_0, coeff_2_1, coeff_2_2);

    picman_context_pop ();

    return transform_result(self, id, "apply 2d matrix transform to");
}

/* for inclusion with the methods of layer and channel objects */
static PyMethodDef drw_methods[] = {
    {"flush",	(PyCFunction)drw_flush,	METH_NOARGS},
    {"update",	(PyCFunction)drw_update,	METH_VARARGS},
    {"merge_shadow",	(PyCFunction)drw_merge_shadow,	METH_VARARGS | METH_KEYWORDS},
    {"free_shadow", (PyCFunction)drw_free_shadow, METH_NOARGS},
    {"fill",	(PyCFunction)drw_fill,	METH_VARARGS | METH_KEYWORDS},
    {"get_tile",	(PyCFunction)drw_get_tile,	METH_VARARGS | METH_KEYWORDS},
    {"get_tile2",	(PyCFunction)drw_get_tile2,	METH_VARARGS | METH_KEYWORDS},
    {"get_pixel_rgn", (PyCFunction)drw_get_pixel_rgn, METH_VARARGS | METH_KEYWORDS},
    {"offset", (PyCFunction)drw_offset, METH_VARARGS | METH_KEYWORDS},
    {"parasite_find",       (PyCFunction)drw_parasite_find, METH_VARARGS},
    {"parasite_attach",     (PyCFunction)drw_parasite_attach, METH_VARARGS},
    {"attach_new_parasite",(PyCFunction)drw_attach_new_parasite,METH_VARARGS | METH_KEYWORDS},
    {"parasite_detach",     (PyCFunction)drw_parasite_detach, METH_VARARGS},
    {"parasite_list",     (PyCFunction)drw_parasite_list, METH_VARARGS},
    {"get_pixel",	(PyCFunction)drw_get_pixel, METH_VARARGS},
    {"set_pixel",	(PyCFunction)drw_set_pixel, METH_VARARGS},
    {"mask_intersect",	(PyCFunction)drw_mask_intersect, METH_NOARGS},
    {"transform_flip",	(PyCFunction)drw_transform_flip, METH_VARARGS | METH_KEYWORDS},
    {"transform_flip_simple",	(PyCFunction)drw_transform_flip_simple, METH_VARARGS | METH_KEYWORDS},
    {"transform_flip_default",	(PyCFunction)drw_transform_flip_default, METH_VARARGS | METH_KEYWORDS},
    {"transform_perspective",	(PyCFunction)drw_transform_perspective, METH_VARARGS | METH_KEYWORDS},
    {"transform_perspective_default",	(PyCFunction)drw_transform_perspective_default, METH_VARARGS | METH_KEYWORDS},
    {"transform_rotate",	(PyCFunction)drw_transform_rotate, METH_VARARGS | METH_KEYWORDS},
    {"transform_rotate_simple",	(PyCFunction)drw_transform_rotate_simple, METH_VARARGS | METH_KEYWORDS},
    {"transform_rotate_default",	(PyCFunction)drw_transform_rotate_default, METH_VARARGS | METH_KEYWORDS},
    {"transform_scale",	(PyCFunction)drw_transform_scale, METH_VARARGS | METH_KEYWORDS},
    {"transform_scale_default",	(PyCFunction)drw_transform_scale_default, METH_VARARGS | METH_KEYWORDS},
    {"transform_shear",	(PyCFunction)drw_transform_shear, METH_VARARGS | METH_KEYWORDS},
    {"transform_shear_default",	(PyCFunction)drw_transform_shear_default, METH_VARARGS | METH_KEYWORDS},
    {"transform_2d",	(PyCFunction)drw_transform_2d, METH_VARARGS | METH_KEYWORDS},
    {"transform_2d_default",	(PyCFunction)drw_transform_2d_default, METH_VARARGS | METH_KEYWORDS},
    {"transform_matrix",	(PyCFunction)drw_transform_matrix, METH_VARARGS | METH_KEYWORDS},
    {"transform_matrix_default",	(PyCFunction)drw_transform_matrix_default, METH_VARARGS | METH_KEYWORDS},
    {NULL, NULL, 0}
};

static PyObject *
drw_get_ID(PyPicmanDrawable *self, void *closure)
{
    return PyInt_FromLong(self->ID);
}

static PyObject *
drw_get_name(PyPicmanDrawable *self, void *closure)
{
    return PyString_FromString(picman_item_get_name(self->ID));
}

static int
drw_set_name(PyPicmanDrawable *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete name");
        return -1;
    }

    if (!PyString_Check(value) && !PyUnicode_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "type mismatch");
        return -1;
    }

    picman_item_set_name(self->ID, PyString_AsString(value));

    return 0;
}

static PyObject *
drw_get_bpp(PyPicmanDrawable *self, void *closure)
{
    return PyInt_FromLong(picman_drawable_bpp(self->ID));
}

static PyObject *
drw_get_has_alpha(PyPicmanDrawable *self, void *closure)
{
    return PyBool_FromLong(picman_drawable_has_alpha(self->ID));
}

static PyObject *
drw_get_height(PyPicmanDrawable *self, void *closure)
{
    return PyInt_FromLong(picman_drawable_height(self->ID));
}

static PyObject *
drw_get_image(PyPicmanDrawable *self, void *closure)
{
    return pypicman_image_new(picman_item_get_image(self->ID));
}

static PyObject *
drw_get_is_rgb(PyPicmanDrawable *self, void *closure)
{
    return PyBool_FromLong(picman_drawable_is_rgb(self->ID));
}

static PyObject *
drw_get_is_gray(PyPicmanDrawable *self, void *closure)
{
    return PyBool_FromLong(picman_drawable_is_gray(self->ID));
}

static PyObject *
drw_get_is_indexed(PyPicmanDrawable *self, void *closure)
{
    return PyBool_FromLong(picman_drawable_is_indexed(self->ID));
}

static PyObject *
drw_get_is_layer_mask(PyPicmanDrawable *self, void *closure)
{
    return PyBool_FromLong(picman_item_is_layer_mask(self->ID));
}

static PyObject *
drw_get_mask_bounds(PyPicmanDrawable *self, void *closure)
{
    gint x1, y1, x2, y2;

    picman_drawable_mask_bounds(self->ID, &x1, &y1, &x2, &y2);
    return Py_BuildValue("(iiii)", x1, y1, x2, y2);
}

static PyObject *
drw_get_offsets(PyPicmanDrawable *self, void *closure)
{
    gint x, y;

    picman_drawable_offsets(self->ID, &x, &y);

    return Py_BuildValue("(ii)", x, y);
}

static PyObject *
drw_get_type(PyPicmanDrawable *self, void *closure)
{
    return PyInt_FromLong(picman_drawable_type(self->ID));
}

static PyObject *
drw_get_type_with_alpha(PyPicmanDrawable *self, void *closure)
{
    return PyInt_FromLong(picman_drawable_type_with_alpha(self->ID));
}

static PyObject *
drw_get_width(PyPicmanDrawable *self, void *closure)
{
    return PyInt_FromLong(picman_drawable_width(self->ID));
}

static PyObject *
drw_get_linked(PyPicmanDrawable *self, void *closure)
{
    return PyBool_FromLong(picman_item_get_linked(self->ID));
}

static int
drw_set_linked(PyPicmanDrawable *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete linked");
        return -1;
    }

    if (!PyInt_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "type mismatch");
	return -1;
    }

    picman_item_set_linked(self->ID, PyInt_AsLong(value));

    return 0;
}

static PyObject *
drw_get_tattoo(PyPicmanDrawable *self, void *closure)
{
    return PyInt_FromLong(picman_item_get_tattoo(self->ID));
}

static int
drw_set_tattoo(PyPicmanDrawable *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete tattoo");
        return -1;
    }

    if (!PyInt_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "type mismatch");
        return -1;
    }

    picman_item_set_tattoo(self->ID, PyInt_AsLong(value));

    return 0;
}

static PyObject *
drw_get_visible(PyPicmanDrawable *self, void *closure)
{
    return PyBool_FromLong(picman_item_get_visible(self->ID));
}

static int
drw_set_visible(PyPicmanDrawable *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete visible");
        return -1;
    }

    if (!PyInt_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "type mismatch");
        return -1;
    }

    picman_item_set_visible(self->ID, PyInt_AsLong(value));

    return 0;
}

static  PyGetSetDef drw_getsets[] = {
    { "ID", (getter)drw_get_ID, (setter)0 },
    { "name", (getter)drw_get_name, (setter)drw_set_name },
    { "bpp", (getter)drw_get_bpp, (setter)0 },
    { "has_alpha", (getter)drw_get_has_alpha, (setter)0 },
    { "height", (getter)drw_get_height, (setter)0 },
    { "image", (getter)drw_get_image, (setter)0 },
    { "is_rgb", (getter)drw_get_is_rgb, (setter)0 },
    { "is_gray", (getter)drw_get_is_gray, (setter)0 },
    { "is_grey", (getter)drw_get_is_gray, (setter)0 },
    { "is_indexed", (getter)drw_get_is_indexed, (setter)0 },
    { "is_layer_mask", (getter)drw_get_is_layer_mask, (setter)0 },
    { "mask_bounds", (getter)drw_get_mask_bounds, (setter)0 },
    { "offsets", (getter)drw_get_offsets, (setter)0 },
    { "type", (getter)drw_get_type, (setter)0 },
    { "type_with_alpha", (getter)drw_get_type_with_alpha, (setter)0 },
    { "width", (getter)drw_get_width, (setter)0 },
    { "linked", (getter)drw_get_linked, (setter)drw_set_linked },
    { "tattoo", (getter)drw_get_tattoo, (setter)drw_set_tattoo },
    { "visible", (getter)drw_get_visible, (setter)drw_set_visible },
    { NULL, (getter)0, (setter)0 }
};

static void
drw_dealloc(PyPicmanDrawable *self)
{
    if (self->drawable)
	picman_drawable_detach(self->drawable);

    PyObject_DEL(self);
}

static PyObject *
drw_repr(PyPicmanDrawable *self)
{
    PyObject *s;
    gchar *name;

    name = picman_item_get_name(self->ID);
    s = PyString_FromFormat("<picman.Drawable '%s'>", name ? name : "(null)");
    g_free(name);

    return s;
}

static int
drw_cmp(PyPicmanDrawable *self, PyPicmanDrawable *other)
{
    if (self->ID == other->ID)
	return 0;
    if (self->ID > other->ID)
	return -1;
    return 1;
}

PyTypeObject PyPicmanDrawable_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "picman.Drawable",                    /* tp_name */
    sizeof(PyPicmanDrawable),             /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)drw_dealloc,            /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)drw_cmp,                   /* tp_compare */
    (reprfunc)drw_repr,                 /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    (hashfunc)0,                        /* tp_hash */
    (ternaryfunc)0,                     /* tp_call */
    (reprfunc)0,                        /* tp_str */
    (getattrofunc)0,                    /* tp_getattro */
    (setattrofunc)0,                    /* tp_setattro */
    0,					/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,	                /* tp_flags */
    NULL, /* Documentation string */
    (traverseproc)0,			/* tp_traverse */
    (inquiry)0,				/* tp_clear */
    (richcmpfunc)0,			/* tp_richcompare */
    0,					/* tp_weaklistoffset */
    (getiterfunc)0,			/* tp_iter */
    (iternextfunc)0,			/* tp_iternext */
    drw_methods,			/* tp_methods */
    0,					/* tp_members */
    drw_getsets,			/* tp_getset */
    &PyPicmanItem_Type,			/* tp_base */
    (PyObject *)0,			/* tp_dict */
    0,					/* tp_descr_get */
    0,					/* tp_descr_set */
    0,					/* tp_dictoffset */
    (initproc)0,                        /* tp_init */
    (allocfunc)0,			/* tp_alloc */
    (newfunc)0,				/* tp_new */
};


PyObject *
pypicman_drawable_new(PicmanDrawable *drawable, gint32 ID)
{
    PyObject *self;

    if (drawable != NULL)
    ID = drawable->drawable_id;

    if (!picman_item_is_valid(ID)) {
	Py_INCREF(Py_None);
	return Py_None;
    }

    /* create the appropriate object type */
    if (picman_item_is_layer(ID))
	self = pypicman_layer_new(ID);
    else
	self = pypicman_channel_new(ID);

    if (self == NULL)
	return NULL;

    if (PyObject_TypeCheck(self, &PyPicmanDrawable_Type))
    ((PyPicmanDrawable *)self)->drawable = drawable;

    return self;
}

/* End of code for Drawable objects */
/* -------------------------------------------------------- */


static PyObject *
lay_copy(PyPicmanLayer *self, PyObject *args, PyObject *kwargs)
{
    int nreturn_vals;
    PicmanParam *return_vals;
    gboolean add_alpha = FALSE;
    gint32 id = -1;

    static char *kwlist[] = { "add_alpha", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i:copy", kwlist,
				     &add_alpha))
	return NULL;

    return_vals = picman_run_procedure("picman-layer-copy",
				     &nreturn_vals,
				     PICMAN_PDB_LAYER, self->ID,
				     PICMAN_PDB_INT32, add_alpha,
				     PICMAN_PDB_END);

    if (return_vals[0].data.d_status == PICMAN_PDB_SUCCESS)
	id = return_vals[1].data.d_layer;
    else
	PyErr_Format(pypicman_error,
		     "could not create new layer copy from layer (ID %d)",
		     self->ID);

    picman_destroy_params(return_vals, nreturn_vals);

    return id != -1 ? pypicman_layer_new(id) : NULL;
}


static PyObject *
lay_add_alpha(PyPicmanLayer *self)
{
    if (!picman_layer_add_alpha(self->ID)) {
	PyErr_Format(pypicman_error, "could not add alpha to layer (ID %d)",
		     self->ID);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
lay_add_mask(PyPicmanLayer *self, PyObject *args)
{
    PyPicmanChannel *mask;

    if (!PyArg_ParseTuple(args, "O!:add_mask", &PyPicmanChannel_Type, &mask))
	return NULL;

    if (!picman_layer_add_mask(self->ID, mask->ID)) {
	PyErr_Format(pypicman_error,
		     "could not add mask (ID %d) to layer (ID %d)",
		     mask->ID, self->ID);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
lay_create_mask(PyPicmanLayer *self, PyObject *args)
{
    int type;
    gint32 id;

    if (!PyArg_ParseTuple(args, "i:create_mask", &type))
	return NULL;

    id = picman_layer_create_mask(self->ID, type);

    if (id == -1) {
	PyErr_Format(pypicman_error,
		     "could not create mask of type %d on layer (ID %d)",
		     type, self->ID);
	return NULL;
    }

    return pypicman_channel_new(id);
}

static PyObject *
lay_remove_mask(PyPicmanLayer *self, PyObject *args)
{
    int mode;

    if (!PyArg_ParseTuple(args, "i:remove_mask", &mode))
	return NULL;

    if (!picman_layer_remove_mask(self->ID, mode)) {
	PyErr_Format(pypicman_error,
		     "could not remove mask from layer (ID %d) with mode %d",
		      self->ID, mode);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
lay_resize(PyPicmanLayer *self, PyObject *args, PyObject *kwargs)
{
    unsigned int new_h, new_w;
    int offs_x = 0, offs_y = 0;

    static char *kwlist[] = { "width", "height", "offset_x", "offset_y", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii|ii:resize", kwlist,
				     &new_w, &new_h, &offs_x, &offs_y))
	return NULL;

    if (!picman_layer_resize(self->ID, new_w, new_h, offs_x, offs_y)) {
	PyErr_Format(pypicman_error,
		     "could not resize layer (ID %d) to size %dx%d "
		     "(offset %d, %d)",
		     self->ID, new_w, new_h, offs_x, offs_y);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
lay_resize_to_image_size(PyPicmanLayer *self)
{
    if (!picman_layer_resize_to_image_size(self->ID)) {
	PyErr_Format(pypicman_error,
		     "could not resize layer (ID %d) to image size",
		     self->ID);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
lay_scale(PyPicmanLayer *self, PyObject *args, PyObject *kwargs)
{
    int new_width, new_height;
    int interpolation = -1;
    gboolean local_origin = FALSE;

    static char *kwlist[] = { "width", "height", "local_origin",
                              "interpolation", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii|ii:scale", kwlist,
				     &new_width, &new_height,
                                     &local_origin, &interpolation))
	return NULL;

    if (interpolation != -1) {
        picman_context_push();
        picman_context_set_interpolation(interpolation);
    }

    if (!picman_layer_scale(self->ID, new_width, new_height, local_origin)) {
        PyErr_Format(pypicman_error,
                     "could not scale layer (ID %d) to size %dx%d",
                     self->ID, new_width, new_height);
        if (interpolation != -1) {
            picman_context_pop();
        }
        return NULL;
    }

    if (interpolation != -1) {
        picman_context_pop();
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
lay_translate(PyPicmanLayer *self, PyObject *args, PyObject *kwargs)
{
    int offs_x, offs_y;

    static char *kwlist[] = { "offset_x", "offset_y", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii:translate", kwlist,
				     &offs_x, &offs_y))
	return NULL;

    if (!picman_layer_translate(self->ID, offs_x, offs_y)) {
	PyErr_Format(pypicman_error,
		     "could not translate layer (ID %d) to offset %d, %d",
		     self->ID, offs_x, offs_y);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
lay_set_offsets(PyPicmanLayer *self, PyObject *args, PyObject *kwargs)
{
    int offs_x, offs_y;

    static char *kwlist[] = { "offset_x", "offset_y", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii:set_offsets", kwlist,
				     &offs_x, &offs_y))
	return NULL;

    if (!picman_layer_set_offsets(self->ID, offs_x, offs_y)) {
	PyErr_Format(pypicman_error,
		     "could not set offset %d, %d on layer (ID %d)",
		     offs_x, offs_y, self->ID);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef lay_methods[] = {
    {"copy",	(PyCFunction)lay_copy,	METH_VARARGS | METH_KEYWORDS},
    {"add_alpha",	(PyCFunction)lay_add_alpha,	METH_NOARGS},
    {"add_mask",        (PyCFunction)lay_add_mask,      METH_VARARGS},
    {"create_mask",	(PyCFunction)lay_create_mask,	METH_VARARGS},
    {"remove_mask",     (PyCFunction)lay_remove_mask,   METH_VARARGS},
    {"resize",	(PyCFunction)lay_resize,	METH_VARARGS | METH_KEYWORDS},
    {"resize_to_image_size",	(PyCFunction)lay_resize_to_image_size,	METH_NOARGS},
    {"scale",	(PyCFunction)lay_scale,	METH_VARARGS | METH_KEYWORDS},
    {"translate",	(PyCFunction)lay_translate,	METH_VARARGS | METH_KEYWORDS},
    {"set_offsets",	(PyCFunction)lay_set_offsets,	METH_VARARGS | METH_KEYWORDS},
    {NULL,		NULL}		/* sentinel */
};

static PyObject *
lay_get_is_floating_sel(PyPicmanLayer *self, void *closure)
{
    return PyBool_FromLong(picman_layer_is_floating_sel(self->ID));
}

static PyObject *
lay_get_mask(PyPicmanLayer *self, void *closure)
{
    gint32 id = picman_layer_get_mask(self->ID);

    if (id == -1) {
	Py_INCREF(Py_None);
	return Py_None;
    }

    return pypicman_channel_new(id);
}

static PyObject *
lay_get_apply_mask(PyPicmanLayer *self, void *closure)
{
    return PyBool_FromLong(picman_layer_get_apply_mask(self->ID));
}

static int
lay_set_apply_mask(PyPicmanLayer *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete apply_mask");
        return -1;
    }

    if (!PyInt_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "type mismatch");
	return -1;
    }

    if (!picman_layer_set_apply_mask(self->ID, PyInt_AsLong(value))) {
	PyErr_Format(pypicman_error,
		     "could not set layer mask on layer (ID %d)",
		     self->ID);
	return -1;
    }

    return 0;
}

static PyObject *
lay_get_edit_mask(PyPicmanLayer *self, void *closure)
{
    return PyBool_FromLong(picman_layer_get_edit_mask(self->ID));
}

static int
lay_set_edit_mask(PyPicmanLayer *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete edit_mask");
        return -1;
    }

    if (!PyInt_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "type mismatch");
	return -1;
    }

    if (!picman_layer_set_edit_mask(self->ID, PyInt_AsLong(value))) {
	PyErr_Format(pypicman_error,
		     "could not set layer mask active on layer (ID %d)",
		     self->ID);
	return -1;
    }

    return 0;
}

static PyObject *
lay_get_mode(PyPicmanLayer *self, void *closure)
{
    return PyInt_FromLong(picman_layer_get_mode(self->ID));
}

static int
lay_set_mode(PyPicmanLayer *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete mode");
        return -1;
    }

    if (!PyInt_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "type mismatch");
	return -1;
    }

    if (!picman_layer_set_mode(self->ID, PyInt_AsLong(value))) {
	PyErr_Format(pypicman_error, "could not set mode on layer (ID %d)",
		     self->ID);
	return -1;
    }

    return 0;
}

static PyObject *
lay_get_opacity(PyPicmanLayer *self, void *closure)
{
    return PyFloat_FromDouble(picman_layer_get_opacity(self->ID));
}

static int
lay_set_opacity(PyPicmanLayer *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete opacity");
        return -1;
    }

    if (!PyFloat_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "type mismatch");
	return -1;
    }

    if (!picman_layer_set_opacity(self->ID, PyFloat_AsDouble(value))) {
	PyErr_Format(pypicman_error, "could not set opacity on layer (ID %d)",
		     self->ID);
	return -1;
    }

    return 0;
}

static PyObject *
lay_get_lock_alpha(PyPicmanLayer *self, void *closure)
{
    return PyBool_FromLong(picman_layer_get_lock_alpha(self->ID));
}

static int
lay_set_lock_alpha(PyPicmanLayer *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError,
			"cannot delete lock_alpha");
        return -1;
    }

    if (!PyInt_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "type mismatch");
	return -1;
    }

    if (!picman_layer_set_lock_alpha(self->ID, PyInt_AsLong(value))) {
	PyErr_Format(pypicman_error,
	             "could not set lock alpha setting on layer (ID %d)",
		     self->ID);
	return -1;
    }

    return 0;
}

static PyObject *
lay_get_show_mask(PyPicmanLayer *self, void *closure)
{
    return PyBool_FromLong(picman_layer_get_show_mask(self->ID));
}

static int
lay_set_show_mask(PyPicmanLayer *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete show_mask");
        return -1;
    }

    if (!PyInt_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "type mismatch");
	return -1;
    }

    if (!picman_layer_set_show_mask(self->ID, PyInt_AsLong(value))) {
	PyErr_Format(pypicman_error,
	             "could not set mask visibility on layer (ID %d)",
		     self->ID);
	return -1;
    }

    return 0;
}

static PyObject *
lay_get_preserve_trans(PyPicmanLayer *self, void *closure)
{
    if (PyErr_Warn(PyExc_DeprecationWarning, "use lock_alpha attribute") < 0)
	return NULL;

    return lay_get_lock_alpha(self, closure);
}

static int
lay_set_preserve_trans(PyPicmanLayer *self, PyObject *value, void *closure)
{
    if (PyErr_Warn(PyExc_DeprecationWarning, "use lock_alpha attribute") < 0)
	return -1;

    return lay_set_lock_alpha(self, value, closure);
}

static PyGetSetDef lay_getsets[] = {
    { "is_floating_sel", (getter)lay_get_is_floating_sel, (setter)0 },
    { "mask", (getter)lay_get_mask, (setter)0 },
    { "apply_mask", (getter)lay_get_apply_mask, (setter)lay_set_apply_mask },
    { "edit_mask", (getter)lay_get_edit_mask, (setter)lay_set_edit_mask },
    { "mode", (getter)lay_get_mode, (setter)lay_set_mode },
    { "opacity", (getter)lay_get_opacity, (setter)lay_set_opacity },
    { "lock_alpha", (getter)lay_get_lock_alpha, (setter)lay_set_lock_alpha },
    { "show_mask", (getter)lay_get_show_mask, (setter)lay_set_show_mask },
    { "preserve_trans", (getter)lay_get_preserve_trans,
      (setter)lay_set_preserve_trans },
    { NULL, (getter)0, (setter)0 }
};

static PyObject *
lay_repr(PyPicmanLayer *self)
{
    PyObject *s;
    gchar *name;

    name = picman_item_get_name(self->ID);
    s = PyString_FromFormat("<picman.Layer '%s'>", name ? name : "(null)");
    g_free(name);

    return s;
}

static int
lay_init(PyPicmanLayer *self, PyObject *args, PyObject *kwargs)
{
    PyPicmanImage *img;
    char *name;
    unsigned int width, height;
    PicmanImageType type = PICMAN_RGB_IMAGE;
    double opacity = 100.0;
    PicmanLayerModeEffects mode = PICMAN_NORMAL_MODE;


    if (!PyArg_ParseTuple(args, "O!sii|idi:picman.Layer.__init__",
			  &PyPicmanImage_Type, &img, &name, &width, &height,
			  &type, &opacity, &mode))
	return -1;

    self->ID = picman_layer_new(img->ID, name, width, height,
			      type, opacity, mode);

    self->drawable = NULL;

    if (self->ID < 0) {
	PyErr_Format(pypicman_error,
		     "could not create %dx%d layer '%s' of type %d on "
		     "image (ID %d)",
		     width, height, name, type, img->ID);
	return -1;
    }

    return 0;
}

PyTypeObject PyPicmanLayer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "picman.Layer",                       /* tp_name */
    sizeof(PyPicmanLayer),                /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)drw_dealloc,            /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)drw_cmp,                   /* tp_compare */
    (reprfunc)lay_repr,                 /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    (hashfunc)0,                        /* tp_hash */
    (ternaryfunc)0,                     /* tp_call */
    (reprfunc)0,                        /* tp_str */
    (getattrofunc)0,                    /* tp_getattro */
    (setattrofunc)0,                    /* tp_setattro */
    0,					/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,	                /* tp_flags */
    NULL, /* Documentation string */
    (traverseproc)0,			/* tp_traverse */
    (inquiry)0,				/* tp_clear */
    (richcmpfunc)0,			/* tp_richcompare */
    0,					/* tp_weaklistoffset */
    (getiterfunc)0,			/* tp_iter */
    (iternextfunc)0,			/* tp_iternext */
    lay_methods,			/* tp_methods */
    0,					/* tp_members */
    lay_getsets,			/* tp_getset */
    &PyPicmanDrawable_Type,		/* tp_base */
    (PyObject *)0,			/* tp_dict */
    0,					/* tp_descr_get */
    0,					/* tp_descr_set */
    0,					/* tp_dictoffset */
    (initproc)lay_init,                 /* tp_init */
    (allocfunc)0,			/* tp_alloc */
    (newfunc)0,				/* tp_new */
};

PyObject *
pypicman_layer_new(gint32 ID)
{
    PyPicmanLayer *self;

    if (!picman_item_is_valid(ID) || !picman_item_is_layer(ID)) {
	Py_INCREF(Py_None);
	return Py_None;
    }

    self = PyObject_NEW(PyPicmanLayer, &PyPicmanLayer_Type);

    if (self == NULL)
	return NULL;

    self->ID = ID;
    self->drawable = NULL;

    return (PyObject *)self;
}

/* End of code for Layer objects */
/* -------------------------------------------------------- */

static PyMethodDef grouplay_methods[] = {
    {NULL,              NULL}           /* sentinel */
};

static PyObject *
grouplay_get_layers(PyPicmanGroupLayer *self, void *closure)
{
    gint32 *layers;
    gint n_layers, i;
    PyObject *ret;

    layers = picman_item_get_children(self->ID, &n_layers);

    ret = PyList_New(n_layers);

    for (i = 0; i < n_layers; i++)
        PyList_SetItem(ret, i, pypicman_group_layer_new(layers[i]));

    g_free(layers);

    return ret;
}

static PyGetSetDef grouplay_getsets[] = {
    { "layers", (getter)grouplay_get_layers, (setter)0 },
    { NULL, (getter)0, (setter)0 }
};

static PyObject *
grouplay_repr(PyPicmanLayer *self)
{
    PyObject *s;
    gchar *name;

    name = picman_item_get_name(self->ID);
    s = PyString_FromFormat("<picman.GroupLayer '%s'>", name ? name : "(null)");
    g_free(name);

    return s;
}

PyTypeObject PyPicmanGroupLayer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "picman.GroupLayer",                  /* tp_name */
    sizeof(PyPicmanGroupLayer),                /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)drw_dealloc,            /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)drw_cmp,                   /* tp_compare */
    (reprfunc)grouplay_repr,                 /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    (hashfunc)0,                        /* tp_hash */
    (ternaryfunc)0,                     /* tp_call */
    (reprfunc)0,                        /* tp_str */
    (getattrofunc)0,                    /* tp_getattro */
    (setattrofunc)0,                    /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                 /* tp_flags */
    NULL, /* Documentation string */
    (traverseproc)0,                    /* tp_traverse */
    (inquiry)0,                         /* tp_clear */
    (richcmpfunc)0,                     /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    (getiterfunc)0,                     /* tp_iter */
    (iternextfunc)0,                    /* tp_iternext */
    grouplay_methods,                   /* tp_methods */
    0,                                  /* tp_members */
    grouplay_getsets,                        /* tp_getset */
    &PyPicmanLayer_Type,                  /* tp_base */
    (PyObject *)0,                      /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)lay_init,                 /* tp_init */
    (allocfunc)0,                       /* tp_alloc */
    (newfunc)0,                         /* tp_new */
};

PyObject *
pypicman_group_layer_new(gint32 ID)
{
    PyPicmanGroupLayer *self;

    if (!picman_item_is_valid(ID) || !picman_item_is_layer(ID)) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    
    if (!picman_item_is_group(ID)) {
        return pypicman_layer_new(ID);
    }

    self = PyObject_NEW(PyPicmanGroupLayer, &PyPicmanGroupLayer_Type);

    if (self == NULL)
        return NULL;

    self->ID = ID;
    self->drawable = NULL;

    return (PyObject *)self;
}

/* End of code for GroupLayer objects */
/* -------------------------------------------------------- */


static PyObject *
chn_copy(PyPicmanChannel *self)
{
    gint32 id;

    id = picman_channel_copy(self->ID);

    if (id == -1) {
	PyErr_Format(pypicman_error,
		     "could not create new channel copy from channel (ID %d)",
		     self->ID);
	return NULL;
    }

    return pypicman_channel_new(id);
}

static PyObject *
chn_combine_masks(PyPicmanChannel *self, PyObject *args, PyObject *kwargs)
{
    PyPicmanChannel *channel2;
    PicmanChannelOps operation;
    int offx = 0, offy = 0;

    static char *kwlist[] = { "channel", "operation", "offset_x", "offset_y",
			      NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!i|ii:combine_masks",
				     kwlist,
				     &PyPicmanChannel_Type, &channel2,
				     &operation, &offx, &offy))
	return NULL;

    if (!picman_channel_combine_masks(self->ID, channel2->ID, operation,
				    offx, offy)) {
	PyErr_Format(pypicman_error,
		     "could not combine masks with channels (ID %d and ID %d) "
		     "with operation %d, offset %d, %d",
		     self->ID, channel2->ID, operation, offx, offy);
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef chn_methods[] = {
    {"copy",	(PyCFunction)chn_copy,	METH_NOARGS},
    {"combine_masks",	(PyCFunction)chn_combine_masks,	METH_VARARGS},
    {NULL,		NULL}		/* sentinel */
};

static PyObject *
chn_get_color(PyPicmanChannel *self, void *closure)
{
    PicmanRGB rgb;

    if (!picman_channel_get_color(self->ID, &rgb)) {
	PyErr_Format(pypicman_error,
		     "could not get compositing color of channel (ID %d)",
		     self->ID);
	return NULL;
    }

    return pypicman_rgb_new(&rgb);
}

static int
chn_set_color(PyPicmanChannel *self, PyObject *value, void *closure)
{
    guchar r, g, b;
    PicmanRGB tmprgb, *rgb;

    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete color");
        return -1;
    }

    if (pypicman_rgb_check(value)) {
	rgb = pyg_boxed_get(value, PicmanRGB);
    } else if (PyTuple_Check(value) &&
	       PyArg_ParseTuple(value, "(BBB)", &r, &g, &b)) {
	picman_rgb_set_uchar(&tmprgb, r, g, b);
	rgb = &tmprgb;
    } else {
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "type mismatch");
	return -1;
    }

    if (!picman_channel_set_color(self->ID, rgb)) {
	PyErr_Format(pypicman_error,
		     "could not set compositing color on channel (ID %d)",
		     self->ID);
	return -1;
    }

    return 0;
}

static PyObject *
chn_get_opacity(PyPicmanLayer *self, void *closure)
{
    return PyFloat_FromDouble(picman_channel_get_opacity(self->ID));
}

static int
chn_set_opacity(PyPicmanLayer *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete opacity");
        return -1;
    }

    if (!PyFloat_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "type mismatch");
	return -1;
    }

    if (!picman_channel_set_opacity(self->ID, PyFloat_AsDouble(value))) {
	PyErr_Format(pypicman_error,
		     "could not set opacity on channel (ID %d)",
		     self->ID);
	return -1;
    }

    return 0;
}

static PyObject *
chn_get_show_masked(PyPicmanLayer *self, void *closure)
{
    return PyBool_FromLong(picman_channel_get_show_masked(self->ID));
}

static int
chn_set_show_masked(PyPicmanLayer *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "cannot delete show_masked");
        return -1;
    }

    if (!PyInt_Check(value)) {
	PyErr_SetString(PyExc_TypeError, "type mismatch");
	return -1;
    }

    if (!picman_channel_set_show_masked(self->ID, PyInt_AsLong(value))) {
	PyErr_Format(pypicman_error,
		     "could not set composite method on channel (ID %d)",
		     self->ID);
	return -1;
    }

    return 0;
}

static PyGetSetDef chn_getsets[] = {
    { "color", (getter)chn_get_color, (setter)chn_set_color },
    { "colour", (getter)chn_get_color, (setter)chn_set_color },
    { "opacity", (getter)chn_get_opacity, (setter)chn_set_opacity },
    { "show_masked", (getter)chn_get_show_masked, (setter)chn_set_show_masked},
    { NULL, (getter)0, (setter)0 }
};

static PyObject *
chn_repr(PyPicmanChannel *self)
{
    PyObject *s;
    gchar *name;

    name = picman_item_get_name(self->ID);
    s = PyString_FromFormat("<picman.Channel '%s'>", name ? name : "(null)");
    g_free(name);

    return s;
}

static int
chn_init(PyPicmanChannel *self, PyObject *args, PyObject *kwargs)
{
    PyPicmanImage *img;
    PyObject *color;
    char *name;
    unsigned int width, height, r, g, b;
    double opacity;
    PicmanRGB tmprgb, *rgb;

    if (!PyArg_ParseTuple(args, "O!siidO:picman.Channel.__init__",
			  &PyPicmanImage_Type, &img, &name, &width,
			  &height, &opacity, &color))
	return -1;

    if (pypicman_rgb_check(color)) {
	rgb = pyg_boxed_get(color, PicmanRGB);
    } else if (PyTuple_Check(color) &&
	       PyArg_ParseTuple(color, "(BBB)", &r, &g, &b)) {
	picman_rgb_set_uchar(&tmprgb, r, g, b);
	rgb = &tmprgb;
    } else {
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "type mismatch");
	return -1;
    }

    self->ID = picman_channel_new(img->ID, name, width, height, opacity, rgb);

    self->drawable = NULL;

    if (self->ID < 0) {
	PyErr_Format(pypicman_error,
		     "could not create %dx%d channel '%s' on image (ID %d)",
		     width, height, name, img->ID);
	return -1;
    }

    return 0;
}

PyTypeObject PyPicmanChannel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "picman.Channel",                     /* tp_name */
    sizeof(PyPicmanChannel),              /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)drw_dealloc,            /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)drw_cmp,                   /* tp_compare */
    (reprfunc)chn_repr,                 /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    (hashfunc)0,                        /* tp_hash */
    (ternaryfunc)0,                     /* tp_call */
    (reprfunc)0,                        /* tp_str */
    (getattrofunc)0,                    /* tp_getattro */
    (setattrofunc)0,                    /* tp_setattro */
    0,					/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,	                /* tp_flags */
    NULL, /* Documentation string */
    (traverseproc)0,			/* tp_traverse */
    (inquiry)0,				/* tp_clear */
    (richcmpfunc)0,			/* tp_richcompare */
    0,					/* tp_weaklistoffset */
    (getiterfunc)0,			/* tp_iter */
    (iternextfunc)0,			/* tp_iternext */
    chn_methods,			/* tp_methods */
    0,					/* tp_members */
    chn_getsets,			/* tp_getset */
    &PyPicmanDrawable_Type,		/* tp_base */
    (PyObject *)0,			/* tp_dict */
    0,					/* tp_descr_get */
    0,					/* tp_descr_set */
    0,					/* tp_dictoffset */
    (initproc)chn_init,                 /* tp_init */
    (allocfunc)0,			/* tp_alloc */
    (newfunc)0,				/* tp_new */
};

PyObject *
pypicman_channel_new(gint32 ID)
{
    PyPicmanChannel *self;

    if (!picman_item_is_valid(ID) || !picman_item_is_channel(ID)) {
	Py_INCREF(Py_None);
	return Py_None;
    }

    self = PyObject_NEW(PyPicmanChannel, &PyPicmanChannel_Type);

    if (self == NULL)
	return NULL;

    self->ID = ID;
    self->drawable = NULL;

    return (PyObject *)self;
}
