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

#undef PICMAN_DISABLE_DEPRECATED
#include "pypicman.h"

#include "pypicmancolor-api.h"

#include <sysmodule.h>

#include <glib-object.h>

#include <pygobject.h>

#include "pypicman-util.h"

#include "pypicman-intl.h"


PyObject *pypicman_error;

#ifndef PG_DEBUG
# define PG_DEBUG 0
#endif


/* End of code for pdbFunc objects */
/* -------------------------------------------------------- */

PicmanPlugInInfo PLUG_IN_INFO = {
    NULL, /* init_proc */
    NULL, /* quit_proc */
    NULL, /* query_proc */
    NULL  /* run_proc */
};

static PyObject *callbacks[] = {
    NULL, NULL, NULL, NULL
};

typedef struct _ProgressData ProgressData;

struct _ProgressData
{
  PyObject *start, *end, *text, *value;
  PyObject *user_data;
};


static void
pypicman_init_proc(void)
{
    PyObject *r;

    r = PyObject_CallFunction(callbacks[0], "()");

    if (!r) {
        PyErr_Print();
        PyErr_Clear();
        return;
    }

    Py_DECREF(r);
}

static void
pypicman_quit_proc(void)
{
    PyObject *r;

    r = PyObject_CallFunction(callbacks[1], "()");

    if (!r) {
        PyErr_Print();
        PyErr_Clear();
        return;
    }

    Py_DECREF(r);
}

static void
pypicman_query_proc(void)
{
    PyObject *r;

    r = PyObject_CallFunction(callbacks[2], "()");

    if (!r) {
        PyErr_Print();
        PyErr_Clear();
        return;
    }

    Py_DECREF(r);
}

static void
pypicman_run_proc(const char *name, int nparams, const PicmanParam *params,
                int *nreturn_vals, PicmanParam **return_vals)
{
    PyObject *args, *ret;
    PicmanParamDef *pd, *rv;
    PicmanPDBProcType t;
    char *b, *h, *a, *c, *d;
    int np, nrv;

    picman_procedural_db_proc_info(name, &b, &h, &a, &c, &d, &t, &np, &nrv,
                                 &pd, &rv);
    g_free(b); g_free(h); g_free(a); g_free(c); g_free(d); g_free(pd);

#if PG_DEBUG > 0
    g_printerr("Params for %s:", name);
    print_GParam(nparams, params);
#endif

    args = pypicman_param_to_tuple(nparams, params);

    if (args == NULL) {
        PyErr_Clear();

        *nreturn_vals = 1;
        *return_vals = g_new(PicmanParam, 1);
        (*return_vals)[0].type = PICMAN_PDB_STATUS;
        (*return_vals)[0].data.d_status = PICMAN_PDB_CALLING_ERROR;

        return;
    }

    ret = PyObject_CallFunction(callbacks[3], "(sO)", name, args);
    Py_DECREF(args);

    if (ret == NULL) {
        PyErr_Print();
        PyErr_Clear();

        *nreturn_vals = 1;
        *return_vals = g_new(PicmanParam, 1);
        (*return_vals)[0].type = PICMAN_PDB_STATUS;
        (*return_vals)[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

        return;
    }

    *return_vals = pypicman_param_from_tuple(ret, rv, nrv);
    g_free(rv);

    if (*return_vals == NULL) {
        PyErr_Clear();

        *nreturn_vals = 1;
        *return_vals = g_new(PicmanParam, 1);
        (*return_vals)[0].type = PICMAN_PDB_STATUS;
        (*return_vals)[0].data.d_status = PICMAN_PDB_EXECUTION_ERROR;

        return;
    }

    Py_DECREF(ret);

    *nreturn_vals = nrv + 1;
    (*return_vals)[0].type = PICMAN_PDB_STATUS;
    (*return_vals)[0].data.d_status = PICMAN_PDB_SUCCESS;
}

static PyObject *
pypicman_main(PyObject *self, PyObject *args)
{
    PyObject *av;
    int argc, i;
    char **argv;
    PyObject *ip;  // init proc
    PyObject *qp;  // quit proc
    PyObject *query;  // query proc
    PyObject *rp;  // run proc

    if (!PyArg_ParseTuple(args, "OOOO:main", &ip, &qp, &query, &rp))
        return NULL;

#define Arg_Check(v) (PyCallable_Check(v) || (v) == Py_None)

    if (!Arg_Check(ip) || !Arg_Check(qp) || !Arg_Check(query) ||
        !Arg_Check(rp)) {
        PyErr_SetString(pypicman_error, "arguments must be callable");
        return NULL;
    }

#undef Arg_Check

    if (query == Py_None) {
        PyErr_SetString(pypicman_error, "a query procedure must be provided");
        return NULL;
    }

    if (ip != Py_None) {
        callbacks[0] = ip;
        PLUG_IN_INFO.init_proc = pypicman_init_proc;
    }

    if (qp != Py_None) {
        callbacks[1] = qp;
        PLUG_IN_INFO.quit_proc = pypicman_quit_proc;
    }

    if (query != Py_None) {
        callbacks[2] = query;
        PLUG_IN_INFO.query_proc = pypicman_query_proc;
    }

    if (rp != Py_None) {
        callbacks[3] = rp;
        PLUG_IN_INFO.run_proc = pypicman_run_proc;
    }

    av = PySys_GetObject("argv");

    argc = PyList_Size(av);
    argv = g_new(char *, argc);

    for (i = 0; i < argc; i++)
        argv[i] = g_strdup(PyString_AsString(PyList_GetItem(av, i)));

    picman_main(&PLUG_IN_INFO, argc, argv);

    if (argv != NULL) {
        for (i = 0; i < argc; i++)
            if (argv[i] != NULL)
                g_free(argv[i]);

        g_free(argv);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_quit(PyObject *self)
{
    picman_quit();

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_message(PyObject *self, PyObject *args)
{
    char *msg;

    if (!PyArg_ParseTuple(args, "s:message", &msg))
        return NULL;

    picman_message(msg);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_exit(PyObject *self, PyObject *args, PyObject *kwargs)
{
    gboolean force = FALSE;
    int nreturn_vals;
    PicmanParam *return_vals;

    static char *kwlist[] = { "force", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i:exit", kwlist, &force))
        return NULL;

    return_vals = picman_run_procedure("picman-quit",
                                     &nreturn_vals,
                                     PICMAN_PDB_INT32, force,
                                     PICMAN_PDB_END);

    if (return_vals[0].data.d_status != PICMAN_PDB_SUCCESS) {
        PyErr_SetString(pypicman_error, "error while exiting");
        return NULL;
    }

    picman_destroy_params(return_vals, nreturn_vals);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_set_data(PyObject *self, PyObject *args)
{
    char *id, *data;
    int bytes, nreturn_vals;
    PicmanParam *return_vals;

    if (!PyArg_ParseTuple(args, "ss#:set_data", &id, &data, &bytes))
        return NULL;

    return_vals = picman_run_procedure("picman-procedural-db-set-data",
                                     &nreturn_vals,
                                     PICMAN_PDB_STRING, id,
                                     PICMAN_PDB_INT32, bytes,
                                     PICMAN_PDB_INT8ARRAY, data,
                                     PICMAN_PDB_END);

    if (return_vals[0].data.d_status != PICMAN_PDB_SUCCESS) {
        PyErr_SetString(pypicman_error, "error occurred while storing");
        return NULL;
    }

    picman_destroy_params(return_vals, nreturn_vals);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_get_data(PyObject *self, PyObject *args)
{
    char *id;
    int nreturn_vals;
    PicmanParam *return_vals;
    PyObject *s;

    if (!PyArg_ParseTuple(args, "s:get_data", &id))
        return NULL;

    return_vals = picman_run_procedure("picman-procedural-db-get-data",
                                     &nreturn_vals,
                                     PICMAN_PDB_STRING, id,
                                     PICMAN_PDB_END);

    if (return_vals[0].data.d_status != PICMAN_PDB_SUCCESS) {
        PyErr_SetString(pypicman_error, "no data for id");
        return NULL;
    }

    s = PyString_FromStringAndSize((char *)return_vals[2].data.d_int8array,
                                   return_vals[1].data.d_int32);
    picman_destroy_params(return_vals, nreturn_vals);

    return s;
}

static PyObject *
pypicman_progress_init(PyObject *self, PyObject *args)
{
    char *msg = NULL;

    if (!PyArg_ParseTuple(args, "|s:progress_init", &msg))
        return NULL;

    picman_progress_init(msg);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_progress_update(PyObject *self, PyObject *args)
{
    double p;

    if (!PyArg_ParseTuple(args, "d:progress_update", &p))
        return NULL;

    picman_progress_update(p);

    Py_INCREF(Py_None);
    return Py_None;
}

static void
pypicman_progress_start(const gchar *message, gboolean cancelable, gpointer data)
{
    ProgressData *pdata = data;
    PyObject *r;

    if (pdata->user_data) {
        r = PyObject_CallFunction(pdata->start, "siO", message, cancelable,
                                  pdata->user_data);
        Py_DECREF(pdata->user_data);
    } else
        r = PyObject_CallFunction(pdata->start, "si", message, cancelable);

    if (!r) {
        PyErr_Print();
        PyErr_Clear();
        return;
    }

    Py_DECREF(r);
}

static void
pypicman_progress_end(gpointer data)
{
    ProgressData *pdata = data;
    PyObject *r;

    if (pdata->user_data) {
        r = PyObject_CallFunction(pdata->end, "O", pdata->user_data);
        Py_DECREF(pdata->user_data);
    } else
        r = PyObject_CallFunction(pdata->end, NULL);

    if (!r) {
        PyErr_Print();
        PyErr_Clear();
        return;
    }

    Py_DECREF(r);
}

static void
pypicman_progress_text(const gchar *message, gpointer data)
{
    ProgressData *pdata = data;
    PyObject *r;

    if (pdata->user_data) {
        r = PyObject_CallFunction(pdata->text, "sO", message, pdata->user_data);
        Py_DECREF(pdata->user_data);
    } else
        r = PyObject_CallFunction(pdata->text, "s", message);

    if (!r) {
        PyErr_Print();
        PyErr_Clear();
        return;
    }

    Py_DECREF(r);
}

static void
pypicman_progress_value(gdouble percentage, gpointer data)
{
    ProgressData *pdata = data;
    PyObject *r;

    if (pdata->user_data) {
        r = PyObject_CallFunction(pdata->value, "dO", percentage,
                                  pdata->user_data);
        Py_DECREF(pdata->user_data);
    } else
        r = PyObject_CallFunction(pdata->value, "d", percentage);

    if (!r) {
        PyErr_Print();
        PyErr_Clear();
        return;
    }

    Py_DECREF(r);
}

static PyObject *
pypicman_progress_install(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PicmanProgressVtable vtable = { 0, };
    const gchar *ret;
    ProgressData *pdata;
    static char *kwlist[] = { "start", "end", "text", "value", "data", NULL };

    pdata = g_new0(ProgressData, 1);

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOOO|O:progress_install",
                                     kwlist,
                                     &pdata->start, &pdata->end,
                                     &pdata->text, &pdata->value,
                                     &pdata->user_data))
        goto cleanup;

#define PROCESS_FUNC(n) G_STMT_START {                                  \
    if (!PyCallable_Check(pdata->n)) {                                  \
        PyErr_SetString(pypicman_error, #n "argument must be callable");  \
        goto cleanup;                                                   \
    }                                                                   \
    Py_INCREF(pdata->n);                                                \
} G_STMT_END

    PROCESS_FUNC(start);
    PROCESS_FUNC(end);
    PROCESS_FUNC(text);
    PROCESS_FUNC(value);

    Py_XINCREF(pdata->user_data);

#undef PROCESS_FUNC

    vtable.start     = pypicman_progress_start;
    vtable.end       = pypicman_progress_end;
    vtable.set_text  = pypicman_progress_text;
    vtable.set_value = pypicman_progress_value;

    ret = picman_progress_install_vtable(&vtable, pdata);

    if (!ret) {
        PyErr_SetString(pypicman_error,
                        "error occurred while installing progress functions");

        Py_DECREF(pdata->start);
        Py_DECREF(pdata->end);
        Py_DECREF(pdata->text);
        Py_DECREF(pdata->value);

        goto cleanup;
    }

    return PyString_FromString(ret);

cleanup:
    g_free(pdata);
    return NULL;
}

static PyObject *
pypicman_progress_uninstall(PyObject *self, PyObject *args)
{
    ProgressData *pdata;
    gchar *callback;

    if (!PyArg_ParseTuple(args, "s:progress_uninstall", &callback))
        return NULL;

    pdata = picman_progress_uninstall(callback);

    if (!pdata) {
        PyErr_SetString(pypicman_error,
                        "error occurred while uninstalling progress functions");
        return NULL;
    }

    Py_DECREF(pdata->start);
    Py_DECREF(pdata->end);
    Py_DECREF(pdata->text);
    Py_DECREF(pdata->value);

    Py_XDECREF(pdata->user_data);

    g_free(pdata);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_image_list(PyObject *self)
{
    gint32 *imgs;
    int nimgs, i;
    PyObject *ret;

    imgs = picman_image_list(&nimgs);
    ret = PyList_New(nimgs);

    for (i = 0; i < nimgs; i++)
        PyList_SetItem(ret, i, (PyObject *)pypicman_image_new(imgs[i]));

    g_free(imgs);

    return ret;
}

static PyObject *
pypicman_install_procedure(PyObject *self, PyObject *args)
{
    char *name, *blurb, *help, *author, *copyright, *date, *menu_path,
         *image_types, *n, *d;
    PicmanParamDef *params, *return_vals;
    int type, nparams, nreturn_vals, i;
    PyObject *pars, *rets;

    if (!PyArg_ParseTuple(args, "sssssszziOO:install_procedure",
                          &name, &blurb, &help,
                          &author, &copyright, &date, &menu_path, &image_types,
                          &type, &pars, &rets))
        return NULL;

    if (!PySequence_Check(pars) || !PySequence_Check(rets)) {
        PyErr_SetString(PyExc_TypeError, "last two args must be sequences");
        return NULL;
    }

    nparams = PySequence_Length(pars);
    nreturn_vals = PySequence_Length(rets);
    params = g_new(PicmanParamDef, nparams);

    for (i = 0; i < nparams; i++) {
        if (!PyArg_ParseTuple(PySequence_GetItem(pars, i), "iss",
                              &(params[i].type), &n, &d)) {
            g_free(params);
            return NULL;
        }

        params[i].name = g_strdup(n);
        params[i].description = g_strdup(d);
    }

    return_vals = g_new(PicmanParamDef, nreturn_vals);

    for (i = 0; i < nreturn_vals; i++) {
        if (!PyArg_ParseTuple(PySequence_GetItem(rets, i), "iss",
                              &(return_vals[i].type), &n, &d)) {
            g_free(params); g_free(return_vals);
            return NULL;
        }

        return_vals[i].name = g_strdup(n);
        return_vals[i].description = g_strdup(d);
    }

    picman_install_procedure(name, blurb, help, author, copyright, date,
                           menu_path, image_types, type, nparams, nreturn_vals,
                           params, return_vals);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_install_temp_proc(PyObject *self, PyObject *args)
{
    char *name, *blurb, *help, *author, *copyright, *date, *menu_path,
        *image_types, *n, *d;
    PicmanParamDef *params, *return_vals;
    int type, nparams, nreturn_vals, i;
    PyObject *pars, *rets;

    if (!PyArg_ParseTuple(args, "sssssszziOO:install_temp_proc",
                          &name, &blurb, &help,
                          &author, &copyright, &date, &menu_path, &image_types,
                          &type, &pars, &rets))
        return NULL;

    if (!PySequence_Check(pars) || !PySequence_Check(rets)) {
        PyErr_SetString(PyExc_TypeError, "last two args must be sequences");
        return NULL;
    }

    nparams = PySequence_Length(pars);
    nreturn_vals = PySequence_Length(rets);
    params = g_new(PicmanParamDef, nparams);

    for (i = 0; i < nparams; i++) {
        if (!PyArg_ParseTuple(PySequence_GetItem(pars, i), "iss",
                              &(params[i].type), &n, &d)) {
            g_free(params);
            return NULL;
        }

        params[i].name = g_strdup(n);
        params[i].description = g_strdup(d);
    }

    return_vals = g_new(PicmanParamDef, nreturn_vals);

    for (i = 0; i < nreturn_vals; i++) {
        if (!PyArg_ParseTuple(PySequence_GetItem(rets, i), "iss",
                              &(return_vals[i].type), &n, &d)) {
            g_free(params); g_free(return_vals);
            return NULL;
        }

        return_vals[i].name = g_strdup(n);
        return_vals[i].description = g_strdup(d);
    }

    picman_install_temp_proc(name, blurb, help, author, copyright, date,
                           menu_path, image_types, type,
                           nparams, nreturn_vals, params, return_vals,
                           pypicman_run_proc);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_uninstall_temp_proc(PyObject *self, PyObject *args)
{
    char *name;

    if (!PyArg_ParseTuple(args, "s:uninstall_temp_proc", &name))
        return NULL;

    picman_uninstall_temp_proc(name);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_register_magic_load_handler(PyObject *self, PyObject *args)
{
    char *name, *extensions, *prefixes, *magics;

    if (!PyArg_ParseTuple(args, "ssss:register_magic_load_handler",
                          &name, &extensions, &prefixes, &magics))
        return NULL;

    picman_register_magic_load_handler(name, extensions, prefixes, magics);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_register_load_handler(PyObject *self, PyObject *args)
{
    char *name, *extensions, *prefixes;

    if (!PyArg_ParseTuple(args, "sss:register_load_handler",
                          &name, &extensions, &prefixes))
        return NULL;

    picman_register_load_handler(name, extensions, prefixes);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_register_save_handler(PyObject *self, PyObject *args)
{
    char *name, *extensions, *prefixes;

    if (!PyArg_ParseTuple(args, "sss:register_save_handler",
                          &name, &extensions, &prefixes))
        return NULL;

    picman_register_save_handler(name, extensions, prefixes);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_domain_register(PyObject *self, PyObject *args)
{
    char *name, *path = NULL;

    if (!PyArg_ParseTuple(args, "s|s:domain_register", &name, &path))
        return NULL;

    picman_plugin_domain_register(name, path);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_menu_register(PyObject *self, PyObject *args)
{
    char *name, *path;

    if (!PyArg_ParseTuple(args, "ss:menu_register", &name, &path))
        return NULL;

    picman_plugin_menu_register(name, path);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_gamma(PyObject *self)
{
    return PyFloat_FromDouble(picman_gamma());
}

static PyObject *
pypicman_gtkrc(PyObject *self)
{
    return PyString_FromString(picman_gtkrc());
}

static PyObject *
pypicman_personal_rc_file(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char *basename, *filename;
    PyObject *ret;

    static char *kwlist[] = { "basename", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "s:personal_rc_file", kwlist,
                                     &basename))
        return NULL;

    filename = picman_personal_rc_file(basename);
    ret = PyString_FromString(filename);
    g_free(filename);

    return ret;
}

static PyObject *
pypicman_context_push(PyObject *self)
{
    picman_context_push();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_context_pop(PyObject *self)
{
    picman_context_pop();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_get_background(PyObject *self)
{
    PicmanRGB rgb;

    picman_context_get_background(&rgb);
    return pypicman_rgb_new(&rgb);
}

static PyObject *
pypicman_get_foreground(PyObject *self)
{
    PicmanRGB rgb;

    picman_context_get_foreground(&rgb);
    return pypicman_rgb_new(&rgb);
}

static PyObject *
pypicman_set_background(PyObject *self, PyObject *args)
{
    PyObject *color;
    PicmanRGB rgb;

    if (PyArg_ParseTuple(args, "O:set_background", &color)) {
        if (!pypicman_rgb_from_pyobject(color, &rgb))
            return NULL;
    } else {
        PyErr_Clear();
        if (!pypicman_rgb_from_pyobject(args, &rgb))
            return NULL;
    }

    picman_context_set_background(&rgb);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_set_foreground(PyObject *self, PyObject *args)
{
    PyObject *color;
    PicmanRGB rgb;

    if (PyArg_ParseTuple(args, "O:set_foreground", &color)) {
        if (!pypicman_rgb_from_pyobject(color, &rgb))
            return NULL;
    } else {
        PyErr_Clear();
        if (!pypicman_rgb_from_pyobject(args, &rgb))
            return NULL;
    }

    picman_context_set_foreground(&rgb);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_gradients_get_list(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char **list, *filter = NULL;
    int num, i;
    PyObject *ret;

    static char *kwlist[] = { "filter", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|s:gradients_get_list", kwlist,
                                     &filter))
        return NULL;

    list = picman_gradients_get_list(filter, &num);

    ret = PyList_New(num);

    for (i = 0; i < num; i++) {
        PyList_SetItem(ret, i, PyString_FromString(list[i]));
    }

    g_strfreev(list);

    return ret;
}

static PyObject *
pypicman_context_get_gradient(PyObject *self)
{
    char *name;
    PyObject *ret;

    name = picman_context_get_gradient();
    ret = PyString_FromString(name);
    g_free(name);

    return ret;
}

static PyObject *
pypicman_gradients_get_gradient(PyObject *self)
{
    if (PyErr_Warn(PyExc_DeprecationWarning, "use picman.context_get_gradient") < 0)
        return NULL;

    return pypicman_context_get_gradient(self);
}

static PyObject *
pypicman_context_set_gradient(PyObject *self, PyObject *args)
{
    char *actv;

    if (!PyArg_ParseTuple(args, "s:gradients_set_gradient", &actv))
        return NULL;

    picman_context_set_gradient(actv);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_gradients_set_gradient(PyObject *self, PyObject *args)
{
    if (PyErr_Warn(PyExc_DeprecationWarning, "use picman.context_set_gradient") < 0)
        return NULL;

    return pypicman_context_set_gradient(self, args);
}

static PyObject *
pypicman_gradient_get_uniform_samples(PyObject *self, PyObject *args)
{
    int num, reverse = FALSE;
    char *name;
    int nsamp;
    double *samp;
    int i, j;
    PyObject *ret;

    if (!PyArg_ParseTuple(args, "si|i:gradient_get_uniform_samples",
                          &name, &num, &reverse))
        return NULL;

    if (!picman_gradient_get_uniform_samples(name, num, reverse, &nsamp, &samp)) {
        PyErr_SetString(pypicman_error, "gradient_get_uniform_samples failed");
        return NULL;
    }

    ret = PyList_New(num);
    for (i = 0, j = 0; i < num; i++, j += 4)
        PyList_SetItem(ret, i, Py_BuildValue("(dddd)", samp[j],
                                             samp[j+1], samp[j+2], samp[j+3]));

    g_free(samp);

    return ret;
}

static PyObject *
pypicman_gradient_get_custom_samples(PyObject *self, PyObject *args)
{
    int num, reverse = FALSE;
    char *name;
    int nsamp;
    double *pos, *samp;
    int i, j;
    PyObject *ret, *item;
    gboolean success;

    if (!PyArg_ParseTuple(args, "sO|i:gradient_get_custom_samples",
                          &name, &ret, &reverse))
        return NULL;

    if (!PySequence_Check(ret)) {
        PyErr_SetString(PyExc_TypeError,
                        "second arg must be a sequence");
        return NULL;
    }

    num = PySequence_Length(ret);
    pos = g_new(gdouble, num);

    for (i = 0; i < num; i++) {
        item = PySequence_GetItem(ret, i);

        if (!PyFloat_Check(item)) {
            PyErr_SetString(PyExc_TypeError,
                            "second arg must be a sequence of floats");
            g_free(pos);
            return NULL;
        }

        pos[i] = PyFloat_AsDouble(item);
    }

    success = picman_gradient_get_custom_samples(name, num, pos, reverse,
                                               &nsamp, &samp);
    g_free(pos);

    if (!success) {
        PyErr_SetString(pypicman_error, "gradient_get_custom_samples failed");
        return NULL;
    }

    ret = PyList_New(num);
    for (i = 0, j = 0; i < num; i++, j += 4)
        PyList_SetItem(ret, i, Py_BuildValue("(dddd)", samp[j],
                                             samp[j+1], samp[j+2], samp[j+3]));

    g_free(samp);

    return ret;
}

static PyObject *
pypicman_gradients_sample_uniform(PyObject *self, PyObject *args)
{
    char *name;
    PyObject *arg_list, *str, *new_args, *ret;

    if (PyErr_Warn(PyExc_DeprecationWarning,
                   "use picman.gradient_get_uniform_samples") < 0)
        return NULL;

    arg_list = PySequence_List(args);

    name = picman_context_get_gradient();

    str = PyString_FromString(name);
    g_free(name);

    PyList_Insert(arg_list, 0, str);
    Py_XDECREF(str);

    new_args = PyList_AsTuple(arg_list);
    Py_XDECREF(arg_list);

    ret = pypicman_gradient_get_uniform_samples(self, new_args);
    Py_XDECREF(new_args);

    return ret;
}

static PyObject *
pypicman_gradients_sample_custom(PyObject *self, PyObject *args)
{
    char *name;
    PyObject *arg_list, *str, *new_args, *ret;

    if (PyErr_Warn(PyExc_DeprecationWarning,
                   "use picman.gradient_get_custom_samples") < 0)
        return NULL;

    arg_list = PySequence_List(args);

    name = picman_context_get_gradient();

    str = PyString_FromString(name);
    g_free(name);

    PyList_Insert(arg_list, 0, str);
    Py_XDECREF(str);

    new_args = PyList_AsTuple(arg_list);
    Py_XDECREF(arg_list);

    ret = pypicman_gradient_get_custom_samples(self, new_args);

    return ret;
}

static PyObject *
pypicman_delete(PyObject *self, PyObject *args)
{
    PyPicmanImage *img;

    if (!PyArg_ParseTuple(args, "O:delete", &img))
        return NULL;

    if (pypicman_image_check(img))
        picman_image_delete(img->ID);
    else if (pypicman_drawable_check(img))
        picman_item_delete(img->ID);
    else if (pypicman_display_check(img))
        picman_display_delete(img->ID);

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
pypicman_displays_flush(PyObject *self)
{
    picman_displays_flush();

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_displays_reconnect(PyObject *self, PyObject *args)
{
    PyPicmanImage *old_img, *new_img;

    if (!PyArg_ParseTuple(args, "O!O!:displays_reconnect",
                          &PyPicmanImage_Type, &old_img,
                          &PyPicmanImage_Type, &new_img))
        return NULL;

    if (!picman_displays_reconnect (old_img->ID, new_img->ID)) {
        PyErr_Format(pypicman_error,
                     "could not reconnect the displays of image (ID %d) "
                     "to image (ID %d)",
                     old_img->ID, new_img->ID);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_tile_cache_size(PyObject *self, PyObject *args)
{
    unsigned long k;

    if (!PyArg_ParseTuple(args, "l:tile_cache_size", &k))
        return NULL;

    picman_tile_cache_size(k);

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
pypicman_tile_cache_ntiles(PyObject *self, PyObject *args)
{
    unsigned long n;

    if (!PyArg_ParseTuple(args, "l:tile_cache_ntiles", &n))
        return NULL;

    picman_tile_cache_ntiles(n);

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *
pypicman_tile_width(PyObject *self)
{
    return PyInt_FromLong(picman_tile_width());
}


static PyObject *
pypicman_tile_height(PyObject *self)
{
    return PyInt_FromLong(picman_tile_height());
}

static PyObject *
pypicman_extension_ack(PyObject *self)
{
    picman_extension_ack();

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_extension_enable(PyObject *self)
{
    picman_extension_enable();

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_extension_process(PyObject *self, PyObject *args)
{
    guint timeout;

    if (!PyArg_ParseTuple(args, "I:extension_process", &timeout))
        return NULL;

    picman_extension_process(timeout);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_parasite_find(PyObject *self, PyObject *args)
{
    char *name;

    if (!PyArg_ParseTuple(args, "s:parasite_find", &name))
        return NULL;

    return pypicman_parasite_new(picman_get_parasite(name));
}

static PyObject *
pypicman_parasite_attach(PyObject *self, PyObject *args)
{
    PyPicmanParasite *parasite;

    if (!PyArg_ParseTuple(args, "O!:parasite_attach",
                          &PyPicmanParasite_Type, &parasite))
        return NULL;

    if (!picman_attach_parasite(parasite->para)) {
        PyErr_Format(pypicman_error, "could not attach parasite '%s'",
                     picman_parasite_name(parasite->para));
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_attach_new_parasite(PyObject *self, PyObject *args)
{
    PicmanParasite *parasite;
    char *name, *data;
    int flags, size;

    if (!PyArg_ParseTuple(args, "sis#:attach_new_parasite", &name, &flags,
                          &data, &size))
        return NULL;

    parasite = picman_parasite_new (name, flags, size, data);

    if (!picman_attach_parasite (parasite)) {
        PyErr_Format(pypicman_error, "could not attach new parasite '%s'", name);
        picman_parasite_free (parasite);
        return NULL;
    }

    picman_parasite_free (parasite);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_parasite_detach(PyObject *self, PyObject *args)
{
    char *name;

    if (!PyArg_ParseTuple(args, "s:parasite_detach", &name))
        return NULL;

    if (!picman_detach_parasite(name)) {
        PyErr_Format(pypicman_error, "could not detach parasite '%s'", name);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_parasite_list(PyObject *self)
{
    gint num_parasites;
    gchar **parasites;

    parasites = picman_get_parasite_list (&num_parasites);

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

    PyErr_SetString(pypicman_error, "could not list parasites");
    return NULL;
}

static PyObject *
pypicman_show_tool_tips(PyObject *self)
{
    return PyBool_FromLong(picman_show_tool_tips());
}

static PyObject *
pypicman_show_help_button(PyObject *self)
{
    return PyBool_FromLong(picman_show_help_button());
}

static PyObject *
pypicman_check_size(PyObject *self)
{
    return PyInt_FromLong(picman_check_size());
}

static PyObject *
pypicman_check_type(PyObject *self)
{
    return PyInt_FromLong(picman_check_type());
}

static PyObject *
pypicman_default_display(PyObject *self)
{
    return pypicman_display_new(picman_default_display());
}

static PyObject *
pypicman_wm_class(PyObject *self)
{
    return PyString_FromString(picman_wm_class());
}

static PyObject *
pypicman_display_name(PyObject *self)
{
    return PyString_FromString(picman_display_name());
}

static PyObject *
pypicman_monitor_number(PyObject *self)
{
    return PyInt_FromLong(picman_monitor_number());
}

static PyObject *
pypicman_get_progname(PyObject *self)
{
    return PyString_FromString(picman_get_progname());
}

static PyObject *
pypicman_user_directory(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PicmanUserDirectory type;
    const char *user_dir;
    PyObject *py_type, *ret;

    static char *kwlist[] = { "type", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O:user_directory", kwlist,
                                     &py_type))
        return NULL;

    if (pyg_enum_get_value(PICMAN_TYPE_USER_DIRECTORY, py_type, (gpointer)&type))
        return NULL;

    user_dir = g_get_user_special_dir(type);

    if (user_dir) {
        ret = PyString_FromString(user_dir);
    } else {
        Py_INCREF(Py_None);
	ret = Py_None;
    }

    return ret;
}

static PyObject *
pypicman_fonts_refresh(PyObject *self)
{
    if (!picman_fonts_refresh()) {
        PyErr_SetString(pypicman_error, "could not refresh fonts");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pypicman_checks_get_shades(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int type;
    guchar light, dark;
    static char *kwlist[] = { "type", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "i:checks_get_shades", kwlist,
                                     &type))
        return NULL;

    if (type < PICMAN_CHECK_TYPE_LIGHT_CHECKS ||
        type > PICMAN_CHECK_TYPE_BLACK_ONLY) {
        PyErr_SetString(PyExc_ValueError, "Invalid check type");
        return NULL;
    }

    picman_checks_get_shades(type, &light, &dark);

    return Py_BuildValue("(ii)", light, dark);
}

static PyObject *
pypicman_fonts_get_list(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char **list, *filter = NULL;
    int num, i;
    PyObject *ret;

    static char *kwlist[] = { "filter", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|s:fonts_get_list", kwlist,
                                     &filter))
        return NULL;

    list = picman_fonts_get_list(filter, &num);

    if (num == 0) {
        PyErr_SetString(pypicman_error, "could not get font list");
        return NULL;
    }

    ret = PyList_New(num);

    for (i = 0; i < num; i++) {
        PyList_SetItem(ret, i, PyString_FromString(list[i]));
    }

    g_strfreev(list);

    return ret;
}

static PyObject *
vectors_to_objects(int num_vectors, int *vectors)
{
    PyObject *ret;
    int i;

    ret = PyList_New(num_vectors);
    if (ret == NULL)
        goto done;

    for (i = 0; i < num_vectors; i++)
        PyList_SetItem(ret, i, pypicman_vectors_new(vectors[i]));

done:
    g_free(vectors);
    return ret;
}

static PyObject *
pypicman_vectors_import_from_file(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyPicmanImage *img;
    PyObject *py_file;
    gboolean merge = FALSE, scale = FALSE;
    int *vectors, num_vectors;
    gboolean success;

    static char *kwlist[] = { "image", "svg_file", "merge", "scale", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!O|ii:vectors_import_from_file", kwlist,
                                     &PyPicmanImage_Type, &img, &py_file,
                                     &merge, &scale))
        return NULL;

    if (PyString_Check(py_file)) {
        success = picman_vectors_import_from_file(img->ID,
                                                PyString_AsString(py_file),
                                                merge, scale,
                                                &num_vectors, &vectors);
    } else {
        PyObject *chunk_size, *buffer, *read_method;

        chunk_size = PyInt_FromLong(16 * 1024);
        if (chunk_size == NULL)
            return NULL;

        buffer = PyString_FromString("");
        if (buffer == NULL) {
            Py_DECREF(chunk_size);
            return NULL;
        }

        read_method = PyString_FromString("read");
        if (read_method == NULL || !PyCallable_Check(read_method)) {
            Py_XDECREF(read_method);
            PyErr_SetString(PyExc_TypeError,
                            "svg_file must be an object that has a \"read\" "
                            "method, or a filename (str)");
            return NULL;
        }

        while (1) {
            PyObject *chunk;
            chunk = PyObject_CallMethodObjArgs(py_file, read_method,
                                               chunk_size, NULL);

            if (!chunk || !PyString_Check(chunk)) {
                Py_XDECREF(chunk);
                Py_DECREF(chunk_size);
                Py_DECREF(buffer);
                Py_DECREF(read_method);
                return NULL;
            }

            if (PyString_GET_SIZE(chunk) != 0) {
                PyString_ConcatAndDel(&buffer, chunk);
                if (buffer == NULL) {
                    Py_DECREF(chunk_size);
                    Py_DECREF(read_method);
                    return NULL;
                }
            } else {
                Py_DECREF(chunk);
                break;
            }
        }

        success = picman_vectors_import_from_string(img->ID,
                                                  PyString_AsString(buffer),
                                                  PyString_Size(buffer),
                                                  merge, scale,
                                                  &num_vectors, &vectors);

        Py_DECREF(chunk_size);
        Py_DECREF(buffer);
        Py_DECREF(read_method);
    }

    if (!success) {
        PyErr_Format(pypicman_error,
                     "Vectors import failed: %s", picman_get_pdb_error());
        return NULL;
    }

    return vectors_to_objects(num_vectors, vectors);
}

static PyObject *
pypicman_vectors_import_from_string(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyPicmanImage *img;
    const char *svg_string;
    int length;
    gboolean merge = FALSE, scale = FALSE;
    int *vectors, num_vectors;
    gboolean success;

    static char *kwlist[] = { "image", "svg_string", "merge", "scale", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!s#|ii:vectors_import_from_string", kwlist,
                                     &PyPicmanImage_Type, &img,
                                     &svg_string, &length,
                                     &merge, &scale))
        return NULL;

    success = picman_vectors_import_from_string(img->ID, svg_string, length,
                                              merge, scale,
                                              &num_vectors, &vectors);

    if (!success) {
        PyErr_Format(pypicman_error,
                     "Vectors import failed: %s", picman_get_pdb_error());
        return NULL;
    }

    return vectors_to_objects(num_vectors, vectors);
}

static PyObject *
id2image(PyObject *self, PyObject *args)
{
    int id;

    if (!PyArg_ParseTuple(args, "i:_id2image", &id))
        return NULL;

    if (id >= 0)
        return pypicman_image_new(id);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
id2drawable(PyObject *self, PyObject *args)
{
    int id;

    if (!PyArg_ParseTuple(args, "i:_id2drawable", &id))
        return NULL;

    if (id >= 0)
        return pypicman_drawable_new(NULL, id);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
id2display(PyObject *self, PyObject *args)
{
    int id;

    if (!PyArg_ParseTuple(args, "i:_id2display", &id))
        return NULL;

    if (id >= 0)
        return pypicman_display_new(id);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
id2vectors(PyObject *self, PyObject *args)
{
    int id;

    if (!PyArg_ParseTuple(args, "i:_id2vectors", &id))
        return NULL;

    if (id >= 0)
        return pypicman_vectors_new(id);

    Py_INCREF(Py_None);
    return Py_None;
}

/* List of methods defined in the module */

static struct PyMethodDef picman_methods[] = {
    {"main",    (PyCFunction)pypicman_main,       METH_VARARGS},
    {"quit",    (PyCFunction)pypicman_quit,       METH_NOARGS},
    {"message", (PyCFunction)pypicman_message,    METH_VARARGS},
    {"exit",    (PyCFunction)pypicman_exit,       METH_VARARGS | METH_KEYWORDS},
    {"set_data",        (PyCFunction)pypicman_set_data,   METH_VARARGS},
    {"get_data",        (PyCFunction)pypicman_get_data,   METH_VARARGS},
    {"progress_init",   (PyCFunction)pypicman_progress_init,      METH_VARARGS},
    {"progress_update", (PyCFunction)pypicman_progress_update,    METH_VARARGS},
    {"progress_install",        (PyCFunction)pypicman_progress_install,   METH_VARARGS | METH_KEYWORDS},
    {"progress_uninstall",      (PyCFunction)pypicman_progress_uninstall, METH_VARARGS},
    {"image_list",      (PyCFunction)pypicman_image_list, METH_NOARGS},
    {"install_procedure",       (PyCFunction)pypicman_install_procedure,  METH_VARARGS},
    {"install_temp_proc",       (PyCFunction)pypicman_install_temp_proc,  METH_VARARGS},
    {"uninstall_temp_proc",     (PyCFunction)pypicman_uninstall_temp_proc,        METH_VARARGS},
    {"register_magic_load_handler",     (PyCFunction)pypicman_register_magic_load_handler,        METH_VARARGS},
    {"register_load_handler",   (PyCFunction)pypicman_register_load_handler,      METH_VARARGS},
    {"register_save_handler",   (PyCFunction)pypicman_register_save_handler,      METH_VARARGS},
    {"domain_register",         (PyCFunction)pypicman_domain_register,    METH_VARARGS},
    {"menu_register",           (PyCFunction)pypicman_menu_register,      METH_VARARGS},
    {"gamma",   (PyCFunction)pypicman_gamma,      METH_NOARGS},
    {"gtkrc",   (PyCFunction)pypicman_gtkrc,      METH_NOARGS},
    {"personal_rc_file",        (PyCFunction)pypicman_personal_rc_file, METH_VARARGS | METH_KEYWORDS},
    {"context_push", (PyCFunction)pypicman_context_push, METH_NOARGS},
    {"context_pop", (PyCFunction)pypicman_context_pop, METH_NOARGS},
    {"get_background",  (PyCFunction)pypicman_get_background,     METH_NOARGS},
    {"get_foreground",  (PyCFunction)pypicman_get_foreground,     METH_NOARGS},
    {"set_background",  (PyCFunction)pypicman_set_background,     METH_VARARGS},
    {"set_foreground",  (PyCFunction)pypicman_set_foreground,     METH_VARARGS},
    {"gradients_get_list",      (PyCFunction)pypicman_gradients_get_list, METH_VARARGS | METH_KEYWORDS},
    {"context_get_gradient",    (PyCFunction)pypicman_context_get_gradient,       METH_NOARGS},
    {"context_set_gradient",    (PyCFunction)pypicman_context_set_gradient,       METH_VARARGS},
    {"gradients_get_gradient",  (PyCFunction)pypicman_gradients_get_gradient,     METH_NOARGS},
    {"gradients_set_gradient",  (PyCFunction)pypicman_gradients_set_gradient,     METH_VARARGS},
    {"gradient_get_uniform_samples",    (PyCFunction)pypicman_gradient_get_uniform_samples,       METH_VARARGS},
    {"gradient_get_custom_samples",     (PyCFunction)pypicman_gradient_get_custom_samples,        METH_VARARGS},
    {"gradients_sample_uniform",        (PyCFunction)pypicman_gradients_sample_uniform,   METH_VARARGS},
    {"gradients_sample_custom", (PyCFunction)pypicman_gradients_sample_custom,    METH_VARARGS},
    {"delete", (PyCFunction)pypicman_delete, METH_VARARGS},
    {"displays_flush", (PyCFunction)pypicman_displays_flush, METH_NOARGS},
    {"displays_reconnect", (PyCFunction)pypicman_displays_reconnect, METH_VARARGS},
    {"tile_cache_size", (PyCFunction)pypicman_tile_cache_size, METH_VARARGS},
    {"tile_cache_ntiles", (PyCFunction)pypicman_tile_cache_ntiles, METH_VARARGS},
    {"tile_width", (PyCFunction)pypicman_tile_width, METH_NOARGS},
    {"tile_height", (PyCFunction)pypicman_tile_height, METH_NOARGS},
    {"extension_ack", (PyCFunction)pypicman_extension_ack, METH_NOARGS},
    {"extension_enable", (PyCFunction)pypicman_extension_enable, METH_NOARGS},
    {"extension_process", (PyCFunction)pypicman_extension_process, METH_VARARGS},
    {"parasite_find",      (PyCFunction)pypicman_parasite_find,      METH_VARARGS},
    {"parasite_attach",    (PyCFunction)pypicman_parasite_attach,    METH_VARARGS},
    {"attach_new_parasite",(PyCFunction)pypicman_attach_new_parasite,METH_VARARGS},
    {"parasite_detach",    (PyCFunction)pypicman_parasite_detach,    METH_VARARGS},
    {"parasite_list",    (PyCFunction)pypicman_parasite_list,    METH_NOARGS},
    {"show_tool_tips",  (PyCFunction)pypicman_show_tool_tips,  METH_NOARGS},
    {"show_help_button",  (PyCFunction)pypicman_show_help_button,  METH_NOARGS},
    {"check_size",  (PyCFunction)pypicman_check_size,  METH_NOARGS},
    {"check_type",  (PyCFunction)pypicman_check_type,  METH_NOARGS},
    {"default_display",  (PyCFunction)pypicman_default_display,  METH_NOARGS},
    {"wm_class", (PyCFunction)pypicman_wm_class,  METH_NOARGS},
    {"display_name", (PyCFunction)pypicman_display_name,  METH_NOARGS},
    {"monitor_number", (PyCFunction)pypicman_monitor_number,      METH_NOARGS},
    {"get_progname", (PyCFunction)pypicman_get_progname,  METH_NOARGS},
    {"user_directory", (PyCFunction)pypicman_user_directory, METH_VARARGS | METH_KEYWORDS},
    {"fonts_refresh", (PyCFunction)pypicman_fonts_refresh,        METH_NOARGS},
    {"fonts_get_list", (PyCFunction)pypicman_fonts_get_list,      METH_VARARGS | METH_KEYWORDS},
    {"checks_get_shades", (PyCFunction)pypicman_checks_get_shades, METH_VARARGS | METH_KEYWORDS},
    {"vectors_import_from_file", (PyCFunction)pypicman_vectors_import_from_file, METH_VARARGS | METH_KEYWORDS},
    {"vectors_import_from_string", (PyCFunction)pypicman_vectors_import_from_string, METH_VARARGS | METH_KEYWORDS},
    {"_id2image", (PyCFunction)id2image, METH_VARARGS},
    {"_id2drawable", (PyCFunction)id2drawable, METH_VARARGS},
    {"_id2display", (PyCFunction)id2display, METH_VARARGS},
    {"_id2vectors", (PyCFunction)id2vectors, METH_VARARGS},
    {NULL,       (PyCFunction)NULL, 0, NULL}            /* sentinel */
};


static struct _PyPicman_Functions pypicman_api_functions = {
    &PyPicmanImage_Type,
    pypicman_image_new,
    &PyPicmanDisplay_Type,
    pypicman_display_new,
    &PyPicmanItem_Type,
    pypicman_item_new,
    &PyPicmanDrawable_Type,
    pypicman_drawable_new,
    &PyPicmanLayer_Type,
    pypicman_layer_new,
    &PyPicmanGroupLayer_Type,
    pypicman_group_layer_new,
    &PyPicmanChannel_Type,
    pypicman_channel_new,
    &PyPicmanVectors_Type,
    pypicman_vectors_new,
};


/* Initialization function for the module (*must* be called initpicman) */

static char picman_module_documentation[] =
"This module provides interfaces to allow you to write picman plugins"
;

void initpicman(void);

PyMODINIT_FUNC
initpicman(void)
{
    PyObject *m;

    PyPicmanPDB_Type.ob_type = &PyType_Type;
    PyPicmanPDB_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanPDB_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanPDB_Type) < 0)
        return;

    PyPicmanPDBFunction_Type.ob_type = &PyType_Type;
    PyPicmanPDBFunction_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanPDBFunction_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanPDBFunction_Type) < 0)
        return;

    PyPicmanImage_Type.ob_type = &PyType_Type;
    PyPicmanImage_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanImage_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanImage_Type) < 0)
        return;

    PyPicmanDisplay_Type.ob_type = &PyType_Type;
    PyPicmanDisplay_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanDisplay_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanDisplay_Type) < 0)
        return;

    PyPicmanLayer_Type.ob_type = &PyType_Type;
    PyPicmanLayer_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanLayer_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanLayer_Type) < 0)
        return;

    PyPicmanGroupLayer_Type.ob_type = &PyType_Type;
    PyPicmanGroupLayer_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanGroupLayer_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanGroupLayer_Type) < 0)
        return;

    PyPicmanChannel_Type.ob_type = &PyType_Type;
    PyPicmanChannel_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanChannel_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanChannel_Type) < 0)
        return;

    PyPicmanTile_Type.ob_type = &PyType_Type;
    PyPicmanTile_Type.tp_alloc = PyType_GenericAlloc;
    if (PyType_Ready(&PyPicmanTile_Type) < 0)
        return;

    PyPicmanPixelRgn_Type.ob_type = &PyType_Type;
    PyPicmanPixelRgn_Type.tp_alloc = PyType_GenericAlloc;
    if (PyType_Ready(&PyPicmanPixelRgn_Type) < 0)
        return;

    PyPicmanParasite_Type.ob_type = &PyType_Type;
    PyPicmanParasite_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanParasite_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanParasite_Type) < 0)
        return;

    PyPicmanVectorsStroke_Type.ob_type = &PyType_Type;
    PyPicmanVectorsStroke_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanVectorsStroke_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanVectorsStroke_Type) < 0)
        return;

    PyPicmanVectorsBezierStroke_Type.ob_type = &PyType_Type;
    PyPicmanVectorsBezierStroke_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanVectorsBezierStroke_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanVectorsBezierStroke_Type) < 0)
        return;

    PyPicmanVectors_Type.ob_type = &PyType_Type;
    PyPicmanVectors_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanVectors_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanVectors_Type) < 0)
        return;

    PyPicmanPixelFetcher_Type.ob_type = &PyType_Type;
    PyPicmanPixelFetcher_Type.tp_alloc = PyType_GenericAlloc;
    PyPicmanPixelFetcher_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPicmanPixelFetcher_Type) < 0)
        return;

    pypicman_init_pygobject();
    init_pypicmancolor();

    /* initialize i18n support */
    bindtextdomain (GETTEXT_PACKAGE "-python", picman_locale_directory ());
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE "-python", "UTF-8");
#endif

    /* set the default python encoding to utf-8 */
    PyUnicode_SetDefaultEncoding("utf-8");

    /* Create the module and add the functions */
    m = Py_InitModule4("picman", picman_methods,
                       picman_module_documentation,
                       NULL, PYTHON_API_VERSION);

    /* Add some symbolic constants to the module */
    pypicman_error = PyErr_NewException("picman.error", PyExc_RuntimeError, NULL);
    PyModule_AddObject(m, "error", pypicman_error);

    PyModule_AddObject(m, "pdb", pypicman_pdb_new());

    /* export the types used in picmanmodule */
    Py_INCREF(&PyPicmanImage_Type);
    PyModule_AddObject(m, "Image", (PyObject *)&PyPicmanImage_Type);

    Py_INCREF(&PyPicmanItem_Type);
    PyModule_AddObject(m, "Item", (PyObject *)&PyPicmanItem_Type);

    Py_INCREF(&PyPicmanDrawable_Type);
    PyModule_AddObject(m, "Drawable", (PyObject *)&PyPicmanDrawable_Type);

    Py_INCREF(&PyPicmanLayer_Type);
    PyModule_AddObject(m, "Layer", (PyObject *)&PyPicmanLayer_Type);
    
    Py_INCREF(&PyPicmanGroupLayer_Type);
    PyModule_AddObject(m, "GroupLayer", (PyObject *)&PyPicmanGroupLayer_Type);

    Py_INCREF(&PyPicmanChannel_Type);
    PyModule_AddObject(m, "Channel", (PyObject *)&PyPicmanChannel_Type);

    Py_INCREF(&PyPicmanDisplay_Type);
    PyModule_AddObject(m, "Display", (PyObject *)&PyPicmanDisplay_Type);

    Py_INCREF(&PyPicmanTile_Type);
    PyModule_AddObject(m, "Tile", (PyObject *)&PyPicmanTile_Type);

    Py_INCREF(&PyPicmanPixelRgn_Type);
    PyModule_AddObject(m, "PixelRgn", (PyObject *)&PyPicmanPixelRgn_Type);

    Py_INCREF(&PyPicmanParasite_Type);
    PyModule_AddObject(m, "Parasite", (PyObject *)&PyPicmanParasite_Type);

    Py_INCREF(&PyPicmanVectorsBezierStroke_Type);
    PyModule_AddObject(m, "VectorsBezierStroke", (PyObject *)&PyPicmanVectorsBezierStroke_Type);

    Py_INCREF(&PyPicmanVectors_Type);
    PyModule_AddObject(m, "Vectors", (PyObject *)&PyPicmanVectors_Type);

    Py_INCREF(&PyPicmanPixelFetcher_Type);
    PyModule_AddObject(m, "PixelFetcher", (PyObject *)&PyPicmanPixelFetcher_Type);

    /* for other modules */
    pypicman_api_functions.pypicman_error = pypicman_error;

    PyModule_AddObject(m, "_PyPicman_API",
                       PyCObject_FromVoidPtr(&pypicman_api_functions, NULL));

    PyModule_AddObject(m, "version",
                       Py_BuildValue("(iii)",
                                     picman_major_version,
                                     picman_minor_version,
                                     picman_micro_version));

    /* Some environment constants */
    PyModule_AddObject(m, "directory",
                       PyString_FromString(picman_directory()));
    PyModule_AddObject(m, "data_directory",
                       PyString_FromString(picman_data_directory()));
    PyModule_AddObject(m, "locale_directory",
                       PyString_FromString(picman_locale_directory()));
    PyModule_AddObject(m, "sysconf_directory",
                       PyString_FromString(picman_sysconf_directory()));
    PyModule_AddObject(m, "plug_in_directory",
                       PyString_FromString(picman_plug_in_directory()));

    /* Check for errors */
    if (PyErr_Occurred())
        Py_FatalError("can't initialize module picman");
}
