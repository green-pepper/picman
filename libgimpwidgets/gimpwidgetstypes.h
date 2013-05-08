/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanwidgetstypes.h
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

#ifndef __PICMAN_WIDGETS_TYPES_H__
#define __PICMAN_WIDGETS_TYPES_H__

#include <libpicmanconfig/picmanconfigtypes.h>

#include <libpicmanwidgets/picmanwidgetsenums.h>

G_BEGIN_DECLS

/* For information look into the html documentation */


typedef struct _PicmanBrowser               PicmanBrowser;
typedef struct _PicmanButton                PicmanButton;
typedef struct _PicmanCellRendererColor     PicmanCellRendererColor;
typedef struct _PicmanCellRendererToggle    PicmanCellRendererToggle;
typedef struct _PicmanChainButton           PicmanChainButton;
typedef struct _PicmanColorArea             PicmanColorArea;
typedef struct _PicmanColorButton           PicmanColorButton;
typedef struct _PicmanColorDisplay          PicmanColorDisplay;
typedef struct _PicmanColorDisplayStack     PicmanColorDisplayStack;
typedef struct _PicmanColorHexEntry         PicmanColorHexEntry;
typedef struct _PicmanColorNotebook         PicmanColorNotebook;
typedef struct _PicmanColorProfileComboBox  PicmanColorProfileComboBox;
typedef struct _PicmanColorProfileStore     PicmanColorProfileStore;
typedef struct _PicmanColorScale            PicmanColorScale;
typedef struct _PicmanColorScales           PicmanColorScales;
typedef struct _PicmanColorSelector         PicmanColorSelector;
typedef struct _PicmanColorSelect           PicmanColorSelect;
typedef struct _PicmanColorSelection        PicmanColorSelection;
typedef struct _PicmanController            PicmanController;
typedef struct _PicmanDialog                PicmanDialog;
typedef struct _PicmanEnumStore             PicmanEnumStore;
typedef struct _PicmanEnumComboBox          PicmanEnumComboBox;
typedef struct _PicmanEnumLabel             PicmanEnumLabel;
typedef struct _PicmanFileEntry             PicmanFileEntry;
typedef struct _PicmanFrame                 PicmanFrame;
typedef struct _PicmanIntComboBox           PicmanIntComboBox;
typedef struct _PicmanIntStore              PicmanIntStore;
typedef struct _PicmanMemsizeEntry          PicmanMemsizeEntry;
typedef struct _PicmanNumberPairEntry       PicmanNumberPairEntry;
typedef struct _PicmanOffsetArea            PicmanOffsetArea;
typedef struct _PicmanPageSelector          PicmanPageSelector;
typedef struct _PicmanPathEditor            PicmanPathEditor;
typedef struct _PicmanPickButton            PicmanPickButton;
typedef struct _PicmanPreview               PicmanPreview;
typedef struct _PicmanPreviewArea           PicmanPreviewArea;
typedef struct _PicmanPixmap                PicmanPixmap;
typedef struct _PicmanRuler                 PicmanRuler;
typedef struct _PicmanScrolledPreview       PicmanScrolledPreview;
typedef struct _PicmanSizeEntry             PicmanSizeEntry;
typedef struct _PicmanStringComboBox        PicmanStringComboBox;
typedef struct _PicmanUnitComboBox          PicmanUnitComboBox;
typedef struct _PicmanUnitMenu              PicmanUnitMenu;
typedef struct _PicmanUnitStore             PicmanUnitStore;
typedef struct _PicmanZoomModel             PicmanZoomModel;


/**
 * PicmanHelpFunc:
 * @help_id:   the help ID
 * @help_data: the help user data
 *
 * This is the prototype for all functions you pass as @help_func to
 * the various PICMAN dialog constructors like picman_dialog_new(),
 * picman_query_int_box() etc.
 *
 * Help IDs are textual identifiers the help system uses to figure
 * which page to display.
 *
 * All these dialog constructors functions call picman_help_connect().
 *
 * In most cases it will be ok to use picman_standard_help_func() which
 * does nothing but passing the @help_id string to picman_help(). If
 * your plug-in needs some more sophisticated help handling you can
 * provide your own @help_func which has to call picman_help() to
 * actually display the help.
 **/
typedef void (* PicmanHelpFunc) (const gchar *help_id,
                               gpointer     help_data);


G_END_DECLS

#endif /* __PICMAN_WIDGETS_TYPES_H__ */
