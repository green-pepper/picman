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

#ifndef __PICMAN_IMAGE_PRIVATE_H__
#define __PICMAN_IMAGE_PRIVATE_H__


typedef struct _PicmanImageFlushAccumulator PicmanImageFlushAccumulator;

struct _PicmanImageFlushAccumulator
{
  gboolean alpha_changed;
  gboolean mask_changed;
  gboolean floating_selection_changed;
  gboolean preview_invalidated;
};


typedef struct _PicmanImagePrivate PicmanImagePrivate;

struct _PicmanImagePrivate
{
  gint               ID;                    /*  provides a unique ID         */

  PicmanPlugInProcedure *load_proc;           /*  procedure used for loading   */
  PicmanPlugInProcedure *save_proc;           /*  last save procedure used     */

  gchar             *display_name;          /*  display basename             */
  gchar             *display_path;          /*  display full path            */
  gint               width;                 /*  width in pixels              */
  gint               height;                /*  height in pixels             */
  gdouble            xresolution;           /*  image x-res, in dpi          */
  gdouble            yresolution;           /*  image y-res, in dpi          */
  PicmanUnit           resolution_unit;       /*  resolution unit              */
  PicmanImageBaseType  base_type;             /*  base picman_image type         */
  PicmanPrecision      precision;             /*  image's precision            */

  guchar            *colormap;              /*  colormap (for indexed)       */
  gint               n_colors;              /*  # of colors (for indexed)    */
  PicmanPalette       *palette;               /*  palette of colormap          */
  const Babl        *babl_palette_rgb;      /*  palette's RGB Babl format    */
  const Babl        *babl_palette_rgba;     /*  palette's RGBA Babl format   */

  gint               dirty;                 /*  dirty flag -- # of ops       */
  guint              dirty_time;            /*  time when image became dirty */
  gint               export_dirty;          /*  'dirty' but for export       */

  gint               undo_freeze_count;     /*  counts the _freeze's         */

  gint               instance_count;        /*  number of instances          */
  gint               disp_count;            /*  number of displays           */

  PicmanTattoo         tattoo_state;          /*  the last used tattoo         */

  PicmanProjection    *projection;            /*  projection layers & channels */
  GeglNode          *graph;                 /*  GEGL projection graph        */
  GeglNode          *visible_mask;          /*  component visibility node    */

  GList             *guides;                /*  guides                       */
  PicmanGrid          *grid;                  /*  grid                         */
  GList             *sample_points;         /*  color sample points          */

  /*  Layer/Channel attributes  */
  PicmanItemTree      *layers;                /*  the tree of layers           */
  PicmanItemTree      *channels;              /*  the tree of masks            */
  PicmanItemTree      *vectors;               /*  the tree of vectors          */
  GSList            *layer_stack;           /*  the layers in MRU order      */

  GQuark             layer_alpha_handler;
  GQuark             channel_name_changed_handler;
  GQuark             channel_color_changed_handler;

  PicmanLayer         *floating_sel;          /*  the FS layer                 */
  PicmanChannel       *selection_mask;        /*  the selection mask channel   */

  PicmanParasiteList  *parasites;             /*  Plug-in parasite data        */

  gboolean           visible[MAX_CHANNELS]; /*  visible channels             */
  gboolean           active[MAX_CHANNELS];  /*  active channels              */

  gboolean           quick_mask_state;      /*  TRUE if quick mask is on       */
  gboolean           quick_mask_inverted;   /*  TRUE if quick mask is inverted */
  PicmanRGB            quick_mask_color;      /*  rgba triplet of the color      */

  /*  Undo apparatus  */
  PicmanUndoStack     *undo_stack;            /*  stack for undo operations    */
  PicmanUndoStack     *redo_stack;            /*  stack for redo operations    */
  gint               group_count;           /*  nested undo groups           */
  PicmanUndoType       pushing_undo_group;    /*  undo group status flag       */

  /*  Signal emission accumulator  */
  PicmanImageFlushAccumulator  flush_accum;
};

#define PICMAN_IMAGE_GET_PRIVATE(image) \
        G_TYPE_INSTANCE_GET_PRIVATE (image, \
                                     PICMAN_TYPE_IMAGE, \
                                     PicmanImagePrivate)

void   picman_image_take_mask (PicmanImage   *image,
                             PicmanChannel *mask);


#endif  /* __PICMAN_IMAGE_PRIVATE_H__ */
