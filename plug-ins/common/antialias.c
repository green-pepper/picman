/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Auntie Alias 0.92 --- image filter plug-in
 *
 * Copyright (C) 2005 Adam D. Moss (adam@picman.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this plug-in (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* This plugin performs a pseudo-antialiasing effect on hard-edged source
 * material.  It does this by performing a 'clever' edge extrapolation for
 * each pixel which is then resampled back to a single pixel for output.
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


/*  Constants  */

#define PLUG_IN_PROC "plug-in-antialias"


/*  Local function prototypes  */

static void   query  (void);
static void   run    (const gchar      *name,
                      gint              nparams,
                      const PicmanParam  *param,
                      gint             *nreturn_vals,
                      PicmanParam       **return_vals);
static void   render (PicmanDrawable     *drawable);


/*  Local variables  */

PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};


MAIN ()


static void
query (void)
{
  static PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode", "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",    "Input image"                  },
    { PICMAN_PDB_DRAWABLE, "drawable", "Input drawable"               }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Antialias using the Scale3X edge-extrapolation "
                             "algorithm"),
                          "Help - write me",
                          "Adam D. Moss <adam@picman.org>",
                          "Adam D. Moss <adam@picman.org>",
                          "2005",
                          N_("_Antialias"),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Enhance");
}

static void
run (const gchar      *name,
     gint              n_params,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[1];
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;

  INIT_I18N ();

  *nreturn_vals = 1;
  *return_vals  = values;

  run_mode = param[0].data.d_int32;

  if (strcmp (name, PLUG_IN_PROC) == 0)
    {
      switch (run_mode)
        {
        case PICMAN_RUN_NONINTERACTIVE:
          if (n_params != 3)
            status = PICMAN_PDB_CALLING_ERROR;
          break;

        case PICMAN_RUN_INTERACTIVE:
          break;

        case PICMAN_RUN_WITH_LAST_VALS:
          break;

        default:
          break;
        }
    }
  else
    {
      status = PICMAN_PDB_CALLING_ERROR;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      PicmanDrawable *drawable;

      drawable = picman_drawable_get (param[2].data.d_drawable);

      picman_progress_init (_("Antialiasing..."));
      picman_tile_cache_ntiles (2 * (drawable->width / picman_tile_width () + 1));

      render (drawable);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();

      picman_drawable_detach (drawable);
    }

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;
}

static int
extrapolate9 (const int bytes,
              unsigned char *E0, unsigned char *E1, unsigned char *E2,
              unsigned char *E3, unsigned char *E4, unsigned char *E5,
              unsigned char *E6, unsigned char *E7, unsigned char *E8,
              unsigned char *A, unsigned char *B, unsigned char *C,
              unsigned char *D, unsigned char *E, unsigned char *F,
              unsigned char *G, unsigned char *H, unsigned char *I)
{
#define PEQ(X,Y) (0==memcmp(X,Y,bytes))
#define PCPY(DST,SRC) do{memcpy(DST,SRC,bytes);}while(0)

  /* an implementation of the Scale3X edge-extrapolation algorithm */
  if ( (!PEQ(B,H)) && (!PEQ(D,F)) )
    {
      if (PEQ(D,B)) PCPY(E0,D); else PCPY(E0,E);
      if ((PEQ(D,B) && !PEQ(E,C)) || (PEQ(B,F) && !PEQ(E,A)))
        PCPY(E1,B); else PCPY(E1,E);
      if (PEQ(B,F)) PCPY(E2,F); else PCPY(E2,E);
      if ((PEQ(D,B) && !PEQ(E,G)) || (PEQ(D,H) && !PEQ(E,A)))
        PCPY(E3,D); else PCPY(E3,E);
      PCPY(E4,E);
      if ((PEQ(B,F) && !PEQ(E,I)) || (PEQ(H,F) && !PEQ(E,C)))
        PCPY(E5,F); else PCPY(E5,E);
      if (PEQ(D,H)) PCPY(E6,D); else PCPY(E6,E);
      if ((PEQ(D,H) && !PEQ(E,I)) || (PEQ(H,F) && !PEQ(E,G)))
        PCPY(E7,H); else PCPY(E7,E);
      if (PEQ(H,F)) PCPY(E8,F); else PCPY(E8,E);
      return 1;
    }
  else
    {
      return 0;
    }

#undef PEQ
#undef PCPY
}

static void
render (PicmanDrawable *drawable)
{
  gint          width, height, bytes;
  gint          x, y, w, h;
  gint          row, col, b;
  PicmanPixelRgn  srcPR, destPR;
  guchar       *rowbefore;
  guchar       *rowthis;
  guchar       *rowafter;
  guchar       *dest;
  guchar       *ninepix;
  gboolean      has_alpha;
  guint         alpha;

  /* get bounds of working area */
  if (! picman_drawable_mask_intersect (drawable->drawable_id, &x, &y, &w, &h))
    return;

  width     = drawable->width;
  height    = drawable->height;
  bytes     = drawable->bpp;
  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
  alpha     = bytes - 1;

  picman_pixel_rgn_init (&srcPR, drawable, 0, 0, width, height, FALSE, FALSE);
  picman_pixel_rgn_init (&destPR, drawable, 0, 0, width, height, TRUE, TRUE);

  rowbefore  = g_new (guchar, (width + 2) * bytes);
  rowthis    = g_new (guchar, (width + 2) * bytes);
  rowafter   = g_new (guchar, (width + 2) * bytes);
  dest       = g_new (guchar, (width + 2) * bytes);
  ninepix    = g_new (guchar, 9 * bytes);

  picman_pixel_rgn_get_row (&srcPR, &rowthis[bytes], 0, y, width);
  memcpy (&rowthis[0], &rowthis[bytes], bytes);
  memcpy (&rowthis[(width+1)*bytes], &rowthis[(width)*bytes], bytes);
  memcpy (rowbefore, rowthis, (width+2)*bytes);
  memcpy (rowafter, rowthis, (width+2)*bytes);

  for (row = y; row < (y + h); ++row)
    {
      guchar *tmp;
      gint    srcrowafter = row + 1;

      if (srcrowafter >= (y + h - 1))
        srcrowafter = y + h - 1;

      /* rotate pointers */
      tmp       = rowbefore;
      rowbefore = rowthis;
      rowthis   = rowafter;
      rowafter  = tmp;

      /* populate new after-row */
      picman_pixel_rgn_get_row (&srcPR, &rowafter[bytes], 0, srcrowafter, width);
      memcpy (&rowafter[0], &rowafter[bytes], bytes);
      memcpy (&rowafter[(width + 1) * bytes], &rowafter[width * bytes], bytes);

      /* this macro returns the current pixel if it has some opacity. Otherwise
       * it returns the center pixel of the current 3x3 area. */
#define USE_IF_ALPHA(p) (((!has_alpha) || *((p)+alpha)) ? (p) : &rowthis[(col+1)*bytes])

      for (col = x; col < (x + w); ++col)
        {
          /* do 9x extrapolation pass */
          if (((!has_alpha) || (rowthis[(col+1)*bytes+alpha])) &&
              extrapolate9 (bytes,
                            &ninepix[0*bytes],
                            &ninepix[1*bytes],
                            &ninepix[2*bytes],
                            &ninepix[3*bytes],
                            &ninepix[4*bytes],
                            &ninepix[5*bytes],
                            &ninepix[6*bytes],
                            &ninepix[7*bytes],
                            &ninepix[8*bytes],
                            USE_IF_ALPHA (&rowbefore[(col+0)*bytes]),
                            USE_IF_ALPHA (&rowbefore[(col+1)*bytes]),
                            USE_IF_ALPHA (&rowbefore[(col+2)*bytes]),
                            USE_IF_ALPHA (&rowthis  [(col+0)*bytes]),
                                          &rowthis  [(col+1)*bytes],
                            USE_IF_ALPHA (&rowthis  [(col+2)*bytes]),
                            USE_IF_ALPHA (&rowafter [(col+0)*bytes]),
                            USE_IF_ALPHA (&rowafter [(col+1)*bytes]),
                            USE_IF_ALPHA (&rowafter [(col+2)*bytes])
                            ))
            {
              /* subsample results and put into dest */
              for (b = 0; b < bytes; ++b)
                {
                  dest[(col * bytes) + b] =
                    (3*ninepix[0*bytes+b]+5*ninepix[1*bytes+b]+3*ninepix[2*bytes+b]+
                     5*ninepix[3*bytes+b]+6*ninepix[4*bytes+b]+5*ninepix[5*bytes+b]+
                     3*ninepix[6*bytes+b]+5*ninepix[7*bytes+b]+3*ninepix[8*bytes+b]+
                     19) / 38;
                }
            }
          else
            {
              memcpy (&dest[col * bytes], &rowthis[(col + 1) * bytes], bytes);
            }
        }

#undef USE_IF_ALPHA

      /* write result row to dest */
      picman_pixel_rgn_set_row (&destPR, &dest[x * bytes],
                              x, row, w);
      if ((row & 31) == 0)
        picman_progress_update ((gdouble) row / (gdouble) h);
    }

  picman_progress_update (1.0);
  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id, x, y, w, h);

  g_free (rowbefore);
  g_free (rowthis);
  g_free (rowafter);
  g_free (dest);
  g_free (ninepix);
}
