/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_DND_H__
#define __PICMAN_DND_H__


#define PICMAN_TARGET_URI_LIST \
        { "text/uri-list", 0, PICMAN_DND_TYPE_URI_LIST }

#define PICMAN_TARGET_TEXT_PLAIN \
        { "text/plain", 0, PICMAN_DND_TYPE_TEXT_PLAIN }

#define PICMAN_TARGET_NETSCAPE_URL \
        { "_NETSCAPE_URL", 0, PICMAN_DND_TYPE_NETSCAPE_URL }

#define PICMAN_TARGET_XDS \
        { "XdndDirectSave0", 0, PICMAN_DND_TYPE_XDS }

#define PICMAN_TARGET_COLOR \
        { "application/x-color", 0, PICMAN_DND_TYPE_COLOR }

#define PICMAN_TARGET_SVG \
        { "image/svg", 0, PICMAN_DND_TYPE_SVG }

#define PICMAN_TARGET_SVG_XML \
        { "image/svg+xml", 0, PICMAN_DND_TYPE_SVG_XML }

/* just here for documentation purposes, the actual list of targets
 * is created dynamically from available GdkPixbuf loaders
 */
#define PICMAN_TARGET_PIXBUF \
        { NULL, 0, PICMAN_DND_TYPE_PIXBUF }

#define PICMAN_TARGET_IMAGE \
        { "application/x-picman-image-id", GTK_TARGET_SAME_APP, PICMAN_DND_TYPE_IMAGE }

#define PICMAN_TARGET_COMPONENT \
        { "application/x-picman-component", GTK_TARGET_SAME_APP, PICMAN_DND_TYPE_COMPONENT }

#define PICMAN_TARGET_LAYER \
        { "application/x-picman-layer-id", GTK_TARGET_SAME_APP, PICMAN_DND_TYPE_LAYER }

#define PICMAN_TARGET_CHANNEL \
        { "application/x-picman-channel-id", GTK_TARGET_SAME_APP, PICMAN_DND_TYPE_CHANNEL }

#define PICMAN_TARGET_LAYER_MASK \
        { "application/x-picman-layer-mask-id", GTK_TARGET_SAME_APP, PICMAN_DND_TYPE_LAYER_MASK }

#define PICMAN_TARGET_VECTORS \
        { "application/x-picman-vectors-id", GTK_TARGET_SAME_APP, PICMAN_DND_TYPE_VECTORS }

#define PICMAN_TARGET_BRUSH \
        { "application/x-picman-brush-name", 0, PICMAN_DND_TYPE_BRUSH }

#define PICMAN_TARGET_PATTERN \
        { "application/x-picman-pattern-name", 0, PICMAN_DND_TYPE_PATTERN }

#define PICMAN_TARGET_GRADIENT \
        { "application/x-picman-gradient-name", 0, PICMAN_DND_TYPE_GRADIENT }

#define PICMAN_TARGET_PALETTE \
        { "application/x-picman-palette-name", 0, PICMAN_DND_TYPE_PALETTE }

#define PICMAN_TARGET_FONT \
        { "application/x-picman-font-name", 0, PICMAN_DND_TYPE_FONT }

#define PICMAN_TARGET_BUFFER \
        { "application/x-picman-buffer-name", GTK_TARGET_SAME_APP, PICMAN_DND_TYPE_BUFFER }

#define PICMAN_TARGET_IMAGEFILE \
        { "application/x-picman-imagefile-name", GTK_TARGET_SAME_APP, PICMAN_DND_TYPE_IMAGEFILE }

#define PICMAN_TARGET_TEMPLATE \
        { "application/x-picman-template-name", GTK_TARGET_SAME_APP, PICMAN_DND_TYPE_TEMPLATE }

#define PICMAN_TARGET_TOOL_INFO \
        { "application/x-picman-tool-info-name", GTK_TARGET_SAME_APP, PICMAN_DND_TYPE_TOOL_INFO }

#define PICMAN_TARGET_DIALOG \
        { "application/x-picman-dialog", GTK_TARGET_SAME_APP, PICMAN_DND_TYPE_DIALOG }


/*  dnd initialization  */

void  picman_dnd_init (Picman *picman);


/*  uri list dnd functions  */

typedef GList * (* PicmanDndDragUriListFunc) (GtkWidget *widget,
                                            gpointer   data);
typedef void    (* PicmanDndDropUriListFunc) (GtkWidget *widget,
                                            gint       x,
                                            gint       y,
                                            GList     *uri_list,
                                            gpointer   data);

void  picman_dnd_uri_list_source_add    (GtkWidget              *widget,
                                       PicmanDndDragUriListFunc  get_uri_list_func,
                                       gpointer                data);
void  picman_dnd_uri_list_source_remove (GtkWidget              *widget);

void  picman_dnd_uri_list_dest_add      (GtkWidget              *widget,
                                       PicmanDndDropUriListFunc  set_uri_list_func,
                                       gpointer                data);
void  picman_dnd_uri_list_dest_remove   (GtkWidget              *widget);


/*  color dnd functions  */

typedef void (* PicmanDndDragColorFunc) (GtkWidget     *widget,
                                       PicmanRGB       *color,
                                       gpointer       data);
typedef void (* PicmanDndDropColorFunc) (GtkWidget     *widget,
                                       gint           x,
                                       gint           y,
                                       const PicmanRGB *color,
                                       gpointer       data);

void  picman_dnd_color_source_add    (GtkWidget            *widget,
                                    PicmanDndDragColorFunc  get_color_func,
                                    gpointer              data);
void  picman_dnd_color_source_remove (GtkWidget            *widget);

void  picman_dnd_color_dest_add      (GtkWidget            *widget,
                                    PicmanDndDropColorFunc  set_color_func,
                                    gpointer              data);
void  picman_dnd_color_dest_remove   (GtkWidget            *widget);


/*  stream dnd functions  */

typedef guchar * (* PicmanDndDragStreamFunc) (GtkWidget    *widget,
                                            gsize        *stream_len,
                                            gpointer      data);
typedef void     (* PicmanDndDropStreamFunc) (GtkWidget    *widget,
                                            gint          x,
                                            gint          y,
                                            const guchar *stream,
                                            gsize         stream_len,
                                            gpointer      data);

void  picman_dnd_svg_source_add    (GtkWidget              *widget,
                                  PicmanDndDragStreamFunc   get_svg_func,
                                  gpointer                data);
void  picman_dnd_svg_source_remove (GtkWidget              *widget);

void  picman_dnd_svg_dest_add      (GtkWidget              *widget,
                                  PicmanDndDropStreamFunc   set_svg_func,
                                  gpointer                data);
void  picman_dnd_svg_dest_remove   (GtkWidget              *widget);


/*  pixbuf dnd functions  */

typedef GdkPixbuf * (* PicmanDndDragPixbufFunc) (GtkWidget    *widget,
                                               gpointer      data);
typedef void        (* PicmanDndDropPixbufFunc) (GtkWidget    *widget,
                                               gint          x,
                                               gint          y,
                                               GdkPixbuf    *pixbuf,
                                               gpointer      data);

void  picman_dnd_pixbuf_source_add    (GtkWidget              *widget,
                                     PicmanDndDragPixbufFunc   get_pixbuf_func,
                                     gpointer                data);
void  picman_dnd_pixbuf_source_remove (GtkWidget              *widget);

void  picman_dnd_pixbuf_dest_add      (GtkWidget              *widget,
                                     PicmanDndDropPixbufFunc   set_pixbuf_func,
                                     gpointer                data);
void  picman_dnd_pixbuf_dest_remove   (GtkWidget              *widget);


/*  component dnd functions  */

typedef PicmanImage * (* PicmanDndDragComponentFunc) (GtkWidget       *widget,
                                                  PicmanContext    **context,
                                                  PicmanChannelType *channel,
                                                  gpointer         data);
typedef void        (* PicmanDndDropComponentFunc) (GtkWidget       *widget,
                                                  gint             x,
                                                  gint             y,
                                                  PicmanImage       *image,
                                                  PicmanChannelType  channel,
                                                  gpointer         data);

void  picman_dnd_component_source_add    (GtkWidget                 *widget,
                                        PicmanDndDragComponentFunc   get_comp_func,
                                        gpointer                   data);
void  picman_dnd_component_source_remove (GtkWidget                 *widget);

void  picman_dnd_component_dest_add      (GtkWidget                 *widget,
                                        PicmanDndDropComponentFunc   set_comp_func,
                                        gpointer                   data);
void  picman_dnd_component_dest_remove   (GtkWidget                 *widget);


/*  PicmanViewable (by GType) dnd functions  */

typedef PicmanViewable * (* PicmanDndDragViewableFunc) (GtkWidget     *widget,
                                                    PicmanContext  **context,
                                                    gpointer       data);
typedef void           (* PicmanDndDropViewableFunc) (GtkWidget     *widget,
                                                    gint           x,
                                                    gint           y,
                                                    PicmanViewable  *viewable,
                                                    gpointer       data);


gboolean picman_dnd_drag_source_set_by_type (GtkWidget               *widget,
                                           GdkModifierType          start_button_mask,
                                           GType                    type,
                                           GdkDragAction            actions);
gboolean picman_dnd_viewable_source_add     (GtkWidget               *widget,
                                           GType                    type,
                                           PicmanDndDragViewableFunc  get_viewable_func,
                                           gpointer                 data);
gboolean picman_dnd_viewable_source_remove  (GtkWidget               *widget,
                                           GType                    type);

gboolean picman_dnd_drag_dest_set_by_type   (GtkWidget               *widget,
                                           GtkDestDefaults          flags,
                                           GType                    type,
                                           GdkDragAction            actions);

gboolean picman_dnd_viewable_dest_add       (GtkWidget               *widget,
                                           GType                    type,
                                           PicmanDndDropViewableFunc  set_viewable_func,
                                           gpointer                 data);
gboolean picman_dnd_viewable_dest_remove    (GtkWidget               *widget,
                                           GType                    type);

PicmanViewable * picman_dnd_get_drag_data     (GtkWidget               *widget);


/*  Direct Save Protocol (XDS)  */

void  picman_dnd_xds_source_add    (GtkWidget               *widget,
                                  PicmanDndDragViewableFunc  get_image_func,
                                  gpointer                 data);
void  picman_dnd_xds_source_remove (GtkWidget               *widget);


#endif /* __PICMAN_DND_H__ */
