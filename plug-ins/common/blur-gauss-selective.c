/* Selective gaussian blur filter for PICMAN, version 0.1
 * Adapted from the original gaussian blur filter by Spencer Kimball and
 * Peter Mattis.
 *
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Copyright (C) 1999 Thom van Os <thom@vanos.com>
 * Copyright (C) 2006 Loren Merritt
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
 *
 * To do:
 *      - support for horizontal or vertical only blur
 *      - use memory more efficiently, smaller regions at a time
 *      - integrating with other convolution matrix based filters ?
 *      - create more selective and adaptive filters
 *      - threading
 *      - optimization
 */

#include "config.h"

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC   "plug-in-sel-gauss"
#define PLUG_IN_BINARY "blur-gauss-selective"
#define PLUG_IN_ROLE   "picman-blur-gauss-selective"

#ifndef ALWAYS_INLINE
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#    define ALWAYS_INLINE inline
#endif
#endif


typedef struct
{
  gdouble  radius;
  gint     maxdelta;
} BlurValues;


/* Declare local functions.
 */
static void      query            (void);
static void      run              (const gchar      *name,
                                   gint              nparams,
                                   const PicmanParam  *param,
                                   gint             *nreturn_vals,
                                   PicmanParam       **return_vals);

static void      sel_gauss        (PicmanDrawable     *drawable,
                                   gdouble           radius,
                                   gint              maxdelta);
static gboolean  sel_gauss_dialog (PicmanDrawable     *drawable);
static void      preview_update   (PicmanPreview      *preview);


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static BlurValues bvals =
{
  5.0, /* radius   */
  50   /* maxdelta */
};

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",  "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",     "Input image (unused)"         },
    { PICMAN_PDB_DRAWABLE, "drawable",  "Input drawable"               },
    { PICMAN_PDB_FLOAT,    "radius",    "Radius of gaussian blur (in pixels, > 0.0)" },
    { PICMAN_PDB_INT32,    "max-delta", "Maximum delta"                }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Blur neighboring pixels, but only in low-contrast areas"),
                          "This filter functions similar to the regular "
                          "gaussian blur filter except that neighbouring "
                          "pixels that differ more than the given maxdelta "
                          "parameter will not be blended with. This way with "
                          "the correct parameters, an image can be smoothed "
                          "out without losing details. However, this filter "
                          "can be rather slow.",
                          "Thom van Os",
                          "Thom van Os",
                          "1999",
                          N_("_Selective Gaussian Blur..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Blur");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[1];
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  PicmanDrawable      *drawable;
  gdouble            radius;

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;

  INIT_I18N ();

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  /* Get the specified drawable */
  drawable = picman_drawable_get (param[2].data.d_drawable);
  picman_tile_cache_ntiles (2 * drawable->ntile_cols);

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /* Possibly retrieve data */
      picman_get_data (PLUG_IN_PROC, &bvals);

      /* First acquire information with a dialog */
      if (! sel_gauss_dialog (drawable))
        return;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /* Make sure all the arguments are there! */
      if (nparams != 5)
        status = PICMAN_PDB_CALLING_ERROR;
      if (status == PICMAN_PDB_SUCCESS)
        {
          bvals.radius   = param[3].data.d_float;
          bvals.maxdelta = CLAMP (param[4].data.d_int32, 0, 255);

          if (bvals.radius <= 0.0)
            status = PICMAN_PDB_CALLING_ERROR;
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /* Possibly retrieve data */
      picman_get_data (PLUG_IN_PROC, &bvals);
      break;

    default:
      break;
    }

  if (status != PICMAN_PDB_SUCCESS)
    {
      values[0].data.d_status = status;
      return;
    }

  /* Make sure that the drawable is gray or RGB color */
  if (picman_drawable_is_rgb (drawable->drawable_id) ||
      picman_drawable_is_gray (drawable->drawable_id))
    {
      picman_progress_init (_("Selective Gaussian Blur"));

      radius = fabs (bvals.radius) + 1.0;

      /* run the gaussian blur */
      sel_gauss (drawable, radius, bvals.maxdelta);

      /* Store data */
      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (PLUG_IN_PROC, &bvals, sizeof (BlurValues));

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();
    }
  else
    {
      picman_message (_("Cannot operate on indexed color images."));
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  picman_drawable_detach (drawable);
  values[0].data.d_status = status;
}

static gboolean
sel_gauss_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *table;
  GtkWidget *spinbutton;
  GtkObject *adj;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Selective Gaussian Blur"), PLUG_IN_ROLE,
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
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);

  g_signal_connect (preview, "invalidated",
                    G_CALLBACK (preview_update),
                    NULL);

  table = gtk_table_new (2, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  spinbutton = picman_spin_button_new (&adj,
                                     bvals.radius, 0.0, G_MAXINT, 1.0, 5.0,
                                     0, 1, 2);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("_Blur radius:"), 0.0, 0.5,
                             spinbutton, 1, TRUE);
  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &bvals.radius);
  g_signal_connect_swapped (spinbutton, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, 1,
                              _("_Max. delta:"), 128, 0,
                              bvals.maxdelta, 0, 255, 1, 8, 0,
                              TRUE, 0, 0,
                              NULL, NULL);
  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &bvals.maxdelta);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
init_matrix (gdouble  radius,
             gdouble *mat,
             gint     num)
{
  gint    dx;
  gdouble sd, c1, c2;

  /* This formula isn't really correct, but it'll do */
  sd = radius / 3.329042969;
  c1 = 1.0 / sqrt (2.0 * G_PI * sd);
  c2 = -2.0 * (sd * sd);

  for (dx = 0; dx < num; dx++)
    mat[dx] = c1 * exp ((dx * dx)/ c2);
}


#if defined(ARCH_X86) && defined(USE_MMX) && defined(__GNUC__)
#define HAVE_ACCEL 1

static ALWAYS_INLINE void
matrixmult_mmx (const guchar  *src,
                guchar        *dest,
                gint           width,
                gint           height,
                const gdouble *mat,
                gint           numrad,
                gint           bytes,
                gboolean       has_alpha,
                gint           maxdelta,
                gboolean       preview_mode)
{
  const gint       rowstride = width * bytes;
  const long long  maxdelta4 = maxdelta * 0x0001000100010001ULL;
  gushort         *imat;
  gdouble          fsum, fscale;
  gint             i, j, x, y, d;

  g_assert (has_alpha ? (bytes == 4) : (bytes == 3 || bytes == 1));

  imat = g_newa (gushort, 2 * numrad + 3);

  fsum = 0.0;
  for (y = 1 - numrad; y < numrad; y++)
    fsum += mat[ABS(y)];

  /* Ensure that one pixel's product fits in 16bits,
   * and that the sum fits in 32bits.
   */
  fscale = MIN (0x100 / mat[0], 0x1000 / fsum);
  for (y = 0; y < numrad; y++)
    imat[numrad - y] = imat[numrad + y] = mat[y] * fscale;

  for (y = numrad; y < numrad + 3; y++)
    imat[numrad + y] = 0;

  for (y = 0; y < height; y++)
    {
      asm volatile (
        "pxor  %%mm7, %%mm7 \n\t":
      );

      for (x = 0; x < width; x++)
        {
          guint r, g, b, fr, fg, fb;
          gint  offset;
          gint  dix;

          r = g = b = fr = fg = fb = 0;

          dix = bytes * (width * y + x);

          if (has_alpha)
            {
              *(guint*) &dest[dix] = *(guint*) &src[dix];

              if (!src[dix + 3])
                continue;
            }

          asm volatile (
            "movd         %0, %%mm6 \n\t"
            "punpcklbw %%mm7, %%mm6 \n\t" /* center pixel */
            :: "m"(src[dix])
          );

          offset = rowstride * (y - numrad) + bytes * (x - numrad);

          if (bytes == 1)
            {
              asm volatile (
                "pshufw $0, %%mm6, %%mm6 \n\t": /* center pixel x4 */
              );
              for (j = 1 - numrad; j < numrad; j++)
                {
                  const guchar *src_b;
                  guint         rowsum  = 0;
                  guint         rowfact = 0;

                  offset += rowstride;

                  if (y + j < 0 || y + j >= height)
                    continue;

                  src_b = src + offset - 3;

                  asm volatile (
                    "pxor  %%mm5, %%mm5 \n\t" /* row fact */
                    "pxor  %%mm4, %%mm4 \n\t" /* row sum  */
                    :
                  );

                  for (i = 1 - numrad; i < numrad; i += 4)
                    {
                      src_b += 4;
                      if (x + i < 0 || x + i >= width)
                        continue;

                      asm volatile (
                        "movd         %0, %%mm0 \n\t"
                        "movq      %%mm6, %%mm1 \n\t"
                        "punpcklbw %%mm7, %%mm0 \n\t" /* one pixel      */
                        "psubusw   %%mm0, %%mm1 \n\t" /* diff           */
                        "movq      %%mm0, %%mm2 \n\t"
                        "psubusw   %%mm6, %%mm2 \n\t"
                        "por       %%mm2, %%mm1 \n\t" /* abs diff       */
                        "pcmpgtw      %1, %%mm1 \n\t" /* threshold      */
                        "pandn        %2, %%mm1 \n\t" /* weight         */
                        "pmullw    %%mm1, %%mm0 \n\t" /* pixel * weight */
                        "paddusw   %%mm1, %%mm5 \n\t" /* fact           */
                        "movq      %%mm0, %%mm2 \n\t"
                        "punpcklwd %%mm7, %%mm0 \n\t"
                        "punpckhwd %%mm7, %%mm2 \n\t"
                        "paddd     %%mm0, %%mm4 \n\t"
                        "paddd     %%mm2, %%mm4 \n\t" /* sum            */
                        :: "m"(*src_b), "m"(maxdelta4), "m"(imat[numrad + i])
                      );
                    }

                  asm volatile (
                    "pshufw $0xb1, %%mm5, %%mm3 \n\t"
                    "paddusw       %%mm3, %%mm5 \n\t"
                    "pshufw $0x0e, %%mm4, %%mm2 \n\t"
                    "pshufw $0x0e, %%mm5, %%mm3 \n\t"
                    "paddd         %%mm2, %%mm4 \n\t"
                    "paddusw       %%mm3, %%mm5 \n\t"
                    "movd          %%mm4, %0    \n\t"
                    "movd          %%mm5, %1    \n\t"
                    :"=g"(rowsum), "=g"(rowfact)
                  );
                  d = imat[numrad + j];
                  r += d * rowsum;
                  fr += d * (gushort) rowfact;
                }

              if (fr)
                dest[dix] = r / fr;
            }
          else
            {
              for (j = 1 - numrad; j < numrad; j++)
                {
                  const guchar *src_b;
                  gushort       rf[4];
                  guint         rr, rg, rb;

                  offset += rowstride;
                  if (y + j < 0 || y + j >= height)
                    continue;

                  src_b = src + offset;

                  asm volatile (
                    "pxor  %%mm5, %%mm5 \n\t" /* row fact   */
                    "pxor  %%mm4, %%mm4 \n\t" /* row sum RG */
                    "pxor  %%mm3, %%mm3 \n\t" /* row sum B  */
                    :
                  );

                  for (i = 1 - numrad; i < numrad; i++)
                    {
                      src_b += bytes;
                      if (x + i < 0 || x + i >= width)
                        continue;

                      if (has_alpha)
                        asm volatile (
                          "movd         %0, %%mm0 \n\t"
                          "movq      %%mm6, %%mm1 \n\t"
                          "punpcklbw %%mm7, %%mm0 \n\t" /* one pixel       */
                          "psubusw   %%mm0, %%mm1 \n\t" /* diff            */
                          "movq      %%mm0, %%mm2 \n\t"
                          "psubusw   %%mm6, %%mm2 \n\t"
                          "por       %%mm2, %%mm1 \n\t" /* abs diff        */
                          "pcmpgtw      %1, %%mm1 \n\t" /* threshold       */
                          "pshufw   $0, %2, %%mm2 \n\t" /* weight          */
                          "pandn     %%mm2, %%mm1 \n\t"
                          "pshufw $0xff, %%mm0, %%mm2 \n\t" /* alpha       */
                          "psllw        $8, %%mm2 \n\t"
                          "pmulhuw   %%mm2, %%mm1 \n\t" /* weight *= alpha */
                          "pmullw    %%mm1, %%mm0 \n\t" /* pixel * weight  */
                          "paddusw   %%mm1, %%mm5 \n\t" /* fact            */
                          "movq      %%mm0, %%mm2 \n\t"
                          "punpcklwd %%mm7, %%mm0 \n\t" /* RG              */
                          "punpckhwd %%mm7, %%mm2 \n\t" /* B               */
                          "paddd     %%mm0, %%mm4 \n\t"
                          "paddd     %%mm2, %%mm3 \n\t"
                          :: "m"(*src_b), "m"(maxdelta4), "m"(imat[numrad + i])
                        );
                      else
                        asm volatile (
                          "movd         %0, %%mm0 \n\t"
                          "movq      %%mm6, %%mm1 \n\t"
                          "punpcklbw %%mm7, %%mm0 \n\t" /* one pixel       */
                          "psubusw   %%mm0, %%mm1 \n\t" /* diff            */
                          "movq      %%mm0, %%mm2 \n\t"
                          "psubusw   %%mm6, %%mm2 \n\t"
                          "por       %%mm2, %%mm1 \n\t" /* abs diff        */
                          "pcmpgtw      %1, %%mm1 \n\t" /* threshold       */
                          "pshufw   $0, %2, %%mm2 \n\t" /* weight          */
                          "pandn     %%mm2, %%mm1 \n\t"
                          "pmullw    %%mm1, %%mm0 \n\t" /* pixel * weight  */
                          "paddusw   %%mm1, %%mm5 \n\t" /* fact            */
                          "movq      %%mm0, %%mm2 \n\t"
                          "punpcklwd %%mm7, %%mm0 \n\t" /* RG              */
                          "punpckhwd %%mm7, %%mm2 \n\t" /* B               */
                          "paddd     %%mm0, %%mm4 \n\t"
                          "paddd     %%mm2, %%mm3 \n\t"
                          :: "m"(*src_b), "m"(maxdelta4), "m"(imat[numrad + i])
                        );
                    }

                  asm volatile (
                    "movd    %%mm4, %0 \n\t"
                    "movd    %%mm3, %2 \n\t"
                    "psrlq  $32, %%mm4 \n\t"
                    "movq    %%mm5, %3 \n\t"
                    "movd    %%mm4, %1 \n\t"
                    :"=g"(rr), "=g"(rg), "=g"(rb), "=m"(*rf)
                    ::"memory"
                  );
                  d = imat[numrad + j];
                  r += d * rr;
                  g += d * rg;
                  b += d * rb;
                  fr += d * rf[0];
                  fg += d * rf[1];
                  fb += d * rf[2];
                }

              if (has_alpha)
                {
                  if (fr)
                    dest[dix+0] = r / fr;
                  if (fg)
                    dest[dix+1] = g / fg;
                  if (fb)
                    dest[dix+2] = b / fb;
                }
              else
                {
                  dest[dix+0] = r / fr;
                  dest[dix+1] = g / fg;
                  dest[dix+2] = b / fb;
                }
            }
        }

      if (!(y % 16) && !preview_mode)
        {
          asm volatile ("emms");
          picman_progress_update ((gdouble) y / (gdouble) height);
        }
    }

  asm volatile ("emms");
}
#endif /* ARCH_X86 && USE_MMX && __GNUC__ */


static ALWAYS_INLINE void
matrixmult_int (const guchar  *src,
                guchar        *dest,
                gint           width,
                gint           height,
                const gdouble *mat,
                gint           numrad,
                gint           bytes,
                gboolean       has_alpha,
                gint           maxdelta,
                gboolean       preview_mode)
{
  const gint  nb        = bytes - (has_alpha ? 1 : 0);
  const gint  rowstride = width * bytes;
  gushort    *imat;
  gdouble     fsum, fscale;
  gint        i, j, b, x, y, d;

#ifdef HAVE_ACCEL
  if (has_alpha ? (bytes == 4) : (bytes == 3 || bytes == 1))
    {
      PicmanCpuAccelFlags cpu = picman_cpu_accel_get_support ();

      if (cpu & (PICMAN_CPU_ACCEL_X86_MMXEXT | PICMAN_CPU_ACCEL_X86_SSE))
        return matrixmult_mmx (src, dest, width, height, mat, numrad,
                               bytes, has_alpha, maxdelta, preview_mode);
    }
#endif

  imat = g_newa (gushort, 2 * numrad);

  fsum = 0.0;
  for (y = 1 - numrad; y < numrad; y++)
    fsum += mat[ABS(y)];

  /* Ensure that the sum fits in 32bits. */
  fscale = 0x1000 / fsum;
  for (y = 0; y < numrad; y++)
    imat[numrad - y] = imat[numrad + y] = mat[y] * fscale;

  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          gint dix = bytes * (width * y + x);

          if (has_alpha)
            dest[dix + nb] = src[dix + nb];

          for (b = 0; b < nb; b++)
            {
              const guchar *src_db = src + dix + b;
              guint         sum    = 0;
              guint         fact   = 0;
              gint          offset;

              offset = rowstride * (y - numrad) + bytes * (x - numrad);

              for (j = 1 - numrad; j < numrad; j++)
                {
                  const guchar *src_b;
                  guint         rowsum  = 0;
                  guint         rowfact = 0;

                  offset += rowstride;
                  if (y + j < 0 || y + j >= height)
                    continue;

                  src_b = src + offset + b;

                  for (i = 1 - numrad; i < numrad; i++)
                    {
                      gint tmp;

                      src_b += bytes;

                      if (x + i < 0 || x + i >= width)
                        continue;

                      tmp = *src_db - *src_b;
                      if (tmp > maxdelta || tmp < -maxdelta)
                        continue;

                      d = imat[numrad+i];
                      if (has_alpha)
                        d *= src_b[nb - b];

                      rowsum += d * *src_b;
                      rowfact += d;
                    }

                  d = imat[numrad+j];

                  if (has_alpha)
                    {
                      rowsum >>= 8;
                      rowfact >>= 8;
                    }

                  sum += d * rowsum;
                  fact += d * rowfact;
                }

              if (fact == 0)
                dest[dix + b] = *src_db;
              else
                dest[dix + b] = sum / fact;
            }
        }

      if (!(y % 16) && !preview_mode)
        picman_progress_update ((gdouble) y / (gdouble) height);
    }
}

/* Force compilation of several versions with inlined constants. */
static void
matrixmult (const guchar  *src,
            guchar        *dest,
            gint           width,
            gint           height,
            const gdouble *mat,
            gint           numrad,
            gint           bytes,
            gboolean       has_alpha,
            gint           maxdelta,
            gboolean       preview_mode)
{
  has_alpha = has_alpha ? 1 : 0;

#define EXPAND(BYTES, ALPHA)\
  if (bytes == BYTES && has_alpha == ALPHA)\
    {\
      matrixmult_int (src, dest, width, height, mat, numrad,\
                      BYTES, ALPHA, maxdelta, preview_mode);\
      return;\
    }

  EXPAND (1, 0)
  EXPAND (2, 1)
  EXPAND (3, 0)
  EXPAND (4, 1)
  EXPAND (bytes, has_alpha)
#undef EXPAND
}

static void
sel_gauss (PicmanDrawable *drawable,
           gdouble       radius,
           gint          maxdelta)
{
  PicmanPixelRgn src_rgn, dest_rgn;
  gint         bytes;
  gboolean     has_alpha;
  guchar      *dest;
  guchar      *src;
  gint         x, y, x2, y2;
  gint         width, height;
  gdouble     *mat;
  gint         numrad;

  if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                      &x, &y, &width, &height))
    return;

  bytes     = drawable->bpp;
  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);

  numrad = (gint) (radius + 1.0);
  mat = g_new (gdouble, numrad);
  init_matrix (radius, mat, numrad);

  x2 = MIN (x + width - 1 + numrad, drawable->width);
  y2 = MIN (y + height - 1 + numrad, drawable->height);

  x = MAX (x - numrad + 1, 0);
  y = MAX (y - numrad + 1, 0);
  width = x2 - x;
  height = y2 - y;

  /*  allocate with extra padding because MMX instructions may read
      more than strictly necessary  */
  src  = g_new (guchar, width * height * bytes + 16);
  dest = g_new (guchar, width * height * bytes);

  picman_pixel_rgn_init (&src_rgn,
                       drawable, x, y, width, height, FALSE, FALSE);
  picman_pixel_rgn_get_rect (&src_rgn, src, x, y, width, height);

  matrixmult (src, dest, width, height, mat, numrad,
              bytes, has_alpha, maxdelta, FALSE);
  picman_progress_update (1.0);

  picman_pixel_rgn_init (&dest_rgn,
                       drawable, x, y, width, height, TRUE, TRUE);
  picman_pixel_rgn_set_rect (&dest_rgn, dest, x, y, width, height);

  /*  merge the shadow, update the drawable  */
  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id, x, y, width, height);

  /* free up buffers */
  g_free (src);
  g_free (dest);
  g_free (mat);
}

static void
preview_update (PicmanPreview *preview)
{
  PicmanDrawable  *drawable;
  glong          bytes;
  gint           x, y;
  guchar        *render_buffer;  /* Buffer to hold rendered image */
  gint           width;          /* Width of preview widget */
  gint           height;         /* Height of preview widget */

  PicmanPixelRgn   srcPR;           /* Pixel region */
  guchar        *src;
  gboolean       has_alpha;
  gint           numrad;
  gdouble       *mat;
  gdouble       radius;

  /* Get drawable info */
  drawable =
    picman_drawable_preview_get_drawable (PICMAN_DRAWABLE_PREVIEW (preview));
  bytes = drawable->bpp;

  /*
   * Setup for filter...
   */
  picman_preview_get_position (preview, &x, &y);
  picman_preview_get_size (preview, &width, &height);

  /* initialize pixel regions */
  picman_pixel_rgn_init (&srcPR, drawable,
                       x, y, width, height,
                       FALSE, FALSE);
  render_buffer = g_new (guchar, width * height * bytes);

  /*  allocate with extra padding because MMX instructions may read
      more than strictly necessary  */
  src = g_new (guchar, width * height * bytes + 16);

  /* render image */
  picman_pixel_rgn_get_rect (&srcPR, src, x, y, width, height);
  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);

  radius = fabs (bvals.radius) + 1.0;
  numrad = (gint) (radius + 1.0);

  mat = g_new (gdouble, numrad);
  init_matrix (radius, mat, numrad);

  matrixmult (src, render_buffer,
              width, height,
              mat, numrad,
              bytes, has_alpha, bvals.maxdelta, TRUE);

  g_free (mat);
  g_free (src);

  /*
   * Draw the preview image on the screen...
   */
  picman_preview_draw_buffer (preview, render_buffer, width * bytes);

  g_free (render_buffer);
}
