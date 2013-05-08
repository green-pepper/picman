/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#ifndef __PICMAN_WIRE_H__
#define __PICMAN_WIRE_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


typedef struct _PicmanWireMessage  PicmanWireMessage;

typedef void     (* PicmanWireReadFunc)    (GIOChannel      *channel,
                                          PicmanWireMessage *msg,
                                          gpointer         user_data);
typedef void     (* PicmanWireWriteFunc)   (GIOChannel      *channel,
                                          PicmanWireMessage *msg,
                                          gpointer         user_data);
typedef void     (* PicmanWireDestroyFunc) (PicmanWireMessage *msg);
typedef gboolean (* PicmanWireIOFunc)      (GIOChannel      *channel,
                                          const guint8    *buf,
                                          gulong           count,
                                          gpointer         user_data);
typedef gboolean (* PicmanWireFlushFunc)   (GIOChannel      *channel,
                                          gpointer         user_data);


struct _PicmanWireMessage
{
  guint32  type;
  gpointer data;
};


void      picman_wire_register      (guint32              type,
                                   PicmanWireReadFunc     read_func,
                                   PicmanWireWriteFunc    write_func,
                                   PicmanWireDestroyFunc  destroy_func);

void      picman_wire_set_reader    (PicmanWireIOFunc       read_func);
void      picman_wire_set_writer    (PicmanWireIOFunc       write_func);
void      picman_wire_set_flusher   (PicmanWireFlushFunc    flush_func);

gboolean  picman_wire_read          (GIOChannel           *channel,
                                   guint8          *buf,
                                   gsize            count,
                                   gpointer         user_data);
gboolean  picman_wire_write         (GIOChannel      *channel,
                                   const guint8    *buf,
                                   gsize            count,
                                   gpointer         user_data);
gboolean  picman_wire_flush         (GIOChannel      *channel,
                                   gpointer         user_data);

gboolean  picman_wire_error         (void);
void      picman_wire_clear_error   (void);

gboolean  picman_wire_read_msg      (GIOChannel          *channel,
                                   PicmanWireMessage *msg,
                                   gpointer         user_data);
gboolean  picman_wire_write_msg     (GIOChannel          *channel,
                                   PicmanWireMessage *msg,
                                   gpointer         user_data);

void      picman_wire_destroy       (PicmanWireMessage *msg);


/*  for internal use in libpicmanbase  */

G_GNUC_INTERNAL gboolean  _picman_wire_read_int32   (GIOChannel     *channel,
                                                   guint32        *data,
                                                   gint            count,
                                                   gpointer        user_data);
G_GNUC_INTERNAL gboolean  _picman_wire_read_int16   (GIOChannel     *channel,
                                                   guint16        *data,
                                                   gint            count,
                                                   gpointer        user_data);
G_GNUC_INTERNAL gboolean  _picman_wire_read_int8    (GIOChannel     *channel,
                                                   guint8         *data,
                                                   gint            count,
                                                   gpointer        user_data);
G_GNUC_INTERNAL gboolean  _picman_wire_read_double  (GIOChannel     *channel,
                                                   gdouble        *data,
                                                   gint            count,
                                                   gpointer        user_data);
G_GNUC_INTERNAL gboolean  _picman_wire_read_string  (GIOChannel     *channel,
                                                   gchar         **data,
                                                   gint            count,
                                                   gpointer        user_data);
G_GNUC_INTERNAL gboolean  _picman_wire_read_color   (GIOChannel     *channel,
                                                   PicmanRGB        *data,
                                                   gint            count,
                                                   gpointer        user_data);
G_GNUC_INTERNAL gboolean  _picman_wire_write_int32  (GIOChannel     *channel,
                                                   const guint32  *data,
                                                   gint            count,
                                                   gpointer        user_data);
G_GNUC_INTERNAL gboolean  _picman_wire_write_int16  (GIOChannel     *channel,
                                                   const guint16  *data,
                                                   gint            count,
                                                   gpointer        user_data);
G_GNUC_INTERNAL gboolean  _picman_wire_write_int8   (GIOChannel     *channel,
                                                   const guint8   *data,
                                                   gint            count,
                                                   gpointer        user_data);
G_GNUC_INTERNAL gboolean  _picman_wire_write_double (GIOChannel     *channel,
                                                   const gdouble  *data,
                                                   gint            count,
                                                   gpointer        user_data);
G_GNUC_INTERNAL gboolean  _picman_wire_write_string (GIOChannel     *channel,
                                                   gchar         **data,
                                                   gint            count,
                                                   gpointer        user_data);
G_GNUC_INTERNAL gboolean  _picman_wire_write_color  (GIOChannel     *channel,
                                                   const PicmanRGB  *data,
                                                   gint            count,
                                                   gpointer        user_data);


G_END_DECLS

#endif /* __PICMAN_WIRE_H__ */
