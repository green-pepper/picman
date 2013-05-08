/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __CORE_TYPES_H__
#define __CORE_TYPES_H__


#include "libpicmanbase/picmanbasetypes.h"
#include "libpicmanmath/picmanmathtypes.h"
#include "libpicmancolor/picmancolortypes.h"
#include "libpicmanmodule/picmanmoduletypes.h"
#include "libpicmanthumb/picmanthumb-types.h"

#include "config/config-types.h"

#include "core/core-enums.h"


/*  former base/ defines  */

#define MAX_CHANNELS  4

#define RED           0
#define GREEN         1
#define BLUE          2
#define ALPHA         3

#define GRAY          0
#define ALPHA_G       1

#define INDEXED       0
#define ALPHA_I       1


/*  defines  */

#define PICMAN_COORDS_MIN_PRESSURE      0.0
#define PICMAN_COORDS_MAX_PRESSURE      1.0
#define PICMAN_COORDS_DEFAULT_PRESSURE  1.0

#define PICMAN_COORDS_MIN_TILT         -1.0
#define PICMAN_COORDS_MAX_TILT          1.0
#define PICMAN_COORDS_DEFAULT_TILT      0.0

#define PICMAN_COORDS_MIN_WHEEL         0.0
#define PICMAN_COORDS_MAX_WHEEL         1.0
#define PICMAN_COORDS_DEFAULT_WHEEL     0.5

#define PICMAN_COORDS_DEFAULT_VELOCITY  0.0

#define PICMAN_COORDS_DEFAULT_DIRECTION 0.0

#define PICMAN_COORDS_DEFAULT_VALUES    { 0.0, 0.0, \
                                        PICMAN_COORDS_DEFAULT_PRESSURE, \
                                        PICMAN_COORDS_DEFAULT_TILT,     \
                                        PICMAN_COORDS_DEFAULT_TILT,     \
                                        PICMAN_COORDS_DEFAULT_WHEEL,    \
                                        PICMAN_COORDS_DEFAULT_VELOCITY, \
                                        PICMAN_COORDS_DEFAULT_DIRECTION }


/*  base classes  */

typedef struct _PicmanObject          PicmanObject;
typedef struct _PicmanViewable        PicmanViewable;
typedef struct _PicmanFilter          PicmanFilter;
typedef struct _PicmanItem            PicmanItem;

typedef struct _Picman                Picman;
typedef struct _PicmanImage           PicmanImage;


/*  containers  */

typedef struct _PicmanContainer         PicmanContainer;
typedef struct _PicmanList              PicmanList;
typedef struct _PicmanDocumentList      PicmanDocumentList;
typedef struct _PicmanDrawableStack     PicmanDrawableStack;
typedef struct _PicmanFilteredContainer PicmanFilteredContainer;
typedef struct _PicmanFilterStack       PicmanFilterStack;
typedef struct _PicmanItemStack         PicmanItemStack;
typedef struct _PicmanTaggedContainer   PicmanTaggedContainer;


/*  not really a container  */

typedef struct _PicmanItemTree          PicmanItemTree;


/*  context objects  */

typedef struct _PicmanContext         PicmanContext;
typedef struct _PicmanFillOptions     PicmanFillOptions;
typedef struct _PicmanStrokeOptions   PicmanStrokeOptions;
typedef struct _PicmanToolOptions     PicmanToolOptions;


/*  info objects  */

typedef struct _PicmanPaintInfo       PicmanPaintInfo;
typedef struct _PicmanToolInfo        PicmanToolInfo;


/*  data objects  */

typedef struct _PicmanDataFactory      PicmanDataFactory;
typedef struct _PicmanData             PicmanData;
typedef struct _PicmanBrush            PicmanBrush;
typedef struct _PicmanBrushCache       PicmanBrushCache;
typedef struct _PicmanBrushClipboard   PicmanBrushClipboard;
typedef struct _PicmanBrushGenerated   PicmanBrushGenerated;
typedef struct _PicmanBrushPipe        PicmanBrushPipe;
typedef struct _PicmanCurve            PicmanCurve;
typedef struct _PicmanDynamics         PicmanDynamics;
typedef struct _PicmanDynamicsOutput   PicmanDynamicsOutput;
typedef struct _PicmanGradient         PicmanGradient;
typedef struct _PicmanPalette          PicmanPalette;
typedef struct _PicmanPattern          PicmanPattern;
typedef struct _PicmanPatternClipboard PicmanPatternClipboard;
typedef struct _PicmanToolPreset       PicmanToolPreset;
typedef struct _PicmanTagCache         PicmanTagCache;


/*  drawable objects  */

typedef struct _PicmanDrawable        PicmanDrawable;
typedef struct _PicmanChannel         PicmanChannel;
typedef struct _PicmanLayerMask       PicmanLayerMask;
typedef struct _PicmanSelection       PicmanSelection;
typedef struct _PicmanLayer           PicmanLayer;
typedef struct _PicmanGroupLayer      PicmanGroupLayer;


/*  undo objects  */

typedef struct _PicmanUndo              PicmanUndo;
typedef struct _PicmanImageUndo         PicmanImageUndo;
typedef struct _PicmanItemUndo          PicmanItemUndo;
typedef struct _PicmanItemPropUndo      PicmanItemPropUndo;
typedef struct _PicmanChannelUndo       PicmanChannelUndo;
typedef struct _PicmanChannelPropUndo   PicmanChannelPropUndo;
typedef struct _PicmanDrawableUndo      PicmanDrawableUndo;
typedef struct _PicmanDrawableModUndo   PicmanDrawableModUndo;
typedef struct _PicmanLayerMaskUndo     PicmanLayerMaskUndo;
typedef struct _PicmanLayerMaskPropUndo PicmanLayerMaskPropUndo;
typedef struct _PicmanLayerUndo         PicmanLayerUndo;
typedef struct _PicmanLayerPropUndo     PicmanLayerPropUndo;
typedef struct _PicmanGroupLayerUndo    PicmanGroupLayerUndo;
typedef struct _PicmanMaskUndo          PicmanMaskUndo;
typedef struct _PicmanGuideUndo         PicmanGuideUndo;
typedef struct _PicmanSamplePointUndo   PicmanSamplePointUndo;
typedef struct _PicmanFloatingSelUndo   PicmanFloatingSelUndo;
typedef struct _PicmanUndoStack         PicmanUndoStack;
typedef struct _PicmanUndoAccumulator   PicmanUndoAccumulator;


/*  misc objects  */

typedef struct _PicmanBuffer          PicmanBuffer;
typedef struct _PicmanEnvironTable    PicmanEnvironTable;
typedef struct _PicmanGuide           PicmanGuide;
typedef struct _PicmanHistogram       PicmanHistogram;
typedef struct _PicmanIdTable         PicmanIdTable;
typedef struct _PicmanImageMap        PicmanImageMap;
typedef struct _PicmanImageMapConfig  PicmanImageMapConfig;
typedef struct _PicmanImagefile       PicmanImagefile;
typedef struct _PicmanInterpreterDB   PicmanInterpreterDB;
typedef struct _PicmanParasiteList    PicmanParasiteList;
typedef struct _PicmanPdbProgress     PicmanPdbProgress;
typedef struct _PicmanProjection      PicmanProjection;
typedef struct _PicmanSubProgress     PicmanSubProgress;
typedef struct _PicmanTag             PicmanTag;
typedef struct _PicmanTreeHandler     PicmanTreeHandler;


/*  interfaces  */

typedef struct _PicmanPickable        PicmanPickable;    /* dummy typedef */
typedef struct _PicmanProgress        PicmanProgress;    /* dummy typedef */
typedef struct _PicmanProjectable     PicmanProjectable; /* dummy typedef */
typedef struct _PicmanTagged          PicmanTagged;      /* dummy typedef */


/*  non-object types  */

typedef struct _PicmanArea            PicmanArea;
typedef struct _PicmanBoundSeg        PicmanBoundSeg;
typedef struct _PicmanCoords          PicmanCoords;
typedef struct _PicmanGradientSegment PicmanGradientSegment;
typedef struct _PicmanPaletteEntry    PicmanPaletteEntry;
typedef struct _PicmanSamplePoint     PicmanSamplePoint;
typedef struct _PicmanScanConvert     PicmanScanConvert;
typedef struct _PicmanTempBuf         PicmanTempBuf;
typedef         guint32             PicmanTattoo;

/* The following hack is made so that we can reuse the definition
 * the cairo definition of cairo_path_t without having to translate
 * between our own version of a bezier description and cairos version.
 *
 * to avoid having to include <cairo.h> in each and every file
 * including this file we only use the "real" definition when cairo.h
 * already has been included and use something else.
 *
 * Note that if you really want to work with PicmanBezierDesc (except just
 * passing pointers to it around) you also need to include <cairo.h>.
 */
#ifdef CAIRO_VERSION
typedef cairo_path_t PicmanBezierDesc;
#else
typedef void * PicmanBezierDesc;
#endif


/*  functions  */

typedef void     (* PicmanInitStatusFunc)    (const gchar      *text1,
                                            const gchar      *text2,
                                            gdouble           percentage);

typedef gboolean (* PicmanObjectFilterFunc)  (const PicmanObject *object,
                                            gpointer          user_data);

typedef gint64   (* PicmanMemsizeFunc)       (gpointer          instance,
                                            gint64           *gui_size);


/*  structs  */

struct _PicmanCoords
{
  gdouble x;
  gdouble y;
  gdouble pressure;
  gdouble xtilt;
  gdouble ytilt;
  gdouble wheel;
  gdouble velocity;
  gdouble direction;
};

/*  temp hack as replacement for GdkSegment  */

typedef struct _PicmanSegment PicmanSegment;

struct _PicmanSegment
{
  gint x1;
  gint y1;
  gint x2;
  gint y2;
};


#include "gegl/picman-gegl-types.h"
#include "paint/paint-types.h"
#include "text/text-types.h"
#include "vectors/vectors-types.h"
#include "pdb/pdb-types.h"
#include "plug-in/plug-in-types.h"


#endif /* __CORE_TYPES_H__ */
