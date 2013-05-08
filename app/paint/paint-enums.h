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

#ifndef __PAINT_ENUMS_H__
#define __PAINT_ENUMS_H__

#if 0
   This file is parsed by two scripts, enumgen.pl in tools/pdbgen,
   and picman-mkenums. All enums that are not marked with
   /*< pdb-skip >*/ are exported to libpicman and the PDB. Enums that are
   not marked with /*< skip >*/ are registered with the GType system.
   If you want the enum to be skipped by both scripts, you have to use
   /*< pdb-skip, skip >*/.

   The same syntax applies to enum values.
#endif


/*
 * these enums that are registered with the type system
 */

#define PICMAN_TYPE_BRUSH_APPLICATION_MODE (picman_brush_application_mode_get_type ())

GType picman_brush_application_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_BRUSH_HARD,
  PICMAN_BRUSH_SOFT,
  PICMAN_BRUSH_PRESSURE  /*< pdb-skip, skip >*/
} PicmanBrushApplicationMode;


#define PICMAN_TYPE_PERSPECTIVE_CLONE_MODE (picman_perspective_clone_mode_get_type ())

GType picman_perspective_clone_mode_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_PERSPECTIVE_CLONE_MODE_ADJUST,  /*< desc="Modify Perspective" >*/
  PICMAN_PERSPECTIVE_CLONE_MODE_PAINT    /*< desc="Perspective Clone"  >*/
} PicmanPerspectiveCloneMode;


#define PICMAN_TYPE_SOURCE_ALIGN_MODE (picman_source_align_mode_get_type ())

GType picman_source_align_mode_get_type (void) G_GNUC_CONST;

typedef enum /*< pdb-skip >*/
{
  PICMAN_SOURCE_ALIGN_NO,          /*< desc="None"        >*/
  PICMAN_SOURCE_ALIGN_YES,         /*< desc="Aligned"     >*/
  PICMAN_SOURCE_ALIGN_REGISTERED,  /*< desc="Registered"  >*/
  PICMAN_SOURCE_ALIGN_FIXED        /*< desc="Fixed"       >*/
} PicmanSourceAlignMode;


#define PICMAN_TYPE_CONVOLVE_TYPE (picman_convolve_type_get_type ())

GType picman_convolve_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_BLUR_CONVOLVE,     /*< desc="Blur"    >*/
  PICMAN_SHARPEN_CONVOLVE,  /*< desc="Sharpen" >*/
  PICMAN_CUSTOM_CONVOLVE    /*< pdb-skip, skip >*/
} PicmanConvolveType;


#define PICMAN_TYPE_INK_BLOB_TYPE (picman_ink_blob_type_get_type ())

GType picman_ink_blob_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_INK_BLOB_TYPE_CIRCLE,  /*< desc="Circle"  >*/
  PICMAN_INK_BLOB_TYPE_SQUARE,  /*< desc="Square"  >*/
  PICMAN_INK_BLOB_TYPE_DIAMOND  /*< desc="Diamond" >*/
} PicmanInkBlobType;


/*
 * non-registered enums; register them if needed
 */

typedef enum  /*< skip, pdb-skip >*/
{
  PICMAN_PAINT_STATE_INIT,    /*  Setup PaintFunc internals                    */
  PICMAN_PAINT_STATE_MOTION,  /*  PaintFunc performs motion-related rendering  */
  PICMAN_PAINT_STATE_FINISH   /*  Cleanup and/or reset PaintFunc operation     */
} PicmanPaintState;


#endif /* __PAINT_ENUMS_H__ */
