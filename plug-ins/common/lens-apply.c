/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Apply lens plug-in --- makes your selected part of the image look like it
 *                        is viewed under a solid lens.
 * Copyright (C) 1997 Morten Eriksen
 * mortene@pvv.ntnu.no
 * (If you do anything cool with this plug-in, or have ideas for
 * improvements (which aren't on my ToDo-list) - send me an email).
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
  TO DO:
  - antialiasing
  - adjustable (R, G, B and A) filter
  - optimize for speed!
  - refraction index warning dialog box when value < 1.0
  - use "true" lens with specified thickness
  - option to apply inverted lens
  - adjustable "c" value in the ellipsoid formula
  - radiobuttons for "ellipsoid" or "only horiz" and "only vert" (like in the
    Ad*b* Ph*t*sh*p Spherify plug-in..)
  - clean up source code
 */

#include "config.h"

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC   "plug-in-applylens"
#define PLUG_IN_BINARY "lens-apply"
#define PLUG_IN_ROLE   "picman-lens-apply"


/* Declare local functions.
 */
static void      query (void);
static void      run   (const gchar      *name,
                        gint              nparams,
                        const PicmanParam  *param,
                        gint             *nreturn_vals,
                        PicmanParam       **return_vals);

static void      drawlens    (PicmanDrawable *drawable,
                              PicmanPreview  *preview);
static gboolean  lens_dialog (PicmanDrawable *drawable);


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

typedef struct
{
  gdouble  refraction;
  gboolean keep_surr;
  gboolean use_bkgr;
  gboolean set_transparent;
} LensValues;

static LensValues lvals =
{
  /* Lens refraction value */
  1.7,
  /* Surroundings options */
  TRUE, FALSE, FALSE
};


MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",          "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",             "Input image (unused)" },
    { PICMAN_PDB_DRAWABLE, "drawable",          "Input drawable" },
    { PICMAN_PDB_FLOAT,    "refraction",        "Lens refraction index" },
    { PICMAN_PDB_INT32,    "keep-surroundings", "Keep lens surroundings { TRUE, FALSE }" },
    { PICMAN_PDB_INT32,    "set-background",    "Set lens surroundings to BG value { TRUE, FALSE }" },
    { PICMAN_PDB_INT32,    "set-transparent",   "Set lens surroundings transparent { TRUE, FALSE }" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Simulate an elliptical lens over the image"),
                          "This plug-in uses Snell's law to draw "
                          "an ellipsoid lens over the image",
                          "Morten Eriksen",
                          "Morten Eriksen",
                          "1997",
                          N_("Apply _Lens..."),
                          "RGB*, GRAY*, INDEXED*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC,
                             "<Image>/Filters/Distorts");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[1];
  PicmanDrawable      *drawable;
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;

  INIT_I18N ();

  run_mode = param[0].data.d_int32;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  *nreturn_vals = 1;
  *return_vals  = values;

  drawable = picman_drawable_get (param[2].data.d_drawable);

  switch(run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      picman_get_data (PLUG_IN_PROC, &lvals);
      if (!lens_dialog (drawable))
        return;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      if (nparams != 7)
        status = PICMAN_PDB_CALLING_ERROR;

      if (status == PICMAN_PDB_SUCCESS)
        {
          lvals.refraction      = param[3].data.d_float;
          lvals.keep_surr       = param[4].data.d_int32;
          lvals.use_bkgr        = param[5].data.d_int32;
          lvals.set_transparent = param[6].data.d_int32;
        }

      if (status == PICMAN_PDB_SUCCESS && (lvals.refraction < 1.0))
        status = PICMAN_PDB_CALLING_ERROR;
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      picman_get_data (PLUG_IN_PROC, &lvals);
      break;

    default:
      break;
    }

  picman_tile_cache_ntiles (2 * (drawable->width / picman_tile_width () + 1));
  picman_progress_init (_("Applying lens"));
  drawlens (drawable, NULL);

  if (run_mode != PICMAN_RUN_NONINTERACTIVE)
    picman_displays_flush ();
  if (run_mode == PICMAN_RUN_INTERACTIVE)
    picman_set_data (PLUG_IN_PROC, &lvals, sizeof (LensValues));

  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

/*
  Ellipsoid formula: x^2/a^2 + y^2/b^2 + z^2/c^2 = 1
 */
static void
find_projected_pos (gfloat  a2,
                    gfloat  b2,
                    gfloat  c2,
                    gfloat  x,
                    gfloat  y,
                    gfloat *projx,
                    gfloat *projy)
{
  gfloat z;
  gfloat nxangle, nyangle, theta1, theta2;
  gfloat ri1 = 1.0;
  gfloat ri2 = lvals.refraction;

  z = sqrt ((1 - x * x / a2 - y * y / b2) * c2);

  nxangle = acos (x / sqrt(x * x + z * z));
  theta1 = G_PI / 2 - nxangle;
  theta2 = asin (sin (theta1) * ri1 / ri2);
  theta2 = G_PI / 2 - nxangle - theta2;
  *projx = x - tan (theta2) * z;

  nyangle = acos (y / sqrt (y * y + z * z));
  theta1 = G_PI / 2 - nyangle;
  theta2 = asin (sin (theta1) * ri1 / ri2);
  theta2 = G_PI / 2 - nyangle - theta2;
  *projy = y - tan (theta2) * z;
}

static void
drawlens (PicmanDrawable *drawable,
          PicmanPreview  *preview)
{
  PicmanImageType  drawtype = picman_drawable_type (drawable->drawable_id);
  PicmanPixelRgn   srcPR, destPR;
  gint           width, height;
  gint           bytes;
  gint           row;
  gint           x1, y1, x2, y2;
  guchar        *src, *dest;
  gint           i, col;
  gfloat         regionwidth, regionheight, dx, dy, xsqr, ysqr;
  gfloat         a, b, c, asqr, bsqr, csqr, x, y;
  glong          pixelpos, pos;
  PicmanRGB        background;
  guchar         bgr_red, bgr_blue, bgr_green;
  guchar         alphaval;

  picman_context_get_background (&background);
  picman_rgb_get_uchar (&background,
                      &bgr_red, &bgr_green, &bgr_blue);

  bytes = drawable->bpp;

  if (preview)
    {
      picman_preview_get_position (preview, &x1, &y1);
      picman_preview_get_size (preview, &width, &height);
      x2 = x1 + width;
      y2 = y1 + height;
      src = picman_drawable_get_thumbnail_data (drawable->drawable_id,
                                              &width, &height, &bytes);
      regionwidth  = width;
      regionheight = height;
    }
  else
    {
      picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);
      regionwidth = x2 - x1;
      regionheight = y2 - y1;
      width = drawable->width;
      height = drawable->height;
      picman_pixel_rgn_init (&srcPR, drawable,
                           0, 0, width, height, FALSE, FALSE);
      picman_pixel_rgn_init (&destPR, drawable,
                           0, 0, width, height, TRUE, TRUE);

      src  = g_new (guchar, regionwidth * regionheight * bytes);
      picman_pixel_rgn_get_rect (&srcPR, src,
                               x1, y1, regionwidth, regionheight);
    }

  dest = g_new (guchar, regionwidth * regionheight * bytes);

  a = regionwidth / 2;
  b = regionheight / 2;

  c = MIN (a, b);

  asqr = a * a;
  bsqr = b * b;
  csqr = c * c;

  for (col = 0; col < regionwidth; col++)
    {
      dx = (gfloat) col - a + 0.5;
      xsqr = dx * dx;
      for (row = 0; row < regionheight; row++)
        {
          pixelpos = (col + row * regionwidth) * bytes;
          dy = -((gfloat) row - b) - 0.5;
          ysqr = dy * dy;
          if (ysqr < (bsqr - (bsqr * xsqr) / asqr))
            {
              find_projected_pos (asqr, bsqr, csqr, dx, dy, &x, &y);
              y = -y;
              pos = ((gint) (y + b) * regionwidth + (gint) (x + a)) * bytes;

              for (i = 0; i < bytes; i++)
                {
                  dest[pixelpos + i] = src[pos + i];
                }
            }
          else
            {
              if (lvals.keep_surr)
                {
                  for (i = 0; i < bytes; i++)
                    {
                      dest[pixelpos + i] = src[pixelpos + i];
                    }
                }
              else
                {
                  if (lvals.set_transparent)
                    alphaval = 0;
                  else
                    alphaval = 255;

                  switch (drawtype)
                    {
                    case PICMAN_INDEXEDA_IMAGE:
                      dest[pixelpos + 1] = alphaval;
                    case PICMAN_INDEXED_IMAGE:
                      dest[pixelpos + 0] = 0;
                      break;

                    case PICMAN_RGBA_IMAGE:
                      dest[pixelpos + 3] = alphaval;
                    case PICMAN_RGB_IMAGE:
                      dest[pixelpos + 0] = bgr_red;
                      dest[pixelpos + 1] = bgr_green;
                      dest[pixelpos + 2] = bgr_blue;
                      break;

                    case PICMAN_GRAYA_IMAGE:
                      dest[pixelpos + 1] = alphaval;
                    case PICMAN_GRAY_IMAGE:
                      dest[pixelpos+0] = bgr_red;
                      break;
                    }
                }
            }
        }

      if (!preview)
        {
          if (((gint) (regionwidth-col) % 5) == 0)
            picman_progress_update ((gdouble) col / (gdouble) regionwidth);
        }
    }

  if (preview)
    {
      picman_preview_draw_buffer (preview, dest, bytes * regionwidth);
    }
  else
    {
      picman_progress_update (1.0);
      picman_pixel_rgn_set_rect (&destPR, dest, x1, y1,
                               regionwidth, regionheight);

      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
      picman_drawable_update (drawable->drawable_id, x1, y1, x2 - x1, y2 - y1);
    }

  g_free (src);
  g_free (dest);
}

static gboolean
lens_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *label;
  GtkWidget *toggle;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *spinbutton;
  GtkObject *adj;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Lens Effect"), PLUG_IN_ROLE,
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

  preview = picman_aspect_preview_new (drawable, NULL);
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);

  g_signal_connect_swapped (preview, "invalidated",
                            G_CALLBACK (drawlens),
                            drawable);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  toggle = gtk_radio_button_new_with_mnemonic_from_widget
    (NULL, _("_Keep original surroundings"));
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), lvals.keep_surr);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &lvals.keep_surr);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  toggle = gtk_radio_button_new_with_mnemonic_from_widget
    (GTK_RADIO_BUTTON (toggle),
     picman_drawable_is_indexed (drawable->drawable_id)
     ? _("_Set surroundings to index 0")
     : _("_Set surroundings to background color"));
  gtk_box_pack_start(GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), lvals.use_bkgr);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &lvals.use_bkgr);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  if (picman_drawable_has_alpha (drawable->drawable_id))
    {
      toggle = gtk_radio_button_new_with_mnemonic_from_widget
        (GTK_RADIO_BUTTON (toggle), _("_Make surroundings transparent"));
      gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                    lvals.set_transparent);
      gtk_widget_show (toggle);

      g_signal_connect (toggle, "toggled",
                        G_CALLBACK (picman_toggle_button_update),
                        &lvals.set_transparent);
      g_signal_connect_swapped (toggle, "toggled",
                                G_CALLBACK (picman_preview_invalidate),
                                preview);
  }

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic (_("_Lens refraction index:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  spinbutton = picman_spin_button_new (&adj, lvals.refraction,
                                     1.0, 100.0, 0.1, 1.0, 0, 1, 2);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), spinbutton);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &lvals.refraction);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_widget_show (hbox);
  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}
