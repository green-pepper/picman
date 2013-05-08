/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#ifndef __PICMAN_UI_H__
#define __PICMAN_UI_H__

#include <gtk/gtk.h>

#include <libpicmanwidgets/picmanwidgets.h>

#define __PICMAN_UI_H_INSIDE__

#include <libpicman/picmanuitypes.h>

#include <libpicman/picmanexport.h>
#include <libpicman/picmanmenu.h>
#include <libpicman/picmanaspectpreview.h>
#include <libpicman/picmandrawablepreview.h>
#include <libpicman/picmanbrushmenu.h>
#include <libpicman/picmanfontmenu.h>
#include <libpicman/picmangradientmenu.h>
#include <libpicman/picmanpalettemenu.h>
#include <libpicman/picmanpatternmenu.h>
#include <libpicman/picmanprocbrowserdialog.h>
#include <libpicman/picmanprocview.h>
#include <libpicman/picmanprogressbar.h>
#include <libpicman/picmanitemcombobox.h>
#include <libpicman/picmanimagecombobox.h>
#include <libpicman/picmanselectbutton.h>
#include <libpicman/picmanbrushselectbutton.h>
#include <libpicman/picmanfontselectbutton.h>
#include <libpicman/picmangradientselectbutton.h>
#include <libpicman/picmanpaletteselectbutton.h>
#include <libpicman/picmanpatternselectbutton.h>
#include <libpicman/picmanzoompreview.h>

#undef __PICMAN_UI_H_INSIDE__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


void        picman_ui_init                          (const gchar *prog_name,
                                                   gboolean     preview);

GdkWindow * picman_ui_get_display_window            (guint32      gdisp_ID);
GdkWindow * picman_ui_get_progress_window           (void);

void        picman_window_set_transient_for_display (GtkWindow   *window,
                                                   guint32      gdisp_ID);
void        picman_window_set_transient             (GtkWindow   *window);

G_END_DECLS

#endif /* __PICMAN_UI_H__ */
