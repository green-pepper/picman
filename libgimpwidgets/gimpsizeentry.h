/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmansizeentry.h
 * Copyright (C) 1999-2000 Sven Neumann <sven@picman.org>
 *                         Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_SIZE_ENTRY_H__
#define __PICMAN_SIZE_ENTRY_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_SIZE_ENTRY            (picman_size_entry_get_type ())
#define PICMAN_SIZE_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SIZE_ENTRY, PicmanSizeEntry))
#define PICMAN_SIZE_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SIZE_ENTRY, PicmanSizeEntryClass))
#define PICMAN_IS_SIZE_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, PICMAN_TYPE_SIZE_ENTRY))
#define PICMAN_IS_SIZE_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SIZE_ENTRY))
#define PICMAN_SIZE_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SIZE_ENTRY, PicmanSizeEntryClass))


typedef struct _PicmanSizeEntryClass  PicmanSizeEntryClass;

typedef struct _PicmanSizeEntryField  PicmanSizeEntryField;

struct _PicmanSizeEntry
{
  GtkTable   parent_instance;

  GSList    *fields;
  gint       number_of_fields;

  GtkWidget *unitmenu;
  PicmanUnit   unit;
  gboolean   menu_show_pixels;
  gboolean   menu_show_percent;

  gboolean                   show_refval;
  PicmanSizeEntryUpdatePolicy  update_policy;
};

struct _PicmanSizeEntryClass
{
  GtkTableClass  parent_class;

  void (* value_changed)  (PicmanSizeEntry *gse);
  void (* refval_changed) (PicmanSizeEntry *gse);
  void (* unit_changed)   (PicmanSizeEntry *gse);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


/* For information look into the C source or the html documentation */

GType       picman_size_entry_get_type (void) G_GNUC_CONST;

GtkWidget * picman_size_entry_new (gint                       number_of_fields,
                                 PicmanUnit                   unit,
                                 const gchar               *unit_format,
                                 gboolean                   menu_show_pixels,
                                 gboolean                   menu_show_percent,
                                 gboolean                   show_refval,
                                 gint                       spinbutton_width,
                                 PicmanSizeEntryUpdatePolicy  update_policy);

void        picman_size_entry_add_field  (PicmanSizeEntry   *gse,
                                        GtkSpinButton   *value_spinbutton,
                                        GtkSpinButton   *refval_spinbutton);

GtkWidget * picman_size_entry_attach_label          (PicmanSizeEntry *gse,
                                                   const gchar   *text,
                                                   gint           row,
                                                   gint           column,
                                                   gfloat         alignment);

void        picman_size_entry_set_resolution        (PicmanSizeEntry *gse,
                                                   gint           field,
                                                   gdouble        resolution,
                                                   gboolean       keep_size);

void        picman_size_entry_set_size              (PicmanSizeEntry *gse,
                                                   gint           field,
                                                   gdouble        lower,
                                                   gdouble        upper);

void        picman_size_entry_set_value_boundaries  (PicmanSizeEntry *gse,
                                                   gint           field,
                                                   gdouble        lower,
                                                   gdouble        upper);

gdouble     picman_size_entry_get_value             (PicmanSizeEntry *gse,
                                                   gint           field);
void        picman_size_entry_set_value             (PicmanSizeEntry *gse,
                                                   gint           field,
                                                   gdouble        value);

void        picman_size_entry_set_refval_boundaries (PicmanSizeEntry *gse,
                                                   gint           field,
                                                   gdouble        lower,
                                                   gdouble        upper);
void        picman_size_entry_set_refval_digits     (PicmanSizeEntry *gse,
                                                   gint           field,
                                                   gint           digits);

gdouble     picman_size_entry_get_refval            (PicmanSizeEntry *gse,
                                                   gint           field);
void        picman_size_entry_set_refval            (PicmanSizeEntry *gse,
                                                   gint           field,
                                                   gdouble        refval);

PicmanUnit    picman_size_entry_get_unit              (PicmanSizeEntry *gse);
void        picman_size_entry_set_unit              (PicmanSizeEntry *gse,
                                                   PicmanUnit       unit);
void        picman_size_entry_show_unit_menu        (PicmanSizeEntry *gse,
                                                   gboolean       show);

void        picman_size_entry_set_pixel_digits      (PicmanSizeEntry *gse,
                                                   gint           digits);

void        picman_size_entry_grab_focus            (PicmanSizeEntry *gse);
void        picman_size_entry_set_activates_default (PicmanSizeEntry *gse,
                                                   gboolean       setting);
GtkWidget * picman_size_entry_get_help_widget       (PicmanSizeEntry *gse,
                                                   gint           field);


G_END_DECLS

#endif /* __PICMAN_SIZE_ENTRY_H__ */
