/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmandocked.h
 * Copyright (C) 2003  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DOCKED_H__
#define __PICMAN_DOCKED_H__


#define PICMAN_TYPE_DOCKED               (picman_docked_interface_get_type ())
#define PICMAN_IS_DOCKED(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DOCKED))
#define PICMAN_DOCKED(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DOCKED, PicmanDocked))
#define PICMAN_DOCKED_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_DOCKED, PicmanDockedInterface))


typedef struct _PicmanDockedInterface PicmanDockedInterface;

/**
 * PicmanDockedInterface:
 *
 * Interface with common methods for stuff that is docked.
 */
struct _PicmanDockedInterface
{
  GTypeInterface base_iface;

  /*  signals  */
  void            (* title_changed)       (PicmanDocked   *docked);

  /*  virtual functions  */
  void            (* set_aux_info)        (PicmanDocked   *docked,
                                           GList        *aux_info);
  GList         * (* get_aux_info)        (PicmanDocked   *docked);

  GtkWidget     * (* get_preview)         (PicmanDocked   *docked,
                                           PicmanContext  *context,
                                           GtkIconSize   size);
  gboolean        (* get_prefer_icon)     (PicmanDocked   *docked);
  PicmanUIManager * (* get_menu)            (PicmanDocked   *docked,
                                           const gchar **ui_path,
                                           gpointer     *popup_data);
  gchar         * (* get_title)           (PicmanDocked   *docked);

  void            (* set_context)         (PicmanDocked   *docked,
                                           PicmanContext  *context);

  gboolean        (* has_button_bar)      (PicmanDocked   *docked);
  void            (* set_show_button_bar) (PicmanDocked   *docked,
                                           gboolean      show);
  gboolean        (* get_show_button_bar) (PicmanDocked   *docked);
};


GType           picman_docked_interface_get_type  (void) G_GNUC_CONST;

void            picman_docked_title_changed       (PicmanDocked   *docked);

void            picman_docked_set_aux_info        (PicmanDocked   *docked,
                                                 GList        *aux_info);
GList         * picman_docked_get_aux_info        (PicmanDocked   *docked);

GtkWidget     * picman_docked_get_preview         (PicmanDocked   *docked,
                                                 PicmanContext  *context,
                                                 GtkIconSize   size);
gboolean        picman_docked_get_prefer_icon     (PicmanDocked   *docked);
PicmanUIManager * picman_docked_get_menu            (PicmanDocked   *docked,
                                                 const gchar **ui_path,
                                                 gpointer     *popup_data);
gchar         * picman_docked_get_title           (PicmanDocked   *docked);

void            picman_docked_set_context         (PicmanDocked   *docked,
                                                 PicmanContext  *context);

gboolean        picman_docked_has_button_bar      (PicmanDocked   *docked);
void            picman_docked_set_show_button_bar (PicmanDocked   *docked,
                                                 gboolean      show);
gboolean        picman_docked_get_show_button_bar (PicmanDocked   *docked);


#endif  /* __PICMAN_DOCKED_H__ */
