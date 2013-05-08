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

#define NO_IMPORT_PYGOBJECT

#include "pypicman.h"
#include "pypicmancolor.h"

#include <libpicmanmath/picmanmath.h>

static PyObject *
rgb_set(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *r = NULL, *g = NULL, *b = NULL, *a = NULL;
    PicmanRGB tmprgb, *rgb;
    static char *kwlist[] = { "r", "g", "b", "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOOO:set", kwlist,
				     &r, &g, &b, &a))
        return NULL;

    if (!r && !g && !b && !a) {
	PyErr_SetString(PyExc_TypeError, "must provide r,g,b or a arguments");
	return NULL;
    }

    if ((r && (!g || !b)) ||
        (g && (!r || !b)) ||
        (b && (!r || !g))) {
	PyErr_SetString(PyExc_TypeError, "must provide all 3 r,g,b arguments");
	return NULL;
    }

    rgb = pyg_boxed_get(self, PicmanRGB);
    tmprgb = *rgb;

#define SET_MEMBER(m)	G_STMT_START {				\
    if (PyInt_Check(m))						\
	tmprgb.m = (double) PyInt_AS_LONG(m) / 255.0;		\
    else if (PyFloat_Check(m))					\
        tmprgb.m = PyFloat_AS_DOUBLE(m);			\
    else {							\
	PyErr_SetString(PyExc_TypeError,			\
			#m " must be an int or a float");	\
	return NULL;						\
    }								\
} G_STMT_END

    if (r) {
	SET_MEMBER(r);
	SET_MEMBER(g);
	SET_MEMBER(b);
    }

    if (a)
	SET_MEMBER(a);

#undef SET_MEMBER

    *rgb = tmprgb;

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rgb_set_alpha(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *py_a;
    PicmanRGB *rgb;
    static char *kwlist[] = { "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
	                             "O:set_alpha", kwlist,
				     &py_a))
        return NULL;

    rgb = pyg_boxed_get(self, PicmanRGB);

    if (PyInt_Check(py_a))
	rgb->a = (double) PyInt_AS_LONG(py_a) / 255.0;
    else if (PyFloat_Check(py_a))
        rgb->a = PyFloat_AS_DOUBLE(py_a);
    else {
	PyErr_SetString(PyExc_TypeError, "a must be an int or a float");
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rgb_add(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *color;
    gboolean with_alpha = FALSE;
    static char *kwlist[] = { "color", "with_alpha", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!|i:add", kwlist,
				     &PyPicmanRGB_Type, &color, &with_alpha))
        return NULL;

    if (with_alpha)
	picman_rgba_add(pyg_boxed_get(self, PicmanRGB),
		      pyg_boxed_get(color, PicmanRGB));
    else
	picman_rgb_add(pyg_boxed_get(self, PicmanRGB),
		     pyg_boxed_get(color, PicmanRGB));

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rgb_subtract(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *color;
    gboolean with_alpha = FALSE;
    static char *kwlist[] = { "color", "with_alpha", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!|i:subtract", kwlist,
				     &PyPicmanRGB_Type, &color, &with_alpha))
        return NULL;

    if (with_alpha)
	picman_rgba_subtract(pyg_boxed_get(self, PicmanRGB),
			   pyg_boxed_get(color, PicmanRGB));
    else
	picman_rgb_subtract(pyg_boxed_get(self, PicmanRGB),
			  pyg_boxed_get(color, PicmanRGB));

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rgb_multiply(PyObject *self, PyObject *args, PyObject *kwargs)
{
    double factor;
    gboolean with_alpha = FALSE;
    static char *kwlist[] = { "factor", "with_alpha", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "d|i:multiply", kwlist,
				     &factor, &with_alpha))
        return NULL;

    if (with_alpha)
	picman_rgba_multiply(pyg_boxed_get(self, PicmanRGB), factor);
    else
	picman_rgb_multiply(pyg_boxed_get(self, PicmanRGB), factor);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rgb_distance(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *color;
    gboolean alpha = FALSE;
    double ret;
    static char *kwlist[] = { "color", "alpha", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!|i:distance", kwlist,
				     &PyPicmanRGB_Type, &color, &alpha))
        return NULL;

    ret = picman_rgb_distance(pyg_boxed_get(self, PicmanRGB),
			    pyg_boxed_get(color, PicmanRGB));


    return PyFloat_FromDouble(ret);
}

static PyObject *
rgb_max(PyObject *self)
{
    return PyFloat_FromDouble(picman_rgb_max(pyg_boxed_get(self, PicmanRGB)));
}

static PyObject *
rgb_min(PyObject *self)
{
    return PyFloat_FromDouble(picman_rgb_min(pyg_boxed_get(self, PicmanRGB)));
}

static PyObject *
rgb_clamp(PyObject *self)
{
    picman_rgb_clamp(pyg_boxed_get(self, PicmanRGB));

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rgb_gamma(PyObject *self, PyObject *args, PyObject *kwargs)
{
    double gamma;
    static char *kwlist[] = { "gamma", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "d:gamma", kwlist, &gamma))
        return NULL;

    picman_rgb_gamma(pyg_boxed_get(self, PicmanRGB), gamma);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rgb_luminance(PyObject *self)
{
    return PyFloat_FromDouble(picman_rgb_luminance(pyg_boxed_get(self, PicmanRGB)));
}

static PyObject *
rgb_composite(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *color;
    int mode = PICMAN_RGB_COMPOSITE_NORMAL;
    static char *kwlist[] = { "color", "mode", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O!|i:composite", kwlist,
				     &PyPicmanRGB_Type, &color, &mode))
        return NULL;

    if (mode < PICMAN_RGB_COMPOSITE_NONE || mode > PICMAN_RGB_COMPOSITE_BEHIND) {
	PyErr_SetString(PyExc_TypeError, "composite type is not valid");
	return NULL;
    }

    picman_rgb_composite(pyg_boxed_get(self, PicmanRGB),
		       pyg_boxed_get(color, PicmanRGB),
		       mode);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rgb_parse_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char *name;
    int len;
    gboolean success;
    static char *kwlist[] = { "name", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s#:parse_name", kwlist,
				     &name, &len))
        return NULL;

    success = picman_rgb_parse_name(pyg_boxed_get(self, PicmanRGB), name, len);

    if (!success) {
	PyErr_SetString(PyExc_ValueError, "unable to parse color name");
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rgb_parse_hex(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char *hex;
    int len;
    gboolean success;
    static char *kwlist[] = { "hex", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s#:parse_hex", kwlist,
				     &hex, &len))
        return NULL;

    success = picman_rgb_parse_hex(pyg_boxed_get(self, PicmanRGB), hex, len);

    if (!success) {
	PyErr_SetString(PyExc_ValueError, "unable to parse hex value");
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rgb_parse_css(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char *css;
    int len;
    gboolean success, with_alpha = FALSE;
    static char *kwlist[] = { "css", "with_alpha", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "s#|i:parse_css", kwlist,
				     &css, &len, &with_alpha))
        return NULL;

    if (with_alpha)
	success = picman_rgba_parse_css(pyg_boxed_get(self, PicmanRGB), css, len);
    else
	success = picman_rgb_parse_css(pyg_boxed_get(self, PicmanRGB), css, len);

    if (!success) {
	PyErr_SetString(PyExc_ValueError, "unable to parse CSS color");
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
rgb_to_hsv(PyObject *self)
{
    PicmanRGB *rgb;
    PicmanHSV hsv;

    rgb = pyg_boxed_get(self, PicmanRGB);

    picman_rgb_to_hsv(rgb, &hsv);

    return pypicman_hsv_new(&hsv);
}

static PyObject *
rgb_to_hsl(PyObject *self)
{
    PicmanRGB *rgb;
    PicmanHSL hsl;

    rgb = pyg_boxed_get(self, PicmanRGB);

    picman_rgb_to_hsl(rgb, &hsl);

    return pypicman_hsl_new(&hsl);
}

static PyObject *
rgb_to_cmyk(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PicmanRGB *rgb;
    PicmanCMYK cmyk;
    gdouble pullout = 1.0;
    static char *kwlist[] = { "pullout", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|d:to_cmyk", kwlist,
				     &pullout))
	return NULL;

    rgb = pyg_boxed_get(self, PicmanRGB);

    picman_rgb_to_cmyk(rgb, pullout, &cmyk);

    return pypicman_cmyk_new(&cmyk);
}

/* __getstate__ isn't exposed */
static PyObject *
rgb_getstate(PyObject *self)
{
    PicmanRGB *rgb;

    rgb = pyg_boxed_get(self, PicmanRGB);

    return Py_BuildValue("dddd", rgb->r, rgb->g, rgb->b, rgb->a);
}

static PyObject *
rgb_reduce(PyObject *self)
{
    return Py_BuildValue("ON", self->ob_type, rgb_getstate(self));
}

static PyMethodDef rgb_methods[] = {
    { "set", (PyCFunction)rgb_set, METH_VARARGS|METH_KEYWORDS },
    { "set_alpha", (PyCFunction)rgb_set_alpha, METH_VARARGS|METH_KEYWORDS },
    { "add", (PyCFunction)rgb_add, METH_VARARGS|METH_KEYWORDS },
    { "subtract", (PyCFunction)rgb_subtract, METH_VARARGS|METH_KEYWORDS },
    { "multiply", (PyCFunction)rgb_multiply, METH_VARARGS|METH_KEYWORDS },
    { "distance", (PyCFunction)rgb_distance, METH_VARARGS|METH_KEYWORDS },
    { "max", (PyCFunction)rgb_max, METH_NOARGS },
    { "min", (PyCFunction)rgb_min, METH_NOARGS },
    { "clamp", (PyCFunction)rgb_clamp, METH_NOARGS },
    { "gamma", (PyCFunction)rgb_gamma, METH_VARARGS|METH_KEYWORDS },
    { "luminance", (PyCFunction)rgb_luminance, METH_NOARGS },
    { "composite", (PyCFunction)rgb_composite, METH_VARARGS|METH_KEYWORDS },
    { "parse_name", (PyCFunction)rgb_parse_name, METH_VARARGS|METH_KEYWORDS },
    { "parse_hex", (PyCFunction)rgb_parse_hex, METH_VARARGS|METH_KEYWORDS },
    { "parse_css", (PyCFunction)rgb_parse_css, METH_VARARGS|METH_KEYWORDS },
    { "to_hsv", (PyCFunction)rgb_to_hsv, METH_NOARGS },
    { "to_hsl", (PyCFunction)rgb_to_hsl, METH_NOARGS },
    { "to_cmyk", (PyCFunction)rgb_to_cmyk, METH_VARARGS|METH_KEYWORDS },
    { "__reduce__", (PyCFunction)rgb_reduce, METH_NOARGS },
    { NULL, NULL, 0 }
};

#define MEMBER_ACCESSOR(m) \
static PyObject *							\
rgb_get_ ## m(PyObject *self, void *closure)				\
{									\
    return PyFloat_FromDouble(pyg_boxed_get(self, PicmanRGB)->m);		\
}									\
static int								\
rgb_set_ ## m(PyObject *self, PyObject *value, void *closure)		\
{									\
    PicmanRGB *rgb = pyg_boxed_get(self, PicmanRGB);			\
    if (value == NULL) {						\
	PyErr_SetString(PyExc_TypeError, "cannot delete value");	\
	return -1;							\
    }									\
    else if (PyInt_Check(value))					\
	rgb->m = (double) PyInt_AS_LONG(value) / 255.0;			\
    else if (PyFloat_Check(value))					\
        rgb->m = PyFloat_AS_DOUBLE(value);				\
    else {								\
	PyErr_SetString(PyExc_TypeError, "type mismatch");		\
	return -1;							\
    }									\
    return 0;								\
}

MEMBER_ACCESSOR(r);
MEMBER_ACCESSOR(g);
MEMBER_ACCESSOR(b);
MEMBER_ACCESSOR(a);

#undef MEMBER_ACCESSOR

static PyGetSetDef rgb_getsets[] = {
    { "r", (getter)rgb_get_r, (setter)rgb_set_r },
    { "g", (getter)rgb_get_g, (setter)rgb_set_g },
    { "b", (getter)rgb_get_b, (setter)rgb_set_b },
    { "a", (getter)rgb_get_a, (setter)rgb_set_a },
    { "red",   (getter)rgb_get_r, (setter)rgb_set_r },
    { "green", (getter)rgb_get_g, (setter)rgb_set_g },
    { "blue",  (getter)rgb_get_b, (setter)rgb_set_b },
    { "alpha", (getter)rgb_get_a, (setter)rgb_set_a },
    { NULL, (getter)0, (setter)0 },
};

static Py_ssize_t
rgb_length(PyObject *self)
{
    return 4;
}

static PyObject *
rgb_getitem(PyObject *self, Py_ssize_t pos)
{
    PicmanRGB *rgb;
    double val;

    if (pos < 0)
        pos += 4;

    if (pos < 0 || pos >= 4) {
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return NULL;
    }

    rgb = pyg_boxed_get(self, PicmanRGB);

    switch (pos) {
    case 0: val = rgb->r; break;
    case 1: val = rgb->g; break;
    case 2: val = rgb->b; break;
    case 3: val = rgb->a; break;
    default:
        g_assert_not_reached();
        return NULL;
    }

    return PyInt_FromLong(ROUND(CLAMP(val, 0.0, 1.0) * 255.0));
}

static int
rgb_setitem(PyObject *self, Py_ssize_t pos, PyObject *value)
{
    if (pos < 0)
        pos += 4;

    if (pos < 0 || pos >= 4) {
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return -1;
    }

    switch (pos) {
    case 0: return rgb_set_r(self, value, NULL);
    case 1: return rgb_set_g(self, value, NULL);
    case 2: return rgb_set_b(self, value, NULL);
    case 3: return rgb_set_a(self, value, NULL);
    default:
        g_assert_not_reached();
        return -1;
    }
}

static PyObject *
rgb_slice(PyObject *self, Py_ssize_t start, Py_ssize_t end)
{
    PyTupleObject *ret;
    Py_ssize_t i;

    if (start < 0)
	start = 0;
    if (end > 4)
	end = 4;
    if (end < start)
	end = start;

    ret = (PyTupleObject *)PyTuple_New(end - start);
    if (ret == NULL)
	return NULL;

    for (i = start; i < end; i++)
	PyTuple_SET_ITEM(ret, i - start, rgb_getitem(self, i));

    return (PyObject *)ret;
}

static PySequenceMethods rgb_as_sequence = {
    rgb_length,
    (binaryfunc)0,
    0,
    rgb_getitem,
    rgb_slice,
    rgb_setitem,
    0,
    (objobjproc)0,
};

static PyObject *
rgb_subscript(PyObject *self, PyObject *item)
{
    if (PyInt_Check(item)) {
	long i = PyInt_AS_LONG(item);
	return rgb_getitem(self, i);
    } else if (PyLong_Check(item)) {
	long i = PyLong_AsLong(item);
	if (i == -1 && PyErr_Occurred())
	    return NULL;
	return rgb_getitem(self, i);
    } else if (PySlice_Check(item)) {
	Py_ssize_t start, stop, step, slicelength, cur, i;
	PyObject *ret;

	if (PySlice_GetIndicesEx((PySliceObject*)item, 4,
				 &start, &stop, &step, &slicelength) < 0)
	    return NULL;

	if (slicelength <= 0) {
	    return PyTuple_New(0);
	} else {
	    ret = PyTuple_New(slicelength);
	    if (!ret)
		return NULL;

	    for (cur = start, i = 0; i < slicelength; cur += step, i++)
		PyTuple_SET_ITEM(ret, i, rgb_getitem(self, cur));

	    return ret;
	}
    } else if (PyString_Check(item)) {
	char *s = PyString_AsString(item);

	if (g_ascii_strcasecmp(s, "r") == 0 ||
	    g_ascii_strcasecmp(s, "red") == 0)
	    return rgb_get_r(self, NULL);
	else if (g_ascii_strcasecmp(s, "g")  == 0 ||
		 g_ascii_strcasecmp(s, "green") == 0)
	    return rgb_get_g(self, NULL);
	else if (g_ascii_strcasecmp(s, "b")  == 0 ||
		 g_ascii_strcasecmp(s, "blue") == 0)
	    return rgb_get_b(self, NULL);
	else if (g_ascii_strcasecmp(s, "a")  == 0 ||
		 g_ascii_strcasecmp(s, "alpha") == 0)
	    return rgb_get_a(self, NULL);
	else {
	    PyErr_SetObject(PyExc_KeyError, item);
	    return NULL;
	}
    } else {
	PyErr_SetString(PyExc_TypeError,
			"indices must be integers");
	return NULL;
    }
}

static PyMappingMethods rgb_as_mapping = {
    rgb_length,
    (binaryfunc)rgb_subscript,
    (objobjargproc)0
};

static long
rgb_hash(PyObject *self)
{
    long ret = -1;

    PyObject *temp = rgb_getstate(self);
    if (temp != NULL) {
	ret = PyObject_Hash(temp);
	Py_DECREF(temp);
    }

    return ret;
}

static PyObject *
rgb_richcompare(PyObject *self, PyObject *other, int op)
{
    PicmanRGB *c1, *c2;
    PyObject *ret;

    if (!pypicman_rgb_check(other)) {
	PyErr_Format(PyExc_TypeError,
		     "can't compare %s to %s",
		     self->ob_type->tp_name, other->ob_type->tp_name);
	return NULL;
    }

    if (op != Py_EQ && op != Py_NE) {
	PyErr_SetString(PyExc_TypeError,
			"can't compare color values using <, <=, >, >=");
	return NULL;
    }

    c1 = pyg_boxed_get(self, PicmanRGB);
    c2 = pyg_boxed_get(other, PicmanRGB);

    if ((c1->r == c2->r && c1->g == c2->g && c1->b == c2->b && c1->a == c2->a) == (op == Py_EQ))
	ret = Py_True;
    else
	ret = Py_False;

    Py_INCREF(ret);
    return ret;
}

static PyObject *
rgb_pretty_print(PyObject *self, gboolean inexact)
{
    PicmanRGB *rgb;
    PyObject *ret = NULL;
    PyObject *r_f = NULL, *g_f = NULL, *b_f = NULL, *a_f = NULL;
    PyObject *r = NULL, *g = NULL, *b = NULL, *a = NULL;
    reprfunc repr;
    const char *prefix;

    if (inexact) {
	repr = PyObject_Str;
	prefix = "RGB ";
    } else {
	repr = PyObject_Repr;
	prefix = self->ob_type->tp_name;
    }

    rgb = pyg_boxed_get(self, PicmanRGB);

    if ((r_f = PyFloat_FromDouble(rgb->r)) == NULL) goto cleanup;
    if ((g_f = PyFloat_FromDouble(rgb->g)) == NULL) goto cleanup;
    if ((b_f = PyFloat_FromDouble(rgb->b)) == NULL) goto cleanup;
    if ((a_f = PyFloat_FromDouble(rgb->a)) == NULL) goto cleanup;

    if ((r = repr(r_f)) == NULL) goto cleanup;
    if ((g = repr(g_f)) == NULL) goto cleanup;
    if ((b = repr(b_f)) == NULL) goto cleanup;
    if ((a = repr(a_f)) == NULL) goto cleanup;

    ret = PyString_FromFormat("%s(%s, %s, %s, %s)",
			      prefix,
			      PyString_AsString(r),
			      PyString_AsString(g),
			      PyString_AsString(b),
			      PyString_AsString(a));

cleanup:
    Py_XDECREF(r); Py_XDECREF(g); Py_XDECREF(b); Py_XDECREF(a);
    Py_XDECREF(r_f); Py_XDECREF(g_f); Py_XDECREF(b_f); Py_XDECREF(a_f);

    return ret;
}

static PyObject *
rgb_repr(PyObject *self)
{
    return rgb_pretty_print(self, FALSE);
}

static PyObject *
rgb_str(PyObject *self)
{
    return rgb_pretty_print(self, TRUE);
}

static int
rgb_init(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    PyObject *r, *g, *b, *a = NULL;
    PicmanRGB rgb;
    static char *kwlist[] = { "r", "g", "b", "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
	                             "OOO|O:set", kwlist,
				     &r, &g, &b, &a))
        return -1;

#define SET_MEMBER(m)	G_STMT_START {				\
    if (PyInt_Check(m))						\
	rgb.m = (double) PyInt_AS_LONG(m) / 255.0;		\
    else if (PyFloat_Check(m))					\
        rgb.m = PyFloat_AS_DOUBLE(m);				\
    else {							\
	PyErr_SetString(PyExc_TypeError,			\
			#m " must be an int or a float");	\
	return -1;						\
    }								\
} G_STMT_END

    SET_MEMBER(r);
    SET_MEMBER(g);
    SET_MEMBER(b);

    if (a)
	SET_MEMBER(a);
    else
        rgb.a = 1.0;

#undef SET_MEMBER

    self->gtype = PICMAN_TYPE_RGB;
    self->free_on_dealloc = TRUE;
    self->boxed = g_boxed_copy(PICMAN_TYPE_RGB, &rgb);

    return 0;
}

PyTypeObject PyPicmanRGB_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "picmancolor.RGB",			/* tp_name */
    sizeof(PyGBoxed),			/* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,			/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,			/* tp_getattr */
    (setattrfunc)0,			/* tp_setattr */
    (cmpfunc)0,				/* tp_compare */
    (reprfunc)rgb_repr,			/* tp_repr */
    (PyNumberMethods*)0,		/* tp_as_number */
    &rgb_as_sequence,			/* tp_as_sequence */
    &rgb_as_mapping,			/* tp_as_mapping */
    (hashfunc)rgb_hash,			/* tp_hash */
    (ternaryfunc)0,			/* tp_call */
    (reprfunc)rgb_str,			/* tp_str */
    (getattrofunc)0,			/* tp_getattro */
    (setattrofunc)0,			/* tp_setattro */
    (PyBufferProcs*)0,			/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,			/* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,			/* tp_traverse */
    (inquiry)0,				/* tp_clear */
    (richcmpfunc)rgb_richcompare,	/* tp_richcompare */
    0,					/* tp_weaklistoffset */
    (getiterfunc)0,			/* tp_iter */
    (iternextfunc)0,			/* tp_iternext */
    rgb_methods,			/* tp_methods */
    0,					/* tp_members */
    rgb_getsets,			/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,			/* tp_descr_get */
    (descrsetfunc)0,			/* tp_descr_set */
    0,					/* tp_dictoffset */
    (initproc)rgb_init,			/* tp_init */
    (allocfunc)0,			/* tp_alloc */
    (newfunc)0,				/* tp_new */
    (freefunc)0,			/* tp_free */
    (inquiry)0				/* tp_is_gc */
};

PyObject *
pypicman_rgb_new(const PicmanRGB *rgb)
{
    return pyg_boxed_new(PICMAN_TYPE_RGB, (gpointer)rgb, TRUE, TRUE);
}


static PyObject *
hsv_set(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *h = NULL, *s = NULL, *v = NULL, *a = NULL;
    PicmanHSV tmphsv, *hsv;
    static char *kwlist[] = { "h", "s", "v", "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOOO:set", kwlist,
				     &h, &s, &v, &a))
        return NULL;

    if (!h && !s && !v && !a) {
	PyErr_SetString(PyExc_TypeError, "must provide h,s,v or a arguments");
	return NULL;
    }

    if ((h && (!s || !v)) ||
        (s && (!h || !v)) ||
        (v && (!h || !s))) {
	PyErr_SetString(PyExc_TypeError, "must provide all 3 h,s,v arguments");
	return NULL;
    }

    hsv = pyg_boxed_get(self, PicmanHSV);
    tmphsv = *hsv;

#define SET_MEMBER(m, s)	G_STMT_START {			\
    if (PyInt_Check(m))						\
        tmphsv.m = (double) PyInt_AS_LONG(m) / s;		\
    else if (PyFloat_Check(m))					\
        tmphsv.m = PyFloat_AS_DOUBLE(m);			\
    else {							\
	PyErr_SetString(PyExc_TypeError,			\
			#m " must be a float");			\
	return NULL;						\
    }								\
} G_STMT_END

    if (h) {
	SET_MEMBER(h, 360.0);
	SET_MEMBER(s, 100.0);
	SET_MEMBER(v, 100.0);
    }

    if (a)
	SET_MEMBER(a, 255.0);

#undef SET_MEMBER

    *hsv = tmphsv;

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
hsv_set_alpha(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *py_a;
    PicmanHSV *hsv;
    static char *kwlist[] = { "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
	                             "O:set_alpha", kwlist,
				     &py_a))
        return NULL;

    hsv = pyg_boxed_get(self, PicmanHSV);

    if (PyInt_Check(py_a))
        hsv->a = (double) PyInt_AS_LONG(py_a) / 255.0;
    else if (PyFloat_Check(py_a))
        hsv->a = PyFloat_AS_DOUBLE(py_a);
    else {
	PyErr_SetString(PyExc_TypeError, "a must be a float");
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
hsv_clamp(PyObject *self)
{
    picman_hsv_clamp(pyg_boxed_get(self, PicmanHSV));

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
hsv_to_rgb(PyObject *self)
{
    PicmanHSV *hsv;
    PicmanRGB rgb;

    hsv = pyg_boxed_get(self, PicmanHSV);

    picman_hsv_to_rgb(hsv, &rgb);

    return pypicman_rgb_new(&rgb);
}

/* __getstate__ isn't exposed */
static PyObject *
hsv_getstate(PyObject *self)
{
    PicmanHSV *hsv;

    hsv = pyg_boxed_get(self, PicmanHSV);

    return Py_BuildValue("dddd", hsv->h, hsv->s, hsv->v, hsv->a);
}

static PyObject *
hsv_reduce(PyObject *self)
{
    return Py_BuildValue("ON", self->ob_type, hsv_getstate(self));
}

static PyMethodDef hsv_methods[] = {
    { "set", (PyCFunction)hsv_set, METH_VARARGS|METH_KEYWORDS },
    { "set_alpha", (PyCFunction)hsv_set_alpha, METH_VARARGS|METH_KEYWORDS },
    { "clamp", (PyCFunction)hsv_clamp, METH_NOARGS },
    { "to_rgb", (PyCFunction)hsv_to_rgb, METH_NOARGS },
    { "__reduce__", (PyCFunction)hsv_reduce, METH_NOARGS },
    { NULL, NULL, 0 }
};

#define MEMBER_ACCESSOR(m, s) \
static PyObject *							\
hsv_get_ ## m(PyObject *self, void *closure)				\
{									\
    return PyFloat_FromDouble(pyg_boxed_get(self, PicmanHSV)->m);		\
}									\
static int								\
hsv_set_ ## m(PyObject *self, PyObject *value, void *closure)		\
{									\
    PicmanHSV *hsv = pyg_boxed_get(self, PicmanHSV);			\
    if (value == NULL) {						\
	PyErr_SetString(PyExc_TypeError, "cannot delete value");	\
	return -1;							\
    }									\
    else if (PyInt_Check(value))					\
        hsv->m = (double) PyInt_AS_LONG(value) / s;			\
    else if (PyFloat_Check(value))					\
        hsv->m = PyFloat_AS_DOUBLE(value);				\
    else {								\
	PyErr_SetString(PyExc_TypeError, "type mismatch");		\
	return -1;							\
    }									\
    return 0;								\
}

MEMBER_ACCESSOR(h, 360.0);
MEMBER_ACCESSOR(s, 100.0);
MEMBER_ACCESSOR(v, 100.0);
MEMBER_ACCESSOR(a, 255.0);

#undef MEMBER_ACCESSOR

static PyGetSetDef hsv_getsets[] = {
    { "h", (getter)hsv_get_h, (setter)hsv_set_h },
    { "s", (getter)hsv_get_s, (setter)hsv_set_s },
    { "v", (getter)hsv_get_v, (setter)hsv_set_v },
    { "a", (getter)hsv_get_a, (setter)hsv_set_a },
    { "hue", (getter)hsv_get_h, (setter)hsv_set_h },
    { "saturation", (getter)hsv_get_s, (setter)hsv_set_s },
    { "value", (getter)hsv_get_v, (setter)hsv_set_v },
    { "alpha", (getter)hsv_get_a, (setter)hsv_set_a },
    { NULL, (getter)0, (setter)0 },
};

static Py_ssize_t
hsv_length(PyObject *self)
{
    return 4;
}

static PyObject *
hsv_getitem(PyObject *self, Py_ssize_t pos)
{
    PicmanHSV *hsv;
    double val, scale_factor;

    if (pos < 0)
        pos += 4;

    if (pos < 0 || pos >= 4) {
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return NULL;
    }

    hsv = pyg_boxed_get(self, PicmanHSV);

    switch (pos) {
    case 0: val = hsv->h; scale_factor = 360.0; break;
    case 1: val = hsv->s; scale_factor = 100.0; break;
    case 2: val = hsv->v; scale_factor = 100.0; break;
    case 3: val = hsv->a; scale_factor = 255.0; break;
    default:
        g_assert_not_reached();
        return NULL;
    }

    return PyInt_FromLong(ROUND(CLAMP(val, 0.0, 1.0) * scale_factor));
}

static int
hsv_setitem(PyObject *self, Py_ssize_t pos, PyObject *value)
{
    if (pos < 0)
        pos += 4;

    if (pos < 0 || pos >= 4) {
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return -1;
    }

    switch (pos) {
    case 0: return hsv_set_h(self, value, NULL);
    case 1: return hsv_set_s(self, value, NULL);
    case 2: return hsv_set_v(self, value, NULL);
    case 3: return hsv_set_a(self, value, NULL);
    default:
        g_assert_not_reached();
        return -1;
    }
}

static PyObject *
hsv_slice(PyObject *self, Py_ssize_t start, Py_ssize_t end)
{
    PyTupleObject *ret;
    Py_ssize_t i;

    if (start < 0)
	start = 0;
    if (end > 4)
	end = 4;
    if (end < start)
	end = start;

    ret = (PyTupleObject *)PyTuple_New(end - start);
    if (ret == NULL)
	return NULL;

    for (i = start; i < end; i++)
	PyTuple_SET_ITEM(ret, i - start, hsv_getitem(self, i));

    return (PyObject *)ret;
}

static PySequenceMethods hsv_as_sequence = {
    hsv_length,
    (binaryfunc)0,
    0,
    hsv_getitem,
    hsv_slice,
    hsv_setitem,
    0,
    (objobjproc)0,
};

static PyObject *
hsv_subscript(PyObject *self, PyObject *item)
{
    if (PyInt_Check(item)) {
	long i = PyInt_AS_LONG(item);
	return hsv_getitem(self, i);
    } else if (PyLong_Check(item)) {
	long i = PyLong_AsLong(item);
	if (i == -1 && PyErr_Occurred())
	    return NULL;
	return hsv_getitem(self, i);
    } else if (PySlice_Check(item)) {
	Py_ssize_t start, stop, step, slicelength, cur, i;
	PyObject *ret;

	if (PySlice_GetIndicesEx((PySliceObject*)item, 4,
				 &start, &stop, &step, &slicelength) < 0)
	    return NULL;

	if (slicelength <= 0) {
	    return PyTuple_New(0);
	} else {
	    ret = PyTuple_New(slicelength);
	    if (!ret)
		return NULL;

	    for (cur = start, i = 0; i < slicelength; cur += step, i++)
		PyTuple_SET_ITEM(ret, i, hsv_getitem(self, cur));

	    return ret;
	}
    } else if (PyString_Check(item)) {
	char *s = PyString_AsString(item);

	if (g_ascii_strcasecmp(s, "h") == 0 ||
	    g_ascii_strcasecmp(s, "hue") == 0)
	    return hsv_get_h(self, NULL);
	else if (g_ascii_strcasecmp(s, "s")  == 0 ||
		 g_ascii_strcasecmp(s, "saturation") == 0)
	    return hsv_get_s(self, NULL);
	else if (g_ascii_strcasecmp(s, "v")  == 0 ||
		 g_ascii_strcasecmp(s, "value") == 0)
	    return hsv_get_v(self, NULL);
	else if (g_ascii_strcasecmp(s, "a")  == 0 ||
		 g_ascii_strcasecmp(s, "alpha") == 0)
	    return hsv_get_a(self, NULL);
	else {
	    PyErr_SetObject(PyExc_KeyError, item);
	    return NULL;
	}
    } else {
	PyErr_SetString(PyExc_TypeError,
			"indices must be integers");
	return NULL;
    }
}

static PyMappingMethods hsv_as_mapping = {
    hsv_length,
    (binaryfunc)hsv_subscript,
    (objobjargproc)0
};

static long
hsv_hash(PyObject *self)
{
    long ret = -1;

    PyObject *temp = hsv_getstate(self);
    if (temp != NULL) {
	ret = PyObject_Hash(temp);
	Py_DECREF(temp);
    }

    return ret;
}

static PyObject *
hsv_richcompare(PyObject *self, PyObject *other, int op)
{
    PicmanHSV *c1, *c2;
    PyObject *ret;

    if (!pypicman_hsv_check(other)) {
	PyErr_Format(PyExc_TypeError,
		     "can't compare %s to %s",
		     self->ob_type->tp_name, other->ob_type->tp_name);
	return NULL;
    }

    if (op != Py_EQ && op != Py_NE) {
	PyErr_SetString(PyExc_TypeError,
			"can't compare color values using <, <=, >, >=");
	return NULL;
    }

    c1 = pyg_boxed_get(self, PicmanHSV);
    c2 = pyg_boxed_get(other, PicmanHSV);

    if ((c1->h == c2->h && c1->s == c2->s && c1->v == c2->v && c1->a == c2->a) == (op == Py_EQ))
	ret = Py_True;
    else
	ret = Py_False;

    Py_INCREF(ret);
    return ret;
}

static PyObject *
hsv_pretty_print(PyObject *self, gboolean inexact)
{
    PicmanHSV *hsv;
    PyObject *ret = NULL;
    PyObject *h_f = NULL, *s_f = NULL, *v_f = NULL, *a_f = NULL;
    PyObject *h = NULL, *s = NULL, *v = NULL, *a = NULL;
    reprfunc repr;
    const char *prefix;

    if (inexact) {
	repr = PyObject_Str;
	prefix = "HSV ";
    } else {
	repr = PyObject_Repr;
	prefix = self->ob_type->tp_name;
    }

    hsv = pyg_boxed_get(self, PicmanHSV);

    if ((h_f = PyFloat_FromDouble(hsv->h)) == NULL) goto cleanup;
    if ((s_f = PyFloat_FromDouble(hsv->s)) == NULL) goto cleanup;
    if ((v_f = PyFloat_FromDouble(hsv->v)) == NULL) goto cleanup;
    if ((a_f = PyFloat_FromDouble(hsv->a)) == NULL) goto cleanup;

    if ((h = repr(h_f)) == NULL) goto cleanup;
    if ((s = repr(s_f)) == NULL) goto cleanup;
    if ((v = repr(v_f)) == NULL) goto cleanup;
    if ((a = repr(a_f)) == NULL) goto cleanup;

    ret = PyString_FromFormat("%s(%s, %s, %s, %s)",
			      prefix,
			      PyString_AsString(h),
			      PyString_AsString(s),
			      PyString_AsString(v),
			      PyString_AsString(a));

cleanup:
    Py_XDECREF(h); Py_XDECREF(s); Py_XDECREF(v); Py_XDECREF(a);
    Py_XDECREF(h_f); Py_XDECREF(s_f); Py_XDECREF(v_f); Py_XDECREF(a_f);

    return ret;
}

static PyObject *
hsv_repr(PyObject *self)
{
    return hsv_pretty_print(self, FALSE);
}

static PyObject *
hsv_str(PyObject *self)
{
    return hsv_pretty_print(self, TRUE);
}

static int
hsv_init(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    PyObject *h, *s, *v, *a = NULL;
    PicmanHSV hsv;
    static char *kwlist[] = { "h", "s", "v", "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
	                             "OOO|O:set", kwlist,
				     &h, &s, &v, &a))
        return -1;

#define SET_MEMBER(m, s)	G_STMT_START {			\
    if (PyInt_Check(m))						\
        hsv.m = (double) PyInt_AS_LONG(m) / s;			\
    else if (PyFloat_Check(m))					\
        hsv.m = PyFloat_AS_DOUBLE(m);				\
    else {							\
	PyErr_SetString(PyExc_TypeError,			\
			#m " must be an int or a float");	\
	return -1;						\
    }								\
} G_STMT_END

    SET_MEMBER(h, 360.0);
    SET_MEMBER(s, 100.0);
    SET_MEMBER(v, 100.0);

    if (a)
	SET_MEMBER(a, 255.0);
    else
        hsv.a = 1.0;

#undef SET_MEMBER

    self->gtype = PICMAN_TYPE_HSV;
    self->free_on_dealloc = TRUE;
    self->boxed = g_boxed_copy(PICMAN_TYPE_HSV, &hsv);

    return 0;
}

PyTypeObject PyPicmanHSV_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "picmancolor.HSV",			/* tp_name */
    sizeof(PyGBoxed),			/* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,			/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,			/* tp_getattr */
    (setattrfunc)0,			/* tp_setattr */
    (cmpfunc)0,				/* tp_compare */
    (reprfunc)hsv_repr,			/* tp_repr */
    (PyNumberMethods*)0,		/* tp_as_number */
    &hsv_as_sequence,			/* tp_as_sequence */
    &hsv_as_mapping,			/* tp_as_mapping */
    (hashfunc)hsv_hash,			/* tp_hash */
    (ternaryfunc)0,			/* tp_call */
    (reprfunc)hsv_str,			/* tp_repr */
    (getattrofunc)0,			/* tp_getattro */
    (setattrofunc)0,			/* tp_setattro */
    (PyBufferProcs*)0,			/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,			/* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,			/* tp_traverse */
    (inquiry)0,				/* tp_clear */
    (richcmpfunc)hsv_richcompare,	/* tp_richcompare */
    0,					/* tp_weaklistoffset */
    (getiterfunc)0,			/* tp_iter */
    (iternextfunc)0,			/* tp_iternext */
    hsv_methods,			/* tp_methods */
    0,					/* tp_members */
    hsv_getsets,			/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,			/* tp_descr_get */
    (descrsetfunc)0,			/* tp_descr_set */
    0,					/* tp_dictoffset */
    (initproc)hsv_init,			/* tp_init */
    (allocfunc)0,			/* tp_alloc */
    (newfunc)0,				/* tp_new */
    (freefunc)0,			/* tp_free */
    (inquiry)0				/* tp_is_gc */
};

PyObject *
pypicman_hsv_new(const PicmanHSV *hsv)
{
    return pyg_boxed_new(PICMAN_TYPE_HSV, (gpointer)hsv, TRUE, TRUE);
}


static PyObject *
hsl_set(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *h = NULL, *s = NULL, *l = NULL, *a = NULL;
    PicmanHSL tmphsl, *hsl;
    static char *kwlist[] = { "h", "s", "l", "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOOO:set", kwlist,
				     &h, &s, &l, &a))
        return NULL;

    if (!h && !s && !l && !a) {
	PyErr_SetString(PyExc_TypeError, "must provide h,s,l or a arguments");
	return NULL;
    }

    if ((h && (!s || !l)) ||
        (s && (!h || !l)) ||
        (l && (!h || !s))) {
	PyErr_SetString(PyExc_TypeError, "must provide all 3 h,s,l arguments");
	return NULL;
    }

    hsl = pyg_boxed_get(self, PicmanHSL);
    tmphsl = *hsl;

#define SET_MEMBER(m, s)	G_STMT_START {			\
    if (PyInt_Check(m))						\
        tmphsl.m = (double) PyInt_AS_LONG(m) / s;		\
    else if (PyFloat_Check(m))					\
        tmphsl.m = PyFloat_AS_DOUBLE(m);			\
    else {							\
	PyErr_SetString(PyExc_TypeError,			\
			#m " must be a float");			\
	return NULL;						\
    }								\
} G_STMT_END

    if (h) {
	SET_MEMBER(h, 360.0);
	SET_MEMBER(s, 100.0);
	SET_MEMBER(l, 100.0);
    }

    if (a)
	SET_MEMBER(a, 255.0);

#undef SET_MEMBER

    *hsl = tmphsl;

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
hsl_set_alpha(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *py_a;
    PicmanHSL *hsl;
    static char *kwlist[] = { "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
	                             "O:set_alpha", kwlist,
				     &py_a))
        return NULL;

    hsl = pyg_boxed_get(self, PicmanHSL);

    if (PyInt_Check(py_a))
        hsl->a = (double) PyInt_AS_LONG(py_a) / 255.0;
    else if (PyFloat_Check(py_a))
        hsl->a = PyFloat_AS_DOUBLE(py_a);
    else {
	PyErr_SetString(PyExc_TypeError, "a must be a float");
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
hsl_to_rgb(PyObject *self)
{
    PicmanHSL *hsl;
    PicmanRGB rgb;

    hsl = pyg_boxed_get(self, PicmanHSL);

    picman_hsl_to_rgb(hsl, &rgb);

    return pypicman_rgb_new(&rgb);
}

/* __getstate__ isn't exposed */
static PyObject *
hsl_getstate(PyObject *self)
{
    PicmanHSL *hsl;

    hsl = pyg_boxed_get(self, PicmanHSL);

    return Py_BuildValue("dddd", hsl->h, hsl->s, hsl->l, hsl->a);
}

static PyObject *
hsl_reduce(PyObject *self)
{
    return Py_BuildValue("ON", self->ob_type, hsl_getstate(self));
}

static PyMethodDef hsl_methods[] = {
    { "set", (PyCFunction)hsl_set, METH_VARARGS|METH_KEYWORDS },
    { "set_alpha", (PyCFunction)hsl_set_alpha, METH_VARARGS|METH_KEYWORDS },
    { "to_rgb", (PyCFunction)hsl_to_rgb, METH_NOARGS },
    { "__reduce__", (PyCFunction)hsl_reduce, METH_NOARGS },
    { NULL, NULL, 0 }
};

#define MEMBER_ACCESSOR(m, s) \
static PyObject *							\
hsl_get_ ## m(PyObject *self, void *closure)				\
{									\
    return PyFloat_FromDouble(pyg_boxed_get(self, PicmanHSL)->m);		\
}									\
static int								\
hsl_set_ ## m(PyObject *self, PyObject *value, void *closure)		\
{									\
    PicmanHSL *hsl = pyg_boxed_get(self, PicmanHSL);			\
    if (value == NULL) {						\
	PyErr_SetString(PyExc_TypeError, "cannot delete value");	\
	return -1;							\
    }									\
    else if (PyInt_Check(value))					\
        hsl->m = (double) PyInt_AS_LONG(value) / s;			\
    else if (PyFloat_Check(value))					\
        hsl->m = PyFloat_AS_DOUBLE(value);				\
    else {								\
	PyErr_SetString(PyExc_TypeError, "type mismatch");		\
	return -1;							\
    }									\
    return 0;								\
}

MEMBER_ACCESSOR(h, 360.0);
MEMBER_ACCESSOR(s, 100.0);
MEMBER_ACCESSOR(l, 100.0);
MEMBER_ACCESSOR(a, 255.0);

#undef MEMBER_ACCESSOR

static PyGetSetDef hsl_getsets[] = {
    { "h", (getter)hsl_get_h, (setter)hsl_set_h },
    { "s", (getter)hsl_get_s, (setter)hsl_set_s },
    { "l", (getter)hsl_get_l, (setter)hsl_set_l },
    { "a", (getter)hsl_get_a, (setter)hsl_set_a },
    { "hue", (getter)hsl_get_h, (setter)hsl_set_h },
    { "saturation", (getter)hsl_get_s, (setter)hsl_set_s },
    { "lightness", (getter)hsl_get_l, (setter)hsl_set_l },
    { "alpha", (getter)hsl_get_a, (setter)hsl_set_a },
    { NULL, (getter)0, (setter)0 },
};

static Py_ssize_t
hsl_length(PyObject *self)
{
    return 4;
}

static PyObject *
hsl_getitem(PyObject *self, Py_ssize_t pos)
{
    PicmanHSL *hsl;
    double val, scale_factor;

    if (pos < 0)
        pos += 4;

    if (pos < 0 || pos >= 4) {
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return NULL;
    }

    hsl = pyg_boxed_get(self, PicmanHSL);

    switch (pos) {
    case 0: val = hsl->h; scale_factor = 360.0; break;
    case 1: val = hsl->s; scale_factor = 100.0; break;
    case 2: val = hsl->l; scale_factor = 100.0; break;
    case 3: val = hsl->a; scale_factor = 255.0; break;
    default:
        g_assert_not_reached();
        return NULL;
    }

    return PyInt_FromLong(ROUND(CLAMP(val, 0.0, 1.0) * scale_factor));
}

static int
hsl_setitem(PyObject *self, Py_ssize_t pos, PyObject *value)
{
    if (pos < 0)
        pos += 4;

    if (pos < 0 || pos >= 4) {
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return -1;
    }

    switch (pos) {
    case 0: return hsl_set_h(self, value, NULL);
    case 1: return hsl_set_s(self, value, NULL);
    case 2: return hsl_set_l(self, value, NULL);
    case 3: return hsl_set_a(self, value, NULL);
    default:
        g_assert_not_reached();
        return -1;
    }
}

static PyObject *
hsl_slice(PyObject *self, Py_ssize_t start, Py_ssize_t end)
{
    PyTupleObject *ret;
    Py_ssize_t i;

    if (start < 0)
        start = 0;
    if (end > 4)
        end = 4;
    if (end < start)
        end = start;

    ret = (PyTupleObject *)PyTuple_New(end - start);
    if (ret == NULL)
        return NULL;

    for (i = start; i < end; i++)
        PyTuple_SET_ITEM(ret, i - start, hsl_getitem(self, i));

    return (PyObject *)ret;
}

static PySequenceMethods hsl_as_sequence = {
    hsl_length,
    (binaryfunc)0,
    0,
    hsl_getitem,
    hsl_slice,
    hsl_setitem,
    0,
    (objobjproc)0,
};

static PyObject *
hsl_subscript(PyObject *self, PyObject *item)
{
    if (PyInt_Check(item)) {
        long i = PyInt_AS_LONG(item);
        return hsl_getitem(self, i);
    } else if (PyLong_Check(item)) {
        long i = PyLong_AsLong(item);
        if (i == -1 && PyErr_Occurred())
            return NULL;
        return hsl_getitem(self, i);
    } else if (PySlice_Check(item)) {
        Py_ssize_t start, stop, step, slicelength, cur, i;
        PyObject *ret;

        if (PySlice_GetIndicesEx((PySliceObject*)item, 4,
                                 &start, &stop, &step, &slicelength) < 0)
            return NULL;

        if (slicelength <= 0) {
            return PyTuple_New(0);
        } else {
            ret = PyTuple_New(slicelength);
            if (!ret)
                return NULL;

            for (cur = start, i = 0; i < slicelength; cur += step, i++)
                PyTuple_SET_ITEM(ret, i, hsl_getitem(self, cur));

            return ret;
        }
    } else if (PyString_Check(item)) {
        char *s = PyString_AsString(item);

        if (g_ascii_strcasecmp(s, "h") == 0 ||
            g_ascii_strcasecmp(s, "hue") == 0)
            return hsl_get_h(self, NULL);
        else if (g_ascii_strcasecmp(s, "s")  == 0 ||
                 g_ascii_strcasecmp(s, "saturation") == 0)
            return hsl_get_s(self, NULL);
        else if (g_ascii_strcasecmp(s, "l")  == 0 ||
                 g_ascii_strcasecmp(s, "lightness") == 0)
            return hsl_get_l(self, NULL);
        else if (g_ascii_strcasecmp(s, "a")  == 0 ||
                 g_ascii_strcasecmp(s, "alpha") == 0)
            return hsl_get_a(self, NULL);
        else {
            PyErr_SetObject(PyExc_KeyError, item);
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError,
                        "indices must be integers");
        return NULL;
    }
}

static PyMappingMethods hsl_as_mapping = {
    hsl_length,
    (binaryfunc)hsl_subscript,
    (objobjargproc)0
};

static long
hsl_hash(PyObject *self)
{
    long ret = -1;

    PyObject *temp = hsl_getstate(self);
    if (temp != NULL) {
	ret = PyObject_Hash(temp);
	Py_DECREF(temp);
    }

    return ret;
}

static PyObject *
hsl_richcompare(PyObject *self, PyObject *other, int op)
{
    PicmanHSL *c1, *c2;
    PyObject *ret;

    if (!pypicman_hsl_check(other)) {
	PyErr_Format(PyExc_TypeError,
		     "can't compare %s to %s",
		     self->ob_type->tp_name, other->ob_type->tp_name);
	return NULL;
    }

    if (op != Py_EQ && op != Py_NE) {
	PyErr_SetString(PyExc_TypeError,
			"can't compare color values using <, <=, >, >=");
	return NULL;
    }

    c1 = pyg_boxed_get(self, PicmanHSL);
    c2 = pyg_boxed_get(other, PicmanHSL);

    if ((c1->h == c2->h && c1->s == c2->s && c1->l == c2->l && c1->a == c2->a) == (op == Py_EQ))
	ret = Py_True;
    else
	ret = Py_False;

    Py_INCREF(ret);
    return ret;
}

static PyObject *
hsl_pretty_print(PyObject *self, gboolean inexact)
{
    PicmanHSL *hsl;
    PyObject *ret = NULL;
    PyObject *h_f = NULL, *s_f = NULL, *l_f = NULL, *a_f = NULL;
    PyObject *h = NULL, *s = NULL, *l = NULL, *a = NULL;
    reprfunc repr;
    const char *prefix;

    if (inexact) {
	repr = PyObject_Str;
	prefix = "HSL ";
    } else {
	repr = PyObject_Repr;
	prefix = self->ob_type->tp_name;
    }

    hsl = pyg_boxed_get(self, PicmanHSL);

    if ((h_f = PyFloat_FromDouble(hsl->h)) == NULL) goto cleanup;
    if ((s_f = PyFloat_FromDouble(hsl->s)) == NULL) goto cleanup;
    if ((l_f = PyFloat_FromDouble(hsl->l)) == NULL) goto cleanup;
    if ((a_f = PyFloat_FromDouble(hsl->a)) == NULL) goto cleanup;

    if ((h = repr(h_f)) == NULL) goto cleanup;
    if ((s = repr(s_f)) == NULL) goto cleanup;
    if ((l = repr(l_f)) == NULL) goto cleanup;
    if ((a = repr(a_f)) == NULL) goto cleanup;

    ret = PyString_FromFormat("%s(%s, %s, %s, %s)",
			      prefix,
			      PyString_AsString(h),
			      PyString_AsString(s),
			      PyString_AsString(l),
			      PyString_AsString(a));

cleanup:
    Py_XDECREF(h); Py_XDECREF(s); Py_XDECREF(l); Py_XDECREF(a);
    Py_XDECREF(h_f); Py_XDECREF(s_f); Py_XDECREF(l_f); Py_XDECREF(a_f);

    return ret;
}

static PyObject *
hsl_repr(PyObject *self)
{
    return hsl_pretty_print(self, FALSE);
}

static PyObject *
hsl_str(PyObject *self)
{
    return hsl_pretty_print(self, TRUE);
}

static int
hsl_init(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    PyObject *h, *s, *l, *a = NULL;
    PicmanHSL hsl;
    static char *kwlist[] = { "h", "s", "l", "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
	                             "OOO|O:set", kwlist,
				     &h, &s, &l, &a))
        return -1;

#define SET_MEMBER(m, s)	G_STMT_START {			\
    if (PyInt_Check(m))						\
        hsl.m = (double) PyInt_AS_LONG(m) / s;			\
    else if (PyFloat_Check(m))					\
        hsl.m = PyFloat_AS_DOUBLE(m);				\
    else {							\
	PyErr_SetString(PyExc_TypeError,			\
			#m " must be a float");			\
	return -1;						\
    }								\
} G_STMT_END

    SET_MEMBER(h, 360.0);
    SET_MEMBER(s, 100.0);
    SET_MEMBER(l, 100.0);

    if (a)
	SET_MEMBER(a, 255.0);
    else
        hsl.a = 1.0;

#undef SET_MEMBER

    self->gtype = PICMAN_TYPE_HSL;
    self->free_on_dealloc = TRUE;
    self->boxed = g_boxed_copy(PICMAN_TYPE_HSL, &hsl);

    return 0;
}

PyTypeObject PyPicmanHSL_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "picmancolor.HSL",			/* tp_name */
    sizeof(PyGBoxed),			/* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,			/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,			/* tp_getattr */
    (setattrfunc)0,			/* tp_setattr */
    (cmpfunc)0,				/* tp_compare */
    (reprfunc)hsl_repr,			/* tp_repr */
    (PyNumberMethods*)0,		/* tp_as_number */
    &hsl_as_sequence,			/* tp_as_sequence */
    &hsl_as_mapping,			/* tp_as_mapping */
    (hashfunc)hsl_hash,			/* tp_hash */
    (ternaryfunc)0,			/* tp_call */
    (reprfunc)hsl_str,			/* tp_repr */
    (getattrofunc)0,			/* tp_getattro */
    (setattrofunc)0,			/* tp_setattro */
    (PyBufferProcs*)0,			/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,			/* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,			/* tp_traverse */
    (inquiry)0,				/* tp_clear */
    (richcmpfunc)hsl_richcompare,	/* tp_richcompare */
    0,					/* tp_weaklistoffset */
    (getiterfunc)0,			/* tp_iter */
    (iternextfunc)0,			/* tp_iternext */
    hsl_methods,			/* tp_methods */
    0,					/* tp_members */
    hsl_getsets,			/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,			/* tp_descr_get */
    (descrsetfunc)0,			/* tp_descr_set */
    0,					/* tp_dictoffset */
    (initproc)hsl_init,			/* tp_init */
    (allocfunc)0,			/* tp_alloc */
    (newfunc)0,				/* tp_new */
    (freefunc)0,			/* tp_free */
    (inquiry)0				/* tp_is_gc */
};

PyObject *
pypicman_hsl_new(const PicmanHSL *hsl)
{
    return pyg_boxed_new(PICMAN_TYPE_HSL, (gpointer)hsl, TRUE, TRUE);
}


static PyObject *
cmyk_set(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *c = NULL, *m = NULL, *y = NULL, *k = NULL, *a = NULL;
    PicmanCMYK tmpcmyk, *cmyk;
    static char *kwlist[] = { "c", "m", "y", "k", "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOOOO:set", kwlist,
				     &c, &m, &y, &k, &a))
        return NULL;

    if (!c && !y && !m && !k && !a) {
	PyErr_SetString(PyExc_TypeError, "must provide c,m,y,k or a arguments");
	return NULL;
    }

    if ((c && (!m || !y || !k)) ||
        (m && (!c || !y || !k)) ||
        (y && (!c || !m || !k)) ||
        (k && (!c || !m || !y))) {
	PyErr_SetString(PyExc_TypeError, "must provide all 4 c,m,y,k arguments");
	return NULL;
    }

    cmyk = pyg_boxed_get(self, PicmanCMYK);
    tmpcmyk = *cmyk;

#define SET_MEMBER(m)	G_STMT_START {				\
    if (PyInt_Check(m))						\
	tmpcmyk.m = (double) PyInt_AS_LONG(m) / 255.0;		\
    else if (PyFloat_Check(m))					\
        tmpcmyk.m = PyFloat_AS_DOUBLE(m);			\
    else {							\
	PyErr_SetString(PyExc_TypeError,			\
			#m " must be an int or a float");	\
	return NULL;						\
    }								\
} G_STMT_END

    if (c) {
	SET_MEMBER(c);
	SET_MEMBER(y);
	SET_MEMBER(m);
	SET_MEMBER(k);
    }

    if (a)
	SET_MEMBER(a);

#undef SET_MEMBER

    *cmyk = tmpcmyk;

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
cmyk_set_alpha(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *py_a;
    PicmanCMYK *cmyk;
    static char *kwlist[] = { "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
	                             "O:set_alpha", kwlist,
				     &py_a))
        return NULL;

    cmyk = pyg_boxed_get(self, PicmanCMYK);

    if (PyInt_Check(py_a))
	cmyk->a = (double) PyInt_AS_LONG(py_a) / 255.0;
    else if (PyFloat_Check(py_a))
        cmyk->a = PyFloat_AS_DOUBLE(py_a);
    else {
	PyErr_SetString(PyExc_TypeError, "a must be an int or a float");
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

/* __getstate__ isn't exposed */
static PyObject *
cmyk_getstate(PyObject *self)
{
    PicmanCMYK *cmyk;

    cmyk = pyg_boxed_get(self, PicmanCMYK);

    return Py_BuildValue("ddddd", cmyk->c, cmyk->m, cmyk->y, cmyk->k, cmyk->a);
}

static PyObject *
cmyk_reduce(PyObject *self)
{
    return Py_BuildValue("ON", self->ob_type, cmyk_getstate(self));
}

static PyMethodDef cmyk_methods[] = {
    { "set", (PyCFunction)cmyk_set, METH_VARARGS|METH_KEYWORDS },
    { "set_alpha", (PyCFunction)cmyk_set_alpha, METH_VARARGS|METH_KEYWORDS },
    { "__reduce__", (PyCFunction)cmyk_reduce, METH_NOARGS },
    { NULL, NULL, 0 }
};

#define MEMBER_ACCESSOR(m) \
static PyObject *							\
cmyk_get_ ## m(PyObject *self, void *closure)				\
{									\
    return PyFloat_FromDouble(pyg_boxed_get(self, PicmanCMYK)->m);	\
}									\
static int								\
cmyk_set_ ## m(PyObject *self, PyObject *value, void *closure)		\
{									\
    PicmanCMYK *cmyk = pyg_boxed_get(self, PicmanCMYK);			\
    if (value == NULL) {						\
	PyErr_SetString(PyExc_TypeError, "cannot delete value");	\
	return -1;							\
    }									\
    else if (PyInt_Check(value))					\
	cmyk->m = (double) PyInt_AS_LONG(value) / 255.0;		\
    else if (PyFloat_Check(value))					\
        cmyk->m = PyFloat_AS_DOUBLE(value);				\
    else {								\
	PyErr_SetString(PyExc_TypeError, "type mismatch");		\
	return -1;							\
    }									\
    return 0;								\
}

MEMBER_ACCESSOR(c);
MEMBER_ACCESSOR(m);
MEMBER_ACCESSOR(y);
MEMBER_ACCESSOR(k);
MEMBER_ACCESSOR(a);

#undef MEMBER_ACCESSOR

static PyGetSetDef cmyk_getsets[] = {
    { "c", (getter)cmyk_get_c, (setter)cmyk_set_c },
    { "m", (getter)cmyk_get_m, (setter)cmyk_set_m },
    { "y", (getter)cmyk_get_y, (setter)cmyk_set_y },
    { "k", (getter)cmyk_get_k, (setter)cmyk_set_k },
    { "a", (getter)cmyk_get_a, (setter)cmyk_set_a },
    { "cyan", (getter)cmyk_get_c, (setter)cmyk_set_c },
    { "magenta", (getter)cmyk_get_m, (setter)cmyk_set_m },
    { "yellow", (getter)cmyk_get_y, (setter)cmyk_set_y },
    { "black", (getter)cmyk_get_k, (setter)cmyk_set_k },
    { "alpha", (getter)cmyk_get_a, (setter)cmyk_set_a },
    { NULL, (getter)0, (setter)0 },
};

static Py_ssize_t
cmyk_length(PyObject *self)
{
    return 5;
}

static PyObject *
cmyk_getitem(PyObject *self, Py_ssize_t pos)
{
    PicmanCMYK *cmyk;
    double val;

    if (pos < 0)
        pos += 5;

    if (pos < 0 || pos >= 5) {
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return NULL;
    }

    cmyk = pyg_boxed_get(self, PicmanCMYK);

    switch (pos) {
    case 0: val = cmyk->c; break;
    case 1: val = cmyk->m; break;
    case 2: val = cmyk->y; break;
    case 3: val = cmyk->k; break;
    case 4: val = cmyk->a; break;
    default:
        g_assert_not_reached();
        return NULL;
    }

    return PyInt_FromLong(ROUND(CLAMP(val, 0.0, 1.0) * 255.0));
}

static int
cmyk_setitem(PyObject *self, Py_ssize_t pos, PyObject *value)
{
    if (pos < 0)
        pos += 5;

    if (pos < 0 || pos >= 5) {
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return -1;
    }

    switch (pos) {
    case 0: return cmyk_set_c(self, value, NULL);
    case 1: return cmyk_set_m(self, value, NULL);
    case 2: return cmyk_set_y(self, value, NULL);
    case 3: return cmyk_set_k(self, value, NULL);
    case 4: return cmyk_set_a(self, value, NULL);
    default:
        g_assert_not_reached();
        return -1;
    }
}

static PyObject *
cmyk_slice(PyObject *self, Py_ssize_t start, Py_ssize_t end)
{
    PyTupleObject *ret;
    Py_ssize_t i;

    if (start < 0)
        start = 0;
    if (end > 5)
        end = 5;
    if (end < start)
        end = start;

    ret = (PyTupleObject *)PyTuple_New(end - start);
    if (ret == NULL)
        return NULL;

    for (i = start; i < end; i++)
        PyTuple_SET_ITEM(ret, i - start, cmyk_getitem(self, i));

    return (PyObject *)ret;
}

static PySequenceMethods cmyk_as_sequence = {
    cmyk_length,
    (binaryfunc)0,
    0,
    cmyk_getitem,
    cmyk_slice,
    cmyk_setitem,
    0,
    (objobjproc)0,
};

static PyObject *
cmyk_subscript(PyObject *self, PyObject *item)
{
    if (PyInt_Check(item)) {
        long i = PyInt_AS_LONG(item);
        return cmyk_getitem(self, i);
    } else if (PyLong_Check(item)) {
        long i = PyLong_AsLong(item);
        if (i == -1 && PyErr_Occurred())
            return NULL;
        return cmyk_getitem(self, i);
    } else if (PySlice_Check(item)) {
        Py_ssize_t start, stop, step, slicelength, cur, i;
        PyObject *ret;

        if (PySlice_GetIndicesEx((PySliceObject*)item, 5,
                                 &start, &stop, &step, &slicelength) < 0)
            return NULL;

        if (slicelength <= 0) {
            return PyTuple_New(0);
        } else {
            ret = PyTuple_New(slicelength);
            if (!ret)
                return NULL;

            for (cur = start, i = 0; i < slicelength; cur += step, i++)
                PyTuple_SET_ITEM(ret, i, cmyk_getitem(self, cur));

            return ret;
        }
    } else if (PyString_Check(item)) {
        char *s = PyString_AsString(item);

        if (g_ascii_strcasecmp(s, "c") == 0 ||
            g_ascii_strcasecmp(s, "cyan") == 0)
            return cmyk_get_c(self, NULL);
        else if (g_ascii_strcasecmp(s, "m")  == 0 ||
                 g_ascii_strcasecmp(s, "magenta") == 0)
            return cmyk_get_m(self, NULL);
        else if (g_ascii_strcasecmp(s, "y")  == 0 ||
                 g_ascii_strcasecmp(s, "yellow") == 0)
            return cmyk_get_y(self, NULL);
        else if (g_ascii_strcasecmp(s, "k")  == 0 ||
                 g_ascii_strcasecmp(s, "black") == 0)
            return cmyk_get_k(self, NULL);
        else if (g_ascii_strcasecmp(s, "a")  == 0 ||
                 g_ascii_strcasecmp(s, "alpha") == 0)
            return cmyk_get_a(self, NULL);
        else {
            PyErr_SetObject(PyExc_KeyError, item);
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError,
                        "indices must be integers");
        return NULL;
    }
}

static PyMappingMethods cmyk_as_mapping = {
    cmyk_length,
    (binaryfunc)cmyk_subscript,
    (objobjargproc)0
};

static long
cmyk_hash(PyObject *self)
{
    long ret = -1;

    PyObject *temp = cmyk_getstate(self);
    if (temp != NULL) {
	ret = PyObject_Hash(temp);
	Py_DECREF(temp);
    }

    return ret;
}

static PyObject *
cmyk_richcompare(PyObject *self, PyObject *other, int op)
{
    PicmanCMYK *c1, *c2;
    PyObject *ret;

    if (!pypicman_cmyk_check(other)) {
	PyErr_Format(PyExc_TypeError,
		     "can't compare %s to %s",
		     self->ob_type->tp_name, other->ob_type->tp_name);
	return NULL;
    }

    if (op != Py_EQ && op != Py_NE) {
	PyErr_SetString(PyExc_TypeError,
			"can't compare color values using <, <=, >, >=");
	return NULL;
    }

    c1 = pyg_boxed_get(self, PicmanCMYK);
    c2 = pyg_boxed_get(other, PicmanCMYK);

    if ((c1->c == c2->c && c1->m == c2->m && c1->y == c2->y && c1->k == c2->k && c1->a == c2->a) == (op == Py_EQ))
	ret = Py_True;
    else
	ret = Py_False;

    Py_INCREF(ret);
    return ret;
}

static PyObject *
cmyk_pretty_print(PyObject *self, gboolean inexact)
{
    PicmanCMYK *cmyk;
    PyObject *ret = NULL;
    PyObject *c_f = NULL, *m_f = NULL, *y_f = NULL, *k_f = NULL, *a_f = NULL;
    PyObject *c = NULL, *m = NULL, *y = NULL, *k = NULL, *a = NULL;
    reprfunc repr;
    const char *prefix;

    if (inexact) {
	repr = PyObject_Str;
	prefix = "CMYK ";
    } else {
	repr = PyObject_Repr;
	prefix = self->ob_type->tp_name;
    }

    cmyk = pyg_boxed_get(self, PicmanCMYK);

    if ((c_f = PyFloat_FromDouble(cmyk->c)) == NULL) goto cleanup;
    if ((m_f = PyFloat_FromDouble(cmyk->m)) == NULL) goto cleanup;
    if ((y_f = PyFloat_FromDouble(cmyk->y)) == NULL) goto cleanup;
    if ((k_f = PyFloat_FromDouble(cmyk->k)) == NULL) goto cleanup;
    if ((a_f = PyFloat_FromDouble(cmyk->a)) == NULL) goto cleanup;

    if ((c = repr(c_f)) == NULL) goto cleanup;
    if ((m = repr(m_f)) == NULL) goto cleanup;
    if ((y = repr(y_f)) == NULL) goto cleanup;
    if ((k = repr(k_f)) == NULL) goto cleanup;
    if ((a = repr(a_f)) == NULL) goto cleanup;

    ret = PyString_FromFormat("%s(%s, %s, %s, %s, %s)",
			      prefix,
			      PyString_AsString(c),
			      PyString_AsString(m),
			      PyString_AsString(y),
			      PyString_AsString(k),
			      PyString_AsString(a));

cleanup:
    Py_XDECREF(c); Py_XDECREF(m); Py_XDECREF(y); Py_XDECREF(k); Py_XDECREF(a);
    Py_XDECREF(c_f); Py_XDECREF(m_f); Py_XDECREF(y_f); Py_XDECREF(k_f); Py_XDECREF(a_f);

    return ret;
}

static PyObject *
cmyk_repr(PyObject *self)
{
    return cmyk_pretty_print(self, FALSE);
}

static PyObject *
cmyk_str(PyObject *self)
{
    return cmyk_pretty_print(self, TRUE);
}

static int
cmyk_init(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    PyObject *c, *m, *y, *k, *a = NULL;
    PicmanCMYK cmyk;
    static char *kwlist[] = { "c", "m", "y", "k", "a", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
	                             "OOOO|O:set", kwlist,
				     &c, &m, &y, &k, &a))
        return -1;

#define SET_MEMBER(m)	G_STMT_START {				\
    if (PyInt_Check(m))						\
	cmyk.m = (double) PyInt_AS_LONG(m) / 255.0;		\
    else if (PyFloat_Check(m))					\
        cmyk.m = PyFloat_AS_DOUBLE(m);				\
    else {							\
	PyErr_SetString(PyExc_TypeError,			\
			#m " must be an int or a float");	\
	return -1;						\
    }								\
} G_STMT_END

    SET_MEMBER(c);
    SET_MEMBER(m);
    SET_MEMBER(y);
    SET_MEMBER(k);

    if (a)
	SET_MEMBER(a);
    else
        cmyk.a = 1.0;

#undef SET_MEMBER

    self->gtype = PICMAN_TYPE_CMYK;
    self->free_on_dealloc = TRUE;
    self->boxed = g_boxed_copy(PICMAN_TYPE_CMYK, &cmyk);

    return 0;
}

PyTypeObject PyPicmanCMYK_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "picmancolor.CMYK",			/* tp_name */
    sizeof(PyGBoxed),			/* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,			/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,			/* tp_getattr */
    (setattrfunc)0,			/* tp_setattr */
    (cmpfunc)0,				/* tp_compare */
    (reprfunc)cmyk_repr,		/* tp_repr */
    (PyNumberMethods*)0,		/* tp_as_number */
    &cmyk_as_sequence,			/* tp_as_sequence */
    &cmyk_as_mapping,			/* tp_as_mapping */
    (hashfunc)cmyk_hash,		/* tp_hash */
    (ternaryfunc)0,			/* tp_call */
    (reprfunc)cmyk_str,			/* tp_repr */
    (getattrofunc)0,			/* tp_getattro */
    (setattrofunc)0,			/* tp_setattro */
    (PyBufferProcs*)0,			/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,			/* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,			/* tp_traverse */
    (inquiry)0,				/* tp_clear */
    (richcmpfunc)cmyk_richcompare,	/* tp_richcompare */
    0,					/* tp_weaklistoffset */
    (getiterfunc)0,			/* tp_iter */
    (iternextfunc)0,			/* tp_iternext */
    cmyk_methods,			/* tp_methods */
    0,					/* tp_members */
    cmyk_getsets,			/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,			/* tp_descr_get */
    (descrsetfunc)0,			/* tp_descr_set */
    0,					/* tp_dictoffset */
    (initproc)cmyk_init,		/* tp_init */
    (allocfunc)0,			/* tp_alloc */
    (newfunc)0,				/* tp_new */
    (freefunc)0,			/* tp_free */
    (inquiry)0				/* tp_is_gc */
};

PyObject *
pypicman_cmyk_new(const PicmanCMYK *cmyk)
{
    return pyg_boxed_new(PICMAN_TYPE_CMYK, (gpointer)cmyk, TRUE, TRUE);
}

int
pypicman_rgb_from_pyobject(PyObject *object, PicmanRGB *color)
{
    g_return_val_if_fail(color != NULL, FALSE);

    if (pypicman_rgb_check(object)) {
        *color = *pyg_boxed_get(object, PicmanRGB);
        return 1;
    } else if (PyString_Check(object)) {
        if (picman_rgb_parse_css (color, PyString_AsString(object), -1)) {
            return 1;
        } else {
            PyErr_SetString(PyExc_TypeError, "unable to parse color string");
            return 0;
        }
    } else if (PySequence_Check(object)) {
        PyObject *r, *g, *b, *a = NULL;

        if (!PyArg_ParseTuple(object, "OOO|O", &r, &g, &b, &a))
            return 0;

#define SET_MEMBER(m)	G_STMT_START {				\
    if (PyInt_Check(m))						\
        color->m = (double) PyInt_AS_LONG(m) / 255.0;		\
    else if (PyFloat_Check(m))					\
        color->m = PyFloat_AS_DOUBLE(m);			\
    else {							\
	PyErr_SetString(PyExc_TypeError,			\
			#m " must be an int or a float");	\
	return 0;						\
    }								\
} G_STMT_END

        SET_MEMBER(r);
        SET_MEMBER(g);
        SET_MEMBER(b);

        if (a)
            SET_MEMBER(a);
        else
            color->a = 1.0;

        picman_rgb_clamp(color);

        return 1;
    }

    PyErr_SetString(PyExc_TypeError, "could not convert to PicmanRGB");
    return 0;
}
