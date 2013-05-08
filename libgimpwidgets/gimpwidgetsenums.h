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
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_WIDGETS_ENUMS_H__
#define __PICMAN_WIDGETS_ENUMS_H__


G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/**
 * PicmanAspectType:
 * @PICMAN_ASPECT_SQUARE:    it's a 1:1 square
 * @PICMAN_ASPECT_PORTRAIT:  it's higher than it's wide
 * @PICMAN_ASPECT_LANDSCAPE: it's wider than it's high
 *
 * Aspect ratios.
 **/
#define PICMAN_TYPE_ASPECT_TYPE (picman_aspect_type_get_type ())

GType picman_aspect_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_ASPECT_SQUARE,    /*< desc="Square"    >*/
  PICMAN_ASPECT_PORTRAIT,  /*< desc="Portrait"  >*/
  PICMAN_ASPECT_LANDSCAPE  /*< desc="Landscape" >*/
} PicmanAspectType;


/**
 * PicmanChainPosition:
 * @PICMAN_CHAIN_TOP:    the chain is on top
 * @PICMAN_CHAIN_LEFT:   the chain is to the left
 * @PICMAN_CHAIN_BOTTOM: the chain is on bottom
 * @PICMAN_CHAIN_RIGHT:  the chain is to the right
 *
 * Possible chain positions for #PicmanChainButton.
 **/
#define PICMAN_TYPE_CHAIN_POSITION (picman_chain_position_get_type ())

GType picman_chain_position_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_CHAIN_TOP,
  PICMAN_CHAIN_LEFT,
  PICMAN_CHAIN_BOTTOM,
  PICMAN_CHAIN_RIGHT
} PicmanChainPosition;


/**
 * PicmanColorAreaType:
 * @PICMAN_COLOR_AREA_FLAT:         don't display transparency
 * @PICMAN_COLOR_AREA_SMALL_CHECKS: display transparency using small checks
 * @PICMAN_COLOR_AREA_LARGE_CHECKS: display transparency using large checks
 *
 * The types of transparency display for #PicmanColorArea.
 **/
#define PICMAN_TYPE_COLOR_AREA_TYPE (picman_color_area_type_get_type ())

GType picman_color_area_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_COLOR_AREA_FLAT = 0,
  PICMAN_COLOR_AREA_SMALL_CHECKS,
  PICMAN_COLOR_AREA_LARGE_CHECKS
} PicmanColorAreaType;


/**
 * PicmanColorSelectorChannel:
 * @PICMAN_COLOR_SELECTOR_HUE:        the hue channel
 * @PICMAN_COLOR_SELECTOR_SATURATION: the saturation channel
 * @PICMAN_COLOR_SELECTOR_VALUE:      the value channel
 * @PICMAN_COLOR_SELECTOR_RED:        the red channel
 * @PICMAN_COLOR_SELECTOR_GREEN:      the green channel
 * @PICMAN_COLOR_SELECTOR_BLUE:       the blue channel
 * @PICMAN_COLOR_SELECTOR_ALPHA:      the alpha channel
 *
 * An enum to specify the types of color channels edited in
 * #PicmanColorSelector widgets.
 **/
#define PICMAN_TYPE_COLOR_SELECTOR_CHANNEL (picman_color_selector_channel_get_type ())

GType picman_color_selector_channel_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_COLOR_SELECTOR_HUE,        /*< desc="_H", help="Hue"        >*/
  PICMAN_COLOR_SELECTOR_SATURATION, /*< desc="_S", help="Saturation" >*/
  PICMAN_COLOR_SELECTOR_VALUE,      /*< desc="_V", help="Value"      >*/
  PICMAN_COLOR_SELECTOR_RED,        /*< desc="_R", help="Red"        >*/
  PICMAN_COLOR_SELECTOR_GREEN,      /*< desc="_G", help="Green"      >*/
  PICMAN_COLOR_SELECTOR_BLUE,       /*< desc="_B", help="Blue"       >*/
  PICMAN_COLOR_SELECTOR_ALPHA       /*< desc="_A", help="Alpha"      >*/
} PicmanColorSelectorChannel;


/**
 * PicmanPageSelectorTarget:
 * @PICMAN_PAGE_SELECTOR_TARGET_LAYERS: import as layers of one image
 * @PICMAN_PAGE_SELECTOR_TARGET_IMAGES: import as separate images
 *
 * Import targets for #PicmanPageSelector.
 **/
#define PICMAN_TYPE_PAGE_SELECTOR_TARGET (picman_page_selector_target_get_type ())

GType picman_page_selector_target_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_PAGE_SELECTOR_TARGET_LAYERS, /*< desc="Layers" >*/
  PICMAN_PAGE_SELECTOR_TARGET_IMAGES  /*< desc="Images" >*/
} PicmanPageSelectorTarget;


/**
 * PicmanSizeEntryUpdatePolicy:
 * @PICMAN_SIZE_ENTRY_UPDATE_NONE:       the size entry's meaning is up to the user
 * @PICMAN_SIZE_ENTRY_UPDATE_SIZE:       the size entry displays values
 * @PICMAN_SIZE_ENTRY_UPDATE_RESOLUTION: the size entry displays resolutions
 *
 * Update policies for #PicmanSizeEntry.
 **/
#define PICMAN_TYPE_SIZE_ENTRY_UPDATE_POLICY (picman_size_entry_update_policy_get_type ())

GType picman_size_entry_update_policy_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_SIZE_ENTRY_UPDATE_NONE       = 0,
  PICMAN_SIZE_ENTRY_UPDATE_SIZE       = 1,
  PICMAN_SIZE_ENTRY_UPDATE_RESOLUTION = 2
} PicmanSizeEntryUpdatePolicy;


/**
 * PicmanZoomType:
 * @PICMAN_ZOOM_IN:       zoom in
 * @PICMAN_ZOOM_OUT:      zoom out
 * @PICMAN_ZOOM_IN_MORE:  zoom in a lot
 * @PICMAN_ZOOM_OUT_MORE: zoom out a lot
 * @PICMAN_ZOOM_IN_MAX:   zoom in as far as possible
 * @PICMAN_ZOOM_OUT_MAX:  zoom out as far as possible
 * @PICMAN_ZOOM_TO:       zoom to a specific zoom factor
 *
 * the zoom types for #PicmanZoomModel.
 **/
#define PICMAN_TYPE_ZOOM_TYPE (picman_zoom_type_get_type ())

GType picman_zoom_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_ZOOM_IN,        /*< desc="Zoom in"  >*/
  PICMAN_ZOOM_OUT,       /*< desc="Zoom out" >*/
  PICMAN_ZOOM_IN_MORE,   /*< skip >*/
  PICMAN_ZOOM_OUT_MORE,  /*< skip >*/
  PICMAN_ZOOM_IN_MAX,    /*< skip >*/
  PICMAN_ZOOM_OUT_MAX,   /*< skip >*/
  PICMAN_ZOOM_TO         /*< skip >*/
} PicmanZoomType;


G_END_DECLS

#endif  /* __PICMAN_WIDGETS_ENUMS_H__ */
