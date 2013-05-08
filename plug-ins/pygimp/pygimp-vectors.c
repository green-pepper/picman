/* -*- Mode: C; c-basic-offset: 4 -*-
 * Picman-Python - allows the writing of Picman plugins in Python.
 * Copyright (C) 2006  Manish Singh <yosh@picman.org>
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

#include "pypicman.h"


static PyObject *vectors_bezier_stroke_new(PyPicmanVectors *vectors, int stroke);


typedef struct {
    PyObject_HEAD
    gint32 vectors_ID;
    int stroke;
} PyPicmanVectorsStroke;

static PyObject *
vs_get_length(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    double precision;
    double length;

    static char *kwlist[] = { "precision", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "d:get_length", kwlist,
                                     &precision))
        return NULL;

    length = picman_vectors_stroke_get_length(self->vectors_ID, self->stroke,
                                            precision);

    return PyFloat_FromDouble(length);
}

static PyObject *
vs_get_point_at_dist(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    double dist, precision;
    double x, y, slope;
    gboolean valid;
    PyObject *ret;

    static char *kwlist[] = { "dist", "precision", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "dd:get_point_at_dist", kwlist,
                                     &dist, &precision))
        return NULL;

    picman_vectors_stroke_get_point_at_dist(self->vectors_ID, self->stroke,
                                          dist, precision,
                                          &x, &y, &slope, &valid);

    ret = PyTuple_New(4);
    if (ret == NULL)
        return NULL;

    PyTuple_SetItem(ret, 0, PyFloat_FromDouble(x));
    PyTuple_SetItem(ret, 1, PyFloat_FromDouble(y));
    PyTuple_SetItem(ret, 2, PyFloat_FromDouble(slope));
    PyTuple_SetItem(ret, 3, PyBool_FromLong(valid));

    return ret;
}

static PyObject *
vs_close(PyPicmanVectorsStroke *self)
{
    picman_vectors_stroke_close(self->vectors_ID, self->stroke);
    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
vs_translate(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    double off_x, off_y;

    static char *kwlist[] = { "off_x", "off_y", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "dd:translate", kwlist,
                                     &off_x, &off_y))
        return NULL;

    picman_vectors_stroke_translate(self->vectors_ID, self->stroke, off_x, off_y);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
vs_scale(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    double scale_x, scale_y;

    static char *kwlist[] = { "scale_x", "scale_y", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "dd:scale", kwlist,
                                     &scale_x, &scale_y))
        return NULL;

    picman_vectors_stroke_scale(self->vectors_ID, self->stroke, scale_x, scale_y);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
vs_rotate(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    double center_x, center_y, angle;

    static char *kwlist[] = { "center_x", "center_y", "angle", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ddd:rotate", kwlist,
                                     &center_x, &center_y, &angle))
        return NULL;

    picman_vectors_stroke_rotate(self->vectors_ID, self->stroke, center_x,
                               center_y, angle);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
vs_flip(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    int    flip_type;
    double axis;

    static char *kwlist[] = { "flip_type", "axis", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "id:flip", kwlist,
                                     &flip_type, &axis))
        return NULL;

    picman_vectors_stroke_flip(self->vectors_ID, self->stroke, flip_type, axis);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
vs_flip_free(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    double x1,y1,x2,y2;

    static char *kwlist[] = { "x1", "y1", "x2", "y2", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "dddd:flip_free", kwlist,
                                     &x1, &y1, &x2, &y2))
        return NULL;

    picman_vectors_stroke_flip_free(self->vectors_ID, self->stroke,
                                  x1, y1, x2, y2);
    Py_INCREF(Py_None);
    return Py_None;
}



static PyObject *
vs_interpolate(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    double precision;
    double *coords;
    int i, num_coords;
    gboolean closed;
    PyObject *ret, *ret_coords;

    static char *kwlist[] = { "precision", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "d:interpolate", kwlist,
                                     &precision))
        return NULL;

    coords = picman_vectors_stroke_interpolate(self->vectors_ID, self->stroke,
                                             precision, &num_coords, &closed);

    ret = PyTuple_New(2);
    if (ret == NULL)
        return NULL;

    ret_coords = PyList_New(num_coords);
    if (ret_coords == NULL) {
        Py_DECREF(ret);
        return NULL;
    }

    for (i = 0; i < num_coords; i++)
        PyList_SetItem(ret_coords, i, PyFloat_FromDouble(coords[i]));

    PyTuple_SetItem(ret, 0, ret_coords);
    PyTuple_SetItem(ret, 1, PyBool_FromLong(closed));

    return ret;
}

static PyMethodDef vs_methods[] = {
    { "get_length", (PyCFunction)vs_get_length, METH_VARARGS | METH_KEYWORDS },
    { "get_point_at_dist", (PyCFunction)vs_get_point_at_dist, METH_VARARGS | METH_KEYWORDS },
    { "close", (PyCFunction)vs_close, METH_NOARGS },
    { "translate", (PyCFunction)vs_translate, METH_VARARGS | METH_KEYWORDS },
    { "scale", (PyCFunction)vs_scale, METH_VARARGS | METH_KEYWORDS },
    { "rotate", (PyCFunction)vs_rotate, METH_VARARGS | METH_KEYWORDS },
    { "flip", (PyCFunction)vs_flip, METH_VARARGS | METH_KEYWORDS },
    { "flip_free", (PyCFunction)vs_flip_free, METH_VARARGS | METH_KEYWORDS },
    { "interpolate", (PyCFunction)vs_interpolate, METH_VARARGS | METH_KEYWORDS },
    { NULL, NULL, 0 }
};

static PyObject *
vs_get_ID(PyPicmanVectorsStroke *self, void *closure)
{
    return PyInt_FromLong(self->stroke);
}

static PyObject *
vs_get_vectors_ID(PyPicmanVectorsStroke *self, void *closure)
{
    return PyInt_FromLong(self->vectors_ID);
}

static PyObject *
vs_get_points(PyPicmanVectorsStroke *self, void *closure)
{
    double *controlpoints;
    int i, num_points;
    gboolean closed;
    PyObject *ret, *ret_points;

    picman_vectors_stroke_get_points(self->vectors_ID, self->stroke,
                                   &num_points, &controlpoints, &closed);

    ret = PyTuple_New(2);
    if (ret == NULL)
        return NULL;

    ret_points = PyList_New(num_points);
    if (ret_points == NULL) {
        Py_DECREF(ret);
        return NULL;
    }

    for (i = 0; i < num_points; i++)
        PyList_SetItem(ret_points, i, PyFloat_FromDouble(controlpoints[i]));

    PyTuple_SetItem(ret, 0, ret_points);
    PyTuple_SetItem(ret, 1, PyBool_FromLong(closed));

    return ret;
}

static PyGetSetDef vs_getsets[] = {
    { "ID", (getter)vs_get_ID, (setter)0 },
    { "vectors_ID", (getter)vs_get_vectors_ID, (setter)0 },
    { "points", (getter)vs_get_points, (setter)0 },
    { NULL, (getter)0, (setter)0 }
};

static void
vs_dealloc(PyPicmanVectorsStroke *self)
{
    PyObject_DEL(self);
}

static PyObject *
vs_repr(PyPicmanVectorsStroke *self)
{
    PyObject *s;
    char *name;

    name = picman_item_get_name(self->vectors_ID);
    s = PyString_FromFormat("<picman.VectorsStroke %d of picman.Vectors '%s'>",
                            self->stroke, name ? name : "(null)");
    g_free(name);

    return s;
}

static int
vs_cmp(PyPicmanVectorsStroke *self, PyPicmanVectorsStroke *other)
{
    if (self->vectors_ID == other->vectors_ID) {
        if (self->stroke == other->stroke)
            return 0;
        if (self->stroke > other->stroke)
            return -1;
        return 1;
    }
    if (self->vectors_ID > other->vectors_ID)
        return -1;
    return 1;
}

PyTypeObject PyPicmanVectorsStroke_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "picman.VectorsStroke",               /* tp_name */
    sizeof(PyPicmanVectorsStroke),        /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)vs_dealloc,             /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)vs_cmp,                    /* tp_compare */
    (reprfunc)vs_repr,                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    (hashfunc)0,                        /* tp_hash */
    (ternaryfunc)0,                     /* tp_call */
    (reprfunc)0,                        /* tp_str */
    (getattrofunc)0,                    /* tp_getattro */
    (setattrofunc)0,                    /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    NULL, /* Documentation string */
    (traverseproc)0,                    /* tp_traverse */
    (inquiry)0,                         /* tp_clear */
    (richcmpfunc)0,                     /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    (getiterfunc)0,                     /* tp_iter */
    (iternextfunc)0,                    /* tp_iternext */
    vs_methods,                         /* tp_methods */
    0,                                  /* tp_members */
    vs_getsets,                         /* tp_getset */
    (PyTypeObject *)0,                  /* tp_base */
    (PyObject *)0,                      /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)0,                        /* tp_init */
    (allocfunc)0,                       /* tp_alloc */
    (newfunc)0,                         /* tp_new */
};


static PyObject *
vbs_new_moveto(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    PyPicmanVectors *vectors;
    double x0, y0;
    int stroke;

    static char *kwlist[] = { "vectors", "x0", "y0", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!dd:new_moveto", kwlist,
                                     &PyPicmanVectors_Type, &vectors,
                                     &x0, &y0))
        return NULL;

    stroke = picman_vectors_bezier_stroke_new_moveto(vectors->ID, x0, y0);

    return vectors_bezier_stroke_new(vectors, stroke);
}

static PyObject *
vbs_new_ellipse(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    PyPicmanVectors *vectors;
    double x0, y0, radius_x, radius_y, angle;
    int stroke;

    static char *kwlist[] = { "vectors", "x0", "y0", "radius_x", "radius_y",
                              "angle", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!ddddd:new_ellipse", kwlist,
                                     &PyPicmanVectors_Type, &vectors,
                                     &x0, &y0, &radius_x, &radius_y, &angle))
        return NULL;

    stroke = picman_vectors_bezier_stroke_new_ellipse(vectors->ID, x0, y0,
                                                    radius_x, radius_y, angle);

    return vectors_bezier_stroke_new(vectors, stroke);
}

static PyObject *
vbs_lineto(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    double x0, y0;

    static char *kwlist[] = { "x0", "y0", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "dd:lineto", kwlist,
                                     &x0, &y0))
        return NULL;

    picman_vectors_bezier_stroke_lineto(self->vectors_ID, self->stroke, x0, y0);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
vbs_conicto(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    double x0, y0, x1, y1;

    static char *kwlist[] = { "x0", "y0", "x1", "y1", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "dddd:conicto", kwlist,
                                     &x0, &y0, &x1, &y1))
        return NULL;

    picman_vectors_bezier_stroke_conicto(self->vectors_ID, self->stroke,
                                       x0, y0, x1, y1);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
vbs_cubicto(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    double x0, y0, x1, y1, x2, y2;

    static char *kwlist[] = { "x0", "y0", "x1", "y1", "x2", "y2", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "dddddd:cubicto", kwlist,
                                     &x0, &y0, &x1, &y1, &x2, &y2))
        return NULL;

    picman_vectors_bezier_stroke_cubicto(self->vectors_ID, self->stroke,
                                       x0, y0, x1, y1, x2, y2);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef vbs_methods[] = {
    { "new_moveto", (PyCFunction)vbs_new_moveto, METH_VARARGS | METH_KEYWORDS | METH_CLASS },
    { "new_ellipse", (PyCFunction)vbs_new_ellipse, METH_VARARGS | METH_KEYWORDS | METH_CLASS },
    { "lineto", (PyCFunction)vbs_lineto, METH_VARARGS | METH_KEYWORDS },
    { "conicto", (PyCFunction)vbs_conicto, METH_VARARGS | METH_KEYWORDS },
    { "cubicto", (PyCFunction)vbs_cubicto, METH_VARARGS | METH_KEYWORDS },
    { NULL, NULL, 0 }
};

static PyObject *
vbs_repr(PyPicmanVectorsStroke *self)
{
    PyObject *s;
    char *name;

    name = picman_item_get_name(self->vectors_ID);
    s = PyString_FromFormat("<picman.VectorsBezierStroke %d of picman.Vectors '%s'>",
                            self->stroke, name ? name : "(null)");
    g_free(name);

    return s;
}

static int
vbs_init(PyPicmanVectorsStroke *self, PyObject *args, PyObject *kwargs)
{
    PyPicmanVectors *vectors;
    double *controlpoints;
    gboolean closed = FALSE;
    PyObject *py_controlpoints, *item;
    int i, num_points;

    static char *kwlist[] = { "vectors", "controlpoints", "closed", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!O|i:picman.VectorsBezierStroke.__init__",
                                     kwlist,
                                     &PyPicmanVectors_Type, &vectors,
                                     &py_controlpoints, &closed))
        return -1;

    if (!PySequence_Check(py_controlpoints)) {
        PyErr_SetString(PyExc_TypeError,
                        "controlpoints must be a sequence");
        return -1;
    }

    num_points = PySequence_Length(py_controlpoints);
    controlpoints = g_new(gdouble, num_points);

    for (i = 0; i < num_points; i++) {
        item = PySequence_GetItem(py_controlpoints, i);

        if (!PyFloat_Check(item)) {
            PyErr_SetString(PyExc_TypeError,
                            "controlpoints must be a sequence of floats");
            g_free(controlpoints);
            return -1;
        }

        controlpoints[i] = PyFloat_AsDouble(item);
    }

    self->vectors_ID = vectors->ID;
    self->stroke =
        picman_vectors_stroke_new_from_points(self->vectors_ID,
                                            PICMAN_VECTORS_STROKE_TYPE_BEZIER,
                                            num_points, controlpoints, closed);

    g_free(controlpoints);

    return 0;
}

PyTypeObject PyPicmanVectorsBezierStroke_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "picman.VectorsBezierStroke",         /* tp_name */
    sizeof(PyPicmanVectorsStroke),        /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)vs_dealloc,             /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)vs_cmp,                    /* tp_compare */
    (reprfunc)vbs_repr,                 /* tp_repr */
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
    vbs_methods,                        /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    &PyPicmanVectorsStroke_Type,          /* tp_base */
    (PyObject *)0,                      /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)vbs_init,                 /* tp_init */
    (allocfunc)0,                       /* tp_alloc */
    (newfunc)0,                         /* tp_new */
};

static PyObject *
vectors_bezier_stroke_new(PyPicmanVectors *vectors, int stroke)
{
    PyPicmanVectorsStroke *self;

    self = PyObject_NEW(PyPicmanVectorsStroke, &PyPicmanVectorsBezierStroke_Type);

    if (self == NULL)
        return NULL;

    self->vectors_ID = vectors->ID;
    self->stroke = stroke;

    return (PyObject *)self;
}


static PyObject *
vectors_remove_stroke(PyPicmanVectors *self, PyObject *args, PyObject *kwargs)
{
    int stroke_id ;
    /* PyPicmanVectorsStroke *stroke; */
    PyObject *stroke = NULL;

    static char *kwlist[] = { "stroke", NULL };

    PyArg_ParseTupleAndKeywords(args, kwargs, "O:remove_stroke", kwlist, &stroke);

    if (PyInt_Check(stroke))
        stroke_id = PyInt_AsLong(stroke);
    else if (PyObject_IsInstance(stroke, (PyObject *) &PyPicmanVectorsStroke_Type))
        stroke_id = ((PyPicmanVectorsStroke *) stroke)->stroke;
    else  {
        PyErr_SetString(PyExc_TypeError, "stroke must be a picman.VectorsBezierStroke object or an Integer");
        return NULL;
    }

    picman_vectors_remove_stroke(self->ID, stroke_id);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
vectors_to_selection(PyPicmanVectors *self, PyObject *args, PyObject *kwargs)
{
    PicmanChannelOps operation = PICMAN_CHANNEL_OP_REPLACE;
    gboolean antialias = TRUE, feather = FALSE;
    double feather_radius_x = 0.0, feather_radius_y = 0.0;

    static char *kwlist[] = { "operation", "antialias", "feather",
                              "feather_radius_x", "feather_radius_y", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|iiidd:to_selection", kwlist,
                                     &operation, &antialias, &feather,
                                     &feather_radius_x, &feather_radius_y))
        return NULL;

    picman_context_push();
    picman_context_set_antialias(antialias);
    picman_context_set_feather(feather);
    picman_context_set_feather_radius(feather_radius_x, feather_radius_y);
    picman_image_select_item(picman_item_get_image(self->ID), operation, self->ID);
    picman_context_pop();

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
vectors_parasite_find(PyPicmanVectors *self, PyObject *args)
{
    char *name;

    if (!PyArg_ParseTuple(args, "s:parasite_find", &name))
        return NULL;

    return pypicman_parasite_new(picman_item_get_parasite(self->ID, name));
}

static PyObject *
vectors_parasite_attach(PyPicmanVectors *self, PyObject *args)
{
    PyPicmanParasite *parasite;

    if (!PyArg_ParseTuple(args, "O!:parasite_attach", &PyPicmanParasite_Type,
                          &parasite))
        return NULL;

    if (!picman_item_attach_parasite(self->ID, parasite->para)) {
        PyErr_Format(pypicman_error,
                     "could not attach parasite '%s' to vectors (ID %d)",
                     parasite->para->name, self->ID);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
vectors_parasite_detach(PyPicmanVectors *self, PyObject *args)
{
    char *name;

    if (!PyArg_ParseTuple(args, "s:parasite_detach", &name))
        return NULL;

    if (!picman_item_detach_parasite(self->ID, name)) {
        PyErr_Format(pypicman_error,
                     "could not detach parasite '%s' from vectors (ID %d)",
                     name, self->ID);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
vectors_parasite_list(PyPicmanVectors *self)
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

    PyErr_Format(pypicman_error, "could not list parasites on vectors (ID %d)",
                 self->ID);
    return NULL;
}

static PyMethodDef vectors_methods[] = {
    { "remove_stroke",
      (PyCFunction)vectors_remove_stroke,
      METH_VARARGS | METH_KEYWORDS },
    { "to_selection",
      (PyCFunction)vectors_to_selection,
      METH_VARARGS | METH_KEYWORDS },
    { "parasite_find",
      (PyCFunction)vectors_parasite_find,
      METH_VARARGS },
    { "parasite_attach",
      (PyCFunction)vectors_parasite_attach,
      METH_VARARGS },
    { "parasite_detach",
      (PyCFunction)vectors_parasite_detach,
      METH_VARARGS },
    { "parasite_list",
      (PyCFunction)vectors_parasite_list,
      METH_NOARGS },
    { NULL, NULL, 0 }
};

static PyObject *
vectors_get_image(PyPicmanVectors *self, void *closure)
{
    return pypicman_image_new(picman_item_get_image(self->ID));
}

static PyObject *
vectors_get_ID(PyPicmanVectors *self, void *closure)
{
    return PyInt_FromLong(self->ID);
}

static PyObject *
vectors_get_name(PyPicmanVectors *self, void *closure)
{
    return PyString_FromString(picman_item_get_name(self->ID));
}

static int
vectors_set_name(PyPicmanVectors *self, PyObject *value, void *closure)
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
vectors_get_visible(PyPicmanVectors *self, void *closure)
{
    return PyBool_FromLong(picman_item_get_visible(self->ID));
}

static int
vectors_set_visible(PyPicmanVectors *self, PyObject *value, void *closure)
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

static PyObject *
vectors_get_linked(PyPicmanVectors *self, void *closure)
{
    return PyBool_FromLong(picman_item_get_linked(self->ID));
}

static int
vectors_set_linked(PyPicmanVectors *self, PyObject *value, void *closure)
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
vectors_get_tattoo(PyPicmanVectors *self, void *closure)
{
    return PyInt_FromLong(picman_item_get_tattoo(self->ID));
}

static int
vectors_set_tattoo(PyPicmanVectors *self, PyObject *value, void *closure)
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
vectors_get_strokes(PyPicmanVectors *self, void *closure)
{
    int *strokes;
    int i, num_strokes;
    PyObject *ret;

    strokes = picman_vectors_get_strokes(self->ID, &num_strokes);

    ret = PyList_New(num_strokes);
    if (ret == NULL)
        return NULL;

    for (i = 0; i < num_strokes; i++)
        PyList_SetItem(ret, i, vectors_bezier_stroke_new(self, strokes[i]));

    g_free(strokes);

    return ret;
}

static PyGetSetDef vectors_getsets[] = {
    { "ID", (getter)vectors_get_ID, (setter)0 },
    { "image", (getter)vectors_get_image, (setter)0 },
    { "name", (getter)vectors_get_name, (setter)vectors_set_name },
    { "visible", (getter)vectors_get_visible, (setter)vectors_set_visible },
    { "linked", (getter)vectors_get_linked, (setter)vectors_set_linked },
    { "tattoo", (getter)vectors_get_tattoo, (setter)vectors_set_tattoo },
    { "strokes", (getter)vectors_get_strokes, (setter)0 },
    { NULL, (getter)0, (setter)0 }
};

static void
vectors_dealloc(PyPicmanVectors *self)
{
    PyObject_DEL(self);
}

static PyObject *
vectors_repr(PyPicmanVectors *self)
{
    PyObject *s;
    char *name;

    name = picman_item_get_name(self->ID);
    s = PyString_FromFormat("<picman.Vectors '%s'>", name ? name : "(null)");
    g_free(name);

    return s;
}

static int
vectors_cmp(PyPicmanVectors *self, PyPicmanVectors *other)
{
    if (self->ID == other->ID)
        return 0;
    if (self->ID > other->ID)
        return -1;
    return 1;
}

static int
vectors_init(PyPicmanVectors *self, PyObject *args, PyObject *kwargs)
{
    PyPicmanImage *img;
    char *name;

    static char *kwlist[] = { "image", "name", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!s:picman.Vectors.__init__",
                                     kwlist,
                                     &PyPicmanImage_Type, &img, &name))
        return -1;

    self->ID = picman_vectors_new(img->ID, name);

    if (self->ID < 0) {
        PyErr_Format(pypicman_error,
                     "could not create vectors '%s' on image (ID %d)",
                     name, img->ID);
        return -1;
    }

    return 0;
}

PyTypeObject PyPicmanVectors_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "picman.Vectors",                     /* tp_name */
    sizeof(PyPicmanVectors),              /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)vectors_dealloc,        /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)vectors_cmp,               /* tp_compare */
    (reprfunc)vectors_repr,             /* tp_repr */
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
    vectors_methods,                    /* tp_methods */
    0,                                  /* tp_members */
    vectors_getsets,                    /* tp_getset */
    &PyPicmanItem_Type,                   /* tp_base */
    (PyObject *)0,                      /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)vectors_init,             /* tp_init */
    (allocfunc)0,                       /* tp_alloc */
    (newfunc)0,                         /* tp_new */
};

PyObject *
pypicman_vectors_new(gint32 ID)
{
    PyPicmanVectors *self;

    if (!picman_item_is_valid(ID)) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    self = PyObject_NEW(PyPicmanVectors, &PyPicmanVectors_Type);

    if (self == NULL)
        return NULL;

    self->ID = ID;

    return (PyObject *)self;
}
