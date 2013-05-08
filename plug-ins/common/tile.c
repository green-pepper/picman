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

/*
 * This filter tiles an image to arbitrary width and height
 */
#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC   "plug-in-tile"
#define PLUG_IN_BINARY "tile"
#define PLUG_IN_ROLE   "picman-tile"


typedef struct
{
  gint new_width;
  gint new_height;
  gint constrain;
  gint new_image;
} TileVals;


/* Declare local functions.
 */
static void      query  (void);
static void      run    (const gchar      *name,
                         gint              nparams,
                         const PicmanParam  *param,
                         gint             *nreturn_vals,
                         PicmanParam       **return_vals);

static gint32    tile          (gint32     image_id,
                                gint32     drawable_id,
                                gint32    *layer_id);

static gboolean  tile_dialog   (gint32     image_ID,
                                gint32     drawable_ID);


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static TileVals tvals =
{
  1,     /* new_width  */
  1,     /* new_height */
  TRUE,  /* constrain  */
  TRUE   /* new_image  */
};


MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",  "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",      "Input image (unused)"        },
    { PICMAN_PDB_DRAWABLE, "drawable",   "Input drawable"              },
    { PICMAN_PDB_INT32,    "new-width",  "New (tiled) image width"     },
    { PICMAN_PDB_INT32,    "new-height", "New (tiled) image height"    },
    { PICMAN_PDB_INT32,    "new-image",  "Create a new image?"         }
  };

  static const PicmanParamDef return_vals[] =
  {
    { PICMAN_PDB_IMAGE, "new-image", "Output image (-1 if new-image == FALSE)" },
    { PICMAN_PDB_LAYER, "new-layer", "Output layer (-1 if new-image == FALSE)" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Create an array of copies of the image"),
                          "This function creates a new image with a single "
                          "layer sized to the specified 'new_width' and "
                          "'new_height' parameters.  The specified drawable "
                          "is tiled into this layer.  The new layer will have "
                          "the same type as the specified drawable and the "
                          "new image will have a corresponding base type.",
                          "Spencer Kimball & Peter Mattis",
                          "Spencer Kimball & Peter Mattis",
                          "1996-1997",
                          N_("_Tile..."),
                          "RGB*, GRAY*, INDEXED*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args),
                          G_N_ELEMENTS (return_vals),
                          args, return_vals);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Map");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam  values[3];
  PicmanRunMode       run_mode;
  PicmanPDBStatusType status    = PICMAN_PDB_SUCCESS;
  gint32            new_layer = -1;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  *nreturn_vals = 3;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;
  values[1].type          = PICMAN_PDB_IMAGE;
  values[2].type          = PICMAN_PDB_LAYER;

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &tvals);

      /*  First acquire information with a dialog  */
      if (! tile_dialog (param[1].data.d_image,
                         param[2].data.d_drawable))
        return;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 6)
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
      else
        {
          tvals.new_width  = param[3].data.d_int32;
          tvals.new_height = param[4].data.d_int32;
          tvals.new_image  = param[5].data.d_int32 ? TRUE : FALSE;

          if (tvals.new_width < 0 || tvals.new_height < 0)
            status = PICMAN_PDB_CALLING_ERROR;
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &tvals);
      break;

    default:
      break;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      picman_progress_init (_("Tiling"));

      values[1].data.d_image = tile (param[1].data.d_image,
                                     param[2].data.d_drawable,
                                     &new_layer);
      values[2].data.d_layer = new_layer;

      /*  Store data  */
      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (PLUG_IN_PROC, &tvals, sizeof (TileVals));

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        {
          if (tvals.new_image)
            picman_display_new (values[1].data.d_image);
          else
            picman_displays_flush ();
        }
    }

  values[0].data.d_status = status;
}

static gint32
tile (gint32  image_id,
      gint32  drawable_id,
      gint32 *layer_id)
{
  PicmanPixelRgn       src_rgn;
  PicmanPixelRgn       dest_rgn;
  PicmanDrawable      *drawable;
  PicmanDrawable      *new_layer;
  PicmanImageBaseType  image_type   = PICMAN_RGB;
  gint32             new_image_id = 0;
  gint               old_width;
  gint               old_height;
  gint               i, j;
  gint               progress;
  gint               max_progress;
  gpointer           pr;

  /* sanity check parameters */
  if (tvals.new_width < 1 || tvals.new_height < 1)
    {
      *layer_id = -1;
      return -1;
    }

  /* initialize */
  old_width  = picman_drawable_width  (drawable_id);
  old_height = picman_drawable_height (drawable_id);

  if (tvals.new_image)
    {
      /*  create  a new image  */
      switch (picman_drawable_type (drawable_id))
        {
        case PICMAN_RGB_IMAGE:
        case PICMAN_RGBA_IMAGE:
          image_type = PICMAN_RGB;
          break;

        case PICMAN_GRAY_IMAGE:
        case PICMAN_GRAYA_IMAGE:
          image_type = PICMAN_GRAY;
          break;

        case PICMAN_INDEXED_IMAGE:
        case PICMAN_INDEXEDA_IMAGE:
          image_type = PICMAN_INDEXED;
          break;
        }

      new_image_id = picman_image_new (tvals.new_width, tvals.new_height,
                                     image_type);
      picman_image_undo_disable (new_image_id);

      *layer_id = picman_layer_new (new_image_id, _("Background"),
                                  tvals.new_width, tvals.new_height,
                                  picman_drawable_type (drawable_id),
                                  100, PICMAN_NORMAL_MODE);

      if (*layer_id == -1)
        return -1;

      picman_image_insert_layer (new_image_id, *layer_id, -1, 0);
      new_layer = picman_drawable_get (*layer_id);

      /*  Get the source drawable  */
      drawable = picman_drawable_get (drawable_id);
    }
  else
    {
      picman_image_undo_group_start (image_id);

      picman_image_resize (image_id,
                         tvals.new_width, tvals.new_height,
                         0, 0);

      if (picman_item_is_layer (drawable_id))
        picman_layer_resize (drawable_id,
                           tvals.new_width, tvals.new_height,
                           0, 0);

      /*  Get the source drawable  */
      drawable = picman_drawable_get (drawable_id);
      new_layer = drawable;
    }

  /*  progress  */
  progress = 0;
  max_progress = tvals.new_width * tvals.new_height;

  /*  tile...  */
  for (i = 0; i < tvals.new_height; i += old_height)
    {
      gint height = old_height;

      if (height + i > tvals.new_height)
        height = tvals.new_height - i;

      for (j = 0; j < tvals.new_width; j += old_width)
        {
          gint width = old_width;
          gint c;

          if (width + j > tvals.new_width)
            width = tvals.new_width - j;

          picman_pixel_rgn_init (&src_rgn, drawable,
                               0, 0, width, height, FALSE, FALSE);
          picman_pixel_rgn_init (&dest_rgn, new_layer,
                               j, i, width, height, TRUE, FALSE);

          for (pr = picman_pixel_rgns_register (2, &src_rgn, &dest_rgn), c = 0;
               pr != NULL;
               pr = picman_pixel_rgns_process (pr), c++)
            {
              gint k;

              for (k = 0; k < src_rgn.h; k++)
                memcpy (dest_rgn.data + k * dest_rgn.rowstride,
                        src_rgn.data + k * src_rgn.rowstride,
                        src_rgn.w * src_rgn.bpp);

              progress += src_rgn.w * src_rgn.h;

              if (c % 16 == 0)
                picman_progress_update ((gdouble) progress /
                                      (gdouble) max_progress);
            }
        }
    }
  picman_progress_update (1.0);

  picman_drawable_update (new_layer->drawable_id,
                        0, 0, new_layer->width, new_layer->height);

  picman_drawable_detach (drawable);

  if (tvals.new_image)
    {
      picman_drawable_detach (new_layer);

      /*  copy the colormap, if necessary  */
      if (image_type == PICMAN_INDEXED)
        {
          guchar *cmap;
          gint    ncols;

          cmap = picman_image_get_colormap (image_id, &ncols);
          picman_image_set_colormap (new_image_id, cmap, ncols);
          g_free (cmap);
        }

      picman_image_undo_enable (new_image_id);
    }
  else
    {
      picman_image_undo_group_end (image_id);
    }

  return new_image_id;
}

static gboolean
tile_dialog (gint32 image_ID,
             gint32 drawable_ID)
{
  GtkWidget *dlg;
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *sizeentry;
  GtkWidget *chainbutton;
  GtkWidget *toggle;
  gint       width;
  gint       height;
  gdouble    xres;
  gdouble    yres;
  PicmanUnit   unit;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  width  = picman_drawable_width (drawable_ID);
  height = picman_drawable_height (drawable_ID);
  unit   = picman_image_get_unit (image_ID);
  picman_image_get_resolution (image_ID, &xres, &yres);

  tvals.new_width  = width;
  tvals.new_height = height;

  dlg = picman_dialog_new (_("Tile"), PLUG_IN_ROLE,
                         NULL, 0,
                         picman_standard_help_func, PLUG_IN_PROC,

                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                         GTK_STOCK_OK,     GTK_RESPONSE_OK,

                         NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dlg),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  picman_window_set_transient (GTK_WINDOW (dlg));

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))),
                      vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  frame = picman_frame_new (_("Tile to New Size"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  sizeentry = picman_coordinates_new (unit, "%a", TRUE, TRUE, 8,
                                    PICMAN_SIZE_ENTRY_UPDATE_SIZE,

                                    tvals.constrain, TRUE,

                                    _("_Width:"), width, xres,
                                    1, PICMAN_MAX_IMAGE_SIZE,
                                    0, width,

                                    _("_Height:"), height, yres,
                                    1, PICMAN_MAX_IMAGE_SIZE,
                                    0, height);
  gtk_container_add (GTK_CONTAINER (frame), sizeentry);
  gtk_table_set_row_spacing (GTK_TABLE (sizeentry), 1, 6);
  gtk_widget_show (sizeentry);

  chainbutton = GTK_WIDGET (PICMAN_COORDINATES_CHAINBUTTON (sizeentry));

  toggle = gtk_check_button_new_with_mnemonic (_("C_reate new image"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), tvals.new_image);
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &tvals.new_image);

  gtk_widget_show (dlg);

  run = (picman_dialog_run (PICMAN_DIALOG (dlg)) == GTK_RESPONSE_OK);

  if (run)
    {
      tvals.new_width =
        RINT (picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (sizeentry), 0));
      tvals.new_height =
        RINT (picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (sizeentry), 1));

      tvals.constrain =
        picman_chain_button_get_active (PICMAN_CHAIN_BUTTON (chainbutton));
    }

  gtk_widget_destroy (dlg);

  return run;
}
