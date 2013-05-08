/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanratioentry.h
 * Copyright (C) 2006  Simon Budig       <simon@picman.org>
 * Copyright (C) 2007  Sven Neumann      <sven@picman.org>
 * Copyright (C) 2007  Martin Nordholts  <martin@svn.gnome.org>
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

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_NUMBER_PAIR_ENTRY_H__
#define __PICMAN_NUMBER_PAIR_ENTRY_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_NUMBER_PAIR_ENTRY            (picman_number_pair_entry_get_type ())
#define PICMAN_NUMBER_PAIR_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_NUMBER_PAIR_ENTRY, PicmanNumberPairEntry))
#define PICMAN_NUMBER_PAIR_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_NUMBER_PAIR_ENTRY, PicmanNumberPairEntryClass))
#define PICMAN_IS_NUMBER_PAIR_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_NUMBER_PAIR_ENTRY))
#define PICMAN_IS_NUMBER_PAIR_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_NUMBER_PAIR_ENTRY))
#define PICMAN_NUMBER_PAIR_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_NUMBER_PAIR_AREA, PicmanNumberPairEntryClass))


typedef struct _PicmanNumberPairEntryClass   PicmanNumberPairEntryClass;


struct _PicmanNumberPairEntry
{
  GtkEntry   parent_instance;

  gpointer   priv;
};

struct _PicmanNumberPairEntryClass
{
  GtkEntryClass  parent_class;

  void (* numbers_changed) (PicmanNumberPairEntry *entry);
  void (* ratio_changed)   (PicmanNumberPairEntry *entry);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType          picman_number_pair_entry_get_type           (void) G_GNUC_CONST;
GtkWidget *    picman_number_pair_entry_new                (const gchar         *separators,
                                                          gboolean             allow_simplification,
                                                          gdouble              min_valid_value,
                                                          gdouble              max_valid_value);
void           picman_number_pair_entry_set_default_values (PicmanNumberPairEntry *entry,
                                                          gdouble              left,
                                                          gdouble              right);
void           picman_number_pair_entry_get_default_values (PicmanNumberPairEntry *entry,
                                                          gdouble             *left,
                                                          gdouble             *right);
void           picman_number_pair_entry_set_values         (PicmanNumberPairEntry *entry,
                                                          gdouble              left,
                                                          gdouble              right);
void           picman_number_pair_entry_get_values         (PicmanNumberPairEntry *entry,
                                                          gdouble             *left,
                                                          gdouble             *right);

void           picman_number_pair_entry_set_default_text   (PicmanNumberPairEntry *entry,
                                                          const gchar         *string);
const gchar *  picman_number_pair_entry_get_default_text   (PicmanNumberPairEntry *entry);

void           picman_number_pair_entry_set_ratio          (PicmanNumberPairEntry *entry,
                                                          gdouble              ratio);
gdouble        picman_number_pair_entry_get_ratio          (PicmanNumberPairEntry *entry);

void           picman_number_pair_entry_set_aspect         (PicmanNumberPairEntry *entry,
                                                          PicmanAspectType       aspect);
PicmanAspectType picman_number_pair_entry_get_aspect         (PicmanNumberPairEntry *entry);

void           picman_number_pair_entry_set_user_override  (PicmanNumberPairEntry *entry,
                                                          gboolean             user_override);
gboolean       picman_number_pair_entry_get_user_override  (PicmanNumberPairEntry *entry);


G_END_DECLS

#endif /* __PICMAN_NUMBER_PAIR_ENTRY_H__ */
