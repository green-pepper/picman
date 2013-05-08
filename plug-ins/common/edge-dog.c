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
 * Picman plug-in dog.c:  (C) 2004 William Skaggs
 *
 * Edge detection using the "Difference of Gaussians" method.
 * Finds edges by doing two Gaussian blurs with different radius, and
 * subtracting the results.  Blurring is done using code taken from
 * gauss_rle.c (as of Picman 2.1, incorporated into gauss.c).
 */

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC   "plug-in-dog"
#define PLUG_IN_BINARY "edge-dog"
#define PLUG_IN_ROLE   "picman-edge-dog"


typedef struct
{
  gdouble  inner;
  gdouble  outer;
  gboolean normalize;
  gboolean invert;
} DoGValues;


/* Declare local functions.
 */
static void      query  (void);
static void      run    (const gchar      *name,
                         gint              nparams,
                         const PicmanParam  *param,
                         gint             *nreturn_vals,
                         PicmanParam       **return_vals);

static gint      dog_dialog           (gint32        image_ID,
                                       PicmanDrawable *drawable);

static void      gauss_rle            (PicmanDrawable *drawable,
                                       gdouble       radius,
                                       gint          pass,
                                       gboolean      show_progress);

static void      compute_difference   (PicmanDrawable *drawable,
                                       PicmanDrawable *drawable1,
                                       PicmanDrawable *drawable2,
                                       guchar       *maxval);

static void      normalize_invert     (PicmanDrawable *drawable,
                                       gboolean      normalize,
                                       guint         maxval,
                                       gboolean      invert);

static void      dog                  (gint32        image_ID,
                                       PicmanDrawable *drawable,
                                       gdouble       inner,
                                       gdouble       outer,
                                       gboolean      show_progress);

static void      preview_update_preview  (PicmanPreview  *preview,
                                          PicmanDrawable *drawable);
static void      change_radius_callback  (GtkWidget    *widget,
                                          gpointer      data);



/*
 * Gaussian blur helper functions
 */
static gint *    make_curve              (gdouble    sigma,
                                          gint      *length);
static void      run_length_encode       (guchar    *src,
                                          gint      *dest,
                                          gint       bytes,
                                          gint       width);


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static DoGValues dogvals =
{
  3.0,  /* inner radius  */
  1.0,  /* outer radius  */
  TRUE, /* normalize     */
  TRUE  /* invert        */
};

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",  "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",     "Input image" },
    { PICMAN_PDB_DRAWABLE, "drawable",  "Input drawable" },
    { PICMAN_PDB_FLOAT,    "inner",     "Radius of inner gaussian blur (in pixels, > 0.0)" },
    { PICMAN_PDB_FLOAT,    "outer",     "Radius of outer gaussian blur (in pixels, > 0.0)" },
    { PICMAN_PDB_INT32,    "normalize", "Normalize { TRUE, FALSE }" },
    { PICMAN_PDB_INT32,    "invert",    "Invert { TRUE, FALSE }" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Edge detection with control of edge thickness"),
                          "Applies two Gaussian blurs to the drawable, and "
                          "subtracts the results.  This is robust and widely "
                          "used method for detecting edges.",
                          "Spencer Kimball, Peter Mattis, Sven Neumann, William Skaggs",
                          "Spencer Kimball, Peter Mattis, Sven Neumann, William Skaggs",
                          "1995-2004",
                          N_("_Difference of Gaussians..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[2];
  gint32             image_ID;
  PicmanDrawable      *drawable;
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  GError            *error  = NULL;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  if (! picman_item_is_layer (param[2].data.d_drawable))
    {
      g_set_error (&error, 0, 0, "%s",
                   _("Can operate on layers only "
                     "(but was called on channel or mask)."));
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      /*  Get the specified image and drawable  */
      image_ID = param[1].data.d_image;
      drawable = picman_drawable_get (param[2].data.d_drawable);

      /*  set the tile cache size so that the gaussian blur works well  */
      picman_tile_cache_ntiles (2 *
                              (MAX (drawable->width, drawable->height) /
                               picman_tile_width () + 1));

      if (strcmp (name, PLUG_IN_PROC) == 0)
        {
          switch (run_mode)
            {
            case PICMAN_RUN_INTERACTIVE:
              /*  Possibly retrieve data  */
              picman_get_data (PLUG_IN_PROC, &dogvals);

              /*  First acquire information with a dialog  */
              if (! dog_dialog (image_ID, drawable))
                return;
              break;

            case PICMAN_RUN_NONINTERACTIVE:
              /*  Make sure all the arguments are there!  */
              if (nparams != 7)
                status = PICMAN_PDB_CALLING_ERROR;

              if (status == PICMAN_PDB_SUCCESS)
                {
                  dogvals.inner     = param[3].data.d_float;
                  dogvals.outer     = param[4].data.d_float;
                  dogvals.normalize = param[5].data.d_int32;
                  dogvals.invert    = param[6].data.d_int32;

                  if (dogvals.inner <= 0.0 || dogvals.outer <= 0.0)
                    status = PICMAN_PDB_CALLING_ERROR;
                }
              break;

            case PICMAN_RUN_WITH_LAST_VALS:
              /*  Possibly retrieve data  */
              picman_get_data (PLUG_IN_PROC, &dogvals);
              break;

            default:
              break;
            }
        }
      else
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      /*  Make sure that the drawable is gray or RGB color  */
      if (picman_drawable_is_rgb (drawable->drawable_id) ||
          picman_drawable_is_gray (drawable->drawable_id))
        {
          picman_progress_init (_("DoG Edge Detect"));

          /*  run the Difference of Gaussians  */
          picman_image_undo_group_start (image_ID);

          dog (image_ID, drawable, dogvals.inner, dogvals.outer, TRUE);

          picman_image_undo_group_end (image_ID);

          picman_progress_update (1.0);

          /*  Store data  */
          if (run_mode == PICMAN_RUN_INTERACTIVE)
            picman_set_data (PLUG_IN_PROC, &dogvals, sizeof (DoGValues));

          if (run_mode != PICMAN_RUN_NONINTERACTIVE)
            picman_displays_flush ();
        }
      else
        {
          status        = PICMAN_PDB_EXECUTION_ERROR;
          *nreturn_vals = 2;

          values[1].type          = PICMAN_PDB_STRING;
          values[1].data.d_string = _("Cannot operate on indexed color images.");
        }

      picman_drawable_detach (drawable);
    }

  if (status != PICMAN_PDB_SUCCESS && error)
    {
      *nreturn_vals = 2;
      values[1].type          = PICMAN_PDB_STRING;
      values[1].data.d_string = error->message;
    }

  values[0].data.d_status = status;
}

static gint
dog_dialog (gint32        image_ID,
            PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *frame;
  GtkWidget *button;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *coord;
  PicmanUnit   unit;
  gdouble    xres;
  gdouble    yres;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("DoG Edge Detect"), PLUG_IN_ROLE,
                            NULL, 0,
                            picman_standard_help_func, PLUG_IN_PROC,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,     GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  picman_window_set_transient (GTK_WINDOW (dialog));

  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);

  preview = picman_drawable_preview_new (drawable, NULL);
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, FALSE, FALSE, 0);
  gtk_widget_show (preview);

  g_signal_connect (preview, "invalidated",
                    G_CALLBACK (preview_update_preview),
                    drawable);

  frame = picman_frame_new (_("Smoothing Parameters"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  Get the image resolution and unit  */
  picman_image_get_resolution (image_ID, &xres, &yres);
  unit = picman_image_get_unit (image_ID);

  coord = picman_coordinates_new (unit, "%a", TRUE, FALSE, -1,
                                PICMAN_SIZE_ENTRY_UPDATE_SIZE,

                                FALSE,
                                TRUE,

                                _("_Radius 1:"), dogvals.inner, xres,
                                0, 8 * MAX (drawable->width, drawable->height),
                                0, 0,

                                _("R_adius 2:"), dogvals.outer, yres,
                                0, 8 * MAX (drawable->width, drawable->height),
                                0, 0);

  gtk_container_add (GTK_CONTAINER (frame), coord);
  gtk_widget_show (coord);

  picman_size_entry_set_pixel_digits (PICMAN_SIZE_ENTRY (coord), 1);
  g_signal_connect (coord, "value-changed",
                    G_CALLBACK (change_radius_callback),
                    preview);

  button = gtk_check_button_new_with_mnemonic (_("_Normalize"));
  gtk_box_pack_start (GTK_BOX (main_vbox), button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), dogvals.normalize);
  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &dogvals.normalize);
  g_signal_connect_swapped (button, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_mnemonic (_("_Invert"));
  gtk_box_pack_start (GTK_BOX (main_vbox), button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), dogvals.invert);
  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &dogvals.invert);
  g_signal_connect_swapped (button, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);
  gtk_widget_show (button);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  if (run)
    {
      dogvals.inner = picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (coord), 0);
      dogvals.outer = picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (coord), 1);
    }

  gtk_widget_destroy (dialog);


  return run;
}


/* Convert from separated to premultiplied alpha, on a single scan line. */
static void
multiply_alpha (guchar *buf,
                gint    width,
                gint    bytes)
{
  gint    i, j;
  gdouble alpha;

  for (i = 0; i < width * bytes; i += bytes)
    {
      alpha = buf[i + bytes - 1] * (1.0 / 255.0);

      for (j = 0; j < bytes - 1; j++)
        buf[i + j] *= alpha;
    }
}

/* Convert from premultiplied to separated alpha, on a single scan
   line. */
static void
separate_alpha (guchar *buf,
                gint    width,
                gint    bytes)
{
  gint    i, j;
  guchar  alpha;
  gdouble recip_alpha;
  gint    new_val;

  for (i = 0; i < width * bytes; i += bytes)
    {
      alpha = buf[i + bytes - 1];

      if (alpha != 0 && alpha != 255)
        {
          recip_alpha = 255.0 / alpha;

          for (j = 0; j < bytes - 1; j++)
            {
              new_val = buf[i + j] * recip_alpha;
              buf[i + j] = MIN (255, new_val);
            }
        }
    }
}

static void
dog (gint32        image_ID,
     PicmanDrawable *drawable,
     gdouble       inner,
     gdouble       outer,
     gboolean      show_progress)
{
  PicmanDrawable *drawable1;
  PicmanDrawable *drawable2;
  gint32        drawable_id = drawable->drawable_id;
  gint32        layer1;
  gint32        layer2;
  gint          width, height;
  gint          x1, y1, x2, y2;
  guchar        maxval = 255;

  picman_drawable_mask_bounds (drawable_id, &x1, &y1, &x2, &y2);

  width  = (x2 - x1);
  height = (y2 - y1);

  picman_drawable_flush (drawable);

  layer1 = picman_layer_copy (drawable_id);
  picman_item_set_visible (layer1, FALSE);
  picman_item_set_name (layer1, "dog_scratch_layer1");
  picman_image_insert_layer (image_ID, layer1,
                           picman_item_get_parent (drawable_id), 0);

  layer2 = picman_layer_copy (drawable_id);
  picman_item_set_visible (layer2, FALSE);
  picman_item_set_name (layer2, "dog_scratch_layer2");
  picman_image_insert_layer (image_ID, layer2,
                           picman_item_get_parent (drawable_id), 0);

  drawable1 = picman_drawable_get (layer1);
  drawable2 = picman_drawable_get (layer2);

  gauss_rle (drawable1, inner, 0, show_progress);
  gauss_rle (drawable2, outer, 1, show_progress);

  compute_difference (drawable, drawable1, drawable2, &maxval);

  picman_drawable_detach (drawable1);
  picman_drawable_detach (drawable2);

  picman_image_remove_layer (image_ID, layer1);
  picman_image_remove_layer (image_ID, layer2);

  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable_id, TRUE);
  picman_drawable_update (drawable_id, x1, y1, width, height);

  if (dogvals.normalize || dogvals.invert)
    /* picman_invert doesn't work properly with previews due to shadow handling
     * so reimplement it here - see Bug 557380
     */
    {
      normalize_invert (drawable, dogvals.normalize, maxval, dogvals.invert);
      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable_id, TRUE);
      picman_drawable_update (drawable_id, x1, y1, width, height);
    }
}


static void
compute_difference (PicmanDrawable *drawable,
                    PicmanDrawable *drawable1,
                    PicmanDrawable *drawable2,
                    guchar       *maxval)
{
  PicmanPixelRgn src1_rgn, src2_rgn, dest_rgn;
  gint         width, height;
  gint         bpp;
  gpointer     pr;
  gint         x, y, k;
  gint         x1, y1, x2, y2;
  gboolean     has_alpha;

  *maxval = 0;

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);

  width  = (x2 - x1);
  height = (y2 - y1);

  if (width < 1 || height < 1)
    return;

  bpp = drawable->bpp;
  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);

  picman_pixel_rgn_init (&src1_rgn,
                       drawable1, 0, 0, drawable1->width, drawable1->height,
                       FALSE, FALSE);
  picman_pixel_rgn_init (&src2_rgn,
                       drawable2, 0, 0, drawable1->width, drawable1->height,
                       FALSE, FALSE);
  picman_pixel_rgn_init (&dest_rgn,
                       drawable, 0, 0, drawable->width, drawable->height,
                       TRUE, TRUE);

  for (pr = picman_pixel_rgns_register (3, &src1_rgn, &src2_rgn, &dest_rgn);
       pr != NULL;
       pr = picman_pixel_rgns_process (pr))
    {
      guchar *src1 = src1_rgn.data;
      guchar *src2 = src2_rgn.data;
      guchar *dest = dest_rgn.data;
      gint    row  = src1_rgn.y - y1;

      for (y = 0; y < src1_rgn.h; y++, row++)
        {
          guchar *s1  = src1;
          guchar *s2  = src2;
          guchar *d   = dest;
          gint    col = src1_rgn.x - x1;

          for (x = 0; x < src1_rgn.w; x++, col++)
            {

              if (has_alpha)
                {
                  for (k = 0; k < bpp-1; k++)
                    {
                      d[k] = CLAMP0255 (s1[k] - s2[k]);
                      *maxval = MAX (d[k], *maxval);
                    }
                }
              else
                {
                  for (k = 0; k < bpp; k++)
                    {
                      d[k] = CLAMP0255 (s1[k] - s2[k]);
                      *maxval = MAX (d[k], *maxval);
                    }
                }

              s1 += bpp;
              s2 += bpp;
              d += bpp;
            }

          src1 += src1_rgn.rowstride;
          src2 += src2_rgn.rowstride;
          dest += dest_rgn.rowstride;
        }
    }
}


static void
normalize_invert (PicmanDrawable *drawable,
                  gboolean      normalize,
                  guint         maxval,
                  gboolean      invert)
{
  PicmanPixelRgn src_rgn, dest_rgn;
  gint         bpp;
  gpointer     pr;
  gint         x, y, k;
  gint         x1, y1, x2, y2;
  gboolean     has_alpha;
  gdouble      factor;

  if (normalize && maxval != 0) {
      factor = 255.0 / maxval;
    }
  else
    factor = 1.0;

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);
  bpp = drawable->bpp;
  has_alpha = picman_drawable_has_alpha(drawable->drawable_id);

  picman_pixel_rgn_init (&src_rgn,
                       drawable, 0, 0, drawable->width, drawable->height,
                       FALSE, FALSE);
  picman_pixel_rgn_init (&dest_rgn,
                       drawable, 0, 0, drawable->width, drawable->height,
                       TRUE, TRUE);

  for (pr = picman_pixel_rgns_register (2, &src_rgn, &dest_rgn);
       pr != NULL;
       pr = picman_pixel_rgns_process (pr))
    {
      guchar *src  = src_rgn.data;
      guchar *dest = dest_rgn.data;
      gint    row  = src_rgn.y - y1;

      for (y = 0; y < src_rgn.h; y++, row++)
        {
          guchar *s   = src;
          guchar *d   = dest;
          gint    col = src_rgn.x - x1;

          for (x = 0; x < src_rgn.w; x++, col++)
            {

              if (has_alpha)
                {
                  for (k = 0; k < bpp-1; k++)
                    {
                      d[k] = factor * s[k];
                      if (invert)
                        d[k] = 255 - d[k];
                    }
                }
              else
                {
                  for (k = 0; k < bpp; k++)
                    {
                      d[k] = factor * s[k];
                      if (invert)
                        d[k] = 255 - d[k];
                    }
                }

              s += bpp;
              d += bpp;
            }

          src += src_rgn.rowstride;
          dest += dest_rgn.rowstride;
        }
    }
}


static void
gauss_rle (PicmanDrawable *drawable,
           gdouble       radius,
           gint          pass,
           gboolean      show_progress)
{
  PicmanPixelRgn src_rgn, dest_rgn;
  gint     width, height;
  gint     bytes;
  gint     has_alpha;
  guchar  *dest, *dp;
  guchar  *src, *sp;
  gint    *buf, *bb;
  gint     pixels;
  gint     total = 1;
  gint     x1, y1, x2, y2;
  gint     i, row, col, b;
  gint     start, end;
  gdouble  progress, max_progress;
  gint    *curve;
  gint    *sum = NULL;
  gint     val;
  gint     length;
  gint     initial_p, initial_m;
  gdouble  std_dev;

  if (radius <= 0.0)
    return;

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);

  width  = (x2 - x1);
  height = (y2 - y1);

  if (width < 1 || height < 1)
    return;

  bytes = drawable->bpp;
  has_alpha = picman_drawable_has_alpha(drawable->drawable_id);

  buf = g_new (gint, MAX (width, height) * 2);

  /*  allocate buffers for source and destination pixels  */
  src = g_new (guchar, MAX (width, height) * bytes);
  dest = g_new (guchar, MAX (width, height) * bytes);

  picman_pixel_rgn_init (&src_rgn,
                       drawable, 0, 0, drawable->width, drawable->height,
                       FALSE, FALSE);
  picman_pixel_rgn_init (&dest_rgn,
                       drawable, 0, 0, drawable->width, drawable->height,
                       TRUE, TRUE);

  progress = 0.0;
  max_progress  = 2 * width * height;

  /*  First the vertical pass  */
  radius = fabs (radius) + 1.0;
  std_dev = sqrt (-(radius * radius) / (2 * log (1.0 / 255.0)));

  curve = make_curve (std_dev, &length);
  sum = g_new (gint, 2 * length + 1);

  sum[0] = 0;

  for (i = 1; i <= length*2; i++)
    sum[i] = curve[i-length-1] + sum[i-1];
  sum += length;

  total = sum[length] - sum[-length];

  for (col = 0; col < width; col++)
    {
      picman_pixel_rgn_get_col (&src_rgn, src, col + x1, y1, (y2 - y1));

      if (has_alpha)
        multiply_alpha (src, height, bytes);

      sp = src;
      dp = dest;

      for (b = 0; b < bytes; b++)
        {
          initial_p = sp[b];
          initial_m = sp[(height-1) * bytes + b];

          /*  Determine a run-length encoded version of the row  */
          run_length_encode (sp + b, buf, bytes, height);

          for (row = 0; row < height; row++)
            {
              start = (row < length) ? -row : -length;
              end = (height <= (row + length) ?
                     (height - row - 1) : length);

              val = 0;
              i = start;
              bb = buf + (row + i) * 2;

              if (start != -length)
                val += initial_p * (sum[start] - sum[-length]);

              while (i < end)
                {
                  pixels = bb[0];
                  i += pixels;

                  if (i > end)
                    i = end;

                  val += bb[1] * (sum[i] - sum[start]);
                  bb += (pixels * 2);
                  start = i;
                }

              if (end != length)
                val += initial_m * (sum[length] - sum[end]);

              dp[row * bytes + b] = val / total;
            }
        }

      if (has_alpha)
        separate_alpha (dest, height, bytes);

      picman_pixel_rgn_set_col (&dest_rgn, dest, col + x1, y1, (y2 - y1));

      if (show_progress)
        {
          progress += height;

          if ((col % 32) == 0)
            picman_progress_update (0.5 * (pass + (progress / max_progress)));
        }
    }

  /*  prepare for the horizontal pass  */
  picman_pixel_rgn_init (&src_rgn,
                       drawable, 0, 0, drawable->width, drawable->height,
                       FALSE, TRUE);

  /*  Now the horizontal pass  */
  for (row = 0; row < height; row++)
    {
      picman_pixel_rgn_get_row (&src_rgn, src, x1, row + y1, (x2 - x1));
      if (has_alpha)
        multiply_alpha (src, width, bytes);

      sp = src;
      dp = dest;

      for (b = 0; b < bytes; b++)
        {
          initial_p = sp[b];
          initial_m = sp[(width-1) * bytes + b];

          /*  Determine a run-length encoded version of the row  */
          run_length_encode (sp + b, buf, bytes, width);

          for (col = 0; col < width; col++)
            {
              start = (col < length) ? -col : -length;
              end = (width <= (col + length)) ? (width - col - 1) : length;

              val = 0;
              i = start;
              bb = buf + (col + i) * 2;

              if (start != -length)
                val += initial_p * (sum[start] - sum[-length]);

              while (i < end)
                {
                  pixels = bb[0];
                  i += pixels;

                  if (i > end)
                    i = end;

                  val += bb[1] * (sum[i] - sum[start]);
                  bb += (pixels * 2);
                  start = i;
                }

              if (end != length)
                val += initial_m * (sum[length] - sum[end]);

              dp[col * bytes + b] = val / total;
            }
        }

      if (has_alpha)
        separate_alpha (dest, width, bytes);

      picman_pixel_rgn_set_row (&dest_rgn, dest, x1, row + y1, (x2 - x1));

      if (show_progress)
        {
          progress += width;

          if ((row % 32) == 0)
            picman_progress_update (0.5 * (pass + (progress / max_progress)));
        }
    }

  /*  merge the shadow, update the drawable  */
  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id, x1, y1, (x2 - x1), (y2 - y1));

  /*  free buffers  */
  g_free (buf);
  g_free (src);
  g_free (dest);
}

/*
 * The equations: g(r) = exp (- r^2 / (2 * sigma^2))
 *                   r = sqrt (x^2 + y ^2)
 */

static gint *
make_curve (gdouble  sigma,
            gint    *length)
{
  gint    *curve;
  gdouble  sigma2;
  gdouble  l;
  gint     temp;
  gint     i, n;

  sigma2 = 2 * sigma * sigma;
  l = sqrt (-sigma2 * log (1.0 / 255.0));

  n = ceil (l) * 2;
  if ((n % 2) == 0)
    n += 1;

  curve = g_new (gint, n);

  *length = n / 2;
  curve += *length;
  curve[0] = 255;

  for (i = 1; i <= *length; i++)
    {
      temp = (gint) (exp (- (i * i) / sigma2) * 255);
      curve[-i] = temp;
      curve[i] = temp;
    }

  return curve;
}

static void
run_length_encode (guchar *src,
                   gint   *dest,
                   gint    bytes,
                   gint    width)
{
  gint start;
  gint i;
  gint j;
  guchar last;

  last = *src;
  src += bytes;
  start = 0;

  for (i = 1; i < width; i++)
    {
      if (*src != last)
        {
          for (j = start; j < i; j++)
            {
              *dest++ = (i - j);
              *dest++ = last;
            }
          start = i;
          last = *src;
        }
      src += bytes;
    }

  for (j = start; j < i; j++)
    {
      *dest++ = (i - j);
      *dest++ = last;
    }
}

static void
preview_update_preview (PicmanPreview  *preview,
                        PicmanDrawable *drawable)
{
  gint          x1, y1;
  gint          width, height;
  gint          bpp;
  guchar       *buffer;
  PicmanPixelRgn  src_rgn;
  PicmanPixelRgn  preview_rgn;
  gint32        image_id, src_image_id;
  gint32        preview_id;
  PicmanDrawable *preview_drawable;

  bpp = picman_drawable_bpp (drawable->drawable_id);

  picman_preview_get_position (preview, &x1, &y1);
  picman_preview_get_size (preview, &width, &height);

  buffer = g_new (guchar, width * height * bpp);

  picman_pixel_rgn_init (&src_rgn, drawable,
                       x1, y1, width, height, FALSE, FALSE);
  picman_pixel_rgn_get_rect (&src_rgn, buffer,
                           x1, y1, width, height);

  /* set up picman drawable for rendering preview into */
  src_image_id = picman_item_get_image (drawable->drawable_id);
  image_id = picman_image_new (width, height,
                             picman_image_base_type (src_image_id));
  preview_id = picman_layer_new (image_id, "preview", width, height,
                               picman_drawable_type (drawable->drawable_id),
                               100,
                               PICMAN_NORMAL_MODE);
  preview_drawable = picman_drawable_get (preview_id);
  picman_image_insert_layer (image_id, preview_id, -1, 0);
  picman_layer_set_offsets (preview_id, 0, 0);
  picman_pixel_rgn_init (&preview_rgn, preview_drawable,
                       0, 0, width, height, TRUE, TRUE);
  picman_pixel_rgn_set_rect (&preview_rgn, buffer,
                           0, 0, width, height);
  picman_drawable_flush (preview_drawable);
  picman_drawable_merge_shadow (preview_id, TRUE);
  picman_drawable_update (preview_id, 0, 0, width, height);

  dog (image_id, preview_drawable, dogvals.inner, dogvals.outer, FALSE);

  picman_pixel_rgn_get_rect (&preview_rgn, buffer,
                           0, 0, width, height);

  picman_preview_draw_buffer (preview, buffer, width * bpp);

  picman_image_delete (image_id);
  g_free (buffer);
}

static void
change_radius_callback (GtkWidget *coord,
                        gpointer   data)
{
  PicmanPreview *preview = PICMAN_PREVIEW (data);

  dogvals.inner = picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (coord), 0);
  dogvals.outer = picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (coord), 1);

  picman_preview_invalidate (preview);
}
