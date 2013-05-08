/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#include "config.h"

#include <glib.h>  /* lcms.h uses the "inline" keyword */

#include <string.h>

#ifdef G_OS_WIN32
#define STRICT
#include <windows.h>
#define LCMS_WIN_TYPES_ALREADY_DEFINED
#endif

#include <lcms2.h>

#include <gegl.h>
#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_QUARTZ
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreServices/CoreServices.h>
#endif

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanmodule/picmanmodule.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "libpicman/libpicman-intl.h"


#define CDISPLAY_TYPE_LCMS            (cdisplay_lcms_get_type ())
#define CDISPLAY_LCMS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CDISPLAY_TYPE_LCMS, CdisplayLcms))
#define CDISPLAY_LCMS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CDISPLAY_TYPE_LCMS, CdisplayLcmsClass))
#define CDISPLAY_IS_LCMS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CDISPLAY_TYPE_LCMS))
#define CDISPLAY_IS_LCMS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CDISPLAY_TYPE_LCMS))


typedef struct _CdisplayLcms      CdisplayLcms;
typedef struct _CdisplayLcmsClass CdisplayLcmsClass;

struct _CdisplayLcms
{
  PicmanColorDisplay  parent_instance;

  cmsHTRANSFORM     transform;
};

struct _CdisplayLcmsClass
{
  PicmanColorDisplayClass parent_instance;
};


GType               cdisplay_lcms_get_type             (void);

static void         cdisplay_lcms_finalize             (GObject           *object);

static GtkWidget  * cdisplay_lcms_configure            (PicmanColorDisplay  *display);
static void         cdisplay_lcms_convert_surface      (PicmanColorDisplay  *display,
                                                        cairo_surface_t   *surface);
static void         cdisplay_lcms_changed              (PicmanColorDisplay  *display);

static cmsHPROFILE  cdisplay_lcms_get_rgb_profile      (CdisplayLcms      *lcms);
static cmsHPROFILE  cdisplay_lcms_get_display_profile  (CdisplayLcms      *lcms);
static cmsHPROFILE  cdisplay_lcms_get_printer_profile  (CdisplayLcms      *lcms);

static void         cdisplay_lcms_attach_labelled      (GtkTable          *table,
                                                        gint               row,
                                                        const gchar       *text,
                                                        GtkWidget         *widget);
static void         cdisplay_lcms_update_profile_label (CdisplayLcms      *lcms,
                                                        const gchar       *name);
static void         cdisplay_lcms_notify_profile       (GObject           *config,
                                                        GParamSpec        *pspec,
                                                        CdisplayLcms      *lcms);


static const PicmanModuleInfo cdisplay_lcms_info =
{
  PICMAN_MODULE_ABI_VERSION,
  N_("Color management display filter using ICC color profiles"),
  "Sven Neumann",
  "v0.3",
  "(c) 2005 - 2007, released under the GPL",
  "2005 - 2007"
};

G_DEFINE_DYNAMIC_TYPE (CdisplayLcms, cdisplay_lcms,
                       PICMAN_TYPE_COLOR_DISPLAY)

G_MODULE_EXPORT const PicmanModuleInfo *
picman_module_query (GTypeModule *module)
{
  return &cdisplay_lcms_info;
}

G_MODULE_EXPORT gboolean
picman_module_register (GTypeModule *module)
{
  cdisplay_lcms_register_type (module);

  return TRUE;
}

static void
cdisplay_lcms_class_init (CdisplayLcmsClass *klass)
{
  GObjectClass          *object_class  = G_OBJECT_CLASS (klass);
  PicmanColorDisplayClass *display_class = PICMAN_COLOR_DISPLAY_CLASS (klass);

  object_class->finalize         = cdisplay_lcms_finalize;

  display_class->name            = _("Color Management");
  display_class->help_id         = "picman-colordisplay-lcms";
  display_class->stock_id        = PICMAN_STOCK_DISPLAY_FILTER_LCMS;

  display_class->configure       = cdisplay_lcms_configure;
  display_class->convert_surface = cdisplay_lcms_convert_surface;
  display_class->changed         = cdisplay_lcms_changed;
}

static void
cdisplay_lcms_class_finalize (CdisplayLcmsClass *klass)
{
}

static void
cdisplay_lcms_init (CdisplayLcms *lcms)
{
  lcms->transform = NULL;
}

static void
cdisplay_lcms_finalize (GObject *object)
{
  CdisplayLcms *lcms = CDISPLAY_LCMS (object);

  if (lcms->transform)
    {
      cmsDeleteTransform (lcms->transform);
      lcms->transform = NULL;
    }

  G_OBJECT_CLASS (cdisplay_lcms_parent_class)->finalize (object);
}

static void
cdisplay_lcms_profile_get_info (cmsHPROFILE   profile,
                                gchar       **name,
                                gchar       **info)
{
  if (profile)
    {
      cmsUInt32Number  descSize;
      gchar           *descData;

      descSize = cmsGetProfileInfoASCII (profile, cmsInfoDescription,
                                         "en", "US", NULL, 0);
      if (descSize > 0)
        {
          descData = g_new (gchar, descSize + 1);
          descSize = cmsGetProfileInfoASCII (profile, cmsInfoDescription,
                                             "en", "US", descData, descSize);
          if (descSize > 0)
            *name = descData;
          else
            g_free (descData);
        }

      if (! *name)
        {
          descSize = cmsGetProfileInfoASCII (profile, cmsInfoModel,
                                             "en", "US", NULL, 0);
          if (descSize > 0)
            {
              descData = g_new (gchar, descSize + 1);
              descSize = cmsGetProfileInfoASCII(profile, cmsInfoModel,
                                                "en", "US", descData, descSize);
              if (descSize > 0)
                *name = descData;
              else
                g_free (descData);
            }
        }

      if (*name && ! g_utf8_validate (*name, -1, NULL))
        {
          g_free (*name);
          *name = g_strdup (_("(invalid UTF-8 string)"));
        }

      if (! *name)
	{
	  /* a color profile without a name */
	  *name = g_strdup (_("(unnamed profile)"));
	}

      descSize = cmsGetProfileInfoASCII (profile, cmsInfoManufacturer,
                                         "en", "US", NULL, 0);
      if (descSize > 0)
        {
          descData = g_new (gchar, descSize + 1);
          descSize = cmsGetProfileInfoASCII (profile, cmsInfoManufacturer,
                                             "en", "US", descData, descSize);
          if (descSize > 0)
            *info = descData;
          else
            g_free (descData);
        }

      if (*info && ! g_utf8_validate (*info, -1, NULL))
        {
          g_free (*info);
          *info = NULL;
        }
    }
  else
    {
      *name = g_strdup (_("None"));
      *info = NULL;
    }
}

static GtkWidget *
cdisplay_lcms_configure (PicmanColorDisplay *display)
{
  CdisplayLcms *lcms   = CDISPLAY_LCMS (display);
  GObject      *config = G_OBJECT (picman_color_display_get_config (display));
  GtkWidget    *vbox;
  GtkWidget    *hint;
  GtkWidget    *table;
  GtkWidget    *label;
  gint          row = 0;

  if (! config)
    return NULL;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);

  hint = picman_hint_box_new (_("This filter takes its configuration "
                              "from the Color Management section "
                              "in the Preferences dialog."));
  gtk_box_pack_start (GTK_BOX (vbox), hint, FALSE, FALSE, 0);
  gtk_widget_show (hint);

  table = gtk_table_new (5, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacing (GTK_TABLE (table), 0, 12);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  cdisplay_lcms_attach_labelled (GTK_TABLE (table), row++,
                                 _("Mode of operation:"),
                                 picman_prop_enum_label_new (config, "mode"));

  label = gtk_label_new (NULL);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  g_object_set_data (G_OBJECT (lcms), "rgb-profile", label);
  cdisplay_lcms_attach_labelled (GTK_TABLE (table), row++,
                                 _("Image profile:"),
                                 label);
  cdisplay_lcms_update_profile_label (lcms, "rgb-profile");

  label = gtk_label_new (NULL);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  g_object_set_data (G_OBJECT (lcms), "display-profile", label);
  cdisplay_lcms_attach_labelled (GTK_TABLE (table), row++,
                                 _("Monitor profile:"),
                                 label);
  cdisplay_lcms_update_profile_label (lcms, "display-profile");

  label = gtk_label_new (NULL);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  g_object_set_data (G_OBJECT (lcms), "printer-profile", label);
  cdisplay_lcms_attach_labelled (GTK_TABLE (table), row++,
                                 _("Print simulation profile:"),
                                 label);
  cdisplay_lcms_update_profile_label (lcms, "printer-profile");

  g_signal_connect_object (config, "notify",
                           G_CALLBACK (cdisplay_lcms_notify_profile),
                           lcms, 0);

  return vbox;
}

static void
cdisplay_lcms_convert_surface (PicmanColorDisplay *display,
                               cairo_surface_t  *surface)
{
  CdisplayLcms   *lcms   = CDISPLAY_LCMS (display);
  gint            width  = cairo_image_surface_get_width (surface);
  gint            height = cairo_image_surface_get_height (surface);
  gint            stride = cairo_image_surface_get_stride (surface);
  guchar         *buf    = cairo_image_surface_get_data (surface);
  cairo_format_t  fmt    = cairo_image_surface_get_format (surface);
  guchar         *rowbuf;
  gint            x, y;
  guchar          r, g, b, a;

  if (fmt != CAIRO_FORMAT_ARGB32)
    return;

  if (! lcms->transform)
    return;

  rowbuf = g_malloc (stride);

  for (y = 0; y < height; y++, buf += stride)
    {
      /* Switch buf from ARGB premul to ARGB non-premul, since lcms ignores the
       * alpha channel.  The macro takes care of byte order.
       */
      for (x = 0; x < width; x++)
        {
          PICMAN_CAIRO_ARGB32_GET_PIXEL (buf + 4*x, r, g, b, a);
          rowbuf[4*x+0] = a;
          rowbuf[4*x+1] = r;
          rowbuf[4*x+2] = g;
          rowbuf[4*x+3] = b;
        }

      cmsDoTransform (lcms->transform, rowbuf, rowbuf, width);

      /* And back to ARGB premul */
      for (x = 0; x < width; x++)
        {
          a = rowbuf[4*x+0];
          r = rowbuf[4*x+1];
          g = rowbuf[4*x+2];
          b = rowbuf[4*x+3];
          PICMAN_CAIRO_ARGB32_SET_PIXEL (buf + 4*x, r, g, b, a);
        }
    }

  g_free (rowbuf);
}

static void
cdisplay_lcms_changed (PicmanColorDisplay *display)
{
  CdisplayLcms    *lcms   = CDISPLAY_LCMS (display);
  PicmanColorConfig *config = picman_color_display_get_config (display);

  cmsHPROFILE      src_profile   = NULL;
  cmsHPROFILE      dest_profile  = NULL;
  cmsHPROFILE      proof_profile = NULL;
  cmsUInt32Number  flags         = 0;
  cmsUInt16Number  alarmCodes[cmsMAXCHANNELS] = { 0, };

  if (lcms->transform)
    {
      cmsDeleteTransform (lcms->transform);
      lcms->transform = NULL;
    }

  if (! config)
    return;

  switch (config->mode)
    {
    case PICMAN_COLOR_MANAGEMENT_OFF:
      return;

    case PICMAN_COLOR_MANAGEMENT_SOFTPROOF:
      proof_profile = cdisplay_lcms_get_printer_profile (lcms);
      /*  fallthru  */

    case PICMAN_COLOR_MANAGEMENT_DISPLAY:
      src_profile = cdisplay_lcms_get_rgb_profile (lcms);
      dest_profile = cdisplay_lcms_get_display_profile (lcms);
      break;
    }

  if (config->display_intent ==
      PICMAN_COLOR_RENDERING_INTENT_RELATIVE_COLORIMETRIC)
    {
      flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
    }

  if (proof_profile)
    {
      if (! src_profile)
        src_profile = cmsCreate_sRGBProfile ();

      if (! dest_profile)
        dest_profile = cmsCreate_sRGBProfile ();

      flags |= cmsFLAGS_SOFTPROOFING;

      if (config->simulation_gamut_check)
        {
          guchar r, g, b;

          flags |= cmsFLAGS_GAMUTCHECK;

          picman_rgb_get_uchar (&config->out_of_gamut_color, &r, &g, &b);

          alarmCodes[0] = (cmsUInt16Number) r * 256;
          alarmCodes[1] = (cmsUInt16Number) g * 256;
          alarmCodes[2] = (cmsUInt16Number) b * 256;

          cmsSetAlarmCodes (alarmCodes);
        }

      lcms->transform = cmsCreateProofingTransform (src_profile, TYPE_ARGB_8,
                                                    dest_profile, TYPE_ARGB_8,
                                                    proof_profile,
                                                    config->simulation_intent,
                                                    config->display_intent,
                                                    flags);
      cmsCloseProfile (proof_profile);
    }
  else if (src_profile || dest_profile)
    {
      if (! src_profile)
        src_profile = cmsCreate_sRGBProfile ();

      if (! dest_profile)
        dest_profile = cmsCreate_sRGBProfile ();

      lcms->transform = cmsCreateTransform (src_profile, TYPE_ARGB_8,
                                            dest_profile, TYPE_ARGB_8,
                                            config->display_intent,
                                            flags);
    }

  if (dest_profile)
    cmsCloseProfile (dest_profile);

  if (src_profile)
    cmsCloseProfile (src_profile);
}

static gboolean
cdisplay_lcms_profile_is_rgb (cmsHPROFILE profile)
{
  return (cmsGetColorSpace (profile) == cmsSigRgbData);
}

static cmsHPROFILE
cdisplay_lcms_get_rgb_profile (CdisplayLcms *lcms)
{
  PicmanColorConfig  *config;
  PicmanColorManaged *managed;
  cmsHPROFILE       profile = NULL;

  managed = picman_color_display_get_managed (PICMAN_COLOR_DISPLAY (lcms));

  if (managed)
    {
      gsize         len;
      const guint8 *data = picman_color_managed_get_icc_profile (managed, &len);

      if (data)
        profile = cmsOpenProfileFromMem ((gpointer) data, len);

      if (profile &&
          ! cdisplay_lcms_profile_is_rgb (profile))
        {
          cmsCloseProfile (profile);
          profile = NULL;
        }
    }

  if (! profile)
    {
      config = picman_color_display_get_config (PICMAN_COLOR_DISPLAY (lcms));

      if (config->rgb_profile)
        profile = cmsOpenProfileFromFile (config->rgb_profile, "r");
    }

  return profile;
}

static GdkScreen *
cdisplay_lcms_get_screen (CdisplayLcms *lcms,
                          gint         *monitor)
{
  PicmanColorManaged *managed;
  GdkScreen        *screen;

  managed = picman_color_display_get_managed (PICMAN_COLOR_DISPLAY (lcms));

  if (GTK_IS_WIDGET (managed))
    screen = gtk_widget_get_screen (GTK_WIDGET (managed));
  else
    screen = gdk_screen_get_default ();

  g_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);

  if (GTK_IS_WIDGET (managed) && gtk_widget_is_drawable (GTK_WIDGET (managed)))
    {
      GtkWidget *widget = GTK_WIDGET (managed);

      *monitor = gdk_screen_get_monitor_at_window (screen,
                                                   gtk_widget_get_window (widget));
    }
  else
    {
      *monitor = 0;
    }

  return screen;
}


static cmsHPROFILE
cdisplay_lcms_get_display_profile (CdisplayLcms *lcms)
{
  PicmanColorConfig *config;
  cmsHPROFILE      profile = NULL;

  config = picman_color_display_get_config (PICMAN_COLOR_DISPLAY (lcms));

#if defined GDK_WINDOWING_X11
  if (config->display_profile_from_gdk)
    {
      GdkScreen *screen;
      GdkAtom    type    = GDK_NONE;
      gint       format  = 0;
      gint       nitems  = 0;
      gint       monitor = 0;
      gchar     *atom_name;
      guchar    *data    = NULL;

      screen = cdisplay_lcms_get_screen (lcms, &monitor);

      if (monitor > 0)
        atom_name = g_strdup_printf ("_ICC_PROFILE_%d", monitor);
      else
        atom_name = g_strdup ("_ICC_PROFILE");

      if (gdk_property_get (gdk_screen_get_root_window (screen),
                            gdk_atom_intern (atom_name, FALSE),
                            GDK_NONE,
                            0, 64 * 1024 * 1024, FALSE,
                            &type, &format, &nitems, &data) && nitems > 0)
        {
          profile = cmsOpenProfileFromMem (data, nitems);
          g_free (data);
        }

      g_free (atom_name);
    }

#elif defined GDK_WINDOWING_QUARTZ
  if (config->display_profile_from_gdk)
    {
      CMProfileRef  prof    = NULL;
      gint          monitor = 0;

      cdisplay_lcms_get_screen (lcms, &monitor);

      CMGetProfileByAVID (monitor, &prof);

      if (prof)
        {
          CFDataRef data;

          data = CMProfileCopyICCData (NULL, prof);
          CMCloseProfile (prof);

          if (data)
            {
              UInt8 *buffer = g_malloc (CFDataGetLength (data));

              /* We cannot use CFDataGetBytesPtr(), because that returns
               * a const pointer where cmsOpenProfileFromMem wants a
               * non-const pointer.
               */
              CFDataGetBytes (data, CFRangeMake (0, CFDataGetLength (data)),
                              buffer);

              profile = cmsOpenProfileFromMem (buffer, CFDataGetLength (data));

              g_free (buffer);
              CFRelease (data);
            }
        }
    }

#elif defined G_OS_WIN32
  if (config->display_profile_from_gdk)
    {
      HDC hdc = GetDC (NULL);

      if (hdc)
        {
          gchar *path;
          gint32 len = 0;

          GetICMProfile (hdc, &len, NULL);
          path = g_new (gchar, len);

          if (GetICMProfile (hdc, &len, path))
            profile = cmsOpenProfileFromFile (path, "r");

          g_free (path);
          ReleaseDC (NULL, hdc);
        }
    }
#endif

  if (! profile && config->display_profile)
    profile = cmsOpenProfileFromFile (config->display_profile, "r");

  return profile;
}

static cmsHPROFILE
cdisplay_lcms_get_printer_profile (CdisplayLcms *lcms)
{
  PicmanColorConfig *config;

  config = picman_color_display_get_config (PICMAN_COLOR_DISPLAY (lcms));

  if (config->printer_profile)
    return cmsOpenProfileFromFile (config->printer_profile, "r");

  return NULL;
}

static void
cdisplay_lcms_attach_labelled (GtkTable    *table,
                               gint         row,
                               const gchar *text,
                               GtkWidget   *widget)
{
  GtkWidget *label;

  label = g_object_new (GTK_TYPE_LABEL,
                        "label",  text,
                        "xalign", 1.0,
                        "yalign", 0.5,
                        NULL);

  picman_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);
  gtk_table_attach (table, label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (label);

  if (GTK_IS_LABEL (widget))
    gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);

  gtk_table_attach (table, widget, 1, 2, row, row + 1,
                    GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  gtk_widget_show (widget);
}

static void
cdisplay_lcms_update_profile_label (CdisplayLcms *lcms,
                                    const gchar  *name)
{
  GtkWidget   *label;
  cmsHPROFILE  profile = NULL;
  gchar       *text    = NULL;
  gchar       *tooltip = NULL;

  label = g_object_get_data (G_OBJECT (lcms), name);

  if (! label)
    return;

  if (strcmp (name, "rgb-profile") == 0)
    {
      profile = cdisplay_lcms_get_rgb_profile (lcms);
    }
  else if (g_str_has_prefix (name, "display-profile"))
    {
      profile = cdisplay_lcms_get_display_profile (lcms);
    }
  else if (strcmp (name, "printer-profile") == 0)
    {
      profile = cdisplay_lcms_get_printer_profile (lcms);
    }
  else
    {
      g_return_if_reached ();
    }

  cdisplay_lcms_profile_get_info (profile, &text, &tooltip);

  gtk_label_set_text (GTK_LABEL (label), text);
  picman_help_set_help_data (label, tooltip, NULL);

  g_free (text);
  g_free (tooltip);

  if (profile)
    cmsCloseProfile (profile);
}

static void
cdisplay_lcms_notify_profile (GObject      *config,
                              GParamSpec   *pspec,
                              CdisplayLcms *lcms)
{
  cdisplay_lcms_update_profile_label (lcms, pspec->name);
}
