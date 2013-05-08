/*
 * This is the FlareFX plug-in for PICMAN 0.99
 * Version 1.05
 *
 * Copyright (C) 1997-1998 Karl-Johan Andersson (t96kja@student.tdb.uu.se)
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
 */

/*
 * Please send any comments or suggestions to me,
 * Karl-Johan Andersson (t96kja@student.tdb.uu.se)
 *
 * TODO:
 * - add "streaks" from lightsource
 * - improve the user interface
 * - speed it up
 * - more flare types, more control (color, size, intensity...)
 *
 * Missing something? - please contact me!
 *
 * May 2000 - tim copperfield [timecop@japan.co.jp]
 * preview window now draws a "mini flarefx" to show approximate
 * positioning after final render.
 *
 * Note, the algorithm does not render into an alpha channel.
 * Therefore, changed RGB* to RGB in the capabilities.
 * Someone who actually knows something about graphics should
 * take a look to see why this doesn't render on alpha channel :)
 *
 */

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC   "plug-in-flarefx"
#define PLUG_IN_BINARY "flarefx"
#define PLUG_IN_ROLE   "picman-flarefx"

/* --- Typedefs --- */
typedef struct
{
  gint     posx;
  gint     posy;
} FlareValues;

typedef struct REFLECT
{
  PicmanRGB ccol;
  gfloat  size;
  gint    xp;
  gint    yp;
  gint    type;
} Reflect;

typedef struct
{
  PicmanDrawable *drawable;
  PicmanPreview  *preview;
  GtkWidget    *coords;
} FlareCenter;


/* --- Declare local functions --- */
static void        query                          (void);
static void        run                            (const gchar      *name,
                                                   gint              nparams,
                                                   const PicmanParam  *param,
                                                   gint             *nreturn_vals,
                                                   PicmanParam       **return_vals);

static void        FlareFX                        (PicmanDrawable     *drawable,
                                                   PicmanPreview      *preview);
static gboolean    flare_dialog                   (PicmanDrawable     *drawable);

static GtkWidget * flare_center_create            (PicmanDrawable     *drawable,
                                                   PicmanPreview      *preview);
static void        flare_center_coords_update     (PicmanSizeEntry    *coords,
                                                   FlareCenter      *center);
static void        flare_center_preview_realize   (GtkWidget        *widget,
                                                   FlareCenter      *center);
static gboolean    flare_center_preview_expose    (GtkWidget        *widget,
                                                   GdkEvent         *event,
                                                   FlareCenter      *center);
static gboolean    flare_center_preview_events    (GtkWidget        *widget,
                                                   GdkEvent         *event,
                                                   FlareCenter      *center);

static void mcolor  (guchar  *s,
                     gfloat   h);
static void mglow   (guchar  *s,
                     gfloat   h);
static void minner  (guchar  *s,
                     gfloat   h);
static void mouter  (guchar  *s,
                     gfloat   h);
static void mhalo   (guchar  *s,
                     gfloat   h);
static void initref (gint     sx,
                     gint     sy,
                     gint     width,
                     gint     height,
                     gint     matt);
static void fixpix  (guchar  *data,
                     float    procent,
                     PicmanRGB *colpro);
static void mrt1    (guchar  *s,
                     Reflect *ref,
                     gint     col,
                     gint     row);
static void mrt2    (guchar  *s,
                     Reflect *ref,
                     gint     col,
                     gint     row);
static void mrt3    (guchar  *s,
                     Reflect *ref,
                     gint     col,
                     gint     row);
static void mrt4    (guchar  *s,
                     Reflect *ref,
                     gint     col,
                     gint     row);


/* --- Variables --- */
const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static FlareValues fvals =
{
  128, 128   /* posx, posy */
};

static gfloat     scolor, sglow, sinner, souter; /* size     */
static gfloat     shalo;
static gint       xs, ys;
static gint       numref;
static PicmanRGB    color, glow, inner, outer, halo;
static Reflect    ref1[19];
static gboolean   show_cursor = TRUE;


/* --- Functions --- */
MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode", "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",    "Input image (unused)" },
    { PICMAN_PDB_DRAWABLE, "drawable", "Input drawable"       },
    { PICMAN_PDB_INT32,    "pos-x",    "X-position"           },
    { PICMAN_PDB_INT32,    "pos-y",    "Y-position"           }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Add a lens flare effect"),
                          "Adds a lens flare effects.  Makes your image look "
                          "like it was snapped with a cheap camera with a lot "
                          "of lens :)",
                          "Karl-Johan Andersson", /* Author */
                          "Karl-Johan Andersson", /* Copyright */
                          "May 2000",
                          N_("Lens _Flare..."),
                          "RGB*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC,
                             "<Image>/Filters/Light and Shadow/Light");
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

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  /*  Get the specified drawable  */
  drawable = picman_drawable_get (param[2].data.d_drawable);

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &fvals);

      /*  First acquire information with a dialog  */
      if (! flare_dialog (drawable))
        {
          picman_drawable_detach (drawable);
          return;
        }
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 5)
        status = PICMAN_PDB_CALLING_ERROR;
      if (status == PICMAN_PDB_SUCCESS)
        {
          fvals.posx = (gint) param[3].data.d_int32;
          fvals.posy = (gint) param[4].data.d_int32;
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &fvals);
      break;

    default:
      break;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      /*  Make sure that the drawable is gray or RGB color  */
      if (picman_drawable_is_rgb (drawable->drawable_id) ||
          picman_drawable_is_gray (drawable->drawable_id))
        {
          picman_progress_init (_("Render lens flare"));
          picman_tile_cache_ntiles (2 *
                                  (drawable->width / picman_tile_width () + 1));

          FlareFX (drawable, NULL);

          if (run_mode != PICMAN_RUN_NONINTERACTIVE)
            picman_displays_flush ();

          /*  Store data  */
          if (run_mode == PICMAN_RUN_INTERACTIVE)
            picman_set_data (PLUG_IN_PROC, &fvals, sizeof (FlareValues));
        }
      else
        {
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
    }

  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}


static gboolean
flare_dialog (PicmanDrawable *drawable)
{
  GtkWidget   *dialog;
  GtkWidget   *main_vbox;
  GtkWidget   *preview;
  GtkWidget   *frame;
  gboolean     run;

  picman_ui_init (PLUG_IN_BINARY, TRUE);

  dialog = picman_dialog_new (_("Lens Flare"), PLUG_IN_ROLE,
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

  preview = picman_zoom_preview_new (drawable);
  gtk_widget_add_events (PICMAN_PREVIEW (preview)->area,
                         GDK_POINTER_MOTION_MASK);
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);

  g_signal_connect_swapped (preview, "invalidated",
                            G_CALLBACK (FlareFX),
                            drawable);

  frame = flare_center_create (drawable, PICMAN_PREVIEW (preview));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

/* --- Filter functions --- */
static void
FlareFX (PicmanDrawable *drawable,
         PicmanPreview  *preview)
{
  PicmanPixelRgn  srcPR, destPR;
  gint          width, height;
  gint          bytes;
  guchar       *cur_row, *s;
  guchar       *src  = NULL;
  guchar       *dest = NULL;
  gint          row, col, i;
  gint          x1, y1, x2, y2;
  gint          matt;
  gfloat        hyp;
  gdouble       zoom = 0.0;

  bytes  = drawable->bpp;
  if (preview)
    {
      src = picman_zoom_preview_get_source (PICMAN_ZOOM_PREVIEW (preview),
                                          &width, &height, &bytes);

      zoom = picman_zoom_preview_get_factor (PICMAN_ZOOM_PREVIEW (preview));

      picman_preview_transform (preview,
                              fvals.posx, fvals.posy, &xs, &ys);

      x1 = 0;
      y1 = 0;
      x2 = width;
      y2 = height;
      dest = g_new (guchar, bytes * width * height);
    }
  else
    {
      picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);
      width  = drawable->width;
      height = drawable->height;

      xs = fvals.posx; /* set x,y of flare center */
      ys = fvals.posy;
      /*  initialize the pixel regions  */
      picman_pixel_rgn_init (&srcPR, drawable, 0, 0, width, height, FALSE, FALSE);
      picman_pixel_rgn_init (&destPR, drawable, 0, 0, width, height, TRUE, TRUE);
    }

  if (preview)
    matt = width * zoom;
  else
    matt = width;

  cur_row = g_new (guchar, (x2 - x1) * bytes);

  scolor = (gfloat) matt * 0.0375;
  sglow  = (gfloat) matt * 0.078125;
  sinner = (gfloat) matt * 0.1796875;
  souter = (gfloat) matt * 0.3359375;
  shalo  = (gfloat) matt * 0.084375;

  color.r = 239.0/255.0; color.g = 239.0/255.0; color.b = 239.0/255.0;
  glow.r  = 245.0/255.0; glow.g  = 245.0/255.0; glow.b  = 245.0/255.0;
  inner.r = 255.0/255.0; inner.g = 38.0/255.0;  inner.b = 43.0/255.0;
  outer.r = 69.0/255.0;  outer.g = 59.0/255.0;  outer.b = 64.0/255.0;
  halo.r  = 80.0/255.0;  halo.g  = 15.0/255.0;  halo.b  = 4.0/255.0;

  initref (xs, ys, width, height, matt);

  /*  Loop through the rows */
  for (row = y1; row < y2; row++) /* y-coord */
    {
      if (preview)
        memcpy (cur_row, src + row * width * bytes, width * bytes);
      else
        picman_pixel_rgn_get_row (&srcPR, cur_row, x1, row, x2-x1);

      s = cur_row;
      for (col = x1; col < x2; col++) /* x-coord */
        {
          hyp = hypot (col-xs, row-ys);

          mcolor (s, hyp); /* make color */
          mglow (s, hyp);  /* make glow  */
          minner (s, hyp); /* make inner */
          mouter (s, hyp); /* make outer */
          mhalo (s, hyp);  /* make halo  */

          for (i = 0; i < numref; i++)
            {
              switch (ref1[i].type)
                {
                case 1:
                  mrt1 (s, ref1 + i, col, row);
                  break;
                case 2:
                  mrt2 (s, ref1 + i, col, row);
                  break;
                case 3:
                  mrt3 (s, ref1 + i, col, row);
                  break;
                case 4:
                  mrt4 (s, ref1 + i, col, row);
                  break;
                }
            }
          s += bytes;
        }
      if (preview)
        {
          memcpy (dest + row * width * bytes, cur_row, width * bytes);
        }
      else
        {
          /*  store the dest  */
          picman_pixel_rgn_set_row (&destPR, cur_row, x1, row, (x2 - x1));
        }

      if ((row % 5) == 0 && !preview)
        picman_progress_update ((double) row / (double) (y2 - y1));
    }

  if (preview)
    {
      picman_preview_draw_buffer (preview, dest, width * bytes);
      g_free (src);
      g_free (dest);
    }
  else
    {
      picman_progress_update (1.0);
      /*  update the textured region  */
      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
      picman_drawable_update (drawable->drawable_id, x1, y1, (x2 - x1), (y2 - y1));
    }

  g_free (cur_row);
}

static void
mcolor (guchar *s,
        gfloat  h)
{
  gfloat procent;

  procent  = scolor - h;
  procent /= scolor;

  if (procent > 0.0)
    {
      procent *= procent;
      fixpix (s, procent, &color);
    }
}

static void
mglow (guchar *s,
       gfloat  h)
{
  gfloat procent;

  procent  = sglow - h;
  procent /= sglow;

  if (procent > 0.0)
    {
      procent *= procent;
      fixpix (s, procent, &glow);
    }
}

static void
minner (guchar *s,
        gfloat  h)
{
  gfloat procent;

  procent  = sinner - h;
  procent /= sinner;

  if (procent > 0.0)
    {
      procent *= procent;
      fixpix (s, procent, &inner);
    }
}

static void
mouter (guchar *s,
        gfloat  h)
{
  gfloat procent;

  procent  = souter - h;
  procent /= souter;

  if (procent > 0.0)
    fixpix (s, procent, &outer);
}

static void
mhalo (guchar *s,
       gfloat  h)
{
  gfloat procent;

  procent  = h - shalo;
  procent /= (shalo * 0.07);
  procent  = fabs (procent);

  if (procent < 1.0)
    fixpix (s, 1.0 - procent, &halo);
}

static void
fixpix (guchar   *data,
        float     procent,
        PicmanRGB  *colpro)
{
  data[0] += (255 - data[0]) * procent * colpro->r;
  data[1] += (255 - data[1]) * procent * colpro->g;
  data[2] += (255 - data[2]) * procent * colpro->b;
}

static void
initref (gint sx,
         gint sy,
         gint width,
         gint height,
         gint matt)
{
  gint xh, yh, dx, dy;

  xh = width / 2; yh = height / 2;
  dx = xh - sx;   dy = yh - sy;
  numref = 19;
  ref1[0].type=1; ref1[0].size=(gfloat)matt*0.027;
  ref1[0].xp=0.6699*dx+xh; ref1[0].yp=0.6699*dy+yh;
  ref1[0].ccol.r=0.0; ref1[0].ccol.g=14.0/255.0; ref1[0].ccol.b=113.0/255.0;
  ref1[1].type=1; ref1[1].size=(gfloat)matt*0.01;
  ref1[1].xp=0.2692*dx+xh; ref1[1].yp=0.2692*dy+yh;
  ref1[1].ccol.r=90.0/255.0; ref1[1].ccol.g=181.0/255.0; ref1[1].ccol.b=142.0/255.0;
  ref1[2].type=1; ref1[2].size=(gfloat)matt*0.005;
  ref1[2].xp=-0.0112*dx+xh; ref1[2].yp=-0.0112*dy+yh;
  ref1[2].ccol.r=56.0/255.0; ref1[2].ccol.g=140.0/255.0; ref1[2].ccol.b=106.0/255.0;
  ref1[3].type=2; ref1[3].size=(gfloat)matt*0.031;
  ref1[3].xp=0.6490*dx+xh; ref1[3].yp=0.6490*dy+yh;
  ref1[3].ccol.r=9.0/255.0; ref1[3].ccol.g=29.0/255.0; ref1[3].ccol.b=19.0/255.0;
  ref1[4].type=2; ref1[4].size=(gfloat)matt*0.015;
  ref1[4].xp=0.4696*dx+xh; ref1[4].yp=0.4696*dy+yh;
  ref1[4].ccol.r=24.0/255.0; ref1[4].ccol.g=14.0/255.0; ref1[4].ccol.b=0.0;
  ref1[5].type=2; ref1[5].size=(gfloat)matt*0.037;
  ref1[5].xp=0.4087*dx+xh; ref1[5].yp=0.4087*dy+yh;
  ref1[5].ccol.r=24.0/255.0; ref1[5].ccol.g=14.0/255.0; ref1[5].ccol.b=0.0;
  ref1[6].type=2; ref1[6].size=(gfloat)matt*0.022;
  ref1[6].xp=-0.2003*dx+xh; ref1[6].yp=-0.2003*dy+yh;
  ref1[6].ccol.r=42.0/255.0; ref1[6].ccol.g=19.0/255.0; ref1[6].ccol.b=0.0;
  ref1[7].type=2; ref1[7].size=(gfloat)matt*0.025;
  ref1[7].xp=-0.4103*dx+xh; ref1[7].yp=-0.4103*dy+yh;
  ref1[7].ccol.b=17.0/255.0; ref1[7].ccol.g=9.0/255.0; ref1[7].ccol.r=0.0;
  ref1[8].type=2; ref1[8].size=(gfloat)matt*0.058;
  ref1[8].xp=-0.4503*dx+xh; ref1[8].yp=-0.4503*dy+yh;
  ref1[8].ccol.b=10.0/255.0; ref1[8].ccol.g=4.0/255.0; ref1[8].ccol.r=0.0;
  ref1[9].type=2; ref1[9].size=(gfloat)matt*0.017;
  ref1[9].xp=-0.5112*dx+xh; ref1[9].yp=-0.5112*dy+yh;
  ref1[9].ccol.r=5.0/255.0; ref1[9].ccol.g=5.0/255.0; ref1[9].ccol.b=14.0/255.0;
  ref1[10].type=2; ref1[10].size=(gfloat)matt*0.2;
  ref1[10].xp=-1.496*dx+xh; ref1[10].yp=-1.496*dy+yh;
  ref1[10].ccol.r=9.0/255.0; ref1[10].ccol.g=4.0/255.0; ref1[10].ccol.b=0.0;
  ref1[11].type=2; ref1[11].size=(gfloat)matt*0.5;
  ref1[11].xp=-1.496*dx+xh; ref1[11].yp=-1.496*dy+yh;
  ref1[11].ccol.r=9.0/255.0; ref1[11].ccol.g=4.0/255.0; ref1[11].ccol.b=0.0;
  ref1[12].type=3; ref1[12].size=(gfloat)matt*0.075;
  ref1[12].xp=0.4487*dx+xh; ref1[12].yp=0.4487*dy+yh;
  ref1[12].ccol.r=34.0/255.0; ref1[12].ccol.g=19.0/255.0; ref1[12].ccol.b=0.0;
  ref1[13].type=3; ref1[13].size=(gfloat)matt*0.1;
  ref1[13].xp=dx+xh; ref1[13].yp=dy+yh;
  ref1[13].ccol.r=14.0/255.0; ref1[13].ccol.g=26.0/255.0; ref1[13].ccol.b=0.0;
  ref1[14].type=3; ref1[14].size=(gfloat)matt*0.039;
  ref1[14].xp=-1.301*dx+xh; ref1[14].yp=-1.301*dy+yh;
  ref1[14].ccol.r=10.0/255.0; ref1[14].ccol.g=25.0/255.0; ref1[14].ccol.b=13.0/255.0;
  ref1[15].type=4; ref1[15].size=(gfloat)matt*0.19;
  ref1[15].xp=1.309*dx+xh; ref1[15].yp=1.309*dy+yh;
  ref1[15].ccol.r=9.0/255.0; ref1[15].ccol.g=0.0; ref1[15].ccol.b=17.0/255.0;
  ref1[16].type=4; ref1[16].size=(gfloat)matt*0.195;
  ref1[16].xp=1.309*dx+xh; ref1[16].yp=1.309*dy+yh;
  ref1[16].ccol.r=9.0/255.0; ref1[16].ccol.g=16.0/255.0; ref1[16].ccol.b=5.0/255.0;
  ref1[17].type=4; ref1[17].size=(gfloat)matt*0.20;
  ref1[17].xp=1.309*dx+xh; ref1[17].yp=1.309*dy+yh;
  ref1[17].ccol.r=17.0/255.0; ref1[17].ccol.g=4.0/255.0; ref1[17].ccol.b=0.0;
  ref1[18].type=4; ref1[18].size=(gfloat)matt*0.038;
  ref1[18].xp=-1.301*dx+xh; ref1[18].yp=-1.301*dy+yh;
  ref1[18].ccol.r=17.0/255.0; ref1[18].ccol.g=4.0/255.0; ref1[18].ccol.b=0.0;
}

static void
mrt1 (guchar  *s,
      Reflect *ref,
      gint     col,
      gint     row)
{
  gfloat procent;

  procent  = ref->size - hypot (ref->xp - col, ref->yp - row);
  procent /= ref->size;

  if (procent > 0.0)
    {
      procent *= procent;
      fixpix (s, procent, &ref->ccol);
    }
}

static void
mrt2 (guchar *s,
      Reflect *ref,
      gint    col,
      gint    row)
{
  gfloat procent;

  procent  = ref->size - hypot (ref->xp - col, ref->yp - row);
  procent /= (ref->size * 0.15);

  if (procent > 0.0)
    {
      if (procent > 1.0)
        procent = 1.0;

      fixpix (s, procent, &ref->ccol);
    }
}

static void
mrt3 (guchar *s,
      Reflect *ref,
      gint    col,
      gint    row)
{
  gfloat procent;

  procent  = ref->size - hypot (ref->xp - col, ref->yp - row);
  procent /= (ref->size * 0.12);

  if (procent > 0.0)
    {
      if (procent > 1.0)
        procent = 1.0 - (procent * 0.12);

      fixpix (s, procent, &ref->ccol);
    }
}

static void
mrt4 (guchar *s,
      Reflect *ref,
      gint    col,
      gint    row)
{
  gfloat procent;

  procent  = hypot (ref->xp - col, ref->yp - row) - ref->size;
  procent /= (ref->size*0.04);
  procent  = fabs (procent);

  if (procent < 1.0)
    fixpix (s, 1.0 - procent, &ref->ccol);
}

/*=================================================================
    CenterFrame

    A frame that contains one preview and 2 entries, used for positioning
    of the center of Flare.
    This whole thing is just too ugly, but I don't want to dig into it
     - tim
==================================================================*/

/*
 * Create new CenterFrame, and return it (GtkFrame).
 */

static GtkWidget *
flare_center_create (PicmanDrawable *drawable,
                     PicmanPreview  *preview)
{
  FlareCenter *center;
  GtkWidget   *frame;
  GtkWidget   *hbox;
  GtkWidget   *check;
  gint32       image_ID;
  gdouble      res_x;
  gdouble      res_y;

  center = g_new0 (FlareCenter, 1);

  center->drawable = drawable;
  center->preview  = preview;

  frame = picman_frame_new (_("Center of Flare Effect"));

  g_object_set_data (G_OBJECT (frame), "center", center);

  g_signal_connect_swapped (frame, "destroy",
                            G_CALLBACK (g_free),
                            center);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_widget_show (hbox);

  image_ID = picman_item_get_image (drawable->drawable_id);
  picman_image_get_resolution (image_ID, &res_x, &res_y);

  center->coords = picman_coordinates_new (PICMAN_UNIT_PIXEL, "%p", TRUE, TRUE, -1,
                                         PICMAN_SIZE_ENTRY_UPDATE_SIZE,
                                         FALSE, FALSE,

                                         _("_X:"), fvals.posx, res_x,
                                         - (gdouble) drawable->width,
                                         2 * drawable->width,
                                         0, drawable->width,

                                         _("_Y:"), fvals.posy, res_y,
                                         - (gdouble) drawable->height,
                                         2 * drawable->height,
                                         0, drawable->height);

  gtk_table_set_row_spacing (GTK_TABLE (center->coords), 1, 12);
  gtk_box_pack_start (GTK_BOX (hbox), center->coords, FALSE, FALSE, 0);
  gtk_widget_show (center->coords);

  g_signal_connect (center->coords, "value-changed",
                    G_CALLBACK (flare_center_coords_update),
                    center);
  g_signal_connect (center->coords, "refval-changed",
                    G_CALLBACK (flare_center_coords_update),
                    center);

  check = gtk_check_button_new_with_mnemonic (_("Show _position"));
  gtk_table_attach (GTK_TABLE (center->coords), check, 0, 5, 2, 3,
                    GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), show_cursor);
  gtk_widget_show (check);

  g_signal_connect (check, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &show_cursor);
  g_signal_connect_swapped (check, "toggled",
                            G_CALLBACK (gtk_widget_queue_draw),
                            preview->area);

  g_signal_connect (preview->area, "realize",
                    G_CALLBACK (flare_center_preview_realize),
                    center);
  g_signal_connect_after (preview->area, "expose-event",
                          G_CALLBACK (flare_center_preview_expose),
                          center);
  g_signal_connect (preview->area, "event",
                    G_CALLBACK (flare_center_preview_events),
                    center);

  return frame;
}

/*
 *  CenterFrame entry callback
 */
static void
flare_center_coords_update (PicmanSizeEntry *coords,
                            FlareCenter   *center)
{
  fvals.posx = picman_size_entry_get_refval (coords, 0);
  fvals.posy = picman_size_entry_get_refval (coords, 1);

  picman_preview_invalidate (center->preview);
}

/*
 *  Set the preview area's cursor on realize
 */
static void
flare_center_preview_realize (GtkWidget   *widget,
                              FlareCenter *center)
{
  GdkDisplay *display = gtk_widget_get_display (widget);
  GdkCursor  *cursor  = gdk_cursor_new_for_display (display, GDK_CROSSHAIR);

  picman_preview_set_default_cursor (center->preview, cursor);
  gdk_cursor_unref (cursor);
}

/*
 *    Handle the expose event on the preview
 */
static gboolean
flare_center_preview_expose (GtkWidget   *widget,
                             GdkEvent    *event,
                             FlareCenter *center)
{
  if (show_cursor)
    {
      cairo_t *cr;
      gint     x, y;
      gint     width, height;

      cr = gdk_cairo_create (gtk_widget_get_window (center->preview->area));

      picman_preview_transform (center->preview,
                              fvals.posx, fvals.posy,
                              &x, &y);

      picman_preview_get_size (center->preview, &width, &height);

      cairo_move_to (cr, x + 0.5, 0);
      cairo_line_to (cr, x + 0.5, height);

      cairo_move_to (cr, 0,     y + 0.5);
      cairo_line_to (cr, width, y + 0.5);

      cairo_set_line_width (cr, 3.0);
      cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.6);
      cairo_stroke_preserve (cr);

      cairo_set_line_width (cr, 1.0);
      cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.8);
      cairo_stroke (cr);

      cairo_destroy (cr);
    }

  return FALSE;
}

/*
 *    Handle other events on the preview
 */
static gboolean
flare_center_preview_events (GtkWidget   *widget,
                             GdkEvent    *event,
                             FlareCenter *center)
{
  gint tx, ty;

  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      if (! (((GdkEventMotion *) event)->state & GDK_BUTTON1_MASK))
        break;

    case GDK_BUTTON_PRESS:
      {
        GdkEventButton *bevent = (GdkEventButton *) event;

        if (bevent->button == 1)
          {
            picman_preview_untransform (center->preview,
                                      bevent->x, bevent->y,
                                      &tx, &ty);

            g_signal_handlers_block_by_func (center->coords,
                                             flare_center_coords_update,
                                             center);

            picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (center->coords),
                                        0, tx);
            picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (center->coords),
                                        1, ty);

            g_signal_handlers_unblock_by_func (center->coords,
                                               flare_center_coords_update,
                                               center);

            flare_center_coords_update (PICMAN_SIZE_ENTRY (center->coords), center);

            gtk_widget_queue_draw (center->preview->area);

            return TRUE;
          }
      }
      break;

    default:
      break;
    }

  return FALSE;
}
