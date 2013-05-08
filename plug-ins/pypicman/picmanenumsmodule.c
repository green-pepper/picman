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

#include <glib-object.h>

#include <pygobject.h>

#include "pypicman-api.h"
#include "pypicman-util.h"


static void
add_misc_enums(PyObject *m)
{
    PyModule_AddIntConstant(m, "PARASITE_PERSISTENT",
			    PICMAN_PARASITE_PERSISTENT);
    PyModule_AddIntConstant(m, "PARASITE_UNDOABLE",
			    PICMAN_PARASITE_UNDOABLE);
    PyModule_AddIntConstant(m, "PARASITE_ATTACH_PARENT",
			    PICMAN_PARASITE_ATTACH_PARENT);
    PyModule_AddIntConstant(m, "PARASITE_PARENT_PERSISTENT",
			    PICMAN_PARASITE_PARENT_PERSISTENT);
    PyModule_AddIntConstant(m, "PARASITE_PARENT_UNDOABLE",
			    PICMAN_PARASITE_PARENT_UNDOABLE);
    PyModule_AddIntConstant(m, "PARASITE_ATTACH_GRANDPARENT",
			    PICMAN_PARASITE_ATTACH_GRANDPARENT);
    PyModule_AddIntConstant(m, "PARASITE_GRANDPARENT_PERSISTENT",
			    PICMAN_PARASITE_GRANDPARENT_PERSISTENT);
    PyModule_AddIntConstant(m, "PARASITE_GRANDPARENT_UNDOABLE",
			    PICMAN_PARASITE_GRANDPARENT_UNDOABLE);

    PyModule_AddIntConstant(m, "UNIT_PIXEL",
			    PICMAN_UNIT_PIXEL);
    PyModule_AddIntConstant(m, "UNIT_INCH",
			    PICMAN_UNIT_INCH);
    PyModule_AddIntConstant(m, "UNIT_MM",
			    PICMAN_UNIT_MM);
    PyModule_AddIntConstant(m, "UNIT_POINT",
			    PICMAN_UNIT_POINT);
    PyModule_AddIntConstant(m, "UNIT_PICA",
			    PICMAN_UNIT_PICA);

    PyModule_AddIntConstant(m, "MIN_IMAGE_SIZE",
			    PICMAN_MIN_IMAGE_SIZE);
    PyModule_AddIntConstant(m, "MAX_IMAGE_SIZE",
			    PICMAN_MAX_IMAGE_SIZE);

    PyModule_AddObject(m, "MIN_RESOLUTION",
		       PyFloat_FromDouble(PICMAN_MIN_RESOLUTION));
    PyModule_AddObject(m, "MAX_RESOLUTION",
		       PyFloat_FromDouble(PICMAN_MAX_RESOLUTION));

    PyModule_AddObject(m, "MAX_MEMSIZE",
		       PyLong_FromUnsignedLongLong(PICMAN_MAX_MEMSIZE));

    PyModule_AddIntConstant(m, "PIXEL_FETCHER_EDGE_NONE",
                            PICMAN_PIXEL_FETCHER_EDGE_NONE);
    PyModule_AddIntConstant(m, "PIXEL_FETCHER_EDGE_WRAP",
                            PICMAN_PIXEL_FETCHER_EDGE_WRAP);
    PyModule_AddIntConstant(m, "PIXEL_FETCHER_EDGE_SMEAR",
                            PICMAN_PIXEL_FETCHER_EDGE_SMEAR);
    PyModule_AddIntConstant(m, "PIXEL_FETCHER_EDGE_BLACK",
                            PICMAN_PIXEL_FETCHER_EDGE_BLACK);
    PyModule_AddIntConstant(m, "PIXEL_FETCHER_EDGE_BACKGROUND",
                            PICMAN_PIXEL_FETCHER_EDGE_BACKGROUND);
}

static void
add_registered_enums(PyObject *m)
{
    int num_names, i;
    const char **names;

    names = picman_enums_get_type_names(&num_names);

    pyg_enum_add_constants(m, PICMAN_TYPE_CHECK_SIZE, "PICMAN_");
    pyg_enum_add_constants(m, PICMAN_TYPE_CHECK_TYPE, "PICMAN_");

    for (i = 0; i < num_names; i++)
	pyg_enum_add_constants(m, g_type_from_name(names[i]), "PICMAN_");
}


/* Initialization function for the module (*must* be called initpicmanenums) */

static char picmanenums_doc[] =
"This module provides interfaces to allow you to write picman plugins"
;

void init_picmanenums(void);

PyMODINIT_FUNC
init_picmanenums(void)
{
    PyObject *m;

    pypicman_init_pygobject();

    init_pypicman();

    picman_enums_init();

    /* Create the module and add the functions */
    m = Py_InitModule3("_picmanenums", NULL, picmanenums_doc);

    add_misc_enums(m);
    add_registered_enums(m);

    /* Check for errors */
    if (PyErr_Occurred())
	Py_FatalError("can't initialize module _picmanenums");
}
