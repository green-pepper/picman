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

#ifndef __PICMAN_SELECTION_DATA_H__
#define __PICMAN_SELECTION_DATA_H__


/*  uri list  */

void            picman_selection_data_set_uri_list  (GtkSelectionData *selection,
                                                   GList            *uris);
GList         * picman_selection_data_get_uri_list  (GtkSelectionData *selection);


/*  color  */

void            picman_selection_data_set_color     (GtkSelectionData *selection,
                                                   const PicmanRGB    *color);
gboolean        picman_selection_data_get_color     (GtkSelectionData *selection,
                                                   PicmanRGB          *color);


/*  stream (svg/png)  */

void            picman_selection_data_set_stream    (GtkSelectionData *selection,
                                                   const guchar     *stream,
                                                   gsize             stream_length);
const guchar  * picman_selection_data_get_stream    (GtkSelectionData *selection,
                                                   gsize            *stream_length);


/*  curve  */

void            picman_selection_data_set_curve     (GtkSelectionData *selection,
                                                   PicmanCurve        *curve);
PicmanCurve     * picman_selection_data_get_curve     (GtkSelectionData *selection);


/*  image  */

void            picman_selection_data_set_image     (GtkSelectionData *selection,
                                                   PicmanImage        *image);
PicmanImage     * picman_selection_data_get_image     (GtkSelectionData *selection,
                                                   Picman             *picman);


/*  component  */

void            picman_selection_data_set_component (GtkSelectionData *selection,
                                                   PicmanImage        *image,
                                                   PicmanChannelType   channel);
PicmanImage     * picman_selection_data_get_component (GtkSelectionData *selection,
                                                   Picman             *picman,
                                                   PicmanChannelType  *channel);


/*  item  */

void            picman_selection_data_set_item      (GtkSelectionData *selection,
                                                   PicmanItem         *item);
PicmanItem      * picman_selection_data_get_item      (GtkSelectionData *selection,
                                                   Picman             *picman);


/*  various data  */

void            picman_selection_data_set_object    (GtkSelectionData *selection,
                                                   PicmanObject       *object);

PicmanBrush     * picman_selection_data_get_brush     (GtkSelectionData *selection,
                                                   Picman             *picman);
PicmanPattern   * picman_selection_data_get_pattern   (GtkSelectionData *selection,
                                                   Picman             *picman);
PicmanGradient  * picman_selection_data_get_gradient  (GtkSelectionData *selection,
                                                   Picman             *picman);
PicmanPalette   * picman_selection_data_get_palette   (GtkSelectionData *selection,
                                                   Picman             *picman);
PicmanFont      * picman_selection_data_get_font      (GtkSelectionData *selection,
                                                   Picman             *picman);
PicmanBuffer    * picman_selection_data_get_buffer    (GtkSelectionData *selection,
                                                   Picman             *picman);
PicmanImagefile * picman_selection_data_get_imagefile (GtkSelectionData *selection,
                                                   Picman             *picman);
PicmanTemplate  * picman_selection_data_get_template  (GtkSelectionData *selection,
                                                   Picman             *picman);
PicmanToolInfo  * picman_selection_data_get_tool_info (GtkSelectionData *selection,
                                                   Picman             *picman);


#endif /* __PICMAN_SELECTION_DATA_H__ */
