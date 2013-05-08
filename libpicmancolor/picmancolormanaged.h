/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * PicmanColorManaged interface
 * Copyright (C) 2007  Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_COLOR_H_INSIDE__) && !defined (PICMAN_COLOR_COMPILATION)
#error "Only <libpicmancolor/picmancolor.h> can be included directly."
#endif

#ifndef __PICMAN_COLOR_MANAGED_H__
#define __PICMAN_COLOR_MANAGED_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_COLOR_MANAGED               (picman_color_managed_interface_get_type ())
#define PICMAN_IS_COLOR_MANAGED(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_MANAGED))
#define PICMAN_COLOR_MANAGED(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_MANAGED, PicmanColorManaged))
#define PICMAN_COLOR_MANAGED_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_COLOR_MANAGED, PicmanColorManagedInterface))


typedef struct _PicmanColorManagedInterface PicmanColorManagedInterface;

struct _PicmanColorManagedInterface
{
  GTypeInterface  base_iface;

  /*  virtual functions  */
  const guint8 * (* get_icc_profile) (PicmanColorManaged *managed,
                                      gsize            *len);

  /*  signals  */
  void           (* profile_changed) (PicmanColorManaged *managed);
};


GType          picman_color_managed_interface_get_type (void) G_GNUC_CONST;

const guint8 * picman_color_managed_get_icc_profile    (PicmanColorManaged *managed,
                                                      gsize            *len);
void           picman_color_managed_profile_changed    (PicmanColorManaged *managed);


G_END_DECLS

#endif  /* __PICMAN_COLOR_MANAGED_IFACE_H__ */
