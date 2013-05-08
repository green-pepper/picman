/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolordisplay.c
 * Copyright (C) 1999 Manish Singh <yosh@picman.org>
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

#ifndef __PICMAN_COLOR_DISPLAY_H__
#define __PICMAN_COLOR_DISPLAY_H__

G_BEGIN_DECLS

/* For information look at the html documentation */


#define PICMAN_TYPE_COLOR_DISPLAY            (picman_color_display_get_type ())
#define PICMAN_COLOR_DISPLAY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_DISPLAY, PicmanColorDisplay))
#define PICMAN_COLOR_DISPLAY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_DISPLAY, PicmanColorDisplayClass))
#define PICMAN_IS_COLOR_DISPLAY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_DISPLAY))
#define PICMAN_IS_COLOR_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_DISPLAY))
#define PICMAN_COLOR_DISPLAY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_DISPLAY, PicmanColorDisplayClass))


typedef struct _PicmanColorDisplayClass PicmanColorDisplayClass;

struct _PicmanColorDisplay
{
  GObject  parent_instance;

  gboolean enabled;
};

struct _PicmanColorDisplayClass
{
  GObjectClass  parent_class;

  const gchar  *name;
  const gchar  *help_id;

  /*  virtual functions  */

  /*  implementing the PicmanColorDisplay::clone method is deprecated       */
  PicmanColorDisplay * (* clone)           (PicmanColorDisplay *display);

  /*  implementing the PicmanColorDisplay::convert method is deprecated     */
  void               (* convert)         (PicmanColorDisplay *display,
                                          guchar           *buf,
                                          gint              width,
                                          gint              height,
                                          gint              bpp,
                                          gint              bpl);

  /*  implementing the PicmanColorDisplay::load_state method is deprecated  */
  void               (* load_state)      (PicmanColorDisplay *display,
                                          PicmanParasite     *state);

  /*  implementing the PicmanColorDisplay::save_state method is deprecated  */
  PicmanParasite     * (* save_state)      (PicmanColorDisplay *display);

  GtkWidget        * (* configure)       (PicmanColorDisplay *display);

  /*  implementing the PicmanColorDisplay::configure_reset method is deprecated */
  void               (* configure_reset) (PicmanColorDisplay *display);

  /*  signals  */
  void               (* changed)         (PicmanColorDisplay *display);

  const gchar  *stock_id;

  void               (* convert_surface) (PicmanColorDisplay *display,
                                          cairo_surface_t  *surface);

  /* Padding for future expansion */
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType              picman_color_display_get_type    (void) G_GNUC_CONST;

PICMAN_DEPRECATED_FOR(g_object_new)
PicmanColorDisplay * picman_color_display_new         (GType             display_type);
PicmanColorDisplay * picman_color_display_clone       (PicmanColorDisplay *display);

void           picman_color_display_convert_surface (PicmanColorDisplay *display,
                                                   cairo_surface_t  *surface);
PICMAN_DEPRECATED_FOR(picman_color_display_convert_surface)
void           picman_color_display_convert         (PicmanColorDisplay *display,
                                                   guchar           *buf,
                                                   gint              width,
                                                   gint              height,
                                                   gint              bpp,
                                                   gint              bpl);
void           picman_color_display_load_state      (PicmanColorDisplay *display,
                                                   PicmanParasite     *state);
PicmanParasite * picman_color_display_save_state      (PicmanColorDisplay *display);
GtkWidget    * picman_color_display_configure       (PicmanColorDisplay *display);
void           picman_color_display_configure_reset (PicmanColorDisplay *display);

void           picman_color_display_changed         (PicmanColorDisplay *display);

void           picman_color_display_set_enabled     (PicmanColorDisplay *display,
                                                   gboolean          enabled);
gboolean       picman_color_display_get_enabled     (PicmanColorDisplay *display);

PicmanColorConfig  * picman_color_display_get_config  (PicmanColorDisplay *display);
PicmanColorManaged * picman_color_display_get_managed (PicmanColorDisplay *display);


G_END_DECLS

#endif /* __PICMAN_COLOR_DISPLAY_H__ */
