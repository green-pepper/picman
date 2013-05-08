/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandialog.h
 * Copyright (C) 2000-2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DIALOG_H__
#define __PICMAN_DIALOG_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_DIALOG            (picman_dialog_get_type ())
#define PICMAN_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DIALOG, PicmanDialog))
#define PICMAN_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DIALOG, PicmanDialogClass))
#define PICMAN_IS_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DIALOG))
#define PICMAN_IS_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DIALOG))
#define PICMAN_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DIALOG, PicmanDialogClass))


typedef struct _PicmanDialogClass  PicmanDialogClass;

struct _PicmanDialog
{
  GtkDialog  parent_instance;
};

struct _PicmanDialogClass
{
  GtkDialogClass  parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_dialog_get_type           (void) G_GNUC_CONST;

GtkWidget * picman_dialog_new                (const gchar    *title,
                                            const gchar    *role,
                                            GtkWidget      *parent,
                                            GtkDialogFlags  flags,
                                            PicmanHelpFunc    help_func,
                                            const gchar    *help_id,
                                            ...) G_GNUC_NULL_TERMINATED;

GtkWidget * picman_dialog_new_valist         (const gchar    *title,
                                            const gchar    *role,
                                            GtkWidget      *parent,
                                            GtkDialogFlags  flags,
                                            PicmanHelpFunc    help_func,
                                            const gchar    *help_id,
                                            va_list         args);

GtkWidget * picman_dialog_add_button         (PicmanDialog     *dialog,
                                            const gchar    *button_text,
                                            gint            response_id);
void        picman_dialog_add_buttons        (PicmanDialog     *dialog,
                                            ...) G_GNUC_NULL_TERMINATED;
void        picman_dialog_add_buttons_valist (PicmanDialog     *dialog,
                                            va_list         args);

gint        picman_dialog_run                (PicmanDialog     *dialog);

/*  for internal use only!  */
void        picman_dialogs_show_help_button  (gboolean        show);


G_END_DECLS

#endif /* __PICMAN_DIALOG_H__ */
