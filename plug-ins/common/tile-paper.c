/*============================================================================*

  Paper Tile 1.0  -- A PICMAN PLUG-IN

  Copyright (C) 1997-1999 Hirotsuna Mizuno <s1041150@u-aizu.ac.jp>

  This program  is  free software:  you can redistribute it  and/or  modify it
  under the terms of the GNU Public License as published  by the Free Software
  Foundation;  either version 3 of the License,  or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful,  but WITHOUT
  ANY WARRANTY;  without  even  the  implied  warranty  of MERCHANTABILITY  or
  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU General Public License  for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, see <http://www.gnu.org/licenses/>.

 *===========================================================================*/

#include "config.h"

#include <stdlib.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


/*===========================================================================*/
/* DEFINES                                                                   */
/*===========================================================================*/

#define PLUG_IN_PROC   "plug-in-papertile"
#define PLUG_IN_BINARY "tile-paper"
#define PLUG_IN_ROLE   "picman-tile-paper"

/*===========================================================================*/
/* TYPES                                                                     */
/*===========================================================================*/

typedef enum
{
  BACKGROUND_TYPE_TRANSPARENT,
  BACKGROUND_TYPE_INVERTED,
  BACKGROUND_TYPE_IMAGE,
  BACKGROUND_TYPE_FOREGROUND,
  BACKGROUND_TYPE_BACKGROUND,
  BACKGROUND_TYPE_COLOR
} BackgroundType;

typedef enum
{
  FRACTIONAL_TYPE_BACKGROUND,           /* AS BACKGROUND */
  FRACTIONAL_TYPE_IGNORE,               /* NOT OPERATED  */
  FRACTIONAL_TYPE_FORCE                 /* FORCE DIVISION */
} FractionalType;

typedef struct _PluginParams PluginParams;
struct _PluginParams
{
  gint32          tile_width;
  gint32          tile_height;
  gint32          division_x;
  gint32          division_y;
  gdouble         move_max_rate;
  FractionalType  fractional_type;
  gboolean        centering;
  gboolean        wrap_around;
  BackgroundType  background_type;
  PicmanRGB         background_color;
};

/*===========================================================================*/
/* VARIABLES                                                                 */
/*===========================================================================*/

static struct
{
  PluginParams  params;

  gint32        image;
  PicmanDrawable *drawable;
  gboolean      drawable_has_alpha;

  struct
  {
    gint        x0;
    gint        y0;
    gint        x1;
    gint        y1;
    gint        width;
    gint        height;
  } selection;

  PicmanRunMode  run_mode;
  gboolean         run;
} p =
{
  {
    1,                          /* tile_width            */
    1,                          /* tile_height           */
    16,                         /* division_x            */
    16,                         /* division_y            */
    25.0,                       /* move_max_rate         */
    FRACTIONAL_TYPE_BACKGROUND, /* fractional_type       */
    TRUE,                       /* centering             */
    FALSE,                      /* wrap_around           */
    BACKGROUND_TYPE_INVERTED,   /* background_type       */
    { 0.0, 0.0, 1.0, 1.0 }      /* background_color      */
  },

  0,                            /* image                 */
  NULL,                         /* drawable              */
  FALSE,                        /* drawable_has_alpha    */

  { 0, 0, 0, 0, 0, 0 },         /* selection             */

  PICMAN_RUN_INTERACTIVE,         /* run_mode              */
  FALSE                         /* run                   */
};

/*---------------------------------------------------------------------------*/

static void
params_save_to_picman (void)
{
  picman_set_data (PLUG_IN_PROC, &p.params, sizeof p.params);
}

static void
params_load_from_picman (void)
{
  picman_get_data (PLUG_IN_PROC, &p.params);

  if (0 < p.params.division_x)
    {
      p.params.tile_width  = p.drawable->width / p.params.division_x;
      if (0 < p.params.tile_width)
        {
          p.params.division_y = p.drawable->height / p.params.tile_height;
        }
    }

  if (p.params.tile_width <= 0 ||
      p.params.tile_height <= 0 ||
      p.params.division_x <= 0 ||
      p.params.division_y <= 0)
    {
      p.params.tile_width = p.drawable->width;
      p.params.tile_height = p.drawable->height;
      p.params.division_x = p.drawable->width / p.params.tile_width;
      p.params.division_y = p.drawable->height / p.params.tile_height;
    }

  if (!p.drawable_has_alpha)
    {
      if (p.params.background_type == BACKGROUND_TYPE_TRANSPARENT)
        {
          p.params.background_type = BACKGROUND_TYPE_INVERTED;
        }

      picman_rgb_set_alpha (&p.params.background_color, 1.0);
    }
}

/*===========================================================================*/
/* GUI                                                                       */
/*===========================================================================*/

static struct
{
  GtkObject *tile_width_adj;
  GtkObject *tile_height_adj;
  GtkObject *division_x_adj;
  GtkObject *division_y_adj;
} w;

static void
tile_width_adj_changed (GtkAdjustment *adj)
{
  if (p.params.tile_width != (gint)gtk_adjustment_get_value (adj))
    {
      p.params.tile_width  = gtk_adjustment_get_value (adj);
      p.params.division_x = p.drawable->width  / p.params.tile_width;
      gtk_adjustment_set_value (GTK_ADJUSTMENT (w.division_x_adj),
                                p.params.division_x);
    }
}

static void
tile_height_adj_changed (GtkAdjustment *adj)
{
  if (p.params.tile_height != (gint)gtk_adjustment_get_value (adj))
    {
      p.params.tile_height  = gtk_adjustment_get_value (adj);
      p.params.division_y = p.drawable->height / p.params.tile_height;
      gtk_adjustment_set_value (GTK_ADJUSTMENT (w.division_y_adj),
                                p.params.division_y);
    }
}

static void
division_x_adj_changed (GtkAdjustment *adj)
{
  if (p.params.division_x != (gint) gtk_adjustment_get_value (adj))
    {
      p.params.division_x = gtk_adjustment_get_value (adj);
      p.params.tile_width  = p.drawable->width  / p.params.division_x;
      gtk_adjustment_set_value (GTK_ADJUSTMENT (w.tile_width_adj),
                                p.params.tile_width);
    }
}

static void
division_y_adj_changed (GtkAdjustment *adj)
{
  if (p.params.division_y != (gint)gtk_adjustment_get_value (adj))
    {
      p.params.division_y = gtk_adjustment_get_value (adj);
      p.params.tile_height  = p.drawable->height / p.params.division_y;
      gtk_adjustment_set_value (GTK_ADJUSTMENT (w.tile_height_adj),
                                p.params.tile_height);
    }
}

static void
open_dialog (void)
{
  GtkWidget *dialog;
  GtkWidget *main_hbox;
  GtkWidget *button;
  GtkObject *adjustment;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *frame;
  GtkWidget *color_button;

  picman_ui_init (PLUG_IN_BINARY, TRUE);

  dialog = picman_dialog_new (_("Paper Tile"), PLUG_IN_ROLE,
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

  main_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_hbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      main_hbox, TRUE, TRUE, 0);
  gtk_widget_show (main_hbox);

  /* Left */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_box_pack_start (GTK_BOX (main_hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  frame = picman_frame_new (_("Division"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (4, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  button = picman_spin_button_new (&w.division_x_adj, p.params.division_x,
                                 1.0, p.drawable->width, 1.0, 5.0, 0, 1, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("_X:"), 0.0, 0.5,
                             button, 1, TRUE);
  g_signal_connect (w.division_x_adj, "value-changed",
                    G_CALLBACK (division_x_adj_changed),
                    NULL);

  button = picman_spin_button_new (&w.division_y_adj, p.params.division_y,
                                 1.0, p.drawable->width, 1.0, 5.0, 0, 1, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                             _("_Y:"), 0.0, 0.5,
                             button, 1, TRUE);
  g_signal_connect (w.division_y_adj, "value-changed",
                    G_CALLBACK (division_y_adj_changed),
                    NULL);

  button = picman_spin_button_new (&w.tile_width_adj, p.params.tile_width,
                                 1.0, MAX (p.drawable->width,
                                           p.drawable->height),
                                 1.0, 5.0, 0, 1, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 2,
                             _("_Width:"), 0.0, 0.5,
                             button, 1, TRUE);
  g_signal_connect (w.tile_width_adj, "value-changed",
                    G_CALLBACK (tile_width_adj_changed),
                    NULL);

  button = picman_spin_button_new (&w.tile_height_adj, p.params.tile_height,
                                 1.0, MAX (p.drawable->width,
                                 p.drawable->height),
                                 1.0, 5.0, 0, 1, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 3,
                             _("_Height:"), 0.0, 0.5,
                             button, 1, TRUE);
  g_signal_connect (w.tile_height_adj, "value-changed",
                    G_CALLBACK (tile_height_adj_changed),
                    NULL);

  frame = picman_int_radio_group_new (TRUE, _("Fractional Pixels"),
                                    G_CALLBACK (picman_radio_button_update),
                                    &p.params.fractional_type,
                                    p.params.fractional_type,

                                    _("_Background"),
                                    FRACTIONAL_TYPE_BACKGROUND, NULL,
                                    _("_Ignore"),
                                    FRACTIONAL_TYPE_IGNORE, NULL,
                                    _("_Force"),
                                    FRACTIONAL_TYPE_FORCE, NULL,

                                    NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  button = gtk_check_button_new_with_mnemonic(_("C_entering"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                p.params.centering);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &p.params.centering);

  /* Right */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_box_pack_start (GTK_BOX (main_hbox), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  frame = picman_frame_new (_("Movement"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  button = picman_spin_button_new (&adjustment, p.params.move_max_rate,
                                 0.0, 100.0, 1.0, 10.0, 0, 1, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("_Max (%):"), 0.0, 0.5,
                             button, 1, TRUE);
  g_signal_connect (adjustment, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &p.params.move_max_rate);

  button = gtk_check_button_new_with_mnemonic (_("_Wrap around"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                p.params.wrap_around);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 2, 1, 2);
  gtk_widget_show (button);

  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &p.params.wrap_around);

  frame = picman_int_radio_group_new (TRUE, _("Background Type"),
                                    G_CALLBACK (picman_radio_button_update),
                                    &p.params.background_type,
                                    p.params.background_type,

                                    _("_Transparent"),
                                    BACKGROUND_TYPE_TRANSPARENT, NULL,
                                    _("I_nverted image"),
                                    BACKGROUND_TYPE_INVERTED, NULL,
                                    _("Im_age"),
                                    BACKGROUND_TYPE_IMAGE, NULL,
                                    _("Fo_reground color"),
                                    BACKGROUND_TYPE_FOREGROUND, NULL,
                                    _("Bac_kground color"),
                                    BACKGROUND_TYPE_BACKGROUND, NULL,
                                    _("S_elect here:"),
                                    BACKGROUND_TYPE_COLOR, &button,

                                    NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  color_button = picman_color_button_new (_("Background Color"), 100, 16,
                                        &p.params.background_color,
                                        p.drawable_has_alpha ?
                                        PICMAN_COLOR_AREA_SMALL_CHECKS :
                                        PICMAN_COLOR_AREA_FLAT);
  gtk_box_pack_start (GTK_BOX (gtk_bin_get_child (GTK_BIN (frame))),
                      color_button, TRUE, TRUE, 0);
  gtk_widget_show (color_button);

  g_signal_connect (color_button, "color-changed",
                    G_CALLBACK (picman_color_button_get_color),
                    &p.params.background_color);

  g_object_bind_property (button,       "active",
                          color_button, "sensitive",
                          G_BINDING_SYNC_CREATE);

  gtk_widget_show (dialog);

  p.run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);
}

/*===========================================================================*/
/* PLUGIN CORE                                                               */
/*===========================================================================*/
typedef struct _Tile
{
  guint x;
  guint y;
  gint  z;
  guint width;
  guint height;
  gint  move_x;
  gint  move_y;
} Tile;

static gint
tile_compare (const void *x,
              const void *y)
{
  return ((Tile *) x)->z - ((Tile *) y)->z;
}

static inline gdouble
drand (void)
{
  return g_random_double_range (-1, 1);
}

static inline void
random_move (gint *x,
             gint *y,
             gint  max)
{
  gdouble angle  = drand () * G_PI;
  gdouble radius = drand () * (gdouble) max;
  *x = (gint) (radius * cos (angle));
  *y = (gint) (radius * sin (angle));
}

static void
overlap_RGB (guchar       *base,
             const guchar *top)
{
  base[0] = top[0];
  base[1] = top[1];
  base[2] = top[2];
}

static void
overlap_RGBA (guchar       *base,
              const guchar *top)
{
  gdouble R1 = (gdouble) base[0] / 255.0;
  gdouble G1 = (gdouble) base[1] / 255.0;
  gdouble B1 = (gdouble) base[2] / 255.0;
  gdouble A1 = (gdouble) base[3] / 255.0;
  gdouble R2 = (gdouble)  top[0] / 255.0;
  gdouble G2 = (gdouble)  top[1] / 255.0;
  gdouble B2 = (gdouble)  top[2] / 255.0;
  gdouble A2 = (gdouble)  top[3] / 255.0;
  gdouble A3 = A2 + A1 * (1.0 - A2);

  if (0.0 < A3)
    {
      gdouble R3 = (R1 * A1 * (1.0 - A2) + R2 * A2) / A3;
      gdouble G3 = (G1 * A1 * (1.0 - A2) + G2 * A2) / A3;
      gdouble B3 = (B1 * A1 * (1.0 - A2) + B2 * A2) / A3;

      R3 = CLAMP (R3, 0.0, 1.0);
      G3 = CLAMP (G3, 0.0, 1.0);
      B3 = CLAMP (B3, 0.0, 1.0);

      base[0] = (guchar) (R3 * 255.0);
      base[1] = (guchar) (G3 * 255.0);
      base[2] = (guchar) (B3 * 255.0);
      base[3] = (guchar) (A3 * 255.0);
    }
  else
    {
      base[0] = 0;
      base[1] = 0;
      base[2] = 0;
      base[3] = 0;
    }
}

static inline void
filter (void)
{
  static void (* overlap)(guchar *, const guchar *);
  PicmanPixelRgn  src;
  PicmanPixelRgn  dst;
  PicmanRGB       color;
  guchar     pixel[4];
  gint       division_x;
  gint       division_y;
  gint       offset_x;
  gint       offset_y;
  Tile      *tiles;
  gint       numof_tiles;
  Tile      *t;
  gint       i;
  gint       x;
  gint       y;
  gint       move_max_pixels;
  gint       clear_x0;
  gint       clear_y0;
  gint       clear_x1;
  gint       clear_y1;
  gint       clear_width;
  gint       clear_height;
  guchar    *pixels;
  guchar    *buffer;
  gint       dindex;
  gint       sindex;
  gint       px, py;
  GRand     *gr;

  gr = g_rand_new ();

  /* INITIALIZE */
  picman_pixel_rgn_init (&src, p.drawable, 0, 0,
                       p.drawable->width, p.drawable->height, FALSE, FALSE);
  picman_pixel_rgn_init (&dst, p.drawable, 0, 0,
                       p.drawable->width, p.drawable->height, TRUE, TRUE);
  pixels = g_new (guchar,
                  p.drawable->bpp * p.drawable->width * p.drawable->height);
  buffer = g_new (guchar,
                  p.drawable->bpp * p.params.tile_width * p.params.tile_height);

  overlap = p.drawable_has_alpha ? overlap_RGBA : overlap_RGB;

  picman_progress_init (_("Paper Tile"));

  picman_drawable_mask_bounds (p.drawable->drawable_id,
                             &p.selection.x0, &p.selection.y0,
                             &p.selection.x1, &p.selection.y1);
  p.selection.width  = p.selection.x1 - p.selection.x0;
  p.selection.height = p.selection.y1 - p.selection.y0;

  picman_tile_cache_ntiles (2 * (p.selection.width / picman_tile_width () + 1));

  /* TILES */
  division_x = p.params.division_x;
  division_y = p.params.division_y;
  if (p.params.fractional_type == FRACTIONAL_TYPE_FORCE)
    {
      if (0 < p.drawable->width  % p.params.tile_width) division_x++;
      if (0 < p.drawable->height % p.params.tile_height) division_y++;
      if (p.params.centering)
        {
          if (1 < p.drawable->width % p.params.tile_width)
            {
              division_x++;
              offset_x =
                (p.drawable->width % p.params.tile_width) / 2 -
                p.params.tile_width;
            }
          else
            {
              offset_x = 0;
            }

          if (1 < p.drawable->height % p.params.tile_height)
            {
              division_y++;
              offset_y =
                (p.drawable->height % p.params.tile_height) / 2 -
                p.params.tile_height;
            }
          else
            {
              offset_y = 0;
            }
        }
      else
        {
          offset_x = 0;
          offset_y = 0;
        }
    }
  else
    {
      if (p.params.centering)
        {
          offset_x = (p.drawable->width  % p.params.tile_width) / 2;
          offset_y = (p.drawable->height % p.params.tile_height) / 2;
        }
      else
        {
          offset_x = 0;
          offset_y = 0;
        }
    }

  move_max_pixels = p.params.move_max_rate * p.params.tile_width / 100.0;
  numof_tiles = division_x * division_y;
  t = tiles = g_new(Tile, numof_tiles);

  for (y = 0; y < division_y; y++)
    {
      gint srcy = offset_y + p.params.tile_height * y;

      for (x = 0; x < division_x; x++, t++)
        {
          gint srcx = offset_x + p.params.tile_width * x;

          if (srcx < 0)
            {
              t->x     = 0;
              t->width = srcx + p.params.tile_width;
            }
          else if (srcx + p.params.tile_width < p.drawable->width)
            {
              t->x     = srcx;
              t->width = p.params.tile_width;
            }
          else
            {
              t->x     = srcx;
              t->width = p.drawable->width - srcx;
            }

          if (srcy < 0)
            {
              t->y      = 0;
              t->height = srcy + p.params.tile_height;
            }
          else if (srcy + p.params.tile_height < p.drawable->height)
            {
              t->y      = srcy;
              t->height = p.params.tile_height;
            }
          else
            {
              t->y      = srcy;
              t->height = p.drawable->height - srcy;
            }

          t->z = g_rand_int (gr);
          random_move (&t->move_x, &t->move_y, move_max_pixels);
        }
    }

  qsort (tiles, numof_tiles, sizeof *tiles, tile_compare);

  picman_pixel_rgn_get_rect (&src, pixels, 0, 0, p.drawable->width,
                           p.drawable->height);

  if (p.params.fractional_type == FRACTIONAL_TYPE_IGNORE)
    {
      clear_x0     = offset_x;
      clear_y0     = offset_y;
      clear_width  = p.params.tile_width * division_x;
      clear_height = p.params.tile_height * division_y;
    }
  else
    {
      clear_x0     = 0;
      clear_y0     = 0;
      clear_width  = p.drawable->width;
      clear_height = p.drawable->height;
    }

  clear_x1 = clear_x0 + clear_width;
  clear_y1 = clear_y0 + clear_height;

  switch (p.params.background_type)
    {
    case BACKGROUND_TYPE_TRANSPARENT:
      for (y = clear_y0; y < clear_y1; y++)
        {
          for (x = clear_x0; x < clear_x1; x++)
            {
              dindex = p.drawable->bpp * (p.drawable->width * y + x);
              for (i = 0; i < p.drawable->bpp; i++)
                {
                  pixels[dindex+i] = 0;
                }
            }
        }
      break;

    case BACKGROUND_TYPE_INVERTED:
      for (y = clear_y0; y < clear_y1; y++)
        {
          for (x = clear_x0; x < clear_x1; x++)
            {
              dindex = p.drawable->bpp * (p.drawable->width * y + x);
              pixels[dindex+0] = 255 - pixels[dindex+0];
              pixels[dindex+1] = 255 - pixels[dindex+1];
              pixels[dindex+2] = 255 - pixels[dindex+2];
            }
        }
      break;

    case BACKGROUND_TYPE_IMAGE:
      break;

    case BACKGROUND_TYPE_FOREGROUND:
      picman_context_get_foreground (&color);
      picman_rgb_get_uchar (&color, &pixel[0], &pixel[1], &pixel[2]);
      pixel[3] = 255;
      for (y = clear_y0; y < clear_y1; y++)
        {
          for (x = clear_x0; x < clear_x1; x++)
            {
              dindex = p.drawable->bpp * (p.drawable->width * y + x);
              for (i = 0; i < p.drawable->bpp; i++)
                {
                  pixels[dindex+i] = pixel[i];
                }
            }
        }
      break;

    case BACKGROUND_TYPE_BACKGROUND:
      picman_context_get_background (&color);
      picman_rgb_get_uchar (&color, &pixel[0], &pixel[1], &pixel[2]);
      pixel[3] = 255;
      for (y = clear_y0; y < clear_y1; y++)
        {
          for (x = clear_x0; x < clear_x1; x++)
            {
              dindex = p.drawable->bpp * (p.drawable->width * y + x);
              for(i = 0; i < p.drawable->bpp; i++)
                {
                  pixels[dindex+i] = pixel[i];
                }
            }
        }
      break;

    case BACKGROUND_TYPE_COLOR:
      picman_rgba_get_uchar (&p.params.background_color,
                           pixel, pixel + 1, pixel + 2, pixel + 3);
      for (y = clear_y0; y < clear_y1; y++)
        {
          for (x = clear_x0; x < clear_x1; x++)
            {
              dindex = p.drawable->bpp * (p.drawable->width * y + x);
              for(i = 0; i < p.drawable->bpp; i++)
                {
                  pixels[dindex+i] = pixel[i];
                }
            }
        }
      break;
    }

  /* DRAW */
  for (t = tiles, i = 0; i < numof_tiles; i++, t++)
    {
      gint x0 = t->x + t->move_x;
      gint y0 = t->y + t->move_y;

      picman_pixel_rgn_get_rect (&src, buffer, t->x, t->y, t->width, t->height);

      for (y = 0; y < t->height; y++)
        {
          py = y0 + y;
          for (x = 0; x < t->width; x++)
            {
              px = x0 + x;
              sindex = p.drawable->bpp * (t->width * y + x);
              if (0 <= px && px < p.drawable->width &&
                  0 <= py && py < p.drawable->height)
                {
                  dindex = p.drawable->bpp * (p.drawable->width * py + px);
                  overlap(&pixels[dindex], &buffer[sindex]);
                }
              else if (p.params.wrap_around)
                {
                  px = (px + p.drawable->width)  % p.drawable->width;
                  py = (py + p.drawable->height) % p.drawable->height;
                  dindex = p.drawable->bpp * (p.drawable->width * py + px);
                  overlap(&pixels[dindex], &buffer[sindex]);
                }
            }
        }

      picman_progress_update ((gdouble) i / (gdouble) numof_tiles);
    }

  picman_pixel_rgn_set_rect (&dst, pixels, 0, 0, p.drawable->width,
                           p.drawable->height);

  picman_progress_update (1.0);
  picman_drawable_flush (p.drawable);
  picman_drawable_merge_shadow (p.drawable->drawable_id, TRUE);
  picman_drawable_update (p.drawable->drawable_id,
                        p.selection.x0, p.selection.y0,
                        p.selection.width, p.selection.height);

  g_rand_free (gr);
  g_free (buffer);
  g_free (pixels);
  g_free (tiles);
}

/*============================================================================*/
/* PLUGIN INTERFACES                                                          */
/*============================================================================*/

static void
plugin_query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",         "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",            "Input image"                    },
    { PICMAN_PDB_DRAWABLE, "drawable",         "Input drawable"                 },
    { PICMAN_PDB_INT32,    "tile-size",        "Tile size (pixels)"             },
    { PICMAN_PDB_FLOAT,    "move-max",         "Max move rate (%)"              },
    { PICMAN_PDB_INT32,    "fractional-type",  "0:Background 1:Ignore 2:Force"  },
    { PICMAN_PDB_INT32,    "wrap-around",      "Wrap around (bool)"             },
    { PICMAN_PDB_INT32,    "centering",        "Centering (bool)"               },
    { PICMAN_PDB_INT32,    "background-type",  "0:Transparent 1:Inverted 2:Image? 3:FG 4:BG 5:Color"                  },
    { PICMAN_PDB_INT32,    "background-color", "Background color (for bg-type 5)" },
    { PICMAN_PDB_INT32,    "background-alpha", "Opacity (for bg-type 5)"        }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Cut image into paper tiles, and slide them"),
                          "This plug-in cuts an image into paper tiles and "
                          "slides each paper tile.",
                          "Hirotsuna Mizuno <s1041150@u-aizu.ac.jp>",
                          "Copyright (c)1997-1999 Hirotsuna Mizuno",
                          _("September 31, 1999"),
                          N_("_Paper Tile..."),
                          "RGB*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);
}

static void
plugin_run (const gchar      *name,
            gint              numof_params,
            const PicmanParam  *params,
            gint             *numof_return_vals,
            PicmanParam       **return_vals)
{
  PicmanPDBStatusType status = PICMAN_PDB_SUCCESS;

  INIT_I18N ();

  p.run                = FALSE;
  p.run_mode           = params[0].data.d_int32;
  p.image              = params[1].data.d_image;
  p.drawable           = picman_drawable_get(params[2].data.d_drawable);
  p.drawable_has_alpha = picman_drawable_has_alpha(p.drawable->drawable_id);

  if (picman_drawable_is_rgb (p.drawable->drawable_id))
    {
      switch (p.run_mode)
        {
        case PICMAN_RUN_INTERACTIVE:
          params_load_from_picman ();
          open_dialog ();
          break;

        case PICMAN_RUN_NONINTERACTIVE:
          if (numof_params == 11)
            {
              p.params.tile_width       = params[3].data.d_int32;
              p.params.tile_height      = params[3].data.d_int32;
              p.params.division_x       = p.drawable->width  / p.params.tile_width;
              p.params.division_y       = p.drawable->height / p.params.tile_height;
              p.params.move_max_rate    = params[4].data.d_float;
              p.params.fractional_type  = (FractionalType)params[5].data.d_int32;
              p.params.wrap_around      = params[6].data.d_int32;
              p.params.centering        = params[7].data.d_int32;
              p.params.background_type  = (BackgroundType)params[8].data.d_int32;
              p.params.background_color = params[9].data.d_color;

              /*  FIXME:  this used to be the alpha value
                                          params[10].data.d_int32
               */
              p.run = TRUE;
            }
          else
            {
              status = PICMAN_PDB_CALLING_ERROR;
            }
          break;

        case PICMAN_RUN_WITH_LAST_VALS:
          params_load_from_picman ();
          p.run = TRUE;
          break;
        }
    }
  else
    {
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  if (status == PICMAN_PDB_SUCCESS && p.run)
    {
      params_save_to_picman ();

      filter ();

      if (p.run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();
    }

  picman_drawable_detach (p.drawable);

  {
    static PicmanParam return_value[1];
    return_value[0].type          = PICMAN_PDB_STATUS;
    return_value[0].data.d_status = status;
    *numof_return_vals            = 1;
    *return_vals                  = return_value;
  }
}

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,
  NULL,
  plugin_query,
  plugin_run
};

MAIN ()
