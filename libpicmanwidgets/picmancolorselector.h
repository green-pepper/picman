/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorselector.h
 * Copyright (C) 2002 Michael Natterer <mitch@picman.org>
 *
 * based on:
 * Colour selector module
 * Copyright (C) 1999 Austin Donnelly <austin@greenend.org.uk>
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

#ifndef __PICMAN_COLOR_SELECTOR_H__
#define __PICMAN_COLOR_SELECTOR_H__

G_BEGIN_DECLS

/* For information look at the html documentation */


/**
 * PICMAN_COLOR_SELECTOR_SIZE:
 *
 * The suggested size for a color area in a #PicmanColorSelector
 * implementation.
 **/
#define PICMAN_COLOR_SELECTOR_SIZE     150

/**
 * PICMAN_COLOR_SELECTOR_BAR_SIZE:
 *
 * The suggested width for a color bar in a #PicmanColorSelector
 * implementation.
 **/
#define PICMAN_COLOR_SELECTOR_BAR_SIZE 15


#define PICMAN_TYPE_COLOR_SELECTOR            (picman_color_selector_get_type ())
#define PICMAN_COLOR_SELECTOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_SELECTOR, PicmanColorSelector))
#define PICMAN_COLOR_SELECTOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_SELECTOR, PicmanColorSelectorClass))
#define PICMAN_IS_COLOR_SELECTOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_SELECTOR))
#define PICMAN_IS_COLOR_SELECTOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_SELECTOR))
#define PICMAN_COLOR_SELECTOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_SELECTOR, PicmanColorSelectorClass))


typedef struct _PicmanColorSelectorClass PicmanColorSelectorClass;

struct _PicmanColorSelector
{
  GtkBox                    parent_instance;

  gboolean                  toggles_visible;
  gboolean                  toggles_sensitive;
  gboolean                  show_alpha;

  PicmanRGB                   rgb;
  PicmanHSV                   hsv;

  PicmanColorSelectorChannel  channel;
};

struct _PicmanColorSelectorClass
{
  GtkBoxClass  parent_class;

  const gchar *name;
  const gchar *help_id;
  const gchar *stock_id;

  /*  virtual functions  */
  void (* set_toggles_visible)   (PicmanColorSelector        *selector,
                                  gboolean                  visible);
  void (* set_toggles_sensitive) (PicmanColorSelector        *selector,
                                  gboolean                  sensitive);
  void (* set_show_alpha)        (PicmanColorSelector        *selector,
                                  gboolean                  show_alpha);
  void (* set_color)             (PicmanColorSelector        *selector,
                                  const PicmanRGB            *rgb,
                                  const PicmanHSV            *hsv);
  void (* set_channel)           (PicmanColorSelector        *selector,
                                  PicmanColorSelectorChannel  channel);

  /*  signals  */
  void (* color_changed)         (PicmanColorSelector        *selector,
                                  const PicmanRGB            *rgb,
                                  const PicmanHSV            *hsv);
  void (* channel_changed)       (PicmanColorSelector        *selector,
                                  PicmanColorSelectorChannel  channel);

  /*  another virtual function  */
  void (* set_config)            (PicmanColorSelector        *selector,
                                  PicmanColorConfig          *config);

  /* Padding for future expansion */
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_color_selector_get_type         (void) G_GNUC_CONST;
GtkWidget * picman_color_selector_new              (GType              selector_type,
                                                  const PicmanRGB     *rgb,
                                                  const PicmanHSV     *hsv,
                                                  PicmanColorSelectorChannel  channel);

void   picman_color_selector_set_toggles_visible   (PicmanColorSelector *selector,
                                                  gboolean           visible);
void   picman_color_selector_set_toggles_sensitive (PicmanColorSelector *selector,
                                                  gboolean           sensitive);
void   picman_color_selector_set_show_alpha        (PicmanColorSelector *selector,
                                                  gboolean           show_alpha);
void   picman_color_selector_set_color             (PicmanColorSelector *selector,
                                                  const PicmanRGB     *rgb,
                                                  const PicmanHSV     *hsv);
void   picman_color_selector_set_channel           (PicmanColorSelector *selector,
                                                  PicmanColorSelectorChannel  channel);

void   picman_color_selector_color_changed         (PicmanColorSelector *selector);
void   picman_color_selector_channel_changed       (PicmanColorSelector *selector);

void   picman_color_selector_set_config            (PicmanColorSelector *selector,
                                                  PicmanColorConfig   *config);


G_END_DECLS

#endif /* __PICMAN_COLOR_SELECTOR_H__ */
