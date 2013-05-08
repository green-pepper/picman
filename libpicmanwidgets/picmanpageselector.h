/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpageselector.h
 * Copyright (C) 2005 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_PAGE_SELECTOR_H__
#define __PICMAN_PAGE_SELECTOR_H__

G_BEGIN_DECLS

#define PICMAN_TYPE_PAGE_SELECTOR            (picman_page_selector_get_type ())
#define PICMAN_PAGE_SELECTOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PAGE_SELECTOR, PicmanPageSelector))
#define PICMAN_PAGE_SELECTOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PAGE_SELECTOR, PicmanPageSelectorClass))
#define PICMAN_IS_PAGE_SELECTOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PAGE_SELECTOR))
#define PICMAN_IS_PAGE_SELECTOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PAGE_SELECTOR))
#define PICMAN_PAGE_SELECTOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PAGE_SELECTOR, PicmanPageSelectorClass))


typedef struct _PicmanPageSelectorClass  PicmanPageSelectorClass;

struct _PicmanPageSelector
{
  GtkBox    parent_instance;

  gpointer  priv;
};

struct _PicmanPageSelectorClass
{
  GtkBoxClass  parent_class;

  void (* selection_changed) (PicmanPageSelector *selector);
  void (* activate)          (PicmanPageSelector *selector);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_page_selector_get_type           (void) G_GNUC_CONST;

GtkWidget * picman_page_selector_new                (void);

void        picman_page_selector_set_n_pages        (PicmanPageSelector *selector,
                                                   gint              n_pages);
gint        picman_page_selector_get_n_pages        (PicmanPageSelector *selector);

void        picman_page_selector_set_target   (PicmanPageSelector       *selector,
                                             PicmanPageSelectorTarget  target);
PicmanPageSelectorTarget
            picman_page_selector_get_target   (PicmanPageSelector       *selector);

void        picman_page_selector_set_page_thumbnail (PicmanPageSelector *selector,
                                                   gint              page_no,
                                                   GdkPixbuf        *thumbnail);
GdkPixbuf * picman_page_selector_get_page_thumbnail (PicmanPageSelector *selector,
                                                   gint              page_no);

void        picman_page_selector_set_page_label     (PicmanPageSelector *selector,
                                                   gint              page_no,
                                                   const gchar      *label);
gchar     * picman_page_selector_get_page_label     (PicmanPageSelector *selector,
                                                   gint              page_no);

void        picman_page_selector_select_all         (PicmanPageSelector *selector);
void        picman_page_selector_unselect_all       (PicmanPageSelector *selector);
void        picman_page_selector_select_page        (PicmanPageSelector *selector,
                                                   gint              page_no);
void        picman_page_selector_unselect_page      (PicmanPageSelector *selector,
                                                   gint              page_no);
gboolean    picman_page_selector_page_is_selected   (PicmanPageSelector *selector,
                                                   gint              page_no);
gint      * picman_page_selector_get_selected_pages (PicmanPageSelector *selector,
                                                   gint             *n_selected_pages);

void        picman_page_selector_select_range       (PicmanPageSelector *selector,
                                                   const gchar      *range);
gchar     * picman_page_selector_get_selected_range (PicmanPageSelector *selector);

G_END_DECLS

#endif /* __PICMAN_PAGE_SELECTOR_H__ */
