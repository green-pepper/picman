/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanunitstore.h
 * Copyright (C) 2004  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_UNIT_STORE_H__
#define __PICMAN_UNIT_STORE_H__

G_BEGIN_DECLS


enum
{
  PICMAN_UNIT_STORE_UNIT,
  PICMAN_UNIT_STORE_UNIT_FACTOR,
  PICMAN_UNIT_STORE_UNIT_DIGITS,
  PICMAN_UNIT_STORE_UNIT_IDENTIFIER,
  PICMAN_UNIT_STORE_UNIT_SYMBOL,
  PICMAN_UNIT_STORE_UNIT_ABBREVIATION,
  PICMAN_UNIT_STORE_UNIT_SINGULAR,
  PICMAN_UNIT_STORE_UNIT_PLURAL,
  PICMAN_UNIT_STORE_UNIT_SHORT_FORMAT,
  PICMAN_UNIT_STORE_UNIT_LONG_FORMAT,
  PICMAN_UNIT_STORE_UNIT_COLUMNS,
  PICMAN_UNIT_STORE_FIRST_VALUE = PICMAN_UNIT_STORE_UNIT_COLUMNS
};


#define PICMAN_TYPE_UNIT_STORE            (picman_unit_store_get_type ())
#define PICMAN_UNIT_STORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_UNIT_STORE, PicmanUnitStore))
#define PICMAN_UNIT_STORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_UNIT_STORE, PicmanUnitStoreClass))
#define PICMAN_IS_UNIT_STORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_UNIT_STORE))
#define PICMAN_IS_UNIT_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_UNIT_STORE))
#define PICMAN_UNIT_STORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_UNIT_STORE, PicmanUnitStoreClass))


typedef struct _PicmanUnitStoreClass  PicmanUnitStoreClass;

struct _PicmanUnitStore
{
  GObject  parent_instance;
};

struct _PicmanUnitStoreClass
{
  GObjectClass  parent_class;

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};


GType           picman_unit_store_get_type         (void) G_GNUC_CONST;

PicmanUnitStore * picman_unit_store_new              (gint           num_values);

void            picman_unit_store_set_has_pixels   (PicmanUnitStore *store,
                                                  gboolean       has_pixels);
gboolean        picman_unit_store_get_has_pixels   (PicmanUnitStore *store);

void            picman_unit_store_set_has_percent  (PicmanUnitStore *store,
                                                  gboolean       has_percent);
gboolean        picman_unit_store_get_has_percent  (PicmanUnitStore *store);

void            picman_unit_store_set_pixel_value  (PicmanUnitStore *store,
                                                  gint           index,
                                                  gdouble        value);
void            picman_unit_store_set_pixel_values (PicmanUnitStore *store,
                                                  gdouble        first_value,
                                                  ...);
void            picman_unit_store_set_resolution   (PicmanUnitStore *store,
                                                  gint           index,
                                                  gdouble        resolution);
void            picman_unit_store_set_resolutions  (PicmanUnitStore *store,
                                                  gdouble        first_resolution,
                                                  ...);
gdouble         picman_unit_store_get_value        (PicmanUnitStore *store,
                                                  PicmanUnit       unit,
                                                  gint           index);
void            picman_unit_store_get_values       (PicmanUnitStore *store,
                                                  PicmanUnit       unit,
                                                  gdouble       *first_value,
                                                  ...);


G_END_DECLS

#endif  /* __PICMAN_UNIT_STORE_H__ */
