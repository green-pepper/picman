/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanimagefile.c
 *
 * Copyright (C) 2001-2004  Sven Neumann <sven@picman.org>
 *                          Michael Natterer <mitch@picman.org>
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
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanthumb/picmanthumb.h"

#include "core-types.h"

#include "config/picmancoreconfig.h"

#include "gegl/picman-babl.h"
#include "gegl/picman-gegl-utils.h"

#include "picman.h"
#include "picmancontainer.h"
#include "picmancontext.h"
#include "picmanimage.h"
#include "picmanimagefile.h"
#include "picmanmarshal.h"
#include "picmanpickable.h"
#include "picmanprogress.h"

#include "file/file-open.h"
#include "file/file-utils.h"

#include "picman-intl.h"


enum
{
  INFO_CHANGED,
  LAST_SIGNAL
};


typedef struct _PicmanImagefilePrivate PicmanImagefilePrivate;

struct _PicmanImagefilePrivate
{
  Picman          *picman;

  PicmanThumbnail *thumbnail;
  GIcon         *icon;
  GCancellable  *icon_cancellable;

  gchar         *description;
  gboolean       static_desc;
};

#define GET_PRIVATE(imagefile) G_TYPE_INSTANCE_GET_PRIVATE (imagefile, \
                                                            PICMAN_TYPE_IMAGEFILE, \
                                                            PicmanImagefilePrivate)


static void        picman_imagefile_dispose          (GObject        *object);
static void        picman_imagefile_finalize         (GObject        *object);

static void        picman_imagefile_name_changed     (PicmanObject     *object);

static void        picman_imagefile_info_changed     (PicmanImagefile  *imagefile);
static void        picman_imagefile_notify_thumbnail (PicmanImagefile  *imagefile,
                                                    GParamSpec     *pspec);

static GdkPixbuf * picman_imagefile_get_new_pixbuf   (PicmanViewable   *viewable,
                                                    PicmanContext    *context,
                                                    gint            width,
                                                    gint            height);
static GdkPixbuf * picman_imagefile_load_thumb       (PicmanImagefile  *imagefile,
                                                    gint            width,
                                                    gint            height);
static gboolean    picman_imagefile_save_thumb       (PicmanImagefile  *imagefile,
                                                    PicmanImage      *image,
                                                    gint            size,
                                                    gboolean        replace,
                                                    GError        **error);

static gchar     * picman_imagefile_get_description  (PicmanViewable   *viewable,
                                                    gchar         **tooltip);

static void        picman_imagefile_icon_callback    (GObject        *source_object,
                                                    GAsyncResult   *result,
                                                    gpointer        data);

static void     picman_thumbnail_set_info_from_image (PicmanThumbnail  *thumbnail,
                                                    const gchar    *mime_type,
                                                    PicmanImage      *image);
static void     picman_thumbnail_set_info            (PicmanThumbnail  *thumbnail,
                                                    const gchar    *mime_type,
                                                    gint            width,
                                                    gint            height,
                                                    const Babl     *format,
                                                    gint            num_layers);


G_DEFINE_TYPE (PicmanImagefile, picman_imagefile, PICMAN_TYPE_VIEWABLE)

#define parent_class picman_imagefile_parent_class

static guint picman_imagefile_signals[LAST_SIGNAL] = { 0 };


static void
picman_imagefile_class_init (PicmanImagefileClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  gchar             *creator;

  picman_imagefile_signals[INFO_CHANGED] =
    g_signal_new ("info-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImagefileClass, info_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->dispose               = picman_imagefile_dispose;
  object_class->finalize              = picman_imagefile_finalize;

  picman_object_class->name_changed     = picman_imagefile_name_changed;

  viewable_class->name_changed_signal = "info-changed";
  viewable_class->get_new_pixbuf      = picman_imagefile_get_new_pixbuf;
  viewable_class->get_description     = picman_imagefile_get_description;

  g_type_class_ref (PICMAN_TYPE_IMAGE_TYPE);

  creator = g_strdup_printf ("picman-%d.%d",
                             PICMAN_MAJOR_VERSION, PICMAN_MINOR_VERSION);

  picman_thumb_init (creator, NULL);

  g_free (creator);

  g_type_class_add_private (klass, sizeof (PicmanImagefilePrivate));
}

static void
picman_imagefile_init (PicmanImagefile *imagefile)
{
  PicmanImagefilePrivate *private = GET_PRIVATE (imagefile);

  private->thumbnail = picman_thumbnail_new ();

  g_signal_connect_object (private->thumbnail, "notify",
                           G_CALLBACK (picman_imagefile_notify_thumbnail),
                           imagefile, G_CONNECT_SWAPPED);
}

static void
picman_imagefile_dispose (GObject *object)
{
  PicmanImagefilePrivate *private = GET_PRIVATE (object);

  if (private->icon_cancellable)
    {
      g_cancellable_cancel (private->icon_cancellable);
      g_object_unref (private->icon_cancellable);
      private->icon_cancellable = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_imagefile_finalize (GObject *object)
{
  PicmanImagefilePrivate *private = GET_PRIVATE (object);

  if (private->description)
    {
      if (! private->static_desc)
        g_free (private->description);

      private->description = NULL;
    }

  if (private->thumbnail)
    {
      g_object_unref (private->thumbnail);
      private->thumbnail = NULL;
    }

  if (private->icon)
    {
      g_object_unref (private->icon);
      private->icon = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

PicmanImagefile *
picman_imagefile_new (Picman        *picman,
                    const gchar *uri)
{
  PicmanImagefile *imagefile;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  imagefile = g_object_new (PICMAN_TYPE_IMAGEFILE, NULL);

  GET_PRIVATE (imagefile)->picman = picman;

  if (uri)
    picman_object_set_name (PICMAN_OBJECT (imagefile), uri);

  return imagefile;
}

PicmanThumbnail *
picman_imagefile_get_thumbnail (PicmanImagefile *imagefile)
{
  g_return_val_if_fail (PICMAN_IS_IMAGEFILE (imagefile), NULL);

  return GET_PRIVATE (imagefile)->thumbnail;
}

GIcon *
picman_imagefile_get_gicon (PicmanImagefile *imagefile)
{
  PicmanImagefilePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGEFILE (imagefile), NULL);

  private = GET_PRIVATE (imagefile);

  if (private->icon)
    return private->icon;

  if (! private->icon_cancellable)
    {
      GFile *file;

      file = g_file_new_for_uri (picman_object_get_name (imagefile));

      private->icon_cancellable = g_cancellable_new ();

      g_file_query_info_async (file, "standard::icon",
                               G_FILE_QUERY_INFO_NONE,
                               G_PRIORITY_DEFAULT,
                               private->icon_cancellable,
                               picman_imagefile_icon_callback,
                               imagefile);

      g_object_unref (file);
    }

  return NULL;
}

void
picman_imagefile_set_mime_type (PicmanImagefile *imagefile,
                              const gchar   *mime_type)
{
  g_return_if_fail (PICMAN_IS_IMAGEFILE (imagefile));

  g_object_set (GET_PRIVATE (imagefile)->thumbnail,
                "image-mimetype", mime_type,
                NULL);
}

void
picman_imagefile_update (PicmanImagefile *imagefile)
{
  PicmanImagefilePrivate *private;
  gchar                *uri;

  g_return_if_fail (PICMAN_IS_IMAGEFILE (imagefile));

  private = GET_PRIVATE (imagefile);

  picman_viewable_invalidate_preview (PICMAN_VIEWABLE (imagefile));

  g_object_get (private->thumbnail,
                "image-uri", &uri,
                NULL);

  if (uri)
    {
      PicmanImagefile *documents_imagefile = (PicmanImagefile *)
        picman_container_get_child_by_name (private->picman->documents, uri);

      if (documents_imagefile != imagefile &&
          PICMAN_IS_IMAGEFILE (documents_imagefile))
        picman_viewable_invalidate_preview (PICMAN_VIEWABLE (documents_imagefile));

      g_free (uri);
    }
}

void
picman_imagefile_create_thumbnail (PicmanImagefile *imagefile,
                                 PicmanContext   *context,
                                 PicmanProgress  *progress,
                                 gint           size,
                                 gboolean       replace)
{
  PicmanImagefilePrivate *private;
  PicmanThumbnail        *thumbnail;
  PicmanThumbState        image_state;

  g_return_if_fail (PICMAN_IS_IMAGEFILE (imagefile));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  if (size < 1)
    return;

  private = GET_PRIVATE (imagefile);

  thumbnail = private->thumbnail;

  picman_thumbnail_set_uri (thumbnail,
                          picman_object_get_name (imagefile));

  image_state = picman_thumbnail_peek_image (thumbnail);

  if (image_state == PICMAN_THUMB_STATE_REMOTE ||
      image_state >= PICMAN_THUMB_STATE_EXISTS)
    {
      PicmanImage     *image;
      gboolean       success;
      gint           width      = 0;
      gint           height     = 0;
      const gchar   *mime_type  = NULL;
      GError        *error      = NULL;
      const Babl    *format     = NULL;
      gint           num_layers = -1;

      g_object_ref (imagefile);

      image = file_open_thumbnail (private->picman, context, progress,
                                   thumbnail->image_uri, size,
                                   &mime_type, &width, &height,
                                   &format, &num_layers, NULL);

      if (image)
        {
          picman_thumbnail_set_info (private->thumbnail,
                                   mime_type, width, height,
                                   format, num_layers);
        }
      else
        {
          PicmanPDBStatusType  status;

          image = file_open_image (private->picman, context, progress,
                                   thumbnail->image_uri,
                                   thumbnail->image_uri,
                                   FALSE, NULL, PICMAN_RUN_NONINTERACTIVE,
                                   &status, &mime_type, NULL);

          if (image)
            picman_thumbnail_set_info_from_image (private->thumbnail,
                                                mime_type, image);
        }

      if (image)
        {
          success = picman_imagefile_save_thumb (imagefile,
                                               image, size, replace,
                                               &error);

          g_object_unref (image);
        }
      else
        {
          success = picman_thumbnail_save_failure (thumbnail,
                                                 "PICMAN " PICMAN_VERSION,
                                                 &error);
          picman_imagefile_update (imagefile);
        }

      g_object_unref (imagefile);

      if (! success)
        {
          picman_message_literal (private->picman,
				G_OBJECT (progress), PICMAN_MESSAGE_ERROR,
				error->message);
          g_clear_error (&error);
        }
    }
}

/*  The weak version doesn't ref the imagefile but deals gracefully
 *  with an imagefile that is destroyed while the thumbnail is
 *  created. Thia allows one to use this function w/o the need to
 *  block the user interface.
 */
void
picman_imagefile_create_thumbnail_weak (PicmanImagefile *imagefile,
                                      PicmanContext   *context,
                                      PicmanProgress  *progress,
                                      gint           size,
                                      gboolean       replace)
{
  PicmanImagefilePrivate *private;
  PicmanImagefile        *local;
  const gchar          *uri;

  g_return_if_fail (PICMAN_IS_IMAGEFILE (imagefile));

  if (size < 1)
    return;

  private = GET_PRIVATE (imagefile);

  uri = picman_object_get_name (imagefile);
  if (! uri)
    return;

  local = picman_imagefile_new (private->picman, uri);

  g_object_add_weak_pointer (G_OBJECT (imagefile), (gpointer) &imagefile);

  picman_imagefile_create_thumbnail (local, context, progress, size, replace);

  if (imagefile)
    {
      uri = picman_object_get_name (imagefile);

      if (uri &&
          strcmp (uri, picman_object_get_name (local)) == 0)
        {
          picman_imagefile_update (imagefile);
        }

      g_object_remove_weak_pointer (G_OBJECT (imagefile),
                                    (gpointer) &imagefile);
    }

  g_object_unref (local);
}

gboolean
picman_imagefile_check_thumbnail (PicmanImagefile *imagefile)
{
  PicmanImagefilePrivate *private;
  gint                  size;

  g_return_val_if_fail (PICMAN_IS_IMAGEFILE (imagefile), FALSE);

  private = GET_PRIVATE (imagefile);

  size = private->picman->config->thumbnail_size;

  if (size > 0)
    {
      PicmanThumbState  state;

      state = picman_thumbnail_check_thumb (private->thumbnail, size);

      return (state == PICMAN_THUMB_STATE_OK);
    }

  return TRUE;
}

gboolean
picman_imagefile_save_thumbnail (PicmanImagefile *imagefile,
                               const gchar   *mime_type,
                               PicmanImage     *image)
{
  PicmanImagefilePrivate *private;
  gint                  size;
  gboolean              success = TRUE;
  GError               *error   = NULL;

  g_return_val_if_fail (PICMAN_IS_IMAGEFILE (imagefile), FALSE);
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = GET_PRIVATE (imagefile);

  size = private->picman->config->thumbnail_size;

  if (size > 0)
    {
      picman_thumbnail_set_info_from_image (private->thumbnail,
                                          mime_type, image);

      success = picman_imagefile_save_thumb (imagefile,
                                           image, size, FALSE,
                                           &error);
      if (! success)
        {
          picman_message_literal (private->picman, NULL, PICMAN_MESSAGE_ERROR,
				error->message);
          g_clear_error (&error);
        }
    }

  return success;
}


/*  private functions  */

static void
picman_imagefile_name_changed (PicmanObject *object)
{
  PicmanImagefilePrivate *private = GET_PRIVATE (object);

  if (PICMAN_OBJECT_CLASS (parent_class)->name_changed)
    PICMAN_OBJECT_CLASS (parent_class)->name_changed (object);

  picman_thumbnail_set_uri (private->thumbnail, picman_object_get_name (object));
}

static void
picman_imagefile_info_changed (PicmanImagefile *imagefile)
{
  PicmanImagefilePrivate *private = GET_PRIVATE (imagefile);

  if (private->description)
    {
      if (! private->static_desc)
        g_free (private->description);

      private->description = NULL;
    }

  g_signal_emit (imagefile, picman_imagefile_signals[INFO_CHANGED], 0);
}

static void
picman_imagefile_notify_thumbnail (PicmanImagefile *imagefile,
                                 GParamSpec    *pspec)
{
  if (strcmp (pspec->name, "image-state") == 0 ||
      strcmp (pspec->name, "thumb-state") == 0)
    {
      picman_imagefile_info_changed (imagefile);
    }
}

static GdkPixbuf *
picman_imagefile_get_new_pixbuf (PicmanViewable *viewable,
                               PicmanContext  *context,
                               gint          width,
                               gint          height)
{
  PicmanImagefile *imagefile = PICMAN_IMAGEFILE (viewable);

  if (! picman_object_get_name (imagefile))
    return NULL;

  return picman_imagefile_load_thumb (imagefile, width, height);
}

static gchar *
picman_imagefile_get_description (PicmanViewable   *viewable,
                                gchar         **tooltip)
{
  PicmanImagefile        *imagefile = PICMAN_IMAGEFILE (viewable);
  PicmanImagefilePrivate *private   = GET_PRIVATE (imagefile);
  PicmanThumbnail        *thumbnail = private->thumbnail;
  gchar                *basename;

  if (! thumbnail->image_uri)
    return NULL;

  if (tooltip)
    {
      gchar       *filename;
      const gchar *desc;

      filename = file_utils_uri_display_name (thumbnail->image_uri);
      desc     = picman_imagefile_get_desc_string (imagefile);

      if (desc)
        {
          *tooltip = g_strdup_printf ("%s\n%s", filename, desc);
          g_free (filename);
        }
      else
        {
          *tooltip = filename;
        }
    }

  basename = file_utils_uri_display_basename (thumbnail->image_uri);

  if (thumbnail->image_width > 0 && thumbnail->image_height > 0)
    {
      gchar *tmp = basename;

      basename = g_strdup_printf ("%s (%d × %d)",
                                  tmp,
                                  thumbnail->image_width,
                                  thumbnail->image_height);
      g_free (tmp);
    }

  return basename;
}

static void
picman_imagefile_icon_callback (GObject      *source_object,
                              GAsyncResult *result,
                              gpointer      data)
{
  PicmanImagefile        *imagefile;
  PicmanImagefilePrivate *private;
  GFile                *file  = G_FILE (source_object);
  GError               *error = NULL;
  GFileInfo            *file_info;

  file_info = g_file_query_info_finish (file, result, &error);

  if (error)
    {
      /* we were cancelled from dispose() and the imagefile is
       * long gone, bail out
       */
      if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        {
          g_clear_error (&error);
          return;
        }

#ifdef PICMAN_UNSTABLE
      g_printerr ("%s: %s\n", G_STRFUNC, error->message);
#endif

      g_clear_error (&error);
    }

  imagefile = PICMAN_IMAGEFILE (data);
  private   = GET_PRIVATE (imagefile);

  if (file_info)
    {
      private->icon = g_object_ref (g_file_info_get_icon (file_info));
      g_object_unref (file_info);
    }

  if (private->icon_cancellable)
    {
      g_object_unref (private->icon_cancellable);
      private->icon_cancellable = NULL;
    }

  if (private->icon)
    picman_viewable_invalidate_preview (PICMAN_VIEWABLE (imagefile));
}

const gchar *
picman_imagefile_get_desc_string (PicmanImagefile *imagefile)
{
  PicmanImagefilePrivate *private;
  PicmanThumbnail        *thumbnail;

  g_return_val_if_fail (PICMAN_IS_IMAGEFILE (imagefile), NULL);

  private = GET_PRIVATE (imagefile);

  if (private->description)
    return (const gchar *) private->description;

  thumbnail = private->thumbnail;

  switch (thumbnail->image_state)
    {
    case PICMAN_THUMB_STATE_UNKNOWN:
      private->description = NULL;
      private->static_desc = TRUE;
      break;

    case PICMAN_THUMB_STATE_FOLDER:
      private->description = (gchar *) _("Folder");
      private->static_desc = TRUE;
      break;

    case PICMAN_THUMB_STATE_SPECIAL:
      private->description = (gchar *) _("Special File");
      private->static_desc = TRUE;
      break;

    case PICMAN_THUMB_STATE_NOT_FOUND:
      private->description =
        (gchar *) g_strerror (thumbnail->image_not_found_errno);
      private->static_desc = TRUE;
      break;

    default:
      {
        GString *str = g_string_new (NULL);

        if (thumbnail->image_state == PICMAN_THUMB_STATE_REMOTE)
          {
            g_string_append (str, _("Remote File"));
          }

        if (thumbnail->image_filesize > 0)
          {
            gchar *size = g_format_size (thumbnail->image_filesize);

            if (str->len > 0)
              g_string_append_c (str, '\n');

            g_string_append (str, size);
            g_free (size);
          }

        switch (thumbnail->thumb_state)
          {
          case PICMAN_THUMB_STATE_NOT_FOUND:
            if (str->len > 0)
              g_string_append_c (str, '\n');
            g_string_append (str, _("Click to create preview"));
            break;

          case PICMAN_THUMB_STATE_EXISTS:
            if (str->len > 0)
              g_string_append_c (str, '\n');
            g_string_append (str, _("Loading preview..."));
            break;

          case PICMAN_THUMB_STATE_OLD:
            if (str->len > 0)
              g_string_append_c (str, '\n');
            g_string_append (str, _("Preview is out of date"));
            break;

          case PICMAN_THUMB_STATE_FAILED:
            if (str->len > 0)
              g_string_append_c (str, '\n');
            g_string_append (str, _("Cannot create preview"));
            break;

          case PICMAN_THUMB_STATE_OK:
            {
              if (thumbnail->image_state == PICMAN_THUMB_STATE_REMOTE)
                {
                  if (str->len > 0)
                    g_string_append_c (str, '\n');

                  g_string_append (str, _("(Preview may be out of date)"));
                }

              if (thumbnail->image_width > 0 && thumbnail->image_height > 0)
                {
                  if (str->len > 0)
                    g_string_append_c (str, '\n');

                  g_string_append_printf (str,
                                          ngettext ("%d × %d pixel",
                                                    "%d × %d pixels",
                                                    thumbnail->image_height),
                                          thumbnail->image_width,
                                          thumbnail->image_height);
                }

              if (thumbnail->image_type)
                {
                  if (str->len > 0)
                    g_string_append_c (str, '\n');

                  g_string_append (str, gettext (thumbnail->image_type));
                }

              if (thumbnail->image_num_layers > 0)
                {
                  if (thumbnail->image_type)
                    g_string_append_len (str, ", ", 2);
                  else if (str->len > 0)
                    g_string_append_c (str, '\n');

                  g_string_append_printf (str,
                                          ngettext ("%d layer",
                                                    "%d layers",
                                                    thumbnail->image_num_layers),
                                          thumbnail->image_num_layers);
                }
            }
            break;

          default:
            break;
          }

        private->description = g_string_free (str, FALSE);
        private->static_desc = FALSE;
      }
    }

  return (const gchar *) private->description;
}

static GdkPixbuf *
picman_imagefile_load_thumb (PicmanImagefile *imagefile,
                           gint           width,
                           gint           height)
{
  PicmanImagefilePrivate *private   = GET_PRIVATE (imagefile);
  PicmanThumbnail        *thumbnail = private->thumbnail;
  GdkPixbuf            *pixbuf    = NULL;
  GError               *error     = NULL;
  gint                  size      = MAX (width, height);
  gint                  pixbuf_width;
  gint                  pixbuf_height;
  gint                  preview_width;
  gint                  preview_height;

  if (picman_thumbnail_peek_thumb (thumbnail, size) < PICMAN_THUMB_STATE_EXISTS)
    return NULL;

  if (thumbnail->image_state == PICMAN_THUMB_STATE_NOT_FOUND)
    return NULL;

  pixbuf = picman_thumbnail_load_thumb (thumbnail, size, &error);

  if (! pixbuf)
    {
      if (error)
        {
          picman_message (private->picman, NULL, PICMAN_MESSAGE_ERROR,
                        _("Could not open thumbnail '%s': %s"),
                        thumbnail->thumb_filename, error->message);
          g_clear_error (&error);
        }

      return NULL;
    }

  pixbuf_width  = gdk_pixbuf_get_width  (pixbuf);
  pixbuf_height = gdk_pixbuf_get_height (pixbuf);

  picman_viewable_calc_preview_size (pixbuf_width,
                                   pixbuf_height,
                                   width,
                                   height,
                                   TRUE, 1.0, 1.0,
                                   &preview_width,
                                   &preview_height,
                                   NULL);

  if (preview_width < pixbuf_width || preview_height < pixbuf_height)
    {
      GdkPixbuf *scaled = gdk_pixbuf_scale_simple (pixbuf,
                                                   preview_width,
                                                   preview_height,
                                                   GDK_INTERP_BILINEAR);
      g_object_unref (pixbuf);
      pixbuf = scaled;

      pixbuf_width  = preview_width;
      pixbuf_height = preview_height;
    }

  if (gdk_pixbuf_get_n_channels (pixbuf) != 3)
    {
      GdkPixbuf *tmp = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8,
                                       pixbuf_width, pixbuf_height);

      gdk_pixbuf_composite_color (pixbuf, tmp,
                                  0, 0, pixbuf_width, pixbuf_height,
                                  0.0, 0.0, 1.0, 1.0,
                                  GDK_INTERP_NEAREST, 255,
                                  0, 0, PICMAN_CHECK_SIZE_SM,
                                  0x66666666, 0x99999999);

      g_object_unref (pixbuf);
      pixbuf = tmp;
    }

  return pixbuf;
}

static gboolean
picman_imagefile_save_thumb (PicmanImagefile  *imagefile,
                           PicmanImage      *image,
                           gint            size,
                           gboolean        replace,
                           GError        **error)
{
  PicmanImagefilePrivate *private   = GET_PRIVATE (imagefile);
  PicmanThumbnail        *thumbnail = private->thumbnail;
  GdkPixbuf            *pixbuf;
  gint                  width, height;
  gboolean              success = FALSE;

  if (size < 1)
    return TRUE;

  if (picman_image_get_width  (image) <= size &&
      picman_image_get_height (image) <= size)
    {
      width  = picman_image_get_width  (image);
      height = picman_image_get_height (image);

      size = MAX (width, height);
    }
  else
    {
      if (picman_image_get_width (image) < picman_image_get_height (image))
        {
          height = size;
          width  = MAX (1, (size * picman_image_get_width (image) /
                            picman_image_get_height (image)));
        }
      else
        {
          width  = size;
          height = MAX (1, (size * picman_image_get_height (image) /
                            picman_image_get_width (image)));
        }
    }

  /*  we need the projection constructed NOW, not some time later  */
  picman_pickable_flush (PICMAN_PICKABLE (picman_image_get_projection (image)));

  pixbuf = picman_viewable_get_new_pixbuf (PICMAN_VIEWABLE (image),
                                         /* random context, unused */
                                         picman_get_user_context (image->picman),
                                         width, height);

  /*  when layer previews are disabled, we won't get a pixbuf  */
  if (! pixbuf)
    return TRUE;

  success = picman_thumbnail_save_thumb (thumbnail,
                                       pixbuf,
                                       "PICMAN " PICMAN_VERSION,
                                       error);

  g_object_unref (pixbuf);

  if (success)
    {
      if (replace)
        picman_thumbnail_delete_others (thumbnail, size);
      else
        picman_thumbnail_delete_failure (thumbnail);

      picman_imagefile_update (imagefile);
    }

  return success;
}

static void
picman_thumbnail_set_info_from_image (PicmanThumbnail *thumbnail,
                                    const gchar   *mime_type,
                                    PicmanImage     *image)
{
  const Babl *format;

  /*  peek the thumbnail to make sure that mtime and filesize are set  */
  picman_thumbnail_peek_image (thumbnail);

  format = picman_image_get_layer_format (image,
                                        picman_image_has_alpha (image));

  g_object_set (thumbnail,
                "image-mimetype",   mime_type,
                "image-width",      picman_image_get_width  (image),
                "image-height",     picman_image_get_height (image),
                "image-type",       picman_babl_get_description (format),
                "image-num-layers", picman_image_get_n_layers (image),
                NULL);
}

/**
 * picman_thumbnail_set_info:
 * @thumbnail: #PicmanThumbnail object
 * @mime_type:  MIME type of the image associated with this thumbnail
 * @width:      width of the image associated with this thumbnail
 * @height:     height of the image associated with this thumbnail
 * @format:     format of the image (or NULL if the type is not known)
 * @num_layers: number of layers in the image
 *              (or -1 if the number of layers is not known)
 *
 * Set information about the image associated with the @thumbnail object.
 */
static void
picman_thumbnail_set_info (PicmanThumbnail *thumbnail,
                         const gchar   *mime_type,
                         gint           width,
                         gint           height,
                         const Babl    *format,
                         gint           num_layers)
{
  /*  peek the thumbnail to make sure that mtime and filesize are set  */
  picman_thumbnail_peek_image (thumbnail);

  g_object_set (thumbnail,
                "image-mimetype", mime_type,
                "image-width",    width,
                "image-height",   height,
                NULL);

  if (format)
    g_object_set (thumbnail,
                  "image-type", picman_babl_get_description (format),
                  NULL);

  if (num_layers != -1)
    g_object_set (thumbnail,
                  "image-num-layers", num_layers,
                  NULL);
}
