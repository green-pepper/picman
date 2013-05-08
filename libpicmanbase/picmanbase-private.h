/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanbase-private.h
 * Copyright (C) 2003 Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_BASE_PRIVATE_H__
#define __PICMAN_BASE_PRIVATE_H__


typedef struct _PicmanUnitVtable PicmanUnitVtable;

struct _PicmanUnitVtable
{
  gint          (* unit_get_number_of_units)          (void);
  gint          (* unit_get_number_of_built_in_units) (void);

  PicmanUnit      (* unit_new)                          (gchar    *identifier,
                                                       gdouble   factor,
                                                       gint      digits,
                                                       gchar    *symbol,
                                                       gchar    *abbreviation,
                                                       gchar    *singular,
                                                       gchar    *plural);
  gboolean      (* unit_get_deletion_flag)            (PicmanUnit  unit);
  void          (* unit_set_deletion_flag)            (PicmanUnit  unit,
                                                       gboolean  deletion_flag);

  gdouble       (* unit_get_factor)                   (PicmanUnit  unit);
  gint          (* unit_get_digits)                   (PicmanUnit  unit);
  const gchar * (* unit_get_identifier)               (PicmanUnit  unit);
  const gchar * (* unit_get_symbol)                   (PicmanUnit  unit);
  const gchar * (* unit_get_abbreviation)             (PicmanUnit  unit);
  const gchar * (* unit_get_singular)                 (PicmanUnit  unit);
  const gchar * (* unit_get_plural)                   (PicmanUnit  unit);

  void          (* _reserved_1)                       (void);
  void          (* _reserved_2)                       (void);
  void          (* _reserved_3)                       (void);
  void          (* _reserved_4)                       (void);
};


extern PicmanUnitVtable _picman_unit_vtable;


G_BEGIN_DECLS

void  picman_base_init (PicmanUnitVtable *vtable);

G_END_DECLS

#endif /* __PICMAN_BASE_PRIVATE_H__ */
