/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandata.c
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gegl.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"

#ifdef G_OS_WIN32
#include "libpicmanbase/picmanwin32-io.h"
#endif

#include "core-types.h"

#include "picman-utils.h"
#include "picmandata.h"
#include "picmanmarshal.h"
#include "picmantag.h"
#include "picmantagged.h"

#include "picman-intl.h"


enum
{
  DIRTY,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_FILENAME,
  PROP_WRITABLE,
  PROP_DELETABLE,
  PROP_MIME_TYPE
};


typedef struct _PicmanDataPrivate PicmanDataPrivate;

struct _PicmanDataPrivate
{
  gchar  *filename;
  GQuark  mime_type;
  guint   writable  : 1;
  guint   deletable : 1;
  guint   dirty     : 1;
  guint   internal  : 1;
  gint    freeze_count;
  time_t  mtime;

  /* Identifies the PicmanData object across sessions. Used when there
   * is not a filename associated with the object.
   */
  gchar  *identifier;

  GList  *tags;
};

#define PICMAN_DATA_GET_PRIVATE(data) \
        G_TYPE_INSTANCE_GET_PRIVATE (data, PICMAN_TYPE_DATA, PicmanDataPrivate)


static void      picman_data_class_init        (PicmanDataClass       *klass);
static void      picman_data_tagged_iface_init (PicmanTaggedInterface *iface);

static void      picman_data_init              (PicmanData            *data,
                                              PicmanDataClass       *data_class);

static void      picman_data_constructed       (GObject             *object);
static void      picman_data_finalize          (GObject             *object);
static void      picman_data_set_property      (GObject             *object,
                                              guint                property_id,
                                              const GValue        *value,
                                              GParamSpec          *pspec);
static void      picman_data_get_property      (GObject             *object,
                                              guint                property_id,
                                              GValue              *value,
                                              GParamSpec          *pspec);

static gint64    picman_data_get_memsize       (PicmanObject          *object,
                                              gint64              *gui_size);

static void      picman_data_real_dirty        (PicmanData            *data);

static gboolean  picman_data_add_tag           (PicmanTagged          *tagged,
                                              PicmanTag             *tag);
static gboolean  picman_data_remove_tag        (PicmanTagged          *tagged,
                                              PicmanTag             *tag);
static GList *   picman_data_get_tags          (PicmanTagged          *tagged);
static gchar *   picman_data_get_identifier    (PicmanTagged          *tagged);
static gchar *   picman_data_get_checksum      (PicmanTagged          *tagged);


static guint data_signals[LAST_SIGNAL] = { 0 };

static PicmanViewableClass *parent_class = NULL;


GType
picman_data_get_type (void)
{
  static GType data_type = 0;

  if (! data_type)
    {
      const GTypeInfo data_info =
      {
        sizeof (PicmanDataClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) picman_data_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (PicmanData),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) picman_data_init,
      };

      const GInterfaceInfo tagged_info =
      {
        (GInterfaceInitFunc) picman_data_tagged_iface_init,
        NULL,           /* interface_finalize */
        NULL            /* interface_data     */
      };

      data_type = g_type_register_static (PICMAN_TYPE_VIEWABLE,
                                          "PicmanData",
                                          &data_info, 0);

      g_type_add_interface_static (data_type, PICMAN_TYPE_TAGGED, &tagged_info);
    }

  return data_type;
}

static void
picman_data_class_init (PicmanDataClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  data_signals[DIRTY] =
    g_signal_new ("dirty",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDataClass, dirty),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->constructed       = picman_data_constructed;
  object_class->finalize          = picman_data_finalize;
  object_class->set_property      = picman_data_set_property;
  object_class->get_property      = picman_data_get_property;

  picman_object_class->get_memsize  = picman_data_get_memsize;

  klass->dirty                    = picman_data_real_dirty;
  klass->save                     = NULL;
  klass->get_extension            = NULL;
  klass->duplicate                = NULL;

  g_object_class_install_property (object_class, PROP_FILENAME,
                                   g_param_spec_string ("filename", NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_WRITABLE,
                                   g_param_spec_boolean ("writable", NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_DELETABLE,
                                   g_param_spec_boolean ("deletable", NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_MIME_TYPE,
                                   g_param_spec_string ("mime-type", NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (PicmanDataPrivate));
}

static void
picman_data_tagged_iface_init (PicmanTaggedInterface *iface)
{
  iface->add_tag        = picman_data_add_tag;
  iface->remove_tag     = picman_data_remove_tag;
  iface->get_tags       = picman_data_get_tags;
  iface->get_identifier = picman_data_get_identifier;
  iface->get_checksum   = picman_data_get_checksum;
}

static void
picman_data_init (PicmanData      *data,
                PicmanDataClass *data_class)
{
  PicmanDataPrivate *private = PICMAN_DATA_GET_PRIVATE (data);

  private->writable  = TRUE;
  private->deletable = TRUE;
  private->dirty     = TRUE;

  /*  look at the passed class pointer, not at PICMAN_DATA_GET_CLASS(data)
   *  here, because the latter is always PicmanDataClass itself
   */
  if (! data_class->save)
    private->writable = FALSE;

  /*  freeze the data object during construction  */
  picman_data_freeze (data);
}

static void
picman_data_constructed (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_data_thaw (PICMAN_DATA (object));
}

static void
picman_data_finalize (GObject *object)
{
  PicmanDataPrivate *private = PICMAN_DATA_GET_PRIVATE (object);

  if (private->filename)
    {
      g_free (private->filename);
      private->filename = NULL;
    }

  if (private->tags)
    {
      g_list_free_full (private->tags, (GDestroyNotify) g_object_unref);
      private->tags = NULL;
    }

  if (private->identifier)
    {
      g_free (private->identifier);
      private->identifier = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_data_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  PicmanData        *data    = PICMAN_DATA (object);
  PicmanDataPrivate *private = PICMAN_DATA_GET_PRIVATE (data);

  switch (property_id)
    {
    case PROP_FILENAME:
      picman_data_set_filename (data,
                              g_value_get_string (value),
                              private->writable,
                              private->deletable);
      break;

    case PROP_WRITABLE:
      private->writable = g_value_get_boolean (value);
      break;

    case PROP_DELETABLE:
      private->deletable = g_value_get_boolean (value);
      break;

    case PROP_MIME_TYPE:
      if (g_value_get_string (value))
        private->mime_type = g_quark_from_string (g_value_get_string (value));
      else
        private->mime_type = 0;
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_data_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  PicmanDataPrivate *private = PICMAN_DATA_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_FILENAME:
      g_value_set_string (value, private->filename);
      break;

    case PROP_WRITABLE:
      g_value_set_boolean (value, private->writable);
      break;

    case PROP_DELETABLE:
      g_value_set_boolean (value, private->deletable);
      break;

    case PROP_MIME_TYPE:
      g_value_set_string (value, g_quark_to_string (private->mime_type));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_data_get_memsize (PicmanObject *object,
                       gint64     *gui_size)
{
  PicmanDataPrivate *private = PICMAN_DATA_GET_PRIVATE (object);
  gint64           memsize = 0;

  memsize += picman_string_get_memsize (private->filename);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_data_real_dirty (PicmanData *data)
{
  PicmanDataPrivate *private = PICMAN_DATA_GET_PRIVATE (data);

  private->dirty = TRUE;

  picman_viewable_invalidate_preview (PICMAN_VIEWABLE (data));

  /* Emit the "name-changed" to signal general dirtiness */
  picman_object_name_changed (PICMAN_OBJECT (data));
}

static gboolean
picman_data_add_tag (PicmanTagged *tagged,
                   PicmanTag    *tag)
{
  PicmanDataPrivate *private = PICMAN_DATA_GET_PRIVATE (tagged);
  GList           *list;

  for (list = private->tags; list; list = g_list_next (list))
    {
      PicmanTag *this = PICMAN_TAG (list->data);

      if (picman_tag_equals (tag, this))
        return FALSE;
    }

  private->tags = g_list_prepend (private->tags, g_object_ref (tag));

  return TRUE;
}

static gboolean
picman_data_remove_tag (PicmanTagged *tagged,
                      PicmanTag    *tag)
{
  PicmanDataPrivate *private = PICMAN_DATA_GET_PRIVATE (tagged);
  GList           *list;

  for (list = private->tags; list; list = g_list_next (list))
    {
      PicmanTag *this = PICMAN_TAG (list->data);

      if (picman_tag_equals (tag, this))
        {
          private->tags = g_list_delete_link (private->tags, list);
          g_object_unref (tag);
          return TRUE;
        }
    }

  return FALSE;
}

static GList *
picman_data_get_tags (PicmanTagged *tagged)
{
  PicmanDataPrivate *private = PICMAN_DATA_GET_PRIVATE (tagged);

  return private->tags;
}

static gchar *
picman_data_get_identifier (PicmanTagged *tagged)
{
  PicmanDataPrivate *private    = PICMAN_DATA_GET_PRIVATE (tagged);
  gchar           *identifier = NULL;

  if (private->filename)
    {
      const gchar *data_dir = picman_data_directory ();
      const gchar *picman_dir = picman_directory ();
      gchar       *tmp;

      if (g_str_has_prefix (private->filename, data_dir))
        {
          tmp = g_strconcat ("${picman_data_dir}",
                             private->filename + strlen (data_dir),
                             NULL);
          identifier = g_filename_to_utf8 (tmp, -1, NULL, NULL, NULL);
          g_free (tmp);
        }
      else if (g_str_has_prefix (private->filename, picman_dir))
        {
          tmp = g_strconcat ("${picman_dir}",
                             private->filename + strlen (picman_dir),
                             NULL);
          identifier = g_filename_to_utf8 (tmp, -1, NULL, NULL, NULL);
          g_free (tmp);
        }
      else
        {
          identifier = g_filename_to_utf8 (private->filename, -1,
                                           NULL, NULL, NULL);
        }

      if (! identifier)
        {
          g_warning ("Failed to convert '%s' to utf8.\n", private->filename);
          identifier = g_strdup (private->filename);
        }
    }
  else if (private->internal)
    {
      identifier = g_strdup (private->identifier);
    }

  return identifier;
}

static gchar *
picman_data_get_checksum (PicmanTagged *tagged)
{
  return NULL;
}

/**
 * picman_data_save:
 * @data:  object whose contents are to be saved.
 * @error: return location for errors or %NULL
 *
 * Save the object.  If the object is marked as "internal", nothing happens.
 * Otherwise, it is saved to disk, using the file name set by
 * picman_data_set_filename().  If the save is successful, the
 * object is marked as not dirty.  If not, an error message is returned
 * using the @error argument.
 *
 * Returns: %TRUE if the object is internal or the save is successful.
 **/
gboolean
picman_data_save (PicmanData  *data,
                GError   **error)
{
  PicmanDataPrivate *private;
  gboolean         success = FALSE;

  g_return_val_if_fail (PICMAN_IS_DATA (data), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  private = PICMAN_DATA_GET_PRIVATE (data);

  g_return_val_if_fail (private->writable == TRUE, FALSE);

  if (private->internal)
    {
      private->dirty = FALSE;
      return TRUE;
    }

  g_return_val_if_fail (private->filename != NULL, FALSE);

  if (PICMAN_DATA_GET_CLASS (data)->save)
    success = PICMAN_DATA_GET_CLASS (data)->save (data, error);

  if (success)
    {
      struct stat filestat;

      g_stat (private->filename, &filestat);

      private->mtime = filestat.st_mtime;
      private->dirty = FALSE;
    }

  return success;
}

/**
 * picman_data_dirty:
 * @data: a #PicmanData object.
 *
 * Marks @data as dirty.  Unless the object is frozen, this causes
 * its preview to be invalidated, and emits a "dirty" signal.  If the
 * object is frozen, the function has no effect.
 **/
void
picman_data_dirty (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_if_fail (PICMAN_IS_DATA (data));

  private = PICMAN_DATA_GET_PRIVATE (data);

  if (private->freeze_count == 0)
    g_signal_emit (data, data_signals[DIRTY], 0);
}

void
picman_data_clean (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_if_fail (PICMAN_IS_DATA (data));

  private = PICMAN_DATA_GET_PRIVATE (data);

  private->dirty = FALSE;
}

gboolean
picman_data_is_dirty (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DATA (data), FALSE);

  private = PICMAN_DATA_GET_PRIVATE (data);

  return private->dirty;
}

/**
 * picman_data_freeze:
 * @data: a #PicmanData object.
 *
 * Increments the freeze count for the object.  A positive freeze count
 * prevents the object from being treated as dirty.  Any call to this
 * function must be followed eventually by a call to picman_data_thaw().
 **/
void
picman_data_freeze (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_if_fail (PICMAN_IS_DATA (data));

  private = PICMAN_DATA_GET_PRIVATE (data);

  private->freeze_count++;
}

/**
 * picman_data_thaw:
 * @data: a #PicmanData object.
 *
 * Decrements the freeze count for the object.  If the freeze count
 * drops to zero, the object is marked as dirty, and the "dirty"
 * signal is emitted.  It is an error to call this function without
 * having previously called picman_data_freeze().
 **/
void
picman_data_thaw (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_if_fail (PICMAN_IS_DATA (data));

  private = PICMAN_DATA_GET_PRIVATE (data);

  g_return_if_fail (private->freeze_count > 0);

  private->freeze_count--;

  if (private->freeze_count == 0)
    picman_data_dirty (data);
}

gboolean
picman_data_is_frozen (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DATA (data), FALSE);

  private = PICMAN_DATA_GET_PRIVATE (data);

  return private->freeze_count > 0;
}

/**
 * picman_data_delete_from_disk:
 * @data:  a #PicmanData object.
 * @error: return location for errors or %NULL
 *
 * Deletes the object from disk.  If the object is marked as "internal",
 * nothing happens.  Otherwise, if the file exists whose name has been
 * set by picman_data_set_filename(), it is deleted.  Obviously this is
 * a potentially dangerous function, which should be used with care.
 *
 * Returns: %TRUE if the object is internal to Picman, or the deletion is
 *          successful.
 **/
gboolean
picman_data_delete_from_disk (PicmanData  *data,
                            GError   **error)
{
  PicmanDataPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DATA (data), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  private = PICMAN_DATA_GET_PRIVATE (data);

  g_return_val_if_fail (private->filename != NULL, FALSE);
  g_return_val_if_fail (private->deletable == TRUE, FALSE);

  if (private->internal)
    return TRUE;

  if (g_unlink (private->filename) == -1)
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_DELETE,
                   _("Could not delete '%s': %s"),
                   picman_filename_to_utf8 (private->filename),
                   g_strerror (errno));
      return FALSE;
    }

  return TRUE;
}

const gchar *
picman_data_get_extension (PicmanData *data)
{
  g_return_val_if_fail (PICMAN_IS_DATA (data), NULL);

  if (PICMAN_DATA_GET_CLASS (data)->get_extension)
    return PICMAN_DATA_GET_CLASS (data)->get_extension (data);

  return NULL;
}

/**
 * picman_data_set_filename:
 * @data:     A #PicmanData object
 * @filename: File name to assign to @data.
 * @writable: %TRUE if we want to be able to write to this file.
 * @deletable: %TRUE if we want to be able to delete this file.
 *
 * This function assigns a file name to @data, and sets some flags
 * according to the properties of the file.  If @writable is %TRUE,
 * and the user has permission to write or overwrite the requested file
 * name, and a "save" method exists for @data's object type, then
 * @data is marked as writable.
 **/
void
picman_data_set_filename (PicmanData    *data,
                        const gchar *filename,
                        gboolean     writable,
                        gboolean     deletable)
{
  PicmanDataPrivate *private;

  g_return_if_fail (PICMAN_IS_DATA (data));
  g_return_if_fail (filename != NULL);
  g_return_if_fail (g_path_is_absolute (filename));

  private = PICMAN_DATA_GET_PRIVATE (data);

  if (private->internal)
    return;

  if (private->filename)
    g_free (private->filename);

  private->filename  = g_strdup (filename);
  private->writable  = FALSE;
  private->deletable = FALSE;

  /*  if the data is supposed to be writable or deletable,
   *  still check if it really is
   */
  if (writable || deletable)
    {
      gchar *dirname = g_path_get_dirname (filename);

      if ((g_access (filename, F_OK) == 0 &&  /* check if the file exists    */
           g_access (filename, W_OK) == 0) || /* and is writable             */
          (g_access (filename, F_OK) != 0 &&  /* OR doesn't exist            */
           g_access (dirname,  W_OK) == 0))   /* and we can write to its dir */
        {
          private->writable  = writable  ? TRUE : FALSE;
          private->deletable = deletable ? TRUE : FALSE;
        }

      g_free (dirname);

      /*  if we can't save, we are not writable  */
      if (! PICMAN_DATA_GET_CLASS (data)->save)
        private->writable = FALSE;
    }
}

/**
 * picman_data_create_filename:
 * @data:     a #Picmandata object.
 * @dest_dir: directory in which to create a file name.
 *
 * This function creates a unique file name to be used for saving
 * a representation of @data in the directory @dest_dir.  If the
 * user does not have write permission in @dest_dir, then @data
 * is marked as "not writable", so you should check on this before
 * assuming that @data can be saved.
 **/
void
picman_data_create_filename (PicmanData    *data,
                           const gchar *dest_dir)
{
  PicmanDataPrivate *private;
  gchar           *safename;
  gchar           *filename;
  gchar           *fullpath;
  gint             i;
  gint             unum  = 1;
  GError          *error = NULL;

  g_return_if_fail (PICMAN_IS_DATA (data));
  g_return_if_fail (dest_dir != NULL);
  g_return_if_fail (g_path_is_absolute (dest_dir));

  private = PICMAN_DATA_GET_PRIVATE (data);

  if (private->internal)
    return;

  safename = g_filename_from_utf8 (picman_object_get_name (data),
                                   -1, NULL, NULL, &error);
  if (! safename)
    {
      g_warning ("picman_data_create_filename:\n"
                 "g_filename_from_utf8() failed for '%s': %s",
                 picman_object_get_name (data), error->message);
      g_error_free (error);
      return;
    }

  g_strstrip (safename);

  if (safename[0] == '.')
    safename[0] = '-';

  for (i = 0; safename[i]; i++)
    if (strchr ("\\/*?\"`'<>{}|\n\t ;:$^&", safename[i]))
      safename[i] = '-';

  filename = g_strconcat (safename, picman_data_get_extension (data), NULL);

  fullpath = g_build_filename (dest_dir, filename, NULL);

  g_free (filename);

  while (g_file_test (fullpath, G_FILE_TEST_EXISTS))
    {
      g_free (fullpath);

      filename = g_strdup_printf ("%s-%d%s",
                                  safename,
                                  unum++,
                                  picman_data_get_extension (data));

      fullpath = g_build_filename (dest_dir, filename, NULL);

      g_free (filename);
    }

  g_free (safename);

  picman_data_set_filename (data, fullpath, TRUE, TRUE);

  g_free (fullpath);
}

const gchar *
picman_data_get_filename (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DATA (data), NULL);

  private = PICMAN_DATA_GET_PRIVATE (data);

  return private->filename;
}

static const gchar *tag_blacklist[] = { "brushes",
                                        "dynamics",
                                        "patterns",
                                        "palettes",
                                        "gradients",
                                        "tool-presets" };

/**
 * picman_data_set_folder_tags:
 * @data:          a #Picmandata object.
 * @top_directory: the top directory of the currently processed data
 *                 hierarchy, or %NULL if that top directory is
 *                 currently processed itself
 *
 * Sets tags based on all folder names below top_directory. So if the
 * data's filename is /home/foo/.picman/brushes/Flowers/Roses/rose.gbr,
 * it will add "Flowers" and "Roses" tags.
 *
 * if the top directory (as passed, or as derived from the data's
 * filename) does not end with one of the default data directory names
 * (brushes, patterns etc), its name will be added as tag too.
 **/
void
picman_data_set_folder_tags (PicmanData    *data,
                           const gchar *top_directory)
{
  PicmanDataPrivate *private;
  gchar           *dirname;

  g_return_if_fail (PICMAN_IS_DATA (data));

  private = PICMAN_DATA_GET_PRIVATE (data);

  if (private->internal)
    return;

  g_return_if_fail (private->filename != NULL);

  dirname = g_path_get_dirname (private->filename);

  /*  if this data is in a subfolder, walk up the hierarchy and
   *  set each folder on the way as tag, except the top_directory
   */
  if (top_directory)
    {
      size_t top_directory_len = strlen (top_directory);

      g_return_if_fail (g_str_has_prefix (dirname, top_directory) &&
                        (dirname[top_directory_len] == '\0' ||
                         G_IS_DIR_SEPARATOR (dirname[top_directory_len])));

      do
        {
          gchar   *basename = g_path_get_basename (dirname);
          PicmanTag *tag      = picman_tag_new (basename);
          gchar   *tmp;

          picman_tag_set_internal (tag, TRUE);
          picman_tagged_add_tag (PICMAN_TAGGED (data), tag);
          g_object_unref (tag);
          g_free (basename);

          tmp = g_path_get_dirname (dirname);
          g_free (dirname);
          dirname = tmp;
        }
      while (strcmp (dirname, top_directory));
    }

  if (dirname)
    {
      gchar *basename = g_path_get_basename (dirname);
      gint   i;

      for (i = 0; i <  G_N_ELEMENTS (tag_blacklist); i++)
        {
          if (! strcmp (basename, tag_blacklist[i]))
            break;
        }

      if (i == G_N_ELEMENTS (tag_blacklist))
        {
          PicmanTag *tag = picman_tag_new (basename);

          picman_tag_set_internal (tag, TRUE);
          picman_tagged_add_tag (PICMAN_TAGGED (data), tag);
          g_object_unref (tag);
        }

      g_free (basename);
      g_free (dirname);
    }
}

const gchar *
picman_data_get_mime_type (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DATA (data), NULL);

  private = PICMAN_DATA_GET_PRIVATE (data);

  return g_quark_to_string (private->mime_type);
}

gboolean
picman_data_is_writable (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DATA (data), FALSE);

  private = PICMAN_DATA_GET_PRIVATE (data);

  return private->writable;
}

gboolean
picman_data_is_deletable (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DATA (data), FALSE);

  private = PICMAN_DATA_GET_PRIVATE (data);

  return private->deletable;
}

void
picman_data_set_mtime (PicmanData *data,
                     time_t    mtime)
{
  PicmanDataPrivate *private;

  g_return_if_fail (PICMAN_IS_DATA (data));

  private = PICMAN_DATA_GET_PRIVATE (data);

  private->mtime = mtime;
}

time_t
picman_data_get_mtime (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DATA (data), 0);

  private = PICMAN_DATA_GET_PRIVATE (data);

  return private->mtime;
}

/**
 * picman_data_duplicate:
 * @data: a #PicmanData object
 *
 * Creates a copy of @data, if possible.  Only the object data is
 * copied:  the newly created object is not automatically given an
 * object name, file name, preview, etc.
 *
 * Returns: the newly created copy, or %NULL if @data cannot be copied.
 **/
PicmanData *
picman_data_duplicate (PicmanData *data)
{
  g_return_val_if_fail (PICMAN_IS_DATA (data), NULL);

  if (PICMAN_DATA_GET_CLASS (data)->duplicate)
    {
      PicmanData        *new     = PICMAN_DATA_GET_CLASS (data)->duplicate (data);
      PicmanDataPrivate *private = PICMAN_DATA_GET_PRIVATE (new);

      g_object_set (new,
                    "name",      NULL,
                    "writable",  PICMAN_DATA_GET_CLASS (new)->save != NULL,
                    "deletable", TRUE,
                    NULL);

      if (private->filename)
        {
          g_free (private->filename);
          private->filename = NULL;
        }

      return new;
    }

  return NULL;
}

/**
 * picman_data_make_internal:
 * @data: a #PicmanData object.
 *
 * Mark @data as "internal" to Picman, which means that it will not be
 * saved to disk.  Note that if you do this, later calls to
 * picman_data_save() and picman_data_delete_from_disk() will
 * automatically return successfully without giving any warning.
 *
 * The identifier name shall be an untranslated globally unique string
 * that identifies the internal object across sessions.
 **/
void
picman_data_make_internal (PicmanData    *data,
                         const gchar *identifier)
{
  PicmanDataPrivate *private;

  g_return_if_fail (PICMAN_IS_DATA (data));

  private = PICMAN_DATA_GET_PRIVATE (data);

  if (private->filename)
    {
      g_free (private->filename);
      private->filename = NULL;
    }

  private->identifier = g_strdup (identifier);
  private->writable   = FALSE;
  private->deletable  = FALSE;
  private->internal   = TRUE;
}

gboolean
picman_data_is_internal (PicmanData *data)
{
  PicmanDataPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DATA (data), FALSE);

  private = PICMAN_DATA_GET_PRIVATE (data);

  return private->internal;
}

/**
 * picman_data_compare:
 * @data1: a #PicmanData object.
 * @data2: another #PicmanData object.
 *
 * Compares two data objects for use in sorting. Objects marked as
 * "internal" come first, then user-writable objects, then system data
 * files. In these three groups, the objects are sorted alphabetically
 * by name, using picman_object_name_collate().
 *
 * Return value: -1 if @data1 compares before @data2,
 *                0 if they compare equal,
 *                1 if @data1 compares after @data2.
 **/
gint
picman_data_compare (PicmanData *data1,
		   PicmanData *data2)
{
  PicmanDataPrivate *private1 = PICMAN_DATA_GET_PRIVATE (data1);
  PicmanDataPrivate *private2 = PICMAN_DATA_GET_PRIVATE (data2);

  /*  move the internal objects (like the FG -> BG) gradient) to the top  */
  if (private1->internal != private2->internal)
    return private1->internal ? -1 : 1;

  /*  keep user-deletable objects above system resource files  */
  if (private1->deletable != private2->deletable)
    return private1->deletable ? -1 : 1;

  return picman_object_name_collate ((PicmanObject *) data1,
                                   (PicmanObject *) data2);
}

/**
 * picman_data_error_quark:
 *
 * This function is used to implement the PICMAN_DATA_ERROR macro. It
 * shouldn't be called directly.
 *
 * Return value: the #GQuark to identify error in the PicmanData error domain.
 **/
GQuark
picman_data_error_quark (void)
{
  return g_quark_from_static_string ("picman-data-error-quark");
}
