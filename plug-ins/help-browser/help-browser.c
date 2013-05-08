/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * The PICMAN Help Browser
 * Copyright (C) 1999-2008 Sven Neumann <sven@picman.org>
 *                         Michael Natterer <mitch@picman.org>
 *                         Henrik Brix Andersen <brix@picman.org>
 *
 * Some code & ideas taken from the GNOME help browser.
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

#include <string.h>  /*  strlen, strcmp  */

#include <gtk/gtk.h>

#include <libpicman/picman.h>

#include "plug-ins/help/picmanhelp.h"

#include "dialog.h"

#include "libpicman/stdplugins-intl.h"


/*  defines  */

#define PICMAN_HELP_BROWSER_EXT_PROC       "extension-picman-help-browser"
#define PICMAN_HELP_BROWSER_TEMP_EXT_PROC  "extension-picman-help-browser-temp"
#define PLUG_IN_BINARY                   "help-browser"
#define PLUG_IN_ROLE                     "picman-help-browser"


/*  forward declarations  */

static void      query                  (void);
static void      run                    (const gchar      *name,
                                         gint              nparams,
                                         const PicmanParam  *param,
                                         gint             *nreturn_vals,
                                         PicmanParam       **return_vals);

static void      temp_proc_install      (void);
static void      temp_proc_run          (const gchar      *name,
                                         gint              nparams,
                                         const PicmanParam  *param,
                                         gint             *nreturn_vals,
                                         PicmanParam       **return_vals);

static gboolean  help_browser_show_help (const gchar      *help_domain,
                                         const gchar      *help_locales,
                                         const gchar      *help_id);

static PicmanHelpProgress * help_browser_progress_new (void);


/*  local variables  */

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
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,       "run-mode", "The run mode { RUN-INTERACTIVE (0) }" },
    { PICMAN_PDB_INT32,       "num-domain-names", ""    },
    { PICMAN_PDB_STRINGARRAY, "domain-names",     ""    },
    { PICMAN_PDB_INT32,       "num-domain-uris",  ""    },
    { PICMAN_PDB_STRINGARRAY, "domain-uris",      ""    }
  };

  picman_install_procedure (PICMAN_HELP_BROWSER_EXT_PROC,
                          "Browse the PICMAN user manual",
                          "A small and simple HTML browser optimized for "
			  "browsing the PICMAN user manual.",
                          "Sven Neumann <sven@picman.org>, "
			  "Michael Natterer <mitch@picman.org>"
                          "Henrik Brix Andersen <brix@picman.org>",
			  "Sven Neumann, Michael Natterer & Henrik Brix Andersen",
                          "1999-2008",
                          NULL,
                          "",
                          PICMAN_EXTENSION,
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
  PicmanRunMode        run_mode = param[0].data.d_int32;
  PicmanPDBStatusType  status   = PICMAN_PDB_SUCCESS;

  static PicmanParam   values[1];

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  *nreturn_vals = 1;
  *return_vals  = values;

  INIT_I18N ();

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
    case PICMAN_RUN_NONINTERACTIVE:
    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Make sure all the arguments are there!  */
      if (nparams >= 1)
        {
          if (nparams == 5)
            {
              if (! picman_help_init (param[1].data.d_int32,
                                    param[2].data.d_stringarray,
                                    param[3].data.d_int32,
                                    param[4].data.d_stringarray))
                {
                  status = PICMAN_PDB_CALLING_ERROR;
                }
            }

          if (status == PICMAN_PDB_SUCCESS)
            {
              browser_dialog_open (PLUG_IN_BINARY);

              temp_proc_install ();

              picman_extension_ack ();
              picman_extension_enable ();

              gtk_main ();
            }
        }
      else
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
      break;

    default:
      status = PICMAN_PDB_CALLING_ERROR;
      break;
    }

  values[0].data.d_status = status;
}

static void
temp_proc_install (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_STRING, "help-domain",  "Help domain to use" },
    { PICMAN_PDB_STRING, "help-locales", "Language to use"    },
    { PICMAN_PDB_STRING, "help-id",      "Help ID to open"    }
  };

  picman_install_temp_proc (PICMAN_HELP_BROWSER_TEMP_EXT_PROC,
			  "DON'T USE THIS ONE",
			  "(Temporary procedure)",
			  "Sven Neumann <sven@picman.org>, "
			  "Michael Natterer <mitch@picman.org>"
                          "Henrik Brix Andersen <brix@picman.org>",
			  "Sven Neumann, Michael Natterer & Henrik Brix Andersen",
			  "1999-2008",
			  NULL,
			  "",
			  PICMAN_TEMPORARY,
			  G_N_ELEMENTS (args), 0,
			  args, NULL,
			  temp_proc_run);
}

static void
temp_proc_run (const gchar      *name,
	       gint              nparams,
	       const PicmanParam  *param,
	       gint             *nreturn_vals,
	       PicmanParam       **return_vals)
{
  static PicmanParam  values[1];
  PicmanPDBStatusType status = PICMAN_PDB_SUCCESS;

  *nreturn_vals = 1;
  *return_vals  = values;

  /*  make sure all the arguments are there  */
  if (nparams == 3)
    {
      const gchar    *help_domain  = PICMAN_HELP_DEFAULT_DOMAIN;
      const gchar    *help_locales = NULL;
      const gchar    *help_id      = PICMAN_HELP_DEFAULT_ID;

      if (param[0].data.d_string && strlen (param[0].data.d_string))
        help_domain = param[0].data.d_string;

      if (param[1].data.d_string && strlen (param[1].data.d_string))
        help_locales = param[1].data.d_string;

      if (param[2].data.d_string && strlen (param[2].data.d_string))
        help_id = param[2].data.d_string;

      if (! help_browser_show_help (help_domain, help_locales, help_id))
        {
          gtk_main_quit ();
        }
    }

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;
}

static gboolean
help_browser_show_help (const gchar *help_domain,
                        const gchar *help_locales,
                        const gchar *help_id)
{
  PicmanHelpDomain *domain;
  gboolean        success = TRUE;

  domain = picman_help_lookup_domain (help_domain);

  if (domain)
    {
      PicmanHelpProgress *progress = NULL;
      PicmanHelpLocale   *locale;
      GList            *locales;
      gchar            *uri;
      gboolean          fatal_error;

      locales = picman_help_parse_locales (help_locales);

      if (! g_str_has_prefix (domain->help_uri, "file:"))
        progress = help_browser_progress_new ();

      uri = picman_help_domain_map (domain, locales, help_id,
                                  progress, &locale, &fatal_error);

      if (progress)
        picman_help_progress_free (progress);

      g_list_free_full (locales, (GDestroyNotify) g_free);

      if (uri)
        {
          browser_dialog_make_index (domain, locale);
          browser_dialog_load (uri);

          g_free (uri);
        }
      else if (fatal_error)
        {
          success = FALSE;
        }
    }

  return success;
}


static void
help_browser_progress_start (const gchar *message,
                             gboolean     cancelable,
                             gpointer     user_data)
{
  picman_progress_init (message);
}

static void
help_browser_progress_update (gdouble  value,
                              gpointer user_data)
{
  picman_progress_update (value);
}

static void
help_browser_progress_end (gpointer user_data)
{
  picman_progress_end ();
}

static PicmanHelpProgress *
help_browser_progress_new (void)
{
  static const PicmanHelpProgressVTable vtable =
  {
    help_browser_progress_start,
    help_browser_progress_end,
    help_browser_progress_update
  };

  return picman_help_progress_new (&vtable, NULL);
}
