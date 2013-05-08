/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextTool
 * Copyright (C) 2002-2010  Sven Neumann <sven@picman.org>
 *                          Daniel Eddeland <danedde@svn.gnome.org>
 *                          Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_TEXT_TOOL_H__
#define __PICMAN_TEXT_TOOL_H__


#include "picmandrawtool.h"


#define PICMAN_TYPE_TEXT_TOOL            (picman_text_tool_get_type ())
#define PICMAN_TEXT_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEXT_TOOL, PicmanTextTool))
#define PICMAN_IS_TEXT_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEXT_TOOL))
#define PICMAN_TEXT_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TEXT_TOOL, PicmanTextToolClass))
#define PICMAN_IS_TEXT_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TEXT_TOOL))

#define PICMAN_TEXT_TOOL_GET_OPTIONS(t)  (PICMAN_TEXT_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanTextTool       PicmanTextTool;
typedef struct _PicmanTextToolClass  PicmanTextToolClass;

struct _PicmanTextTool
{
  PicmanDrawTool    parent_instance;

  PicmanText       *proxy;
  GList          *pending;
  guint           idle_id;

  gboolean        moving;

  PicmanTextBuffer *buffer;

  PicmanText       *text;
  PicmanTextLayer  *layer;
  PicmanImage      *image;

  GtkWidget      *confirm_dialog;
  PicmanUIManager  *ui_manager;

  gboolean        handle_rectangle_change_complete;
  gboolean        text_box_fixed;

  PicmanTextLayout *layout;
  gboolean        drawing_blocked;

  /* text editor state: */

  GtkWidget      *style_overlay;
  GtkWidget      *style_editor;

  gboolean        selecting;
  GtkTextIter     select_start_iter;
  gboolean        select_words;
  gboolean        select_lines;

  GtkIMContext   *im_context;
  gboolean        needs_im_reset;

  GtkWidget      *preedit_overlay;
  GtkWidget      *preedit_label;

  gchar          *preedit_string;
  gint            preedit_cursor;

  gboolean        overwrite_mode;
  gint            x_pos;

  GtkWidget      *offscreen_window;
  GtkWidget      *proxy_text_view;

  GtkWidget      *editor_dialog;
};

struct _PicmanTextToolClass
{
  PicmanDrawToolClass  parent_class;
};


void       picman_text_tool_register               (PicmanToolRegisterCallback  callback,
                                                  gpointer                  data);

GType      picman_text_tool_get_type               (void) G_GNUC_CONST;

void       picman_text_tool_set_layer              (PicmanTextTool *text_tool,
                                                  PicmanLayer    *layer);

gboolean   picman_text_tool_get_has_text_selection (PicmanTextTool *text_tool);

void       picman_text_tool_delete_selection       (PicmanTextTool *text_tool);
void       picman_text_tool_cut_clipboard          (PicmanTextTool *text_tool);
void       picman_text_tool_copy_clipboard         (PicmanTextTool *text_tool);
void       picman_text_tool_paste_clipboard        (PicmanTextTool *text_tool);

void       picman_text_tool_create_vectors         (PicmanTextTool *text_tool);
void       picman_text_tool_create_vectors_warped  (PicmanTextTool *text_tool);

/*  only for the text editor  */
void       picman_text_tool_clear_layout           (PicmanTextTool *text_tool);
gboolean   picman_text_tool_ensure_layout          (PicmanTextTool *text_tool);


#endif /* __PICMAN_TEXT_TOOL_H__ */
