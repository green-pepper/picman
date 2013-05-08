/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Color management plug-in based on littleCMS
 * Copyright (C) 2006, 2007  Sven Neumann <sven@picman.org>
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

#include <string.h>

#include <glib.h>  /* lcms.h uses the "inline" keyword */

#include <lcms2.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_BINARY          "lcms"
#define PLUG_IN_ROLE            "picman-lcms"

#define PLUG_IN_PROC_SET        "plug-in-icc-profile-set"
#define PLUG_IN_PROC_SET_RGB    "plug-in-icc-profile-set-rgb"

#define PLUG_IN_PROC_APPLY      "plug-in-icc-profile-apply"
#define PLUG_IN_PROC_APPLY_RGB  "plug-in-icc-profile-apply-rgb"

#define PLUG_IN_PROC_INFO       "plug-in-icc-profile-info"
#define PLUG_IN_PROC_FILE_INFO  "plug-in-icc-profile-file-info"


enum
{
  STATUS,
  PROFILE_NAME,
  PROFILE_DESC,
  PROFILE_INFO,
  NUM_RETURN_VALS
};

enum
{
  PROC_SET,
  PROC_SET_RGB,
  PROC_APPLY,
  PROC_APPLY_RGB,
  PROC_INFO,
  PROC_FILE_INFO,
  NONE
};

typedef struct
{
  const gchar *name;
  const gint   min_params;
} Procedure;

typedef struct
{
  PicmanColorRenderingIntent intent;
  gboolean                 bpc;
} LcmsValues;


static void  query (void);
static void  run   (const gchar      *name,
                    gint              nparams,
                    const PicmanParam  *param,
                    gint             *nreturn_vals,
                    PicmanParam       **return_vals);

static PicmanPDBStatusType  lcms_icc_set       (PicmanColorConfig  *config,
                                              gint32            image,
                                              const gchar      *filename);
static PicmanPDBStatusType  lcms_icc_apply     (PicmanColorConfig  *config,
                                              PicmanRunMode       run_mode,
                                              gint32            image,
                                              const gchar      *filename,
                                              PicmanColorRenderingIntent intent,
                                              gboolean          bpc,
                                              gboolean         *dont_ask);
static PicmanPDBStatusType  lcms_icc_info      (PicmanColorConfig  *config,
                                              gint32            image,
                                              gchar           **name,
                                              gchar           **desc,
                                              gchar           **info);
static PicmanPDBStatusType  lcms_icc_file_info (const gchar      *filename,
                                              gchar           **name,
                                              gchar           **desc,
                                              gchar           **info);

static cmsHPROFILE  lcms_image_get_profile       (PicmanColorConfig *config,
                                                  gint32           image,
                                                  guchar          *checksum);
static gboolean     lcms_image_set_profile       (gint32           image,
                                                  cmsHPROFILE      profile,
                                                  const gchar     *filename,
                                                  gboolean         undo_group);
static gboolean     lcms_image_apply_profile     (gint32           image,
                                                  cmsHPROFILE      src_profile,
                                                  cmsHPROFILE      dest_profile,
                                                  const gchar     *filename,
                                                  PicmanColorRenderingIntent intent,
                                                  gboolean          bpc);
static void         lcms_image_transform_rgb     (gint32           image,
                                                  cmsHPROFILE      src_profile,
                                                  cmsHPROFILE      dest_profile,
                                                  PicmanColorRenderingIntent intent,
                                                  gboolean          bpc);
static void         lcms_image_transform_indexed (gint32           image,
                                                  cmsHPROFILE      src_profile,
                                                  cmsHPROFILE      dest_profile,
                                                  PicmanColorRenderingIntent intent,
                                                  gboolean          bpc);
static void         lcms_sRGB_checksum           (guchar          *digest);

static cmsHPROFILE  lcms_load_profile            (const gchar     *filename,
                                                  guchar          *checksum);

static gboolean     lcms_icc_apply_dialog        (gint32           image,
                                                  cmsHPROFILE      src_profile,
                                                  cmsHPROFILE      dest_profile,
                                                  gboolean        *dont_ask);

static PicmanPDBStatusType  lcms_dialog            (PicmanColorConfig *config,
                                                  gint32           image,
                                                  gboolean         apply,
                                                  LcmsValues      *values);


static const PicmanParamDef set_args[] =
{
  { PICMAN_PDB_INT32,  "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }"     },
  { PICMAN_PDB_IMAGE,  "image",        "Input image"                      },
  { PICMAN_PDB_STRING, "profile",      "Filename of an ICC color profile" }
};
static const PicmanParamDef set_rgb_args[] =
{
  { PICMAN_PDB_INT32,  "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }"     },
  { PICMAN_PDB_IMAGE,  "image",        "Input image"                      },
};
static const PicmanParamDef apply_args[] =
{
  { PICMAN_PDB_INT32,  "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }"     },
  { PICMAN_PDB_IMAGE,  "image",        "Input image"                      },
  { PICMAN_PDB_STRING, "profile",      "Filename of an ICC color profile" },
  { PICMAN_PDB_INT32,  "intent",       "Rendering intent (enum PicmanColorRenderingIntent)" },
  { PICMAN_PDB_INT32,  "bpc",          "Black point compensation"         }
};
static const PicmanParamDef apply_rgb_args[] =
{
  { PICMAN_PDB_INT32,  "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }"     },
  { PICMAN_PDB_IMAGE,  "image",        "Input image"                      },
  { PICMAN_PDB_INT32,  "intent",       "Rendering intent (enum PicmanColorRenderingIntent)" },
  { PICMAN_PDB_INT32,  "bpc",          "Black point compensation"         }
};
static const PicmanParamDef info_args[] =
{
  { PICMAN_PDB_IMAGE,  "image",        "Input image"                      },
};
static const PicmanParamDef file_info_args[] =
{
  { PICMAN_PDB_STRING, "profile",      "Filename of an ICC color profile" }
};

static const Procedure procedures[] =
{
  { PLUG_IN_PROC_SET,       2 },
  { PLUG_IN_PROC_SET_RGB,   2 },
  { PLUG_IN_PROC_APPLY,     2 },
  { PLUG_IN_PROC_APPLY_RGB, 2 },
  { PLUG_IN_PROC_INFO,      1 },
  { PLUG_IN_PROC_FILE_INFO, 1 }
};

const PicmanPlugInInfo PLUG_IN_INFO =
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
  static const PicmanParamDef info_return_vals[] =
  {
    { PICMAN_PDB_STRING, "profile-name", "Name"        },
    { PICMAN_PDB_STRING, "profile-desc", "Description" },
    { PICMAN_PDB_STRING, "profile-info", "Info"        }
  };

  picman_install_procedure (PLUG_IN_PROC_SET,
                          N_("Set a color profile on the image"),
                          "This procedure sets an ICC color profile on an "
                          "image using the 'icc-profile' parasite. It does "
                          "not do any color conversion.",
                          "Sven Neumann",
                          "Sven Neumann",
                          "2006, 2007",
                          N_("_Assign Color Profile..."),
                          "RGB*, INDEXED*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (set_args), 0,
                          set_args, NULL);

  picman_install_procedure (PLUG_IN_PROC_SET_RGB,
                          "Set the default RGB color profile on the image",
                          "This procedure sets the user-configured RGB "
                          "profile on an image using the 'icc-profile' "
                          "parasite. If no RGB profile is configured, sRGB "
                          "is assumed and the parasite is unset. This "
                          "procedure does not do any color conversion.",
                          "Sven Neumann",
                          "Sven Neumann",
                          "2006, 2007",
                          N_("Assign default RGB Profile"),
                          "RGB*, INDEXED*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (set_rgb_args), 0,
                          set_rgb_args, NULL);

  picman_install_procedure (PLUG_IN_PROC_APPLY,
                          _("Apply a color profile on the image"),
                          "This procedure transform from the image's color "
                          "profile (or the default RGB profile if none is "
                          "set) to the given ICC color profile. Only RGB "
                          "color profiles are accepted. The profile "
                          "is then set on the image using the 'icc-profile' "
                          "parasite.",
                          "Sven Neumann",
                          "Sven Neumann",
                          "2006, 2007",
                          N_("_Convert to Color Profile..."),
                          "RGB*, INDEXED*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (apply_args), 0,
                          apply_args, NULL);

  picman_install_procedure (PLUG_IN_PROC_APPLY_RGB,
                          "Apply default RGB color profile on the image",
                          "This procedure transform from the image's color "
                          "profile (or the default RGB profile if none is "
                          "set) to the configured default RGB color profile.  "
                          "The profile is then set on the image using the "
                          "'icc-profile' parasite.  If no RGB color profile "
                          "is configured, sRGB is assumed and the parasite "
                          "is unset.",
                          "Sven Neumann",
                          "Sven Neumann",
                          "2006, 2007",
                          N_("Convert to default RGB Profile"),
                          "RGB*, INDEXED*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (apply_rgb_args), 0,
                          apply_rgb_args, NULL);

  picman_install_procedure (PLUG_IN_PROC_INFO,
                          "Retrieve information about an image's color profile",
                          "This procedure returns information about the RGB "
                          "color profile attached to an image. If no RGB "
                          "color profile is attached, sRGB is assumed.",
                          "Sven Neumann",
                          "Sven Neumann",
                          "2006, 2007",
                          N_("Image Color Profile Information"),
                          "*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (info_args),
                          G_N_ELEMENTS (info_return_vals),
                          info_args, info_return_vals);

  picman_install_procedure (PLUG_IN_PROC_FILE_INFO,
                          "Retrieve information about a color profile",
                          "This procedure returns information about an ICC "
                          "color profile on disk.",
                          "Sven Neumann",
                          "Sven Neumann",
                          "2006, 2007",
                          N_("Color Profile Information"),
                          "*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (file_info_args),
                          G_N_ELEMENTS (info_return_vals),
                          file_info_args, info_return_vals);

  picman_plugin_menu_register (PLUG_IN_PROC_SET,
                             "<Image>/Image/Mode/Color Profile");
  picman_plugin_menu_register (PLUG_IN_PROC_APPLY,
                             "<Image>/Image/Mode/Color Profile");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  PicmanPDBStatusType         status   = PICMAN_PDB_CALLING_ERROR;
  gint                      proc     = NONE;
  PicmanRunMode               run_mode = PICMAN_RUN_NONINTERACTIVE;
  gint32                    image    = -1;
  const gchar              *filename = NULL;
  PicmanColorConfig          *config   = NULL;
  gboolean                  dont_ask = FALSE;
  PicmanColorRenderingIntent  intent;
  gboolean                  bpc;
  static PicmanParam          values[6];

  INIT_I18N ();
  gegl_init (NULL, NULL);

  values[0].type = PICMAN_PDB_STATUS;

  *nreturn_vals = 1;
  *return_vals  = values;

  for (proc = 0; proc < G_N_ELEMENTS (procedures); proc++)
    {
      if (strcmp (name, procedures[proc].name) == 0)
        break;
    }

  if (proc == NONE)
    goto done;

  if (nparams < procedures[proc].min_params)
    goto done;

  if (proc != PROC_FILE_INFO)
    config = picman_get_color_configuration ();

  if (config)
    intent = config->display_intent;
  else
    intent = PICMAN_COLOR_RENDERING_INTENT_PERCEPTUAL;

  bpc = (intent == PICMAN_COLOR_RENDERING_INTENT_RELATIVE_COLORIMETRIC);

  switch (proc)
    {
    case PROC_SET:
      run_mode = param[0].data.d_int32;
      image    = param[1].data.d_image;
      if (nparams > 2)
        filename = param[2].data.d_string;
      break;

    case PROC_APPLY:
      run_mode = param[0].data.d_int32;
      image    = param[1].data.d_image;
      if (nparams > 2)
        filename = param[2].data.d_string;
      if (nparams > 3)
        intent = param[3].data.d_int32;
      if (nparams > 4)
        bpc    = param[4].data.d_int32 ? TRUE : FALSE;
      break;

    case PROC_SET_RGB:
      run_mode = param[0].data.d_int32;
      image    = param[1].data.d_image;
      break;

    case PROC_APPLY_RGB:
      run_mode = param[0].data.d_int32;
      image    = param[1].data.d_image;
      if (nparams > 2)
        intent = param[2].data.d_int32;
      if (nparams > 3)
        bpc    = param[3].data.d_int32 ? TRUE : FALSE;
      break;

    case PROC_INFO:
      image    = param[0].data.d_image;
      break;

    case PROC_FILE_INFO:
      filename = param[0].data.d_string;
      break;
    }

  if (run_mode == PICMAN_RUN_INTERACTIVE)
    {
      LcmsValues values = { intent, bpc };

      switch (proc)
        {
        case PROC_SET:
          status = lcms_dialog (config, image, FALSE, &values);
          goto done;

        case PROC_APPLY:
          picman_get_data (name, &values);

          status = lcms_dialog (config, image, TRUE, &values);

          if (status == PICMAN_PDB_SUCCESS)
            picman_set_data (name, &values, sizeof (LcmsValues));
          goto done;

        default:
          break;
        }
    }

  switch (proc)
    {
    case PROC_SET:
    case PROC_SET_RGB:
      status = lcms_icc_set (config, image, filename);
      break;

    case PROC_APPLY:
    case PROC_APPLY_RGB:
      status = lcms_icc_apply (config, run_mode,
                               image, filename, intent, bpc,
                               &dont_ask);

      if (run_mode == PICMAN_RUN_INTERACTIVE)
        {
          *nreturn_vals = 2;

          values[1].type         = PICMAN_PDB_INT32;
          values[1].data.d_int32 = dont_ask;
        }
      break;

    case PROC_INFO:
    case PROC_FILE_INFO:
      {
        gchar *name = NULL;
        gchar *desc = NULL;
        gchar *info = NULL;

        if (proc == PROC_INFO)
          status = lcms_icc_info (config, image, &name, &desc, &info);
        else
          status = lcms_icc_file_info (filename, &name, &desc, &info);

        if (status == PICMAN_PDB_SUCCESS)
          {
            *nreturn_vals = NUM_RETURN_VALS;

            values[PROFILE_NAME].type          = PICMAN_PDB_STRING;
            values[PROFILE_NAME].data.d_string = name;

            values[PROFILE_DESC].type          = PICMAN_PDB_STRING;
            values[PROFILE_DESC].data.d_string = desc;

            values[PROFILE_INFO].type          = PICMAN_PDB_STRING;
            values[PROFILE_INFO].data.d_string = info;
          }
      }
      break;
    }

 done:
  if (run_mode != PICMAN_RUN_NONINTERACTIVE)
    picman_displays_flush ();

  if (config)
    g_object_unref (config);

  values[0].data.d_status = status;
}

static gchar *
lcms_icc_profile_get_name (cmsHPROFILE profile)
{
  cmsUInt32Number  descSize;
  gchar           *descData;
  gchar           *name = NULL;

  descSize = cmsGetProfileInfoASCII (profile, cmsInfoModel,
                                     "en", "US", NULL, 0);
  if (descSize > 0)
    {
      descData = g_new (gchar, descSize + 1);
      descSize = cmsGetProfileInfoASCII (profile, cmsInfoModel,
                                         "en", "US", descData, descSize);
      if (descSize > 0)
        name = picman_any_to_utf8 (descData, -1, NULL);

      g_free (descData);
    }

  return name;
}

static gchar *
lcms_icc_profile_get_desc (cmsHPROFILE profile)
{
  cmsUInt32Number  descSize;
  gchar           *descData;
  gchar           *desc = NULL;

  descSize = cmsGetProfileInfoASCII (profile, cmsInfoDescription,
                                     "en", "US", NULL, 0);
  if (descSize > 0)
    {
      descData = g_new (gchar, descSize + 1);
      descSize = cmsGetProfileInfoASCII (profile, cmsInfoDescription,
                                         "en", "US", descData, descSize);
      if (descSize > 0)
        desc = picman_any_to_utf8 (descData, -1, NULL);

      g_free (descData);
    }

  return desc;
}

static gchar *
lcms_icc_profile_get_info (cmsHPROFILE profile)
{
  cmsUInt32Number  descSize;
  gchar           *descData;
  gchar           *info = NULL;

  descSize = cmsGetProfileInfoASCII (profile, cmsInfoModel,
                                     "en", "US", NULL, 0);
  if (descSize > 0)
    {
      descData = g_new (gchar, descSize + 1);
      descSize = cmsGetProfileInfoASCII (profile, cmsInfoModel,
                                         "en", "US", descData, descSize);
      if (descSize > 0)
        info = picman_any_to_utf8 (descData, -1, NULL);

      g_free (descData);
    }

  return info;
}

static gboolean
lcms_icc_profile_is_rgb (cmsHPROFILE profile)
{
  return (cmsGetColorSpace (profile) == cmsSigRgbData);
}

static PicmanPDBStatusType
lcms_icc_set (PicmanColorConfig *config,
              gint32           image,
              const gchar     *filename)
{
  gboolean success;

  g_return_val_if_fail (PICMAN_IS_COLOR_CONFIG (config), PICMAN_PDB_CALLING_ERROR);
  g_return_val_if_fail (image != -1, PICMAN_PDB_CALLING_ERROR);

  if (filename)
    {
      success = lcms_image_set_profile (image, NULL, filename, TRUE);
    }
  else
    {
      success = lcms_image_set_profile (image, NULL, config->rgb_profile, TRUE);
    }

  return success ? PICMAN_PDB_SUCCESS : PICMAN_PDB_EXECUTION_ERROR;
}

static PicmanPDBStatusType
lcms_icc_apply (PicmanColorConfig          *config,
                PicmanRunMode               run_mode,
                gint32                    image,
                const gchar              *filename,
                PicmanColorRenderingIntent  intent,
                gboolean                  bpc,
                gboolean                 *dont_ask)
{
  PicmanPDBStatusType status       = PICMAN_PDB_SUCCESS;
  cmsHPROFILE       src_profile  = NULL;
  cmsHPROFILE       dest_profile = NULL;
  guchar            src_md5[16];
  guchar            dest_md5[16];

  g_return_val_if_fail (PICMAN_IS_COLOR_CONFIG (config), PICMAN_PDB_CALLING_ERROR);
  g_return_val_if_fail (image != -1, PICMAN_PDB_CALLING_ERROR);

  if (! filename)
    filename = config->rgb_profile;

  if (filename)
    {
      dest_profile = lcms_load_profile (filename, dest_md5);

      if (! dest_profile)
        return PICMAN_PDB_EXECUTION_ERROR;

      if (! lcms_icc_profile_is_rgb (dest_profile))
        {
          g_message (_("Color profile '%s' is not for RGB color space."),
                     picman_filename_to_utf8 (filename));

          cmsCloseProfile (dest_profile);
          return PICMAN_PDB_EXECUTION_ERROR;
        }
    }

  src_profile = lcms_image_get_profile (config, image, src_md5);

  if (src_profile && ! lcms_icc_profile_is_rgb (src_profile))
    {
      g_printerr ("lcms: attached color profile is not for RGB color space "
                  "(skipping)\n");

      cmsCloseProfile (src_profile);
      src_profile = NULL;
    }

  if (! src_profile && ! dest_profile)
    return PICMAN_PDB_SUCCESS;

  if (! src_profile)
    {
      src_profile = cmsCreate_sRGBProfile ();
      lcms_sRGB_checksum (src_md5);
    }

  if (! dest_profile)
    {
      dest_profile = cmsCreate_sRGBProfile ();
      lcms_sRGB_checksum (dest_md5);
    }

  if (memcmp (src_md5, dest_md5, 16) == 0)
    {
      gchar *src_desc  = lcms_icc_profile_get_desc (src_profile);
      gchar *dest_desc = lcms_icc_profile_get_desc (dest_profile);

      cmsCloseProfile (src_profile);
      cmsCloseProfile (dest_profile);

      g_printerr ("lcms: skipping conversion because profiles seem to be equal:\n");
      g_printerr (" %s\n", src_desc);
      g_printerr (" %s\n", dest_desc);

      g_free (src_desc);
      g_free (dest_desc);

      return PICMAN_PDB_SUCCESS;
    }

  if (run_mode == PICMAN_RUN_INTERACTIVE &&
      ! lcms_icc_apply_dialog (image, src_profile, dest_profile, dont_ask))
    {
      status = PICMAN_PDB_CANCEL;
    }

  if (status == PICMAN_PDB_SUCCESS &&
      ! lcms_image_apply_profile (image,
                                  src_profile, dest_profile, filename,
                                  intent, bpc))
    {
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  cmsCloseProfile (src_profile);
  cmsCloseProfile (dest_profile);

  return status;
}

static PicmanPDBStatusType
lcms_icc_info (PicmanColorConfig *config,
               gint32           image,
               gchar          **name,
               gchar          **desc,
               gchar          **info)
{
  cmsHPROFILE profile;

  g_return_val_if_fail (PICMAN_IS_COLOR_CONFIG (config), PICMAN_PDB_CALLING_ERROR);
  g_return_val_if_fail (image != -1, PICMAN_PDB_CALLING_ERROR);

  profile = lcms_image_get_profile (config, image, NULL);

  if (profile && ! lcms_icc_profile_is_rgb (profile))
    {
      g_printerr ("lcms: attached color profile is not for RGB color space "
                  "(skipping)\n");

      cmsCloseProfile (profile);
      profile = NULL;
    }

  if (profile)
    {
      if (name) *name = lcms_icc_profile_get_name (profile);
      if (desc) *desc = lcms_icc_profile_get_desc (profile);
      if (info) *info = lcms_icc_profile_get_info (profile);

      cmsCloseProfile (profile);
    }
  else
    {
      if (name) *name = g_strdup ("sRGB");
      if (desc) *desc = g_strdup ("sRGB built-in");
      if (info) *info = g_strdup (_("Default RGB working space"));
    }

  return PICMAN_PDB_SUCCESS;
}

static PicmanPDBStatusType
lcms_icc_file_info (const gchar  *filename,
                    gchar       **name,
                    gchar       **desc,
                    gchar       **info)
{
  cmsHPROFILE profile;

  if (! g_file_test (filename, G_FILE_TEST_IS_REGULAR))
    return PICMAN_PDB_EXECUTION_ERROR;

  profile = cmsOpenProfileFromFile (filename, "r");

  if (! profile)
    return PICMAN_PDB_EXECUTION_ERROR;

  *name = lcms_icc_profile_get_name (profile);
  *desc = lcms_icc_profile_get_desc (profile);
  *info = lcms_icc_profile_get_info (profile);

  cmsCloseProfile (profile);

  return PICMAN_PDB_SUCCESS;
}

static void
lcms_sRGB_checksum (guchar *digest)
{
  digest[0]  = 0xcb;
  digest[1]  = 0x63;
  digest[2]  = 0x14;
  digest[3]  = 0x56;
  digest[4]  = 0xd4;
  digest[5]  = 0x0a;
  digest[6]  = 0x01;
  digest[7]  = 0x62;
  digest[8]  = 0xa0;
  digest[9]  = 0xdb;
  digest[10] = 0xe6;
  digest[11] = 0x32;
  digest[12] = 0x8b;
  digest[13] = 0xea;
  digest[14] = 0x1a;
  digest[15] = 0x89;
}

static void
lcms_calculate_checksum (const gchar *data,
                         gsize        len,
                         guchar      *digest)
{
  if (digest)
    {
      GChecksum *md5 = g_checksum_new (G_CHECKSUM_MD5);

      g_checksum_update (md5,
                         (const guchar *) data + sizeof (cmsICCHeader),
                         len - sizeof (cmsICCHeader));

      len = 16;
      g_checksum_get_digest (md5, digest, &len);
      g_checksum_free (md5);
    }
}

static cmsHPROFILE
lcms_image_get_profile (PicmanColorConfig *config,
                        gint32           image,
                        guchar          *checksum)
{
  PicmanParasite *parasite;
  cmsHPROFILE   profile = NULL;

  g_return_val_if_fail (image != -1, NULL);

  parasite = picman_image_get_parasite (image, "icc-profile");

  if (parasite)
    {
      profile = cmsOpenProfileFromMem ((gpointer) picman_parasite_data (parasite),
                                       picman_parasite_data_size (parasite));

      if (profile)
        {
          lcms_calculate_checksum (picman_parasite_data (parasite),
                                   picman_parasite_data_size (parasite),
                                   checksum);
        }
      else
        {
          g_message (_("Data attached as 'icc-profile' does not appear to "
                       "be an ICC color profile"));
        }

      picman_parasite_free (parasite);
    }
  else if (config->rgb_profile)
    {
      profile = lcms_load_profile (config->rgb_profile, checksum);
    }

  return profile;
}

static gboolean
lcms_image_set_profile (gint32       image,
                        cmsHPROFILE  profile,
                        const gchar *filename,
                        gboolean     undo_group)
{
  g_return_val_if_fail (image != -1, FALSE);

  if (filename)
    {
      PicmanParasite *parasite;
      GMappedFile  *file;
      GError       *error = NULL;

      file = g_mapped_file_new (filename, FALSE, &error);

      if (! file)
        {
          g_message ("%s", error->message);
          g_error_free (error);

          return FALSE;
        }

      /* check that this file is actually an ICC profile */
      if (! profile)
        {
          profile = cmsOpenProfileFromMem (g_mapped_file_get_contents (file),
                                           g_mapped_file_get_length (file));

          if (profile)
            {
              cmsCloseProfile (profile);
            }
          else
            {
              g_message (_("'%s' does not appear to be an ICC color profile"),
                         picman_filename_to_utf8 (filename));
              return FALSE;
            }
        }

      if (undo_group)
        picman_image_undo_group_start (image);

      parasite = picman_parasite_new ("icc-profile",
                                    PICMAN_PARASITE_PERSISTENT |
                                    PICMAN_PARASITE_UNDOABLE,
                                    g_mapped_file_get_length (file),
                                    g_mapped_file_get_contents (file));

      g_mapped_file_unref (file);

      picman_image_attach_parasite (image, parasite);
      picman_parasite_free (parasite);
    }
  else
    {
      if (undo_group)
        picman_image_undo_group_start (image);

      picman_image_detach_parasite (image, "icc-profile");
    }

  picman_image_detach_parasite (image, "icc-profile-name");

  if (undo_group)
    picman_image_undo_group_end (image);

  return TRUE;
}

static gboolean
lcms_image_apply_profile (gint32                    image,
                          cmsHPROFILE               src_profile,
                          cmsHPROFILE               dest_profile,
                          const gchar              *filename,
                          PicmanColorRenderingIntent  intent,
                          gboolean                  bpc)
{
  gint32 saved_selection = -1;

  picman_image_undo_group_start (image);

  if (! lcms_image_set_profile (image, dest_profile, filename, FALSE))
    {
      picman_image_undo_group_end (image);

      return FALSE;
    }

  {
    gchar  *src  = lcms_icc_profile_get_desc (src_profile);
    gchar  *dest = lcms_icc_profile_get_desc (dest_profile);

      /* ICC color profile conversion */
      picman_progress_init_printf (_("Converting from '%s' to '%s'"), src, dest);

      g_printerr ("lcms: converting from '%s' to '%s'\n", src, dest);

      g_free (dest);
      g_free (src);
  }

  if (! picman_selection_is_empty (image))
    {
      saved_selection = picman_selection_save (image);
      picman_selection_none (image);
    }

  switch (picman_image_base_type (image))
    {
    case PICMAN_RGB:
      lcms_image_transform_rgb (image,
                                src_profile, dest_profile, intent, bpc);
      break;

    case PICMAN_GRAY:
      g_warning ("colorspace conversion not implemented for "
                 "grayscale images");
      break;

    case PICMAN_INDEXED:
      lcms_image_transform_indexed (image,
                                    src_profile, dest_profile, intent, bpc);
      break;
    }

  if (saved_selection != -1)
    {
      picman_image_select_item (image, PICMAN_CHANNEL_OP_REPLACE, saved_selection);
      picman_image_remove_channel (image, saved_selection);
    }

  picman_progress_update (1.0);

  picman_image_undo_group_end (image);

  return TRUE;
}

static void
lcms_image_transform_rgb (gint32                    image,
                          cmsHPROFILE               src_profile,
                          cmsHPROFILE               dest_profile,
                          PicmanColorRenderingIntent  intent,
                          gboolean                  bpc)
{
  cmsHTRANSFORM    transform   = NULL;
  cmsUInt32Number  lcms_format = 0;
  gint            *layers;
  gint             num_layers;
  gint             i;

  layers = picman_image_get_layers (image, &num_layers);

  for (i = 0; i < num_layers; i++)
    {
      gint32          layer_id     = layers[i];
      const Babl     *layer_format = picman_drawable_get_format (layer_id);
      const gboolean  has_alpha    = babl_format_has_alpha (layer_format);
      const Babl     *type         = babl_format_get_type (layer_format, 0);
      const Babl     *iter_format  = NULL;

      if (type == babl_type ("u8"))
        {
          if (has_alpha)
            {
              lcms_format = TYPE_RGBA_8;
              iter_format = babl_format ("R'G'B'A u8");
            }
          else
            {
              lcms_format = TYPE_RGB_8;
              iter_format = babl_format ("R'G'B' u8");
            }
        }
      else if (type == babl_type ("u16"))
        {
          if (has_alpha)
            {
              lcms_format = TYPE_RGBA_16;
              iter_format = babl_format ("R'G'B'A u16");
            }
          else
            {
              lcms_format = TYPE_RGB_16;
              iter_format = babl_format ("R'G'B' u16");
            }
        }
#ifdef TYPE_RGB_HALF_FLT
      /* half float types are only in lcms 2.4 and newer */
      else if (type == babl_type ("half")) /* 16-bit floating point (half) */
        {
          if (has_alpha)
            {
              lcms_format = TYPE_RGBA_HALF_FLT;
              iter_format = babl_format ("R'G'B'A half");
            }
          else
            {
              lcms_format = TYPE_RGB_HALF_FLT;
              iter_format = babl_format ("R'G'B' half");
            }
        }
#endif /* TYPE_RGB_HALF_FLT */
      else if (type == babl_type ("float"))
        {
          if (has_alpha)
            {
              lcms_format = TYPE_RGBA_FLT;
              iter_format = babl_format ("R'G'B'A float");
            }
          else
            {
              lcms_format = TYPE_RGB_FLT;
              iter_format = babl_format ("R'G'B' float");
            }
        }
      else
        {
          g_warning ("layer format has not been coded yet; unable to create transform");
        }

      if (lcms_format != 0)
        {
          transform = cmsCreateTransform (src_profile,  lcms_format,
                                          dest_profile, lcms_format,
                                          intent,
                                          cmsFLAGS_NOOPTIMIZE |
                                          bpc ? cmsFLAGS_BLACKPOINTCOMPENSATION : 0);

          if (transform)
            {
              GeglBuffer         *src_buffer;
              GeglBuffer         *dest_buffer;
              GeglBufferIterator *iter;
              gint                layer_width;
              gint                layer_height;
              gint                layer_bpp;
              gboolean            layer_alpha;
              gdouble             progress_start = (gdouble) i / num_layers;
              gdouble             progress_end   = (gdouble) (i + 1) / num_layers;
              gdouble             range          = progress_end - progress_start;
              gint                count          = 0;
              gint                done           = 0;

              src_buffer   = picman_drawable_get_buffer (layer_id);
              dest_buffer  = picman_drawable_get_shadow_buffer (layer_id);
              layer_width  = gegl_buffer_get_width (src_buffer);
              layer_height = gegl_buffer_get_height (src_buffer);
              layer_bpp    = babl_format_get_bytes_per_pixel (iter_format);
              layer_alpha  = babl_format_has_alpha (iter_format);

              iter = gegl_buffer_iterator_new (src_buffer, NULL, 0,
                                               iter_format,
                                               GEGL_BUFFER_READ, GEGL_ABYSS_NONE);

              gegl_buffer_iterator_add (iter, dest_buffer, NULL, 0,
                                        iter_format,
                                        GEGL_BUFFER_WRITE, GEGL_ABYSS_NONE);

              while (gegl_buffer_iterator_next (iter))
                {
                  /*  lcms doesn't touch the alpha channel, simply
                   *  copy everything to dest before the transform
                   */
                  if (layer_alpha)
                    memcpy (iter->data[1], iter->data[0],
                            iter->length * layer_bpp);

                  cmsDoTransform (transform,
                                  iter->data[0], iter->data[1], iter->length);
                }

              g_object_unref (src_buffer);
              g_object_unref (dest_buffer);

              picman_drawable_merge_shadow (layer_id, TRUE);
              picman_drawable_update (layer_id, 0, 0, layer_width, layer_height);

              if (count++ % 32 == 0)
                {
                  picman_progress_update (progress_start +
                                        (gdouble) done /
                                        (layer_width * layer_height) * range);
                }

              cmsDeleteTransform (transform);
            }
        }
    }

  g_free (layers);
}

static void
lcms_image_transform_indexed (gint32                    image,
                              cmsHPROFILE               src_profile,
                              cmsHPROFILE               dest_profile,
                              PicmanColorRenderingIntent  intent,
                              gboolean                  bpc)
{
  cmsHTRANSFORM    transform;
  guchar          *cmap;
  gint             n_cmap_bytes;
  cmsUInt32Number  format = TYPE_RGB_8;

  cmap = picman_image_get_colormap (image, &n_cmap_bytes);

  transform = cmsCreateTransform (src_profile,  format,
                                  dest_profile, format,
                                  intent,
                                  cmsFLAGS_NOOPTIMIZE |
                                  bpc ? cmsFLAGS_BLACKPOINTCOMPENSATION : 0);

  if (transform)
    {
      cmsDoTransform (transform, cmap, cmap, n_cmap_bytes / 3);
      cmsDeleteTransform (transform);
    }

  else
    {
      g_warning ("cmsCreateTransform() failed!");
    }

  picman_image_set_colormap (image, cmap, n_cmap_bytes);
}

static cmsHPROFILE
lcms_load_profile (const gchar *filename,
                   guchar      *checksum)
{
  cmsHPROFILE  profile;
  GMappedFile *file;
  gchar       *data;
  gsize        len;
  GError      *error = NULL;

  g_return_val_if_fail (filename != NULL, NULL);

  file = g_mapped_file_new (filename, FALSE, &error);

  if (! file)
    {
      g_message ("%s", error->message);
      g_error_free (error);

      return NULL;
    }

  data = g_mapped_file_get_contents (file);
  len = g_mapped_file_get_length (file);

  profile = cmsOpenProfileFromMem (data, len);

  if (profile)
    {
      lcms_calculate_checksum (data, len, checksum);
    }
  else
    {
      g_message (_("Could not load ICC profile from '%s'"),
                 picman_filename_to_utf8 (filename));
    }

  g_mapped_file_unref (file);

  return profile;
}

static GtkWidget *
lcms_icc_profile_src_label_new (gint32       image,
                                cmsHPROFILE  profile)
{
  GtkWidget *vbox;
  GtkWidget *label;
  gchar     *name;
  gchar     *desc;
  gchar     *text;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);

  name = picman_image_get_name (image);
  text = g_strdup_printf (_("The image '%s' has an embedded color profile:"),
                          name);
  g_free (name);

  label = g_object_new (GTK_TYPE_LABEL,
                        "label",   text,
                        "wrap",    TRUE,
                        "justify", GTK_JUSTIFY_LEFT,
                        "xalign",  0.0,
                        "yalign",  0.0,
                        NULL);
  g_free (text);

  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  desc = lcms_icc_profile_get_desc (profile);
  label = g_object_new (GTK_TYPE_LABEL,
                        "label",   desc,
                        "wrap",    TRUE,
                        "justify", GTK_JUSTIFY_LEFT,
                        "xalign",  0.0,
                        "yalign",  0.0,
                        "xpad",    24,
                        NULL);
  g_free (desc);

  picman_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  return vbox;
}

static GtkWidget *
lcms_icc_profile_dest_label_new (cmsHPROFILE  profile)
{
  GtkWidget *label;
  gchar     *desc;
  gchar     *text;

  desc = lcms_icc_profile_get_desc (profile);
  text = g_strdup_printf (_("Convert the image to the RGB working space (%s)?"),
                          desc);
  g_free (desc);

  label = g_object_new (GTK_TYPE_LABEL,
                        "label",   text,
                        "wrap",    TRUE,
                        "justify", GTK_JUSTIFY_LEFT,
                        "xalign",  0.0,
                        "yalign",  0.0,
                        NULL);
  g_free (text);

  return label;
}

static gboolean
lcms_icc_apply_dialog (gint32       image,
                       cmsHPROFILE  src_profile,
                       cmsHPROFILE  dest_profile,
                       gboolean    *dont_ask)
{
  GtkWidget *dialog;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *toggle = NULL;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Convert to RGB working space?"),
                            PLUG_IN_ROLE,
                            NULL, 0,
                            picman_standard_help_func, PLUG_IN_PROC_APPLY,

                            _("_Keep"),    GTK_RESPONSE_CANCEL,

                            NULL);

  button = gtk_dialog_add_button (GTK_DIALOG (dialog),
                                  _("_Convert"), GTK_RESPONSE_OK);
  gtk_button_set_image (GTK_BUTTON (button),
                        gtk_image_new_from_stock (GTK_STOCK_CONVERT,
                                                  GTK_ICON_SIZE_BUTTON));

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  picman_window_set_transient (GTK_WINDOW (dialog));

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  label = lcms_icc_profile_src_label_new (image, src_profile);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  label = lcms_icc_profile_dest_label_new (dest_profile);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  if (dont_ask)
    {
      toggle = gtk_check_button_new_with_mnemonic (_("_Don't ask me again"));
      gtk_box_pack_end (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), FALSE);
      gtk_widget_show (toggle);
    }

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  *dont_ask = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle));

  gtk_widget_destroy (dialog);

  return run;
}

static void
lcms_icc_combo_box_set_active (PicmanColorProfileComboBox *combo,
                               const gchar              *filename)
{
  cmsHPROFILE  profile = NULL;
  gchar       *label   = NULL;

  if (filename)
    profile = lcms_load_profile (filename, NULL);

  if (profile)
    {
      label = lcms_icc_profile_get_desc (profile);
      if (! label)
        label = lcms_icc_profile_get_name (profile);

      cmsCloseProfile (profile);
    }

  picman_color_profile_combo_box_set_active (combo, filename, label);
  g_free (label);
}

static void
lcms_icc_file_chooser_dialog_response (GtkFileChooser           *dialog,
                                       gint                      response,
                                       PicmanColorProfileComboBox *combo)
{
  if (response == GTK_RESPONSE_ACCEPT)
    {
      gchar *filename = gtk_file_chooser_get_filename (dialog);

      if (filename)
        {
          lcms_icc_combo_box_set_active (combo, filename);

          g_free (filename);
        }
    }

  gtk_widget_hide (GTK_WIDGET (dialog));
}

static GtkWidget *
lcms_icc_file_chooser_dialog_new (void)
{
  GtkWidget     *dialog;
  GtkFileFilter *filter;

  dialog = gtk_file_chooser_dialog_new (_("Select destination profile"),
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_OPEN,

                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN,   GTK_RESPONSE_ACCEPT,

                                        NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_ACCEPT,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

#ifndef G_OS_WIN32
  {
    const gchar folder[] = "/usr/share/color/icc";

    if (g_file_test (folder, G_FILE_TEST_IS_DIR))
      gtk_file_chooser_add_shortcut_folder (GTK_FILE_CHOOSER (dialog),
                                            folder, NULL);
  }
#endif

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("All files (*.*)"));
  gtk_file_filter_add_pattern (filter, "*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("ICC color profile (*.icc, *.icm)"));
  gtk_file_filter_add_pattern (filter, "*.[Ii][Cc][Cc]");
  gtk_file_filter_add_pattern (filter, "*.[Ii][Cc][Mm]");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), filter);

  return dialog;
}

static GtkWidget *
lcms_icc_combo_box_new (PicmanColorConfig *config,
                        const gchar     *filename)
{
  GtkWidget   *combo;
  GtkWidget   *dialog;
  gchar       *history;
  gchar       *label;
  gchar       *name;
  cmsHPROFILE  profile;

  dialog = lcms_icc_file_chooser_dialog_new ();
  history = picman_personal_rc_file ("profilerc");

  combo = picman_color_profile_combo_box_new (dialog, history);

  g_free (history);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (lcms_icc_file_chooser_dialog_response),
                    combo);

  if (config->rgb_profile)
    profile = lcms_load_profile (config->rgb_profile, NULL);
  else
    profile = cmsCreate_sRGBProfile ();

  name = lcms_icc_profile_get_desc (profile);
  if (! name)
    name = lcms_icc_profile_get_name (profile);

  cmsCloseProfile (profile);

  label = g_strdup_printf (_("RGB workspace (%s)"), name);
  g_free (name);

  picman_color_profile_combo_box_add (PICMAN_COLOR_PROFILE_COMBO_BOX (combo),
                                    config->rgb_profile, label);
  g_free (label);

  if (filename)
    lcms_icc_combo_box_set_active (PICMAN_COLOR_PROFILE_COMBO_BOX (combo),
                                   filename);
  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

  return combo;
}

static PicmanPDBStatusType
lcms_dialog (PicmanColorConfig *config,
             gint32           image,
             gboolean         apply,
             LcmsValues      *values)
{
  PicmanColorProfileComboBox *box;
  GtkWidget                *dialog;
  GtkWidget                *main_vbox;
  GtkWidget                *frame;
  GtkWidget                *label;
  GtkWidget                *combo;
  cmsHPROFILE               src_profile;
  gchar                    *name;
  gboolean                  success = FALSE;
  gboolean                  run;

  src_profile = lcms_image_get_profile (config, image, NULL);

  if (src_profile && ! lcms_icc_profile_is_rgb (src_profile))
    {
      g_printerr ("lcms: attached color profile is not for RGB color space "
                  "(skipping)\n");

      cmsCloseProfile (src_profile);
      src_profile = NULL;
    }

  if (! src_profile)
    src_profile = cmsCreate_sRGBProfile ();

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (apply ?
                            _("Convert to ICC Color Profile") :
                            _("Assign ICC Color Profile"),
                            PLUG_IN_ROLE,
                            NULL, 0,
                            picman_standard_help_func,
                            apply ? PLUG_IN_PROC_APPLY : PLUG_IN_PROC_SET,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,

                            apply ? GTK_STOCK_CONVERT : _("_Assign"),
                            GTK_RESPONSE_OK,

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

  frame = picman_frame_new (_("Current Color Profile"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  name = lcms_icc_profile_get_desc (src_profile);
  if (! name)
    name = lcms_icc_profile_get_name (src_profile);

  label = gtk_label_new (name);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_container_add (GTK_CONTAINER (frame), label);
  gtk_widget_show (label);

  g_free (name);

  frame = picman_frame_new (apply ? _("Convert to") : _("Assign"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  combo = lcms_icc_combo_box_new (config, NULL);
  gtk_container_add (GTK_CONTAINER (frame), combo);
  gtk_widget_show (combo);

  box = PICMAN_COLOR_PROFILE_COMBO_BOX (combo);

  if (apply)
    {
      GtkWidget *vbox;
      GtkWidget *hbox;
      GtkWidget *toggle;

      vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
      gtk_box_pack_start (GTK_BOX (main_vbox), vbox, FALSE, FALSE, 0);
      gtk_widget_show (vbox);

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      gtk_widget_show (hbox);

      label = gtk_label_new_with_mnemonic (_("_Rendering Intent:"));
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
      gtk_widget_show (label);

      combo = picman_enum_combo_box_new (PICMAN_TYPE_COLOR_RENDERING_INTENT);
      gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
      gtk_widget_show (combo);

      picman_int_combo_box_connect (PICMAN_INT_COMBO_BOX (combo),
                                  values->intent,
                                  G_CALLBACK (picman_int_combo_box_get_active),
                                  &values->intent);

      gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);

      toggle =
        gtk_check_button_new_with_mnemonic (_("_Black Point Compensation"));
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), values->bpc);
      gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
      gtk_widget_show (toggle);

      g_signal_connect (toggle, "toggled",
                        G_CALLBACK (picman_toggle_button_update),
                        &values->bpc);
    }

  while ((run = picman_dialog_run (PICMAN_DIALOG (dialog))) == GTK_RESPONSE_OK)
    {
      gchar       *filename = picman_color_profile_combo_box_get_active (box);
      cmsHPROFILE  dest_profile;

      gtk_widget_set_sensitive (dialog, FALSE);

      if (filename)
        {
          dest_profile = lcms_load_profile (filename, NULL);
        }
      else
        {
          dest_profile = cmsCreate_sRGBProfile ();
        }

      if (dest_profile)
        {
          if (lcms_icc_profile_is_rgb (dest_profile))
            {
              if (apply)
                success = lcms_image_apply_profile (image,
                                                    src_profile, dest_profile,
                                                    filename,
                                                    values->intent,
                                                    values->bpc);
              else
                success = lcms_image_set_profile (image,
                                                  dest_profile, filename, TRUE);
            }
          else
            {
              picman_message (_("Destination profile is not for RGB color space."));
            }

          cmsCloseProfile (dest_profile);
        }

      if (success)
        break;
      else
        gtk_widget_set_sensitive (dialog, TRUE);
    }

  gtk_widget_destroy (dialog);

  cmsCloseProfile (src_profile);

  return (run ?
          (success ? PICMAN_PDB_SUCCESS : PICMAN_PDB_EXECUTION_ERROR) :
          PICMAN_PDB_CANCEL);
}
