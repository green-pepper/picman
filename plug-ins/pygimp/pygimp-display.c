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

static PyMethodDef disp_methods[] = {
    {NULL,	NULL}		/* sentinel */
};

static PyObject *
disp_get_ID(PyPicmanDisplay *self, void *closure)
{
    return PyInt_FromLong(self->ID);
}

static PyGetSetDef disp_getsets[] = {
    { "ID", (getter)disp_get_ID, (setter)0 },
    { NULL, (getter)0, (setter)0 }
};

/* ---------- */


PyObject *
pypicman_display_new(gint32 ID)
{
    PyPicmanDisplay *self;

    if (!picman_display_is_valid(ID)) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    self = PyObject_NEW(PyPicmanDisplay, &PyPicmanDisplay_Type);

    if (self == NULL)
	return NULL;

    self->ID = ID;

    return (PyObject *)self;
}

static void
disp_dealloc(PyPicmanDisplay *self)
{
    PyObject_DEL(self);
}

static PyObject *
disp_repr(PyPicmanDisplay *self)
{
    PyObject *s;

    s = PyString_FromString("<display>");

    return s;
}

static int
disp_init(PyPicmanDisplay *self, PyObject *args, PyObject *kwargs)
{
    PyPicmanImage *img;

    if (!PyArg_ParseTuple(args, "O!:picman.Display.__init__",
			  &PyPicmanImage_Type, &img))
	return -1;

    self->ID = picman_display_new(img->ID);

    if (self->ID < 0) {
	PyErr_Format(pypicman_error, "could not create display for image (ID %d)",
		     img->ID);
	return -1;
    }

    return 0;
}

PyTypeObject PyPicmanDisplay_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "picman.Display",                     /* tp_name */
    sizeof(PyPicmanDisplay),              /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)disp_dealloc,           /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)0,                         /* tp_compare */
    (reprfunc)disp_repr,                /* tp_repr */
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
    disp_methods,			/* tp_methods */
    0,					/* tp_members */
    disp_getsets,			/* tp_getset */
    (PyTypeObject *)0,			/* tp_base */
    (PyObject *)0,			/* tp_dict */
    0,					/* tp_descr_get */
    0,					/* tp_descr_set */
    0,					/* tp_dictoffset */
    (initproc)disp_init,                /* tp_init */
    (allocfunc)0,			/* tp_alloc */
    (newfunc)0,				/* tp_new */
};
