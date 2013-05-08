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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Python.h>

#include <pygobject.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libpicmanthumb/picmanthumb.h>

#include "pypicman-util.h"


void picmanthumb_register_classes(PyObject *d);
void picmanthumb_add_constants(PyObject *module, const gchar *strip_prefix);
extern PyMethodDef picmanthumb_functions[];


static char picmanthumb_doc[] =
"This module provides interfaces to allow you to write picman plugins"
;

void initpicmanthumb(void);

PyMODINIT_FUNC
initpicmanthumb(void)
{
    PyObject *m, *d;

    pypicman_init_pygobject();

    m = Py_InitModule3("picmanthumb", picmanthumb_functions, picmanthumb_doc);
    d = PyModule_GetDict(m);

    picmanthumb_register_classes(d);
    picmanthumb_add_constants(m, "PICMAN_THUMB_");

    if (PyErr_Occurred())
	Py_FatalError("can't initialize module picmanthumb");
}
