/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanbrowser.h
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

#ifndef __PICMAN_BROWSER_H__
#define __PICMAN_BROWSER_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_BROWSER            (picman_browser_get_type ())
#define PICMAN_BROWSER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BROWSER, PicmanBrowser))
#define PICMAN_BROWSER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BROWSER, PicmanBrowserClass))
#define PICMAN_IS_BROWSER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BROWSER))
#define PICMAN_IS_BROWSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BROWSER))
#define PICMAN_BROWSER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BROWSER, PicmanBrowserClass))


typedef struct _PicmanBrowserClass PicmanBrowserClass;

struct _PicmanBrowser
{
  GtkHPaned  parent_instance;

  GtkWidget *left_vbox;

  GtkWidget *search_entry;
  guint      search_timeout_id;

  GtkWidget *search_type_combo;
  gint       search_type;

  GtkWidget *count_label;

  GtkWidget *right_vbox;
  GtkWidget *right_widget;
};

struct _PicmanBrowserClass
{
  GtkHPanedClass  parent_class;

  void (* search) (PicmanBrowser *browser,
                   const gchar *search_string,
                   gint         search_type);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_browser_get_type         (void) G_GNUC_CONST;

GtkWidget * picman_browser_new              (void);

void        picman_browser_add_search_types (PicmanBrowser *browser,
                                           const gchar *first_type_label,
                                           gint         first_type_id,
                                           ...) G_GNUC_NULL_TERMINATED;

void        picman_browser_set_widget       (PicmanBrowser *browser,
                                           GtkWidget   *widget);
void        picman_browser_show_message     (PicmanBrowser *browser,
                                           const gchar *message);


G_END_DECLS

#endif  /*  __PICMAN_BROWSER_H__  */
