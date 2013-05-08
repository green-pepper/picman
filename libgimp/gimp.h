/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picman.h
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_H__
#define __PICMAN_H__

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libpicmanbase/picmanbase.h>
#include <libpicmancolor/picmancolor.h>
#include <libpicmanconfig/picmanconfig.h>
#include <libpicmanmath/picmanmath.h>

#define __PICMAN_H_INSIDE__

#include <libpicman/picmanenums.h>
#include <libpicman/picmantypes.h>

#include <libpicman/picmanbrushes.h>
#include <libpicman/picmanbrushselect.h>
#include <libpicman/picmanchannel.h>
#include <libpicman/picmandrawable.h>
#include <libpicman/picmanfontselect.h>
#include <libpicman/picmanpicmanrc.h>
#include <libpicman/picmangradients.h>
#include <libpicman/picmangradientselect.h>
#include <libpicman/picmanimage.h>
#include <libpicman/picmanlayer.h>
#include <libpicman/picmanpalette.h>
#include <libpicman/picmanpalettes.h>
#include <libpicman/picmanpaletteselect.h>
#include <libpicman/picmanpatterns.h>
#include <libpicman/picmanpatternselect.h>
#include <libpicman/picmanpixbuf.h>
#include <libpicman/picmanpixelfetcher.h>
#include <libpicman/picmanpixelrgn.h>
#include <libpicman/picmanplugin.h>
#include <libpicman/picmanproceduraldb.h>
#include <libpicman/picmanprogress.h>
#include <libpicman/picmanregioniterator.h>
#include <libpicman/picmanselection.h>
#include <libpicman/picmantile.h>
#include <libpicman/picmanvectors.h>

#include <libpicman/picman_pdb_headers.h>

#undef __PICMAN_H_INSIDE__

#ifdef G_OS_WIN32
#include <stdlib.h> /* For __argc and __argv */
#endif

G_BEGIN_DECLS


#define picman_get_data         picman_procedural_db_get_data
#define picman_get_data_size    picman_procedural_db_get_data_size
#define picman_set_data         picman_procedural_db_set_data


typedef void (* PicmanInitProc)  (void);
typedef void (* PicmanQuitProc)  (void);
typedef void (* PicmanQueryProc) (void);
typedef void (* PicmanRunProc)   (const gchar      *name,
                                gint              n_params,
                                const PicmanParam  *param,
                                gint             *n_return_vals,
                                PicmanParam       **return_vals);


/**
 * PicmanPlugInInfo:
 * @init_proc:  called when the picman application initially starts up
 * @quit_proc:  called when the picman application exits
 * @query_proc: called by picman so that the plug-in can inform the
 *              picman of what it does. (ie. installing a procedure database
 *              procedure).
 * @run_proc:   called to run a procedure the plug-in installed in the
 *              procedure database.
 **/
struct _PicmanPlugInInfo
{
  PicmanInitProc  init_proc;
  PicmanQuitProc  quit_proc;
  PicmanQueryProc query_proc;
  PicmanRunProc   run_proc;
};

struct _PicmanParamDef
{
  PicmanPDBArgType  type;
  gchar          *name;
  gchar          *description;
};

struct _PicmanParamRegion
{
  gint32 x;
  gint32 y;
  gint32 width;
  gint32 height;
};

union _PicmanParamData
{
  gint32            d_int32;
  gint16            d_int16;
  guint8            d_int8;
  gdouble           d_float;
  gchar            *d_string;
  gint32           *d_int32array;
  gint16           *d_int16array;
  guint8           *d_int8array;
  gdouble          *d_floatarray;
  gchar           **d_stringarray;
  PicmanRGB          *d_colorarray;
  PicmanRGB           d_color;
  PicmanParamRegion   d_region; /* deprecated */
  gint32            d_display;
  gint32            d_image;
  gint32            d_item;
  gint32            d_layer;
  gint32            d_layer_mask;
  gint32            d_channel;
  gint32            d_drawable;
  gint32            d_selection;
  gint32            d_boundary;
  gint32            d_path; /* deprecated */
  gint32            d_vectors;
  gint32            d_unit;
  PicmanParasite      d_parasite;
  gint32            d_tattoo;
  PicmanPDBStatusType d_status;
};

struct _PicmanParam
{
  PicmanPDBArgType type;
  PicmanParamData  data;
};



/**
 * MAIN:
 *
 * A macro that expands to the appropriate main() function for the
 * platform being compiled for.
 *
 * To use this macro, simply place a line that contains just the code
 * MAIN() at the toplevel of your file.  No semicolon should be used.
 **/

#ifdef G_OS_WIN32

/* Define WinMain() because plug-ins are built as GUI applications. Also
 * define a main() in case some plug-in still is built as a console
 * application.
 */
#  ifdef __GNUC__
#    ifndef _stdcall
#      define _stdcall __attribute__((stdcall))
#    endif
#  endif

#  define MAIN()                                        \
   struct HINSTANCE__;                                  \
                                                        \
   int _stdcall                                         \
   WinMain (struct HINSTANCE__ *hInstance,              \
            struct HINSTANCE__ *hPrevInstance,          \
            char *lpszCmdLine,                          \
            int   nCmdShow);                            \
                                                        \
   int _stdcall                                         \
   WinMain (struct HINSTANCE__ *hInstance,              \
            struct HINSTANCE__ *hPrevInstance,          \
            char *lpszCmdLine,                          \
            int   nCmdShow)                             \
   {                                                    \
     return picman_main (&PLUG_IN_INFO, __argc, __argv);  \
   }                                                    \
                                                        \
   int                                                  \
   main (int argc, char *argv[])                        \
   {                                                    \
     /* Use __argc and __argv here, too, as they work   \
      * better with mingw-w64.				\
      */						\
     return picman_main (&PLUG_IN_INFO, __argc, __argv);  \
   }
#else
#  define MAIN()                                        \
   int                                                  \
   main (int argc, char *argv[])                        \
   {                                                    \
     return picman_main (&PLUG_IN_INFO, argc, argv);      \
   }
#endif


/* The main procedure that must be called with the PLUG_IN_INFO structure
 * and the 'argc' and 'argv' that are passed to "main".
 */
gint           picman_main                (const PicmanPlugInInfo *info,
                                         gint                  argc,
                                         gchar                *argv[]);

/* Forcefully causes the picman library to exit and
 *  close down its connection to main picman application.
 */
void           picman_quit                (void) G_GNUC_NORETURN;


/* Install a procedure in the procedure database.
 */
void           picman_install_procedure   (const gchar        *name,
                                         const gchar        *blurb,
                                         const gchar        *help,
                                         const gchar        *author,
                                         const gchar        *copyright,
                                         const gchar        *date,
                                         const gchar        *menu_label,
                                         const gchar        *image_types,
                                         PicmanPDBProcType     type,
                                         gint                n_params,
                                         gint                n_return_vals,
                                         const PicmanParamDef *params,
                                         const PicmanParamDef *return_vals);

/* Install a temporary procedure in the procedure database.
 */
void           picman_install_temp_proc   (const gchar        *name,
                                         const gchar        *blurb,
                                         const gchar        *help,
                                         const gchar        *author,
                                         const gchar        *copyright,
                                         const gchar        *date,
                                         const gchar        *menu_label,
                                         const gchar        *image_types,
                                         PicmanPDBProcType     type,
                                         gint                n_params,
                                         gint                n_return_vals,
                                         const PicmanParamDef *params,
                                         const PicmanParamDef *return_vals,
                                         PicmanRunProc         run_proc);

/* Uninstall a temporary procedure
 */
void           picman_uninstall_temp_proc (const gchar        *name);

/* Notify the main PICMAN application that the extension is ready to run
 */
void           picman_extension_ack       (void);

/* Enable asynchronous processing of temp_procs
 */
void           picman_extension_enable    (void);

/* Process one temp_proc and return
 */
void           picman_extension_process   (guint            timeout);

/* Run a procedure in the procedure database. The parameters are
 *  specified via the variable length argument list. The return
 *  values are returned in the 'PicmanParam*' array.
 */
PicmanParam    * picman_run_procedure       (const gchar     *name,
                                         gint            *n_return_vals,
                                         ...);

/* Run a procedure in the procedure database. The parameters are
 *  specified as an array of PicmanParam.  The return
 *  values are returned in the 'PicmanParam*' array.
 */
PicmanParam    * picman_run_procedure2      (const gchar     *name,
                                         gint            *n_return_vals,
                                         gint             n_params,
                                         const PicmanParam *params);

/* Destroy the an array of parameters. This is useful for
 *  destroying the return values returned by a call to
 *  'picman_run_procedure'.
 */
void           picman_destroy_params      (PicmanParam       *params,
                                         gint             n_params);

/* Destroy the an array of PicmanParamDef's. This is useful for
 *  destroying the return values returned by a call to
 *  'picman_procedural_db_proc_info'.
 */
void           picman_destroy_paramdefs   (PicmanParamDef    *paramdefs,
                                         gint             n_params);

/* Retrieve the error message for the last procedure call.
 */
const gchar  * picman_get_pdb_error       (void);

/* Return various constants given by the PICMAN core at plug-in config time.
 */
guint          picman_tile_width          (void) G_GNUC_CONST;
guint          picman_tile_height         (void) G_GNUC_CONST;
gint           picman_shm_ID              (void) G_GNUC_CONST;
guchar       * picman_shm_addr            (void) G_GNUC_CONST;
gdouble        picman_gamma               (void) G_GNUC_CONST;
gboolean       picman_show_tool_tips      (void) G_GNUC_CONST;
gboolean       picman_show_help_button    (void) G_GNUC_CONST;
PicmanCheckSize  picman_check_size          (void) G_GNUC_CONST;
PicmanCheckType  picman_check_type          (void) G_GNUC_CONST;
gint32         picman_default_display     (void) G_GNUC_CONST;
const gchar  * picman_wm_class            (void) G_GNUC_CONST;
const gchar  * picman_display_name        (void) G_GNUC_CONST;
gint           picman_monitor_number      (void) G_GNUC_CONST;
guint32        picman_user_time           (void) G_GNUC_CONST;

const gchar  * picman_get_progname        (void) G_GNUC_CONST;

PICMAN_DEPRECATED
gboolean       picman_install_cmap        (void) G_GNUC_CONST;
PICMAN_DEPRECATED
gint           picman_min_colors          (void) G_GNUC_CONST;

PICMAN_DEPRECATED_FOR(picman_get_parasite)
PicmanParasite * picman_parasite_find       (const gchar        *name);
PICMAN_DEPRECATED_FOR(picman_parasite_attach)
gboolean       picman_parasite_attach     (const PicmanParasite *parasite);
PICMAN_DEPRECATED_FOR(picman_parasite_detach)
gboolean       picman_parasite_detach     (const gchar        *name);
PICMAN_DEPRECATED_FOR(picman_get_parasite_list)
gboolean       picman_parasite_list       (gint               *num_parasites,
                                         gchar            ***parasites);
PICMAN_DEPRECATED_FOR(picman_parasite_attach)
gboolean       picman_attach_new_parasite (const gchar        *name,
                                         gint                flags,
                                         gint                size,
                                         gconstpointer       data);


G_END_DECLS

#endif /* __PICMAN_H__ */
