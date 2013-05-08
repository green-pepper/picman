/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanenumlabel.h
 * Copyright (C) 2005  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_ENUM__LABEL_H__
#define __PICMAN_ENUM__LABEL_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_ENUM_LABEL            (picman_enum_label_get_type ())
#define PICMAN_ENUM_LABEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ENUM_LABEL, PicmanEnumLabel))
#define PICMAN_ENUM_LABEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ENUM_LABEL, PicmanEnumLabelClass))
#define PICMAN_IS_ENUM_LABEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ENUM_LABEL))
#define PICMAN_IS_ENUM_LABEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ENUM_LABEL))
#define PICMAN_ENUM_LABEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ENUM_LABEL, PicmanEnumLabelClass))


typedef struct _PicmanEnumLabelClass  PicmanEnumLabelClass;

struct _PicmanEnumLabel
{
  GtkLabel       parent_instance;

  /*< private >*/
  GEnumClass    *enum_class;
};

struct _PicmanEnumLabelClass
{
  GtkLabelClass  parent_class;
};


GType       picman_enum_label_get_type         (void) G_GNUC_CONST;

GtkWidget * picman_enum_label_new              (GType          enum_type,
                                              gint           value);
void        picman_enum_label_set_value        (PicmanEnumLabel *label,
                                              gint           value);


G_END_DECLS

#endif  /* __PICMAN_ENUM_LABEL_H__ */
