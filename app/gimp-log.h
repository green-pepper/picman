/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_LOG_H__
#define __PICMAN_LOG_H__


typedef enum
{
  PICMAN_LOG_TOOL_EVENTS        = 1 << 0,
  PICMAN_LOG_TOOL_FOCUS         = 1 << 1,
  PICMAN_LOG_DND                = 1 << 2,
  PICMAN_LOG_HELP               = 1 << 3,
  PICMAN_LOG_DIALOG_FACTORY     = 1 << 4,
  PICMAN_LOG_MENUS              = 1 << 5,
  PICMAN_LOG_SAVE_DIALOG        = 1 << 6,
  PICMAN_LOG_IMAGE_SCALE        = 1 << 7,
  PICMAN_LOG_SHADOW_TILES       = 1 << 8,
  PICMAN_LOG_SCALE              = 1 << 9,
  PICMAN_LOG_WM                 = 1 << 10,
  PICMAN_LOG_FLOATING_SELECTION = 1 << 11,
  PICMAN_LOG_SHM                = 1 << 12,
  PICMAN_LOG_TEXT_EDITING       = 1 << 13,
  PICMAN_LOG_KEY_EVENTS         = 1 << 14,
  PICMAN_LOG_AUTO_TAB_STYLE     = 1 << 15,
  PICMAN_LOG_INSTANCES          = 1 << 16,
  PICMAN_LOG_RECTANGLE_TOOL     = 1 << 17,
  PICMAN_LOG_BRUSH_CACHE        = 1 << 18
} PicmanLogFlags;


extern PicmanLogFlags picman_log_flags;


void   picman_log_init (void);
void   picman_log      (PicmanLogFlags  flags,
                      const gchar  *function,
                      gint          line,
                      const gchar  *format,
                      ...) G_GNUC_PRINTF (4, 5);
void   picman_logv     (PicmanLogFlags  flags,
                      const gchar  *function,
                      gint          line,
                      const gchar  *format,
                      va_list       args);


#ifdef G_HAVE_ISO_VARARGS

#define PICMAN_LOG(type, ...) \
        G_STMT_START { \
        if (picman_log_flags & PICMAN_LOG_##type) \
          picman_log (PICMAN_LOG_##type, G_STRFUNC, __LINE__, __VA_ARGS__);       \
        } G_STMT_END

#elif defined(G_HAVE_GNUC_VARARGS)

#define PICMAN_LOG(type, format...) \
        G_STMT_START { \
        if (picman_log_flags & PICMAN_LOG_##type) \
          picman_log (PICMAN_LOG_##type, G_STRFUNC, __LINE__, format);  \
        } G_STMT_END

#else /* no varargs macros */

/* need to expand all the short forms
 * to make them known constants at compile time
 */
#define TOOL_EVENTS        PICMAN_LOG_TOOL_EVENTS
#define TOOL_FOCUS         PICMAN_LOG_TOOL_FOCUS
#define DND                PICMAN_LOG_DND
#define HELP               PICMAN_LOG_HELP
#define DIALOG_FACTORY     PICMAN_LOG_DIALOG_FACTORY
#define MENUS              PICMAN_LOG_MENUS
#define SAVE_DIALOG        PICMAN_LOG_SAVE_DIALOG
#define IMAGE_SCALE        PICMAN_LOG_IMAGE_SCALE
#define SHADOW_TILES       PICMAN_LOG_SHADOW_TILES
#define SCALE              PICMAN_LOG_SCALE
#define WM                 PICMAN_LOG_WM
#define FLOATING_SELECTION PICMAN_LOG_FLOATING_SELECTION
#define SHM                PICMAN_LOG_SHM
#define TEXT_EDITING       PICMAN_LOG_TEXT_EDITING
#define KEY_EVENTS         PICMAN_LOG_KEY_EVENTS
#define AUTO_TAB_STYLE     PICMAN_LOG_AUTO_TAB_STYLE
#define INSTANCES          PICMAN_LOG_INSTANCES
#define RECTANGLE_TOOL     PICMAN_LOG_RECTANGLE_TOOL
#define BRUSH_CACHE        PICMAN_LOG_BRUSH_CACHE

#if 0 /* last resort */
#  define PICMAN_LOG /* nothing => no varargs, no log */
#endif

static void
PICMAN_LOG (PicmanLogFlags flags,
          const gchar *format,
          ...)
{
  va_list args;
  va_start (args, format);
  if (picman_log_flags & flags)
    picman_logv (type, "", 0, format, args);
  va_end (args);
}

#endif  /* !__GNUC__ */

#define geimnum(vienna)  picman_l##vienna##l_dialog()
#define fnord(kosmoso)   void picman_##kosmoso##bl_dialog(void);

#endif /* __PICMAN_LOG_H__ */
