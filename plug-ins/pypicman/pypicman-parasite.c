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

#include "pypicman.h"

static PyObject *
para_copy(PyPicmanParasite *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":copy"))
	return NULL;

    return pypicman_parasite_new(picman_parasite_copy(self->para));
}

static PyObject *
para_is_type(PyPicmanParasite *self, PyObject *args)
{
    char *name;

    if (!PyArg_ParseTuple(args, "s:is_type", &name))
	return NULL;

    return PyInt_FromLong(picman_parasite_is_type(self->para, name));
}

static PyObject *
para_has_flag(PyPicmanParasite *self, PyObject *args)
{
    int flag;

    if (!PyArg_ParseTuple(args, "i:has_flag", &flag))
	return NULL;

    return PyInt_FromLong(picman_parasite_has_flag(self->para, flag));
}


static PyMethodDef para_methods[] = {
    {"copy",	(PyCFunction)para_copy,	METH_VARARGS},
    {"is_type",	(PyCFunction)para_is_type,	METH_VARARGS},
    {"has_flag",(PyCFunction)para_has_flag,	METH_VARARGS},

    {NULL,		NULL}		/* sentinel */
};

static PyObject *
para_get_is_persistent(PyPicmanParasite *self, void *closure)
{
    return PyBool_FromLong(picman_parasite_is_persistent(self->para));
}

static PyObject *
para_get_is_undoable(PyPicmanParasite *self, void *closure)
{
    return PyBool_FromLong(picman_parasite_is_undoable(self->para));
}

static PyObject *
para_get_flags(PyPicmanParasite *self, void *closure)
{
    return PyInt_FromLong(picman_parasite_flags(self->para));
}

static PyObject *
para_get_name(PyPicmanParasite *self, void *closure)
{
    return PyString_FromString(picman_parasite_name(self->para));
}

static PyObject *
para_get_data(PyPicmanParasite *self, void *closure)
{
    return PyString_FromStringAndSize(picman_parasite_data(self->para),
				      picman_parasite_data_size(self->para));
}

static PyGetSetDef para_getsets[] = {
    { "is_persistent", (getter)para_get_is_persistent, (setter)0 },
    { "is_undoable", (getter)para_get_is_undoable, (setter)0 },
    { "flags", (getter)para_get_flags, (setter)0 },
    { "name", (getter)para_get_name, (setter)0 },
    { "data", (getter)para_get_data, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

static void
para_dealloc(PyPicmanParasite *self)
{
    picman_parasite_free(self->para);
    PyObject_DEL(self);
}

static PyObject *
para_repr(PyPicmanParasite *self)
{
    PyObject *s;

    s = PyString_FromFormat("<parasite %s>", picman_parasite_name(self->para));

    return s;
}

static PyObject *
para_str(PyPicmanParasite *self)
{
    return PyString_FromStringAndSize(picman_parasite_data(self->para),
				      picman_parasite_data_size(self->para));
}

static int
para_init(PyPicmanParasite *self, PyObject *args, PyObject *kwargs)
{
    char *name;
    int flags, size;
    guint8 *data;

    static char *kwlist[] = { "name", "flags", "data", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "sis#:picman.Parasite.__init__", kwlist,
				     &name, &flags,
				     &data, &size))
	return -1;

    self->para = picman_parasite_new(name, flags, size, data);

    if (!self->para) {
	PyErr_Format(pypicman_error, "could not create parasite '%s'", name);
	return -1;
    }

    return 0;
}


PyTypeObject PyPicmanParasite_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "picman.Parasite",                    /* tp_name */
    sizeof(PyPicmanParasite),             /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)para_dealloc,           /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)0,                         /* tp_compare */
    (reprfunc)para_repr,                /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    (hashfunc)0,                        /* tp_hash */
    (ternaryfunc)0,                     /* tp_call */
    (reprfunc)para_str,                 /* tp_str */
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
    para_methods,			/* tp_methods */
    0,					/* tp_members */
    para_getsets,			/* tp_getset */
    (PyTypeObject *)0,			/* tp_base */
    (PyObject *)0,			/* tp_dict */
    0,					/* tp_descr_get */
    0,					/* tp_descr_set */
    0,					/* tp_dictoffset */
    (initproc)para_init,                /* tp_init */
    (allocfunc)0,			/* tp_alloc */
    (newfunc)0,				/* tp_new */
};

PyObject *
pypicman_parasite_new(PicmanParasite *para)
{
    PyPicmanParasite *self;

    if (!para) {
	Py_INCREF(Py_None);
	return Py_None;
    }

    self = PyObject_NEW(PyPicmanParasite, &PyPicmanParasite_Type);

    if (self == NULL)
	return NULL;

    self->para = para;

    return (PyObject *)self;
}
