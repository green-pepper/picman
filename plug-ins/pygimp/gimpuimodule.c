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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Python.h>

#include <pygobject.h>
#include <pygtk/pygtk.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "pypicmancolor-api.h"
#include "pypicman-api.h"
#include "pypicman-util.h"


void picmanui_register_classes(PyObject *d);
void picmanui_add_constants(PyObject *module, const gchar *strip_prefix);
extern PyMethodDef picmanui_functions[];


static char picmanui_doc[] =
"This module provides interfaces to allow you to write picman plugins"
;

void init_picmanui(void);

PyMODINIT_FUNC
init_picmanui(void)
{
    PyObject *m, *d;
    PyObject *av;
    char *prog_name = "pypicman";

    av = PySys_GetObject("argv");
    if (av != NULL) {
	if (PyList_Check(av) && PyList_Size(av) > 0 &&
	    PyString_Check(PyList_GetItem(av, 0)))
	    prog_name = PyString_AsString(PyList_GetItem(av, 0));
	else
	    PyErr_Warn(PyExc_Warning,
		       "ignoring sys.argv: it must be a list of strings");
    }

    picman_ui_init(prog_name, FALSE);

    pypicman_init_pygobject();

    init_pygtk();
    init_pypicmancolor();
    init_pypicman();

    m = Py_InitModule3("_picmanui", picmanui_functions, picmanui_doc);
    d = PyModule_GetDict(m);

    picmanui_register_classes(d);
    picmanui_add_constants(m, "PICMAN_");

    if (PyErr_Occurred())
	Py_FatalError("can't initialize module _picmanui");
}
