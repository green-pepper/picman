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

#include "config.h"

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanbuffer.h"
#include "core/picmancurve.h"

#include "picmanclipboard.h"
#include "picmanpixbuf.h"
#include "picmanselectiondata.h"

#include "picman-intl.h"


#define PICMAN_CLIPBOARD_KEY "picman-clipboard"


typedef struct _PicmanClipboard PicmanClipboard;

struct _PicmanClipboard
{
  GSList         *pixbuf_formats;

  GtkTargetEntry *target_entries;
  gint            n_target_entries;

  GtkTargetEntry *svg_target_entries;
  gint            n_svg_target_entries;

  GtkTargetEntry *curve_target_entries;
  gint            n_curve_target_entries;

  PicmanBuffer     *buffer;
  gchar          *svg;
  PicmanCurve      *curve;
};


static PicmanClipboard * picman_clipboard_get        (Picman             *picman);
static void            picman_clipboard_clear      (PicmanClipboard    *picman_clip);
static void            picman_clipboard_free       (PicmanClipboard    *picman_clip);

static GdkAtom * picman_clipboard_wait_for_targets (Picman             *picman,
                                                  gint             *n_targets);
static GdkAtom   picman_clipboard_wait_for_buffer  (Picman             *picman);
static GdkAtom   picman_clipboard_wait_for_svg     (Picman             *picman);
static GdkAtom   picman_clipboard_wait_for_curve   (Picman             *picman);

static void      picman_clipboard_send_buffer      (GtkClipboard     *clipboard,
                                                  GtkSelectionData *selection_data,
                                                  guint             info,
                                                  Picman             *picman);
static void      picman_clipboard_send_svg         (GtkClipboard     *clipboard,
                                                  GtkSelectionData *selection_data,
                                                  guint             info,
                                                  Picman             *picman);
static void      picman_clipboard_send_curve       (GtkClipboard     *clipboard,
                                                  GtkSelectionData *selection_data,
                                                  guint             info,
                                                  Picman             *picman);


/*  public functions  */

void
picman_clipboard_init (Picman *picman)
{
  PicmanClipboard *picman_clip;
  GSList        *list;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman_clip = picman_clipboard_get (picman);

  g_return_if_fail (picman_clip == NULL);

  picman_clip = g_slice_new0 (PicmanClipboard);

  g_object_set_data_full (G_OBJECT (picman), PICMAN_CLIPBOARD_KEY,
                          picman_clip, (GDestroyNotify) picman_clipboard_free);

  picman_clip->pixbuf_formats = picman_pixbuf_get_formats ();

  for (list = picman_clip->pixbuf_formats; list; list = g_slist_next (list))
    {
      GdkPixbufFormat *format = list->data;

      if (gdk_pixbuf_format_is_writable (format))
        {
          gchar **mime_types;
          gchar **type;

          mime_types = gdk_pixbuf_format_get_mime_types (format);

          for (type = mime_types; *type; type++)
            picman_clip->n_target_entries++;

          g_strfreev (mime_types);
        }
    }

  if (picman_clip->n_target_entries > 0)
    {
      gint i = 0;

      picman_clip->target_entries = g_new0 (GtkTargetEntry,
                                          picman_clip->n_target_entries);

      for (list = picman_clip->pixbuf_formats; list; list = g_slist_next (list))
        {
          GdkPixbufFormat *format = list->data;

          if (gdk_pixbuf_format_is_writable (format))
            {
              gchar  *format_name;
              gchar **mime_types;
              gchar **type;

              format_name = gdk_pixbuf_format_get_name (format);
              mime_types  = gdk_pixbuf_format_get_mime_types (format);

              for (type = mime_types; *type; type++)
                {
                  const gchar *mime_type = *type;

                  if (picman->be_verbose)
                    g_printerr ("clipboard: writable pixbuf format: %s\n",
                                mime_type);

                  picman_clip->target_entries[i].target = g_strdup (mime_type);
                  picman_clip->target_entries[i].flags  = 0;
                  picman_clip->target_entries[i].info   = i;

                  i++;
                }

              g_strfreev (mime_types);
              g_free (format_name);
            }
        }
    }

  picman_clip->n_svg_target_entries = 2;
  picman_clip->svg_target_entries   = g_new0 (GtkTargetEntry, 2);

  picman_clip->svg_target_entries[0].target = g_strdup ("image/svg");
  picman_clip->svg_target_entries[0].flags  = 0;
  picman_clip->svg_target_entries[0].info   = 0;

  picman_clip->svg_target_entries[1].target = g_strdup ("image/svg+xml");
  picman_clip->svg_target_entries[1].flags  = 0;
  picman_clip->svg_target_entries[1].info   = 1;

  picman_clip->n_curve_target_entries = 1;
  picman_clip->curve_target_entries   = g_new0 (GtkTargetEntry, 1);

  picman_clip->curve_target_entries[0].target = g_strdup ("application/x-picman-curve");
  picman_clip->curve_target_entries[0].flags  = 0;
  picman_clip->curve_target_entries[0].info   = 0;
}

void
picman_clipboard_exit (Picman *picman)
{
  GtkClipboard *clipboard;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);

  if (clipboard && gtk_clipboard_get_owner (clipboard) == G_OBJECT (picman))
    gtk_clipboard_store (clipboard);

  g_object_set_data (G_OBJECT (picman), PICMAN_CLIPBOARD_KEY, NULL);
}

/**
 * picman_clipboard_has_buffer:
 * @picman: pointer to #Picman
 *
 * Tests if there's image data in the clipboard. If the global cut
 * buffer of @picman is empty, this function checks if there's image
 * data in %GDK_SELECTION_CLIPBOARD. This is done in a main-loop
 * similar to gtk_clipboard_wait_is_text_available(). The same caveats
 * apply here.
 *
 * Return value: %TRUE if there's image data in the clipboard, %FALSE otherwise
 **/
gboolean
picman_clipboard_has_buffer (Picman *picman)
{
  PicmanClipboard *picman_clip;
  GtkClipboard  *clipboard;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);

  if (clipboard                                                &&
      gtk_clipboard_get_owner (clipboard)   != G_OBJECT (picman) &&
      picman_clipboard_wait_for_buffer (picman) != GDK_NONE)
    {
      return TRUE;
    }

  picman_clip = picman_clipboard_get (picman);

  return (picman_clip->buffer != NULL);
}

/**
 * picman_clipboard_has_svg:
 * @picman: pointer to #Picman
 *
 * Tests if there's SVG data in %GDK_SELECTION_CLIPBOARD.
 * This is done in a main-loop similar to
 * gtk_clipboard_wait_is_text_available(). The same caveats apply here.
 *
 * Return value: %TRUE if there's SVG data in the clipboard, %FALSE otherwise
 **/
gboolean
picman_clipboard_has_svg (Picman *picman)
{
  PicmanClipboard *picman_clip;
  GtkClipboard  *clipboard;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);

  if (clipboard                                              &&
      gtk_clipboard_get_owner (clipboard) != G_OBJECT (picman) &&
      picman_clipboard_wait_for_svg (picman)  != GDK_NONE)
    {
      return TRUE;
    }

  picman_clip = picman_clipboard_get (picman);

  return (picman_clip->svg != NULL);
}

/**
 * picman_clipboard_has_curve:
 * @picman: pointer to #Picman
 *
 * Tests if there's curve data in %GDK_SELECTION_CLIPBOARD.
 * This is done in a main-loop similar to
 * gtk_clipboard_wait_is_text_available(). The same caveats apply here.
 *
 * Return value: %TRUE if there's curve data in the clipboard, %FALSE otherwise
 **/
gboolean
picman_clipboard_has_curve (Picman *picman)
{
  PicmanClipboard *picman_clip;
  GtkClipboard  *clipboard;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);

  if (clipboard                                              &&
      gtk_clipboard_get_owner (clipboard) != G_OBJECT (picman) &&
      picman_clipboard_wait_for_curve (picman)  != GDK_NONE)
    {
      return TRUE;
    }

  picman_clip = picman_clipboard_get (picman);

  return (picman_clip->curve != NULL);
}

/**
 * picman_clipboard_get_buffer:
 * @picman: pointer to #Picman
 *
 * Retrieves either image data from %GDK_SELECTION_CLIPBOARD or from
 * the global cut buffer of @picman.
 *
 * The returned #PicmanBuffer needs to be unref'ed when it's no longer
 * needed.
 *
 * Return value: a reference to a #PicmanBuffer or %NULL if there's no
 *               image data
 **/
PicmanBuffer *
picman_clipboard_get_buffer (Picman *picman)
{
  PicmanClipboard *picman_clip;
  GtkClipboard  *clipboard;
  GdkAtom        atom;
  PicmanBuffer    *buffer = NULL;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);

  if (clipboard                                                         &&
      gtk_clipboard_get_owner (clipboard)            != G_OBJECT (picman) &&
      (atom = picman_clipboard_wait_for_buffer (picman)) != GDK_NONE)
    {
      GtkSelectionData *data;

      picman_set_busy (picman);

      data = gtk_clipboard_wait_for_contents (clipboard, atom);

      if (data)
        {
          GdkPixbuf *pixbuf = gtk_selection_data_get_pixbuf (data);

          gtk_selection_data_free (data);

          if (pixbuf)
            {
              buffer = picman_buffer_new_from_pixbuf (pixbuf, _("Clipboard"),
                                                    0, 0);
              g_object_unref (pixbuf);
            }
        }

      picman_unset_busy (picman);
    }

  picman_clip = picman_clipboard_get (picman);

  if (! buffer && picman_clip->buffer)
    buffer = g_object_ref (picman_clip->buffer);

  return buffer;
}

/**
 * picman_clipboard_get_svg:
 * @picman: pointer to #Picman
 * @svg_length: returns the size of the SVG stream in bytes
 *
 * Retrieves SVG data from %GDK_SELECTION_CLIPBOARD or from the global
 * SVG buffer of @picman.
 *
 * The returned data needs to be freed when it's no longer needed.
 *
 * Return value: a reference to a #PicmanBuffer or %NULL if there's no
 *               image data
 **/
gchar *
picman_clipboard_get_svg (Picman  *picman,
                        gsize *svg_length)
{
  PicmanClipboard *picman_clip;
  GtkClipboard  *clipboard;
  GdkAtom        atom;
  gchar         *svg = NULL;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (svg_length != NULL, NULL);

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);

  if (clipboard                                                      &&
      gtk_clipboard_get_owner (clipboard)         != G_OBJECT (picman) &&
      (atom = picman_clipboard_wait_for_svg (picman)) != GDK_NONE)
    {
      GtkSelectionData *data;

      picman_set_busy (picman);

      data = gtk_clipboard_wait_for_contents (clipboard, atom);

      if (data)
        {
          const guchar *stream;

          stream = picman_selection_data_get_stream (data, svg_length);

          if (stream)
            svg = g_memdup (stream, *svg_length);

          gtk_selection_data_free (data);
        }

      picman_unset_busy (picman);
    }

  picman_clip = picman_clipboard_get (picman);

  if (! svg && picman_clip->svg)
    {
      svg = g_strdup (picman_clip->svg);
      *svg_length = strlen (svg);
    }

  return svg;
}

/**
 * picman_clipboard_get_curve:
 * @picman: pointer to #Picman
 *
 * Retrieves curve data from %GDK_SELECTION_CLIPBOARD or from the global
 * curve buffer of @picman.
 *
 * The returned curve needs to be unref'ed when it's no longer needed.
 *
 * Return value: a reference to a #PicmanCurve or %NULL if there's no
 *               curve data
 **/
PicmanCurve *
picman_clipboard_get_curve (Picman *picman)
{
  PicmanClipboard *picman_clip;
  GtkClipboard  *clipboard;
  GdkAtom        atom;
  PicmanCurve     *curve = NULL;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);

  if (clipboard                                                        &&
      gtk_clipboard_get_owner (clipboard)           != G_OBJECT (picman) &&
      (atom = picman_clipboard_wait_for_curve (picman)) != GDK_NONE)
    {
      GtkSelectionData *data;

      picman_set_busy (picman);

      data = gtk_clipboard_wait_for_contents (clipboard, atom);

      if (data)
        {
          curve = picman_selection_data_get_curve (data);

          gtk_selection_data_free (data);
        }

      picman_unset_busy (picman);
    }

  picman_clip = picman_clipboard_get (picman);

  if (! curve && picman_clip->curve)
    curve = g_object_ref (picman_clip->curve);

  return curve;
}

/**
 * picman_clipboard_set_buffer:
 * @picman:   pointer to #Picman
 * @buffer: a #PicmanBuffer, or %NULL.
 *
 * Offers the buffer in %GDK_SELECTION_CLIPBOARD.
 **/
void
picman_clipboard_set_buffer (Picman       *picman,
                           PicmanBuffer *buffer)
{
  PicmanClipboard *picman_clip;
  GtkClipboard  *clipboard;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (buffer == NULL || PICMAN_IS_BUFFER (buffer));

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);
  if (! clipboard)
    return;

  picman_clip = picman_clipboard_get (picman);

  picman_clipboard_clear (picman_clip);

  if (buffer)
    {
      picman_clip->buffer = g_object_ref (buffer);

      gtk_clipboard_set_with_owner (clipboard,
                                    picman_clip->target_entries,
                                    picman_clip->n_target_entries,
                                    (GtkClipboardGetFunc) picman_clipboard_send_buffer,
                                    (GtkClipboardClearFunc) NULL,
                                    G_OBJECT (picman));

      /*  mark the first entry (image/png) as suitable for storing  */
      if (picman_clip->n_target_entries > 0)
        gtk_clipboard_set_can_store (clipboard, picman_clip->target_entries, 1);
    }
  else if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (picman))
    {
      gtk_clipboard_clear (clipboard);
    }
}

/**
 * picman_clipboard_set_svg:
 * @picman: pointer to #Picman
 * @svg: a string containing the SVG data, or %NULL
 *
 * Offers SVG data in %GDK_SELECTION_CLIPBOARD.
 **/
void
picman_clipboard_set_svg (Picman        *picman,
                        const gchar *svg)
{
  PicmanClipboard *picman_clip;
  GtkClipboard  *clipboard;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);
  if (! clipboard)
    return;

  picman_clip = picman_clipboard_get (picman);

  picman_clipboard_clear (picman_clip);

  if (svg)
    {
      picman_clip->svg = g_strdup (svg);

      gtk_clipboard_set_with_owner (clipboard,
                                    picman_clip->svg_target_entries,
                                    picman_clip->n_svg_target_entries,
                                    (GtkClipboardGetFunc) picman_clipboard_send_svg,
                                    (GtkClipboardClearFunc) NULL,
                                    G_OBJECT (picman));

      /*  mark the first entry (image/svg) as suitable for storing  */
      gtk_clipboard_set_can_store (clipboard, picman_clip->svg_target_entries, 1);
    }
  else if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (picman))
    {
      gtk_clipboard_clear (clipboard);
    }
}

/**
 * picman_clipboard_set_text:
 * @picman: pointer to #Picman
 * @text: a %NULL-terminated string in UTF-8 encoding
 *
 * Offers @text in %GDK_SELECTION_CLIPBOARD and %GDK_SELECTION_PRIMARY.
 **/
void
picman_clipboard_set_text (Picman        *picman,
                         const gchar *text)
{
  GtkClipboard *clipboard;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (text != NULL);

  picman_clipboard_clear (picman_clipboard_get (picman));

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);
  if (clipboard)
    gtk_clipboard_set_text (clipboard, text, -1);

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_PRIMARY);
  if (clipboard)
    gtk_clipboard_set_text (clipboard, text, -1);
}

/**
 * picman_clipboard_set_curve:
 * @picman: pointer to #Picman
 * @curve: a #PicmanCurve, or %NULL
 *
 * Offers curve data in %GDK_SELECTION_CLIPBOARD.
 **/
void
picman_clipboard_set_curve (Picman      *picman,
                          PicmanCurve *curve)
{
  PicmanClipboard *picman_clip;
  GtkClipboard  *clipboard;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (curve == NULL || PICMAN_IS_CURVE (curve));

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);
  if (! clipboard)
    return;

  picman_clip = picman_clipboard_get (picman);

  picman_clipboard_clear (picman_clip);

  if (curve)
    {
      picman_clip->curve = g_object_ref (curve);

      gtk_clipboard_set_with_owner (clipboard,
                                    picman_clip->curve_target_entries,
                                    picman_clip->n_curve_target_entries,
                                    (GtkClipboardGetFunc) picman_clipboard_send_curve,
                                    (GtkClipboardClearFunc) NULL,
                                    G_OBJECT (picman));

      gtk_clipboard_set_can_store (clipboard, picman_clip->curve_target_entries, 1);
    }
  else if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (picman))
    {
      gtk_clipboard_clear (clipboard);
    }
}


/*  private functions  */

static PicmanClipboard *
picman_clipboard_get (Picman *picman)
{
  return g_object_get_data (G_OBJECT (picman), PICMAN_CLIPBOARD_KEY);
}

static void
picman_clipboard_clear (PicmanClipboard *picman_clip)
{
  if (picman_clip->buffer)
    {
      g_object_unref (picman_clip->buffer);
      picman_clip->buffer = NULL;
    }

  if (picman_clip->svg)
    {
      g_free (picman_clip->svg);
      picman_clip->svg = NULL;
    }

  if (picman_clip->curve)
    {
      g_object_unref (picman_clip->curve);
      picman_clip->curve = NULL;
    }
}

static void
picman_clipboard_free (PicmanClipboard *picman_clip)
{
  gint i;

  picman_clipboard_clear (picman_clip);

  g_slist_free (picman_clip->pixbuf_formats);

  for (i = 0; i < picman_clip->n_target_entries; i++)
    g_free ((gchar *) picman_clip->target_entries[i].target);

  g_free (picman_clip->target_entries);

  for (i = 0; i < picman_clip->n_svg_target_entries; i++)
    g_free ((gchar *) picman_clip->svg_target_entries[i].target);

  g_free (picman_clip->svg_target_entries);

  for (i = 0; i < picman_clip->n_curve_target_entries; i++)
    g_free ((gchar *) picman_clip->curve_target_entries[i].target);

  g_free (picman_clip->curve_target_entries);

  g_slice_free (PicmanClipboard, picman_clip);
}

static GdkAtom *
picman_clipboard_wait_for_targets (Picman *picman,
                                 gint *n_targets)
{
  GtkClipboard *clipboard;

  clipboard = gtk_clipboard_get_for_display (gdk_display_get_default (),
                                             GDK_SELECTION_CLIPBOARD);

  if (clipboard)
    {
      GtkSelectionData *data;
      GdkAtom           atom = gdk_atom_intern_static_string ("TARGETS");

      data = gtk_clipboard_wait_for_contents (clipboard, atom);

      if (data)
        {
          GdkAtom  *targets;
          gboolean  success;

          success = gtk_selection_data_get_targets (data, &targets, n_targets);

          gtk_selection_data_free (data);

          if (success)
            {
              if (picman->be_verbose)
                {
                  gint i;

                  for (i = 0; i < *n_targets; i++)
                    g_printerr ("clipboard: offered type: %s\n",
                                gdk_atom_name (targets[i]));

                  g_printerr ("\n");
                }

              return targets;
            }
        }
    }

  return NULL;
}

static GdkAtom
picman_clipboard_wait_for_buffer (Picman *picman)
{
  PicmanClipboard *picman_clip = picman_clipboard_get (picman);
  GdkAtom       *targets;
  gint           n_targets;
  GdkAtom        result    = GDK_NONE;

  targets = picman_clipboard_wait_for_targets (picman, &n_targets);

  if (targets)
    {
      GSList *list;

      for (list = picman_clip->pixbuf_formats; list; list = g_slist_next (list))
        {
          GdkPixbufFormat  *format = list->data;
          gchar           **mime_types;
          gchar           **type;

          if (picman->be_verbose)
            g_printerr ("clipboard: checking pixbuf format '%s'\n",
                        gdk_pixbuf_format_get_name (format));

          mime_types = gdk_pixbuf_format_get_mime_types (format);

          for (type = mime_types; *type; type++)
            {
              gchar   *mime_type = *type;
              GdkAtom  atom      = gdk_atom_intern (mime_type, FALSE);
              gint     i;

              if (picman->be_verbose)
                g_printerr ("  - checking mime type '%s'\n", mime_type);

              for (i = 0; i < n_targets; i++)
                {
                  if (targets[i] == atom)
                    {
                      result = atom;
                      break;
                    }
                }

              if (result != GDK_NONE)
                break;
            }

          g_strfreev (mime_types);

          if (result != GDK_NONE)
            break;
        }

      g_free (targets);
    }

  return result;
}

static GdkAtom
picman_clipboard_wait_for_svg (Picman *picman)
{
  GdkAtom *targets;
  gint     n_targets;
  GdkAtom  result = GDK_NONE;

  targets = picman_clipboard_wait_for_targets (picman, &n_targets);

  if (targets)
    {
      GdkAtom svg_atom     = gdk_atom_intern_static_string ("image/svg");
      GdkAtom svg_xml_atom = gdk_atom_intern_static_string ("image/svg+xml");
      gint    i;

      for (i = 0; i < n_targets; i++)
        {
          if (targets[i] == svg_atom)
            {
              result = svg_atom;
              break;
            }
          else if (targets[i] == svg_xml_atom)
            {
              result = svg_xml_atom;
              break;
            }
        }

      g_free (targets);
    }

  return result;
}

static GdkAtom
picman_clipboard_wait_for_curve (Picman *picman)
{
  GdkAtom *targets;
  gint     n_targets;
  GdkAtom  result = GDK_NONE;

  targets = picman_clipboard_wait_for_targets (picman, &n_targets);

  if (targets)
    {
      GdkAtom curve_atom = gdk_atom_intern_static_string ("application/x-picman-curve");
      gint    i;

      for (i = 0; i < n_targets; i++)
        {
          if (targets[i] == curve_atom)
            {
              result = curve_atom;
              break;
            }
        }

      g_free (targets);
    }

  return result;
}

static void
picman_clipboard_send_buffer (GtkClipboard     *clipboard,
                            GtkSelectionData *selection_data,
                            guint             info,
                            Picman             *picman)
{
  PicmanClipboard *picman_clip = picman_clipboard_get (picman);
  GdkPixbuf     *pixbuf;

  picman_set_busy (picman);

  pixbuf = picman_viewable_get_pixbuf (PICMAN_VIEWABLE (picman_clip->buffer),
                                     picman_get_user_context (picman),
                                     picman_buffer_get_width (picman_clip->buffer),
                                     picman_buffer_get_height (picman_clip->buffer));

  if (pixbuf)
    {
      if (picman->be_verbose)
        g_printerr ("clipboard: sending pixbuf data as '%s'\n",
                    picman_clip->target_entries[info].target);

      gtk_selection_data_set_pixbuf (selection_data, pixbuf);
    }
  else
    {
      g_warning ("%s: picman_viewable_get_pixbuf() failed", G_STRFUNC);
    }

  picman_unset_busy (picman);
}

static void
picman_clipboard_send_svg (GtkClipboard     *clipboard,
                         GtkSelectionData *selection_data,
                         guint             info,
                         Picman             *picman)
{
  PicmanClipboard *picman_clip = picman_clipboard_get (picman);

  picman_set_busy (picman);

  if (picman_clip->svg)
    {
      if (picman->be_verbose)
        g_printerr ("clipboard: sending SVG data as '%s'\n",
                    picman_clip->svg_target_entries[info].target);

      picman_selection_data_set_stream (selection_data,
                                      (const guchar *) picman_clip->svg,
                                      strlen (picman_clip->svg));
    }

  picman_unset_busy (picman);
}

static void
picman_clipboard_send_curve (GtkClipboard     *clipboard,
                           GtkSelectionData *selection_data,
                           guint             info,
                           Picman             *picman)
{
  PicmanClipboard *picman_clip = picman_clipboard_get (picman);

  picman_set_busy (picman);

  if (picman_clip->curve)
    {
      if (picman->be_verbose)
        g_printerr ("clipboard: sending curve data as '%s'\n",
                    picman_clip->curve_target_entries[info].target);

      picman_selection_data_set_curve (selection_data, picman_clip->curve);
    }

  picman_unset_busy (picman);
}
