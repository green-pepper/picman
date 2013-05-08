/*
 * This is a plug-in for PICMAN.
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
 *
 */

/*
 * Exchange one color with the other (settable threshold to convert from
 * one color-shade to another...might do wonders on certain images, or be
 * totally useless on others).
 *
 * Author: robert@experimental.net
 *
 * Added ability to select "from" color by clicking on the preview image.
 * As a side effect, clicking twice on the same spot reverses the action,
 * i.e. you can click once, see the result, and click again to revert.
 *
 * Also changed update policies for all sliders to delayed.  On a slow machine
 * the algorithm really chewes up CPU time.
 *
 * - timecop@japan.co.jp
 */

#include "config.h"

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC   "plug-in-exchange"
#define PLUG_IN_BINARY "color-exchange"
#define PLUG_IN_ROLE   "picman-color-exchange"

#define SCALE_WIDTH    128


/* datastructure to store parameters in */
typedef struct
{
  PicmanRGB  from;
  PicmanRGB  to;
  PicmanRGB  threshold;
} myParams;

/* lets prototype */
static void     query (void);
static void     run   (const gchar      *name,
                       gint              nparams,
                       const PicmanParam  *param,
                       gint             *nreturn_vals,
                       PicmanParam       **return_vals);

static void     exchange              (PicmanDrawable  *drawable,
                                       PicmanPreview   *preview);

static gboolean exchange_dialog       (PicmanDrawable  *preview);
static void     color_button_callback (GtkWidget     *widget,
                                       gpointer       data);
static void     scale_callback        (GtkAdjustment *adj,
                                       gpointer       data);


/* some global variables */
static myParams xargs =
{
  { 0.0, 0.0, 0.0, 1.0 }, /* from      */
  { 0.0, 0.0, 0.0, 1.0 }, /* to        */
  { 0.0, 0.0, 0.0, 1.0 }  /* threshold */
};

static GtkWidget    *from_colorbutton;
static gboolean      lock_threshold = FALSE;

/* lets declare what we want to do */
const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

/* run program */
MAIN ()

/* tell PICMAN who we are */
static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",        "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",           "Input image"        },
    { PICMAN_PDB_DRAWABLE, "drawable",        "Input drawable"     },
    { PICMAN_PDB_INT8,     "from-red",        "Red value (from)"   },
    { PICMAN_PDB_INT8,     "from-green",      "Green value (from)" },
    { PICMAN_PDB_INT8,     "from-blue",       "Blue value (from)"  },
    { PICMAN_PDB_INT8,     "to-red",          "Red value (to)"     },
    { PICMAN_PDB_INT8,     "to-green",        "Green value (to)"   },
    { PICMAN_PDB_INT8,     "to-blue",         "Blue value (to)"    },
    { PICMAN_PDB_INT8,     "red-threshold",   "Red threshold"      },
    { PICMAN_PDB_INT8,     "green-threshold", "Green threshold"    },
    { PICMAN_PDB_INT8,     "blue-threshold",  "Blue threshold"     }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Swap one color with another"),
                          "Exchange one color with another, optionally setting a threshold "
                          "to convert from one shade to another",
                          "robert@experimental.net",
                          "robert@experimental.net",
                          "June 17th, 1997",
                          N_("_Color Exchange..."),
                          "RGB*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Colors/Map");
}

/* main function */
static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[1];
  PicmanRunMode        runmode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  PicmanDrawable      *drawable;

  *nreturn_vals = 1;
  *return_vals = values;

  INIT_I18N ();

  values[0].type = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  runmode        = param[0].data.d_int32;
  drawable       = picman_drawable_get (param[2].data.d_drawable);

  switch (runmode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /* retrieve stored arguments (if any) */
      picman_get_data (PLUG_IN_PROC, &xargs);
      /* initialize using foreground color */
      picman_context_get_foreground (&xargs.from);

      if (! exchange_dialog (drawable))
        return;
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      picman_get_data (PLUG_IN_PROC, &xargs);
      /*
       * instead of recalling the last-set values,
       * run with the current foreground as 'from'
       * color, making ALT-F somewhat more useful.
       */
      picman_context_get_foreground (&xargs.from);
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      if (nparams != 12)
        {
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
      else
        {
          picman_rgb_set_uchar (&xargs.from,
                              param[3].data.d_int8,
                              param[4].data.d_int8,
                              param[5].data.d_int8);
          picman_rgb_set_uchar (&xargs.to,
                              param[6].data.d_int8,
                              param[7].data.d_int8,
                              param[8].data.d_int8);
          picman_rgb_set_uchar (&xargs.threshold,
                              param[9].data.d_int8,
                              param[10].data.d_int8,
                              param[11].data.d_int8);
        }
      break;

    default:
      break;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      if (picman_drawable_is_rgb (drawable->drawable_id))
        {
          picman_progress_init (_("Color Exchange"));
          picman_tile_cache_ntiles (2 * (drawable->width /
                                       picman_tile_width () + 1));
          exchange (drawable, NULL);
          picman_drawable_detach (drawable);

          /* store our settings */
          if (runmode == PICMAN_RUN_INTERACTIVE)
            picman_set_data (PLUG_IN_PROC, &xargs, sizeof (myParams));

          /* and flush */
          if (runmode != PICMAN_RUN_NONINTERACTIVE)
            picman_displays_flush ();
        }
      else
        status = PICMAN_PDB_EXECUTION_ERROR;
    }
  values[0].data.d_status = status;
}

static gboolean
preview_event_handler (GtkWidget *area,
                       GdkEvent  *event,
                       GtkWidget *preview)
{
  gint            pos;
  guchar         *buf;
  guint32         drawable_id;
  PicmanRGB         color;
  GdkEventButton *button_event = (GdkEventButton *)event;

  buf = PICMAN_PREVIEW_AREA (area)->buf;
  drawable_id = PICMAN_DRAWABLE_PREVIEW (preview)->drawable->drawable_id;

  switch (event->type)
    {
    case GDK_BUTTON_PRESS:
      if (button_event->button == 2)
        {
          pos = event->button.x * picman_drawable_bpp (drawable_id) +
                event->button.y * PICMAN_PREVIEW_AREA (area)->rowstride;

          picman_rgb_set_uchar (&color, buf[pos], buf[pos + 1], buf[pos + 2]);
          picman_color_button_set_color (PICMAN_COLOR_BUTTON (from_colorbutton),
                                       &color);
        }
      break;

   default:
      break;
    }

  return FALSE;
}

/* show our dialog */
static gboolean
exchange_dialog (PicmanDrawable *drawable)
{
  GtkWidget    *dialog;
  GtkWidget    *main_vbox;
  GtkWidget    *hbox;
  GtkWidget    *frame;
  GtkWidget    *preview;
  GtkWidget    *table;
  GtkWidget    *threshold;
  GtkWidget    *colorbutton;
  GtkObject    *adj;
  GtkSizeGroup *group;
  gint          framenumber;
  gboolean      run;

  picman_ui_init (PLUG_IN_BINARY, TRUE);

  dialog = picman_dialog_new (_("Color Exchange"), PLUG_IN_ROLE,
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

  /* do some boxes here */
  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);

  frame = picman_frame_new (_("Middle-Click Inside Preview to "
                            "Pick \"From Color\""));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  preview = picman_drawable_preview_new (drawable, NULL);
  gtk_container_add (GTK_CONTAINER (frame), preview);
  gtk_widget_show (preview);

  g_signal_connect_swapped (preview, "invalidated",
                            G_CALLBACK (exchange),
                            drawable);
  g_signal_connect (PICMAN_PREVIEW (preview)->area, "event",
                    G_CALLBACK (preview_event_handler),
                    preview);

  /*  a hidden color_button to handle the threshold more easily  */
  threshold = picman_color_button_new (NULL, 1, 1,
                                     &xargs.threshold,
                                     PICMAN_COLOR_AREA_FLAT);

  g_signal_connect (threshold, "color-changed",
                    G_CALLBACK (picman_color_button_get_color),
                    &xargs.threshold);
  g_signal_connect (threshold, "color-changed",
                    G_CALLBACK (color_button_callback),
                    &xargs.threshold);
  g_signal_connect_swapped (threshold, "color-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  /* and our scales */

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  for (framenumber = 0; framenumber < 2; framenumber++)
    {
      GtkWidget    *vbox;
      GtkWidget    *image;
      gint          row = 0;

      frame = picman_frame_new (framenumber ? _("To Color") : _("From Color"));
      gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
      gtk_widget_show (frame);

      vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      gtk_container_add (GTK_CONTAINER (frame), vbox);
      gtk_widget_show (vbox);

      table = gtk_table_new (framenumber ? 4 : 8, 4, FALSE);
      gtk_table_set_col_spacings (GTK_TABLE (table), 6);
      gtk_table_set_row_spacings (GTK_TABLE (table), 6);
      gtk_table_set_row_spacing (GTK_TABLE (table), 0, 12);

      if (! framenumber)
        {
          gtk_table_set_row_spacing (GTK_TABLE (table), 1, 2);
          gtk_table_set_row_spacing (GTK_TABLE (table), 3, 2);
          gtk_table_set_row_spacing (GTK_TABLE (table), 5, 2);
        }

      gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
      gtk_widget_show (table);

      colorbutton = picman_color_button_new (framenumber ?
                                           _("Color Exchange: To Color") :
                                           _("Color Exchange: From Color"),
                                           SCALE_WIDTH / 2, 16,
                                           (framenumber ?
                                            &xargs.to : &xargs.from),
                                           PICMAN_COLOR_AREA_FLAT);
      picman_table_attach_aligned (GTK_TABLE (table), 0, row++,
                                 NULL, 0.0, 0.0,
                                 colorbutton, 1, FALSE);

      g_signal_connect (colorbutton, "color-changed",
                        G_CALLBACK (picman_color_button_get_color),
                        framenumber ? &xargs.to : &xargs.from);
      g_signal_connect (colorbutton, "color-changed",
                        G_CALLBACK (color_button_callback),
                        framenumber ? &xargs.to : &xargs.from);
      g_signal_connect_swapped (colorbutton, "color-changed",
                                G_CALLBACK (picman_preview_invalidate),
                                preview);

      if (! framenumber)
        from_colorbutton = colorbutton;

      /*  Red  */
      image = gtk_image_new_from_stock (PICMAN_STOCK_CHANNEL_RED,
                                        GTK_ICON_SIZE_BUTTON);
      gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0.5);
      gtk_table_attach (GTK_TABLE (table), image,
                        0, 1, row, row + 1 + (framenumber ? 0 : 1),
                        GTK_FILL, GTK_FILL, 0, 0);
      gtk_widget_show (image);

      adj = picman_scale_entry_new (GTK_TABLE (table), 1, row++,
                                  _("_Red:"), SCALE_WIDTH, 0,
                                  framenumber ? xargs.to.r : xargs.from.r,
                                  0.0, 1.0, 0.01, 0.1, 3,
                                  TRUE, 0, 0,
                                  NULL, NULL);

      g_object_set_data (G_OBJECT (adj), "colorbutton", colorbutton);
      g_object_set_data (G_OBJECT (colorbutton), "red", adj);

      g_signal_connect (adj, "value-changed",
                        G_CALLBACK (picman_double_adjustment_update),
                        framenumber ? &xargs.to.r : &xargs.from.r);
      g_signal_connect (adj, "value-changed",
                        G_CALLBACK (scale_callback),
                        framenumber ? &xargs.to : &xargs.from);
      g_signal_connect_swapped (adj, "value-changed",
                                G_CALLBACK (picman_preview_invalidate),
                                preview);

      gtk_size_group_add_widget (group, PICMAN_SCALE_ENTRY_LABEL (adj));

      if (! framenumber)
        {
          adj = picman_scale_entry_new (GTK_TABLE (table), 1, row++,
                                      _("R_ed threshold:"), SCALE_WIDTH, 0,
                                      xargs.threshold.r,
                                      0.0, 1.0, 0.01, 0.1, 3,
                                      TRUE, 0, 0,
                                      NULL, NULL);

          g_object_set_data (G_OBJECT (adj), "colorbutton", threshold);
          g_object_set_data (G_OBJECT (threshold), "red", adj);

          g_signal_connect (adj, "value-changed",
                            G_CALLBACK (picman_double_adjustment_update),
                            &xargs.threshold.r);
          g_signal_connect (adj, "value-changed",
                            G_CALLBACK (scale_callback),
                            &xargs.threshold);
          g_signal_connect_swapped (adj, "value-changed",
                                    G_CALLBACK (picman_preview_invalidate),
                                    preview);

          gtk_size_group_add_widget (group, PICMAN_SCALE_ENTRY_LABEL (adj));
        }

      /*  Green  */
      image = gtk_image_new_from_stock (PICMAN_STOCK_CHANNEL_GREEN,
                                        GTK_ICON_SIZE_BUTTON);
      gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0.5);
      gtk_table_attach (GTK_TABLE (table), image,
                        0, 1, row, row + 1 + (framenumber ? 0 : 1),
                        GTK_FILL, GTK_FILL, 0, 0);
      gtk_widget_show (image);

      adj = picman_scale_entry_new (GTK_TABLE (table), 1, row++,
                                  _("_Green:"), SCALE_WIDTH, 0,
                                  framenumber ? xargs.to.g : xargs.from.g,
                                  0.0, 1.0, 0.01, 0.1, 3,
                                  TRUE, 0, 0,
                                  NULL, NULL);

      g_object_set_data (G_OBJECT (adj), "colorbutton", colorbutton);
      g_object_set_data (G_OBJECT (colorbutton), "green", adj);

      g_signal_connect (adj, "value-changed",
                        G_CALLBACK (picman_double_adjustment_update),
                        framenumber ? &xargs.to.g : &xargs.from.g);
      g_signal_connect (adj, "value-changed",
                        G_CALLBACK (scale_callback),
                        framenumber ? &xargs.to : &xargs.from);
      g_signal_connect_swapped (adj, "value-changed",
                                G_CALLBACK (picman_preview_invalidate),
                                preview);

      gtk_size_group_add_widget (group, PICMAN_SCALE_ENTRY_LABEL (adj));

      if (! framenumber)
        {
          adj = picman_scale_entry_new (GTK_TABLE (table), 1, row++,
                                      _("G_reen threshold:"), SCALE_WIDTH, 0,
                                      xargs.threshold.g,
                                      0.0, 1.0, 0.01, 0.1, 3,
                                      TRUE, 0, 0,
                                      NULL, NULL);

          g_object_set_data (G_OBJECT (adj), "colorbutton", threshold);
          g_object_set_data (G_OBJECT (threshold), "green", adj);

          g_signal_connect (adj, "value-changed",
                            G_CALLBACK (picman_double_adjustment_update),
                            &xargs.threshold.g);
          g_signal_connect (adj, "value-changed",
                            G_CALLBACK (scale_callback),
                            &xargs.threshold);
          g_signal_connect_swapped (adj, "value-changed",
                                    G_CALLBACK (picman_preview_invalidate),
                                    preview);

          gtk_size_group_add_widget (group, PICMAN_SCALE_ENTRY_LABEL (adj));
        }

      /*  Blue  */
      image = gtk_image_new_from_stock (PICMAN_STOCK_CHANNEL_BLUE,
                                        GTK_ICON_SIZE_BUTTON);
      gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0.5);
      gtk_table_attach (GTK_TABLE (table), image,
                        0, 1, row, row + 1 + (framenumber ? 0 : 1),
                        GTK_FILL, GTK_FILL, 0, 0);
      gtk_widget_show (image);

      adj = picman_scale_entry_new (GTK_TABLE (table), 1, row++,
                                  _("_Blue:"), SCALE_WIDTH, 0,
                                  framenumber ? xargs.to.b : xargs.from.b,
                                  0.0, 1.0, 0.01, 0.1, 3,
                                  TRUE, 0, 0,
                                  NULL, NULL);

      g_object_set_data (G_OBJECT (adj), "colorbutton", colorbutton);
      g_object_set_data (G_OBJECT (colorbutton), "blue", adj);

      g_signal_connect (adj, "value-changed",
                        G_CALLBACK (picman_double_adjustment_update),
                        framenumber ? &xargs.to.b : &xargs.from.b);
      g_signal_connect (adj, "value-changed",
                        G_CALLBACK (scale_callback),
                        framenumber ? &xargs.to : &xargs.from);
      g_signal_connect_swapped (adj, "value-changed",
                                G_CALLBACK (picman_preview_invalidate),
                                preview);

      gtk_size_group_add_widget (group, PICMAN_SCALE_ENTRY_LABEL (adj));

      if (! framenumber)
        {
          adj = picman_scale_entry_new (GTK_TABLE (table), 1, row++,
                                      _("B_lue threshold:"), SCALE_WIDTH, 0,
                                      xargs.threshold.b,
                                      0.0, 1.0, 0.01, 0.1, 3,
                                      TRUE, 0, 0,
                                      NULL, NULL);

          g_object_set_data (G_OBJECT (adj), "colorbutton", threshold);
          g_object_set_data (G_OBJECT (threshold), "blue", adj);

          g_signal_connect (adj, "value-changed",
                            G_CALLBACK (picman_double_adjustment_update),
                            &xargs.threshold.b);
          g_signal_connect (adj, "value-changed",
                            G_CALLBACK (scale_callback),
                            &xargs.threshold);
          g_signal_connect_swapped (adj, "value-changed",
                                    G_CALLBACK (picman_preview_invalidate),
                                    preview);

          gtk_size_group_add_widget (group, PICMAN_SCALE_ENTRY_LABEL (adj));
        }

      if (! framenumber)
        {
          GtkWidget *button;

          button = gtk_check_button_new_with_mnemonic (_("Lock _thresholds"));
          gtk_table_attach (GTK_TABLE (table), button, 2, 4, row, row + 1,
                            GTK_FILL, 0, 0, 0);
          gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                        lock_threshold);
          gtk_widget_show (button);

          g_signal_connect (button, "clicked",
                            G_CALLBACK (picman_toggle_button_update),
                            &lock_threshold);
          g_signal_connect_swapped (button, "clicked",
                                    G_CALLBACK (picman_preview_invalidate),
                                    preview);
        }
    }

  g_object_unref (group);

  /* show everything */
  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
color_button_callback (GtkWidget *widget,
                       gpointer   data)
{
  GtkObject *red_adj;
  GtkObject *green_adj;
  GtkObject *blue_adj;
  PicmanRGB   *color;

  color = (PicmanRGB *) data;

  red_adj   = (GtkObject *) g_object_get_data (G_OBJECT (widget), "red");
  green_adj = (GtkObject *) g_object_get_data (G_OBJECT (widget), "green");
  blue_adj  = (GtkObject *) g_object_get_data (G_OBJECT (widget), "blue");

  if (red_adj)
    gtk_adjustment_set_value (GTK_ADJUSTMENT (red_adj),   color->r);
  if (green_adj)
    gtk_adjustment_set_value (GTK_ADJUSTMENT (green_adj), color->g);
  if (blue_adj)
    gtk_adjustment_set_value (GTK_ADJUSTMENT (blue_adj),  color->b);
}

static void
scale_callback (GtkAdjustment *adj,
                gpointer       data)
{
  GtkObject *object;
  PicmanRGB   *color;

  color = (PicmanRGB *) data;

  object = g_object_get_data (G_OBJECT (adj), "colorbutton");

  if (PICMAN_IS_COLOR_BUTTON (object))
    {
      if (color == &xargs.threshold && lock_threshold == TRUE)
        {
          gdouble value = gtk_adjustment_get_value (adj);

          picman_rgb_set (color, value, value, value);
        }

      picman_color_button_set_color (PICMAN_COLOR_BUTTON (object), color);
    }
}

/* do the exchanging */
static void
exchange (PicmanDrawable *drawable,
          PicmanPreview  *preview)
{
  PicmanPixelRgn  srcPR, destPR;
  guchar        min_red,  min_green,  min_blue;
  guchar        max_red,  max_green,  max_blue;
  guchar        from_red, from_green, from_blue;
  guchar        to_red,   to_green,   to_blue;
  guchar       *src_row, *dest_row;
  gint          x, y, bpp = drawable->bpp;
  gboolean      has_alpha;
  gint          x1, y1, y2;
  gint          width, height;
  PicmanRGB       min;
  PicmanRGB       max;

  if (preview)
    {
      picman_preview_get_position (preview, &x1, &y1);
      picman_preview_get_size (preview, &width, &height);
    }
  else if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                           &x1, &y1, &width, &height))
    {
      return;
    }

  y2 = y1 + height;

  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
  /* allocate memory */
  src_row = g_new (guchar, drawable->width * bpp);

  picman_rgb_get_uchar (&xargs.from, &from_red, &from_green, &from_blue);
  picman_rgb_get_uchar (&xargs.to,   &to_red,   &to_green,   &to_blue);

  /* get boundary values */
  min = xargs.from;
  picman_rgb_subtract (&min, &xargs.threshold);
  picman_rgb_clamp (&min);
  picman_rgb_get_uchar (&min, &min_red, &min_green, &min_blue);

  max = xargs.from;
  picman_rgb_add (&max, &xargs.threshold);
  picman_rgb_clamp (&max);
  picman_rgb_get_uchar (&max, &max_red, &max_green, &max_blue);

  dest_row = g_new (guchar, drawable->width * bpp);

  picman_pixel_rgn_init (&srcPR, drawable,
                       x1, y1, width, height, FALSE, FALSE);
  picman_pixel_rgn_init (&destPR, drawable,
                       x1, y1, width, height, (preview == NULL), TRUE);

  for (y = y1; y < y2; y++)
    {
      picman_pixel_rgn_get_row (&srcPR, src_row, x1, y, width);

      for (x = 0; x < width; x++)
        {
          guchar pixel_red, pixel_green, pixel_blue;
          guchar new_red, new_green, new_blue;
          guint  idx;

          /* get current pixel-values */
          pixel_red   = src_row[x * bpp];
          pixel_green = src_row[x * bpp + 1];
          pixel_blue  = src_row[x * bpp + 2];

          idx = x * bpp;

          /* want this pixel? */
          if (pixel_red >= min_red &&
              pixel_red <= max_red &&
              pixel_green >= min_green &&
              pixel_green <= max_green &&
              pixel_blue >= min_blue &&
              pixel_blue <= max_blue)
            {
              guchar red_delta, green_delta, blue_delta;

              red_delta   = pixel_red > from_red ?
                pixel_red - from_red : from_red - pixel_red;
              green_delta = pixel_green > from_green ?
                pixel_green - from_green : from_green - pixel_green;
              blue_delta  = pixel_blue > from_blue ?
                pixel_blue - from_blue : from_blue - pixel_blue;

              new_red   = CLAMP (to_red   + red_delta,   0, 255);
              new_green = CLAMP (to_green + green_delta, 0, 255);
              new_blue  = CLAMP (to_blue  + blue_delta,  0, 255);
            }
          else
            {
              new_red   = pixel_red;
              new_green = pixel_green;
              new_blue  = pixel_blue;
            }

          /* fill buffer */
          dest_row[idx + 0] = new_red;
          dest_row[idx + 1] = new_green;
          dest_row[idx + 2] = new_blue;

          /* copy alpha-channel */
          if (has_alpha)
            dest_row[idx + 3] = src_row[x * bpp + 3];
        }
      /* store the dest */
      picman_pixel_rgn_set_row (&destPR, dest_row, x1, y, width);

      /* and tell the user what we're doing */
      if (!preview && (y % 10) == 0)
        picman_progress_update ((gdouble) y / (gdouble) height);
    }

  g_free (src_row);
  g_free (dest_row);

  if (preview)
    {
      picman_drawable_preview_draw_region (PICMAN_DRAWABLE_PREVIEW (preview),
                                         &destPR);
    }
  else
    {
      picman_progress_update (1.0);
      /* update the processed region */
      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
      picman_drawable_update (drawable->drawable_id, x1, y1, width, height);
    }
}
