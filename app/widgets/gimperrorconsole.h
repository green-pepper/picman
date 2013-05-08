/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanerrorconsole.h
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_ERROR_CONSOLE_H__
#define __PICMAN_ERROR_CONSOLE_H__


#include "picmaneditor.h"


#define PICMAN_TYPE_ERROR_CONSOLE            (picman_error_console_get_type ())
#define PICMAN_ERROR_CONSOLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ERROR_CONSOLE, PicmanErrorConsole))
#define PICMAN_ERROR_CONSOLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ERROR_CONSOLE, PicmanErrorConsoleClass))
#define PICMAN_IS_ERROR_CONSOLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ERROR_CONSOLE))
#define PICMAN_IS_ERROR_CONSOLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ERROR_CONSOLE))
#define PICMAN_ERROR_CONSOLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ERROR_CONSOLE, PicmanErrorConsoleClass))


typedef struct _PicmanErrorConsoleClass PicmanErrorConsoleClass;

struct _PicmanErrorConsole
{
  PicmanEditor     parent_instance;

  Picman          *picman;

  GtkTextBuffer *text_buffer;
  GtkWidget     *text_view;

  GtkWidget     *clear_button;
  GtkWidget     *save_button;

  GtkWidget     *file_dialog;
  gboolean       save_selection;
};

struct _PicmanErrorConsoleClass
{
  PicmanEditorClass  parent_class;
};


GType       picman_error_console_get_type (void) G_GNUC_CONST;

GtkWidget * picman_error_console_new      (Picman                *picman,
                                         PicmanMenuFactory     *menu_factory);

void        picman_error_console_add      (PicmanErrorConsole    *console,
                                         PicmanMessageSeverity  severity,
                                         const gchar         *domain,
                                         const gchar         *message);


#endif  /*  __PICMAN_ERROR_CONSOLE_H__  */
