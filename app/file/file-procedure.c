/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995, 1996, 1997 Spencer Kimball and Peter Mattis
 * Copyright (C) 1997 Josh MacDonald
 *
 * file-procedure.c
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib-object.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"

#include "core/core-types.h"

#include "plug-in/picmanpluginprocedure.h"

#include "file-procedure.h"
#include "file-utils.h"

#include "picman-intl.h"


typedef enum
{
  FILE_MATCH_NONE,
  FILE_MATCH_MAGIC,
  FILE_MATCH_SIZE
} FileMatchType;


/*  local function prototypes  */

static PicmanPlugInProcedure * file_proc_find_by_prefix    (GSList       *procs,
                                                          const gchar  *uri,
                                                          gboolean      skip_magic);
static PicmanPlugInProcedure * file_proc_find_by_extension (GSList       *procs,
                                                          const gchar  *uri,
                                                          gboolean      skip_magic,
                                                          gboolean      uri_procs_only);
static PicmanPlugInProcedure * file_proc_find_by_name      (GSList       *procs,
                                                          const gchar  *uri,
                                                          gboolean      skip_magic);

static void                  file_convert_string         (const gchar  *instr,
                                                          gchar        *outmem,
                                                          gint          maxmem,
                                                          gint         *nmem);
static FileMatchType         file_check_single_magic     (const gchar  *offset,
                                                          const gchar  *type,
                                                          const gchar  *value,
                                                          const guchar *file_head,
                                                          gint          headsize,
                                                          FILE         *ifp);
static FileMatchType         file_check_magic_list       (GSList       *magics_list,
                                                          const guchar *head,
                                                          gint          headsize,
                                                          FILE         *ifp);


/*  public functions  */

PicmanPlugInProcedure *
file_procedure_find (GSList       *procs,
                     const gchar  *uri,
                     GError      **error)
{
  PicmanPlugInProcedure *file_proc;
  gchar               *filename;

  g_return_val_if_fail (procs != NULL, NULL);
  g_return_val_if_fail (uri != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  /* First, check magicless prefixes/suffixes: */

  if (! file_proc_find_by_extension (procs, uri, FALSE, TRUE))
    {
      /* If there is not any (with or without magic) file proc that
       * can load the URI by extension directly, try to find a proc
       * that can load the prefix
       */
      file_proc = file_proc_find_by_prefix (procs, uri, TRUE);
    }
  else
    {
      /* Otherwise try to find a magicless file proc that handles the
       * extension
       */
      file_proc = file_proc_find_by_extension (procs, uri, TRUE, FALSE);
    }

  if (file_proc)
    return file_proc;

  filename = file_utils_filename_from_uri (uri);

  /* Then look for magics */
  if (filename)
    {
      GSList              *list;
      PicmanPlugInProcedure *size_matched_proc = NULL;
      FILE                *ifp               = NULL;
      gboolean             opened            = FALSE;
      gint                 head_size         = 0;
      gint                 size_match_count  = 0;
      guchar               head[256];

      for (list = procs; list; list = g_slist_next (list))
        {
          file_proc = list->data;

          if (file_proc->magics_list)
            {
              if (G_UNLIKELY (!opened))
                {
                  ifp = g_fopen (filename, "rb");
                  if (ifp != NULL)
                    head_size = fread ((gchar *) head, 1, sizeof (head), ifp);
                  else
                    g_set_error_literal (error,
                                         G_FILE_ERROR,
                                         g_file_error_from_errno (errno),
                                         g_strerror (errno));
                  opened = TRUE;
                }

              if (head_size >= 4)
                {
                  FileMatchType match_val;

                  match_val = file_check_magic_list (file_proc->magics_list,
                                                     head, head_size,
                                                     ifp);

                  if (match_val == FILE_MATCH_SIZE)
                    {
                      /* Use it only if no other magic matches */
                      size_match_count++;
                      size_matched_proc = file_proc;
                    }
                  else if (match_val != FILE_MATCH_NONE)
                    {
                      fclose (ifp);
                      g_free (filename);

                      return file_proc;
                    }
                }
            }
        }

      if (ifp)
        {
          if (ferror (ifp))
            g_set_error_literal (error, G_FILE_ERROR,
                                 g_file_error_from_errno (errno),
                                 g_strerror (errno));

          fclose (ifp);
        }

      g_free (filename);

      if (size_match_count == 1)
        return size_matched_proc;
    }

  /* As a last resort, try matching by name, not skipping magic procs */
  file_proc = file_proc_find_by_name (procs, uri, FALSE);

  if (file_proc)
    {
      /* we found a procedure, clear error that might have been set */
      g_clear_error (error);
    }
  else
    {
      /* set an error message unless one was already set */
      if (error && *error == NULL)
	g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
			     _("Unknown file type"));
    }

  return file_proc;
}

PicmanPlugInProcedure *
file_procedure_find_by_prefix (GSList      *procs,
                               const gchar *uri)
{
  g_return_val_if_fail (uri != NULL, NULL);

  return file_proc_find_by_prefix (procs, uri, FALSE);
}

PicmanPlugInProcedure *
file_procedure_find_by_extension (GSList      *procs,
                                  const gchar *uri)
{
  g_return_val_if_fail (uri != NULL, NULL);

  return file_proc_find_by_extension (procs, uri, FALSE, FALSE);
}

gboolean
file_procedure_in_group (PicmanPlugInProcedure *file_proc,
                         FileProcedureGroup   group)
{
  const gchar *name        = picman_object_get_name (file_proc);
  gboolean     is_xcf_save = FALSE;
  gboolean     is_filter   = FALSE;
  gboolean     is_uri      = FALSE;

  is_xcf_save = (strcmp (name, "picman-xcf-save") == 0);

  is_filter   = (strcmp (name, "file-gz-save")  == 0 ||
                 strcmp (name, "file-bz2-save") == 0 ||
                 strcmp (name, "file-xz-save")  == 0);

  is_uri      = (strcmp (name, "file-uri-save") == 0);

  switch (group)
    {
    case FILE_PROCEDURE_GROUP_SAVE:
      /* Only .xcf shall pass */
      return is_xcf_save || is_filter || is_uri;

    case FILE_PROCEDURE_GROUP_EXPORT:
      /* Anything but .xcf shall pass */
      return ! is_xcf_save || is_uri;

    case FILE_PROCEDURE_GROUP_OPEN:
      /* No filter applied for Open */
      return TRUE;

    default:
    case FILE_PROCEDURE_GROUP_ANY:
      return TRUE;
    }
}


/*  private functions  */

static PicmanPlugInProcedure *
file_proc_find_by_prefix (GSList      *procs,
                          const gchar *uri,
                          gboolean     skip_magic)
{
  GSList *p;

  for (p = procs; p; p = g_slist_next (p))
    {
      PicmanPlugInProcedure *proc = p->data;
      GSList              *prefixes;

      if (skip_magic && proc->magics_list)
        continue;

      for (prefixes = proc->prefixes_list;
           prefixes;
           prefixes = g_slist_next (prefixes))
        {
          if (g_str_has_prefix (uri, prefixes->data))
            return proc;
        }
     }

  return NULL;
}

static PicmanPlugInProcedure *
file_proc_find_by_extension (GSList      *procs,
                             const gchar *uri,
                             gboolean     skip_magic,
                             gboolean     uri_procs_only)
{
  GSList      *p;
  const gchar *ext;

  ext = file_utils_uri_get_ext (uri);

  if (! (ext && *ext == '.'))
    return NULL;

  ext++;

  for (p = procs; p; p = g_slist_next (p))
    {
      PicmanPlugInProcedure *proc = p->data;
      GSList              *extensions;

      if (uri_procs_only && ! proc->handles_uri)
        continue;

      if (skip_magic && proc->magics_list)
        continue;

      for (extensions = proc->extensions_list;
           extensions;
           extensions = g_slist_next (extensions))
        {
          const gchar *p1 = ext;
          const gchar *p2 = extensions->data;

          while (*p1 && *p2)
            {
              if (g_ascii_tolower (*p1) != g_ascii_tolower (*p2))
                break;

              p1++;
              p2++;
            }

          if (!(*p1) && !(*p2))
            return proc;
        }
    }

  return NULL;
}

static PicmanPlugInProcedure *
file_proc_find_by_name (GSList      *procs,
                        const gchar *uri,
                        gboolean     skip_magic)
{
  PicmanPlugInProcedure *proc;

  proc = file_proc_find_by_extension (procs, uri, skip_magic, TRUE);

  if (! proc)
    proc = file_proc_find_by_prefix (procs, uri, skip_magic);

  if (! proc)
    proc = file_proc_find_by_extension (procs, uri, skip_magic, FALSE);

  return proc;
}

static void
file_convert_string (const gchar *instr,
                     gchar       *outmem,
                     gint         maxmem,
                     gint        *nmem)
{
  /* Convert a string in C-notation to array of char */
  const guchar *uin  = (const guchar *) instr;
  guchar       *uout = (guchar *) outmem;
  guchar        tmp[5], *tmpptr;
  guint         k;

  while ((*uin != '\0') && ((((gchar *) uout) - outmem) < maxmem))
    {
      if (*uin != '\\')   /* Not an escaped character ? */
        {
          *(uout++) = *(uin++);
          continue;
        }

      if (*(++uin) == '\0')
        {
          *(uout++) = '\\';
          break;
        }

      switch (*uin)
        {
          case '0':  case '1':  case '2':  case '3': /* octal */
            for (tmpptr = tmp; (tmpptr - tmp) <= 3;)
              {
                *(tmpptr++) = *(uin++);
                if (   (*uin == '\0') || (!g_ascii_isdigit (*uin))
                    || (*uin == '8') || (*uin == '9'))
                  break;
              }

            *tmpptr = '\0';
            sscanf ((gchar *) tmp, "%o", &k);
            *(uout++) = k;
            break;

          case 'a': *(uout++) = '\a'; uin++; break;
          case 'b': *(uout++) = '\b'; uin++; break;
          case 't': *(uout++) = '\t'; uin++; break;
          case 'n': *(uout++) = '\n'; uin++; break;
          case 'v': *(uout++) = '\v'; uin++; break;
          case 'f': *(uout++) = '\f'; uin++; break;
          case 'r': *(uout++) = '\r'; uin++; break;

          default : *(uout++) = *(uin++); break;
        }
    }

  *nmem = ((gchar *) uout) - outmem;
}

static FileMatchType
file_check_single_magic (const gchar  *offset,
                         const gchar  *type,
                         const gchar  *value,
                         const guchar *file_head,
                         gint          headsize,
                         FILE         *ifp)

{
  FileMatchType found = FILE_MATCH_NONE;
  glong         offs;
  gulong        num_testval;
  gulong        num_operatorval;
  gint          numbytes, k;
  const gchar  *num_operator_ptr;
  gchar         num_operator;

  /* Check offset */
  if (sscanf (offset, "%ld", &offs) != 1)
    return FILE_MATCH_NONE;

  /* Check type of test */
  num_operator_ptr = NULL;
  num_operator     = '\0';

  if (g_str_has_prefix (type, "byte"))
    {
      numbytes = 1;
      num_operator_ptr = type + strlen ("byte");
    }
  else if (g_str_has_prefix (type, "short"))
    {
      numbytes = 2;
      num_operator_ptr = type + strlen ("short");
    }
  else if (g_str_has_prefix (type, "long"))
    {
      numbytes = 4;
      num_operator_ptr = type + strlen ("long");
    }
  else if (g_str_has_prefix (type, "size"))
    {
      numbytes = 5;
    }
  else if (strcmp (type, "string") == 0)
    {
      numbytes = 0;
    }
  else
    {
      return FILE_MATCH_NONE;
    }

  /* Check numerical operator value if present */
  if (num_operator_ptr && (*num_operator_ptr == '&'))
    {
      if (g_ascii_isdigit (num_operator_ptr[1]))
        {
          if (num_operator_ptr[1] != '0')      /* decimal */
            sscanf (num_operator_ptr+1, "%lu", &num_operatorval);
          else if (num_operator_ptr[2] == 'x') /* hexadecimal */
            sscanf (num_operator_ptr+3, "%lx", &num_operatorval);
          else                                 /* octal */
            sscanf (num_operator_ptr+2, "%lo", &num_operatorval);

          num_operator = *num_operator_ptr;
        }
    }

  if (numbytes > 0)   /* Numerical test ? */
    {
      gchar   num_test = '=';
      gulong  fileval  = 0;

      /* Check test value */
      if ((value[0] == '>') || (value[0] == '<'))
        {
          num_test = value[0];
          value++;
        }

      errno = 0;
      num_testval = strtol (value, NULL, 0);

      if (errno != 0)
        return FILE_MATCH_NONE;

      if (numbytes == 5)    /* Check for file size ? */
        {
          struct stat buf;

          if (fstat (fileno (ifp), &buf) < 0)
            return FILE_MATCH_NONE;

          fileval = buf.st_size;
        }
      else if (offs >= 0 &&
               (offs + numbytes <= headsize)) /* We have it in memory ? */
        {
          for (k = 0; k < numbytes; k++)
            fileval = (fileval << 8) | (glong) file_head[offs + k];
        }
      else   /* Read it from file */
        {
          gint c = 0;

          if (fseek (ifp, offs, (offs >= 0) ? SEEK_SET : SEEK_END) < 0)
            return FILE_MATCH_NONE;

          for (k = 0; k < numbytes; k++)
            fileval = (fileval << 8) | (c = getc (ifp));

          if (c == EOF)
            return FILE_MATCH_NONE;
        }

      if (num_operator == '&')
        fileval &= num_operatorval;

      if (num_test == '<')
        found = (fileval < num_testval);
      else if (num_test == '>')
        found = (fileval > num_testval);
      else
        found = (fileval == num_testval);

      if (found && (numbytes == 5))
        found = FILE_MATCH_SIZE;
    }
  else if (numbytes == 0) /* String test */
    {
      gchar mem_testval[256];

      file_convert_string (value,
                           mem_testval, sizeof (mem_testval),
                           &numbytes);

      if (numbytes <= 0)
        return FILE_MATCH_NONE;

      if (offs >= 0 &&
          (offs + numbytes <= headsize)) /* We have it in memory ? */
        {
          found = (memcmp (mem_testval, file_head + offs, numbytes) == 0);
        }
      else   /* Read it from file */
        {
          if (fseek (ifp, offs, (offs >= 0) ? SEEK_SET : SEEK_END) < 0)
            return FILE_MATCH_NONE;

          found = FILE_MATCH_MAGIC;

          for (k = 0; found && (k < numbytes); k++)
            {
              gint c = getc (ifp);

              found = (c != EOF) && (c == (gint) mem_testval[k]);
            }
        }
    }

  return found;
}

static FileMatchType
file_check_magic_list (GSList       *magics_list,
                       const guchar *head,
                       gint          headsize,
                       FILE         *ifp)

{
  const gchar   *offset;
  const gchar   *type;
  const gchar   *value;
  gboolean       and   = FALSE;
  gboolean       found = FALSE;
  FileMatchType  match_val;

  while (magics_list)
    {
      if ((offset      = magics_list->data) == NULL) break;
      if ((magics_list = magics_list->next) == NULL) break;
      if ((type        = magics_list->data) == NULL) break;
      if ((magics_list = magics_list->next) == NULL) break;
      if ((value       = magics_list->data) == NULL) break;

      magics_list = magics_list->next;

      match_val = file_check_single_magic (offset, type, value,
                                           head, headsize,
                                           ifp);
      if (and)
        found = found && (match_val != FILE_MATCH_NONE);
      else
        found = (match_val != FILE_MATCH_NONE);

      and = (strchr (offset, '&') != NULL);

      if (! and && found)
        return match_val;
    }

  return FILE_MATCH_NONE;
}
