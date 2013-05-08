/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextProxy
 * Copyright (C) 2009-2010  Michael Natterer <mitch@picman.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_TEXT_PROXY_H__
#define __PICMAN_TEXT_PROXY_H__


#define PICMAN_TYPE_TEXT_PROXY            (picman_text_proxy_get_type ())
#define PICMAN_TEXT_PROXY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEXT_PROXY, PicmanTextProxy))
#define PICMAN_IS_TEXT_PROXY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEXT_PROXY))
#define PICMAN_TEXT_PROXY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TEXT_PROXY, PicmanTextProxyClass))
#define PICMAN_IS_TEXT_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TEXT_PROXY))


typedef struct _PicmanTextProxy       PicmanTextProxy;
typedef struct _PicmanTextProxyClass  PicmanTextProxyClass;

struct _PicmanTextProxy
{
  GtkTextView  parent_instance;
};

struct _PicmanTextProxyClass
{
  GtkTextViewClass  parent_class;

  void (* change_size)     (PicmanTextProxy *proxy,
                            gdouble        amount);
  void (* change_baseline) (PicmanTextProxy *proxy,
                            gdouble        amount);
  void (* change_kerning)  (PicmanTextProxy *proxy,
                            gdouble        amount);
};


GType       picman_text_proxy_get_type (void) G_GNUC_CONST;

GtkWidget * picman_text_proxy_new      (void);


#endif /* __PICMAN_TEXT_PROXY_H__ */
