/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmangradientselect.h
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

#if !defined (__PICMAN_H_INSIDE__) && !defined (PICMAN_COMPILATION)
#error "Only <libpicman/picman.h> can be included directly."
#endif

#ifndef __PICMAN_GRAIDENT_SELECT_H__
#define __PICMAN_GRADIENT_SELECT_H__

G_BEGIN_DECLS


typedef void (* PicmanRunGradientCallback)   (const gchar   *gradient_name,
                                            gint           width,
                                            const gdouble *grad_data,
                                            gboolean       dialog_closing,
                                            gpointer       user_data);


const gchar * picman_gradient_select_new     (const gchar             *title,
                                            const gchar             *gradient_name,
                                            gint                     sample_size,
                                            PicmanRunGradientCallback  callback,
                                            gpointer                 data);
void          picman_gradient_select_destroy (const gchar             *gradient_callback);


G_END_DECLS

#endif /* __PICMAN_GRADIENT_SELECT_H__ */
