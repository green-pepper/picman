/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanquerybox.h
 * Copyright (C) 1999-2000 Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_QUERY_BOX_H__
#define __PICMAN_QUERY_BOX_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/**
 * PicmanQueryStringCallback:
 * @query_box: The query box.
 * @string:    The entered string.
 * @data:      The user data.
 *
 * Note that you must not g_free() the passed string.
 **/
typedef void (* PicmanQueryStringCallback)  (GtkWidget   *query_box,
                                           const gchar *string,
                                           gpointer     data);

/**
 * PicmanQueryIntCallback:
 * @query_box: The query box.
 * @value:     The entered integer value.
 * @data:      The user data.
 *
 * The callback for an int query box.
 **/
typedef void (* PicmanQueryIntCallback)     (GtkWidget   *query_box,
                                           gint         value,
                                           gpointer     data);

/**
 * PicmanQueryDoubleCallback:
 * @query_box: The query box.
 * @value:     The entered double value.
 * @data:      The user data.
 *
 * The callback for a double query box.
 **/
typedef void (* PicmanQueryDoubleCallback)  (GtkWidget   *query_box,
                                           gdouble      value,
                                           gpointer     data);

/**
 * PicmanQuerySizeCallback:
 * @query_box: The query box.
 * @size:      The entered size in pixels.
 * @unit:      The selected unit from the #PicmanUnitMenu.
 * @data:      The user data.
 *
 * The callback for a size query box.
 **/
typedef void (* PicmanQuerySizeCallback)    (GtkWidget   *query_box,
                                           gdouble      size,
                                           PicmanUnit     unit,
                                           gpointer     data);

/**
 * PicmanQueryBooleanCallback:
 * @query_box: The query box.
 * @value:     The entered boolean value.
 * @data:      The user data.
 *
 * The callback for a boolean query box.
 **/
typedef void (* PicmanQueryBooleanCallback) (GtkWidget   *query_box,
                                           gboolean     value,
                                           gpointer     data);


/**
 * PICMAN_QUERY_BOX_VBOX:
 * @qbox: The query box.
 *
 * A macro to access the #GtkVBox in a #libpicmanwidgets-picmanquerybox.
 * Useful if you want to add more widgets.
 **/
#define PICMAN_QUERY_BOX_VBOX(qbox) g_object_get_data (G_OBJECT (qbox), \
                                                     "picman-query-box-vbox")


/*  some simple query dialogs  */
GtkWidget * picman_query_string_box  (const gchar              *title,
                                    GtkWidget                *parent,
                                    PicmanHelpFunc              help_func,
                                    const gchar              *help_id,
                                    const gchar              *message,
                                    const gchar              *initial,
                                    GObject                  *object,
                                    const gchar              *signal,
                                    PicmanQueryStringCallback   callback,
                                    gpointer                  data);

GtkWidget * picman_query_int_box     (const gchar              *title,
                                    GtkWidget                *parent,
                                    PicmanHelpFunc              help_func,
                                    const gchar              *help_id,
                                    const gchar              *message,
                                    gint                      initial,
                                    gint                      lower,
                                    gint                      upper,
                                    GObject                  *object,
                                    const gchar              *signal,
                                    PicmanQueryIntCallback      callback,
                                    gpointer                  data);

GtkWidget * picman_query_double_box  (const gchar              *title,
                                    GtkWidget                *parent,
                                    PicmanHelpFunc              help_func,
                                    const gchar              *help_id,
                                    const gchar              *message,
                                    gdouble                   initial,
                                    gdouble                   lower,
                                    gdouble                   upper,
                                    gint                      digits,
                                    GObject                  *object,
                                    const gchar              *signal,
                                    PicmanQueryDoubleCallback   callback,
                                    gpointer                  data);

GtkWidget * picman_query_size_box    (const gchar              *title,
                                    GtkWidget                *parent,
                                    PicmanHelpFunc              help_func,
                                    const gchar              *help_id,
                                    const gchar              *message,
                                    gdouble                   initial,
                                    gdouble                   lower,
                                    gdouble                   upper,
                                    gint                      digits,
                                    PicmanUnit                  unit,
                                    gdouble                   resolution,
                                    gboolean                  dot_for_dot,
                                    GObject                  *object,
                                    const gchar              *signal,
                                    PicmanQuerySizeCallback     callback,
                                    gpointer                  data);

GtkWidget * picman_query_boolean_box (const gchar              *title,
                                    GtkWidget                *parent,
                                    PicmanHelpFunc              help_func,
                                    const gchar              *help_id,
                                    const gchar              *stock_id,
                                    const gchar              *message,
                                    const gchar              *true_button,
                                    const gchar              *false_button,
                                    GObject                  *object,
                                    const gchar              *signal,
                                    PicmanQueryBooleanCallback  callback,
                                    gpointer                  data);


G_END_DECLS

#endif /* __PICMAN_QUERY_BOX_H__ */
