/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpaletteselectbutton.c
 * Copyright (C) 2004  Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "picman.h"

#include "picmanuitypes.h"
#include "picmanpaletteselectbutton.h"
#include "picmanuimarshal.h"

#include "libpicman-intl.h"


/**
 * SECTION: picmanpaletteselectbutton
 * @title: PicmanPaletteSelect
 * @short_description: A button which pops up a palette select dialog.
 *
 * A button which pops up a palette select dialog.
 **/


#define PICMAN_PALETTE_SELECT_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), PICMAN_TYPE_PALETTE_SELECT_BUTTON, PicmanPaletteSelectButtonPrivate))

typedef struct _PicmanPaletteSelectButtonPrivate PicmanPaletteSelectButtonPrivate;

struct _PicmanPaletteSelectButtonPrivate
{
  gchar     *title;

  gchar     *palette_name;      /* Local copy */

  GtkWidget *inside;
  GtkWidget *label;
};

enum
{
  PALETTE_SET,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_TITLE,
  PROP_PALETTE_NAME
};


/*  local function prototypes  */

static void   picman_palette_select_button_finalize     (GObject      *object);

static void   picman_palette_select_button_set_property (GObject      *object,
                                                       guint         property_id,
                                                       const GValue *value,
                                                       GParamSpec   *pspec);
static void   picman_palette_select_button_get_property (GObject      *object,
                                                       guint         property_id,
                                                       GValue       *value,
                                                       GParamSpec   *pspec);

static void   picman_palette_select_button_clicked  (PicmanPaletteSelectButton *button);

static void   picman_palette_select_button_callback (const gchar *palette_name,
                                                   gboolean     dialog_closing,
                                                   gpointer     user_data);

static void   picman_palette_select_drag_data_received (PicmanPaletteSelectButton *button,
                                                      GdkDragContext          *context,
                                                      gint                     x,
                                                      gint                     y,
                                                      GtkSelectionData        *selection,
                                                      guint                    info,
                                                      guint                    time);

static GtkWidget * picman_palette_select_button_create_inside (PicmanPaletteSelectButton *palette_button);


static const GtkTargetEntry target = { "application/x-picman-palette-name", 0 };

static guint palette_button_signals[LAST_SIGNAL] = { 0 };


G_DEFINE_TYPE (PicmanPaletteSelectButton, picman_palette_select_button,
               PICMAN_TYPE_SELECT_BUTTON)


static void
picman_palette_select_button_class_init (PicmanPaletteSelectButtonClass *klass)
{
  GObjectClass          *object_class        = G_OBJECT_CLASS (klass);
  PicmanSelectButtonClass *select_button_class = PICMAN_SELECT_BUTTON_CLASS (klass);

  object_class->finalize     = picman_palette_select_button_finalize;
  object_class->set_property = picman_palette_select_button_set_property;
  object_class->get_property = picman_palette_select_button_get_property;

  select_button_class->select_destroy = picman_palette_select_destroy;

  klass->palette_set = NULL;

  /**
   * PicmanPaletteSelectButton:title:
   *
   * The title to be used for the palette selection popup dialog.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_TITLE,
                                   g_param_spec_string ("title",
                                                        "Title",
                                                        "The title to be used for the palette selection popup dialog",
                                                        _("Palette Selection"),
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  /**
   * PicmanPaletteSelectButton:palette-name:
   *
   * The name of the currently selected palette.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_PALETTE_NAME,
                                   g_param_spec_string ("palette-name",
                                                        "Palette name",
                                                        "The name of the currently selected palette",
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE));

  /**
   * PicmanPaletteSelectButton::palette-set:
   * @widget: the object which received the signal.
   * @palette_name: the name of the currently selected palette.
   * @dialog_closing: whether the dialog was closed or not.
   *
   * The ::palette-set signal is emitted when the user selects a palette.
   *
   * Since: PICMAN 2.4
   */
  palette_button_signals[PALETTE_SET] =
    g_signal_new ("palette-set",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanPaletteSelectButtonClass, palette_set),
                  NULL, NULL,
                  _picmanui_marshal_VOID__STRING_BOOLEAN,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  G_TYPE_BOOLEAN);

  g_type_class_add_private (object_class,
                            sizeof (PicmanPaletteSelectButtonPrivate));
}

static void
picman_palette_select_button_init (PicmanPaletteSelectButton *button)
{
  PicmanPaletteSelectButtonPrivate *priv;

  priv = PICMAN_PALETTE_SELECT_BUTTON_GET_PRIVATE (button);

  priv->palette_name = NULL;

  priv->inside = picman_palette_select_button_create_inside (button);
  gtk_container_add (GTK_CONTAINER (button), priv->inside);
}

/**
 * picman_palette_select_button_new:
 * @title:        Title of the dialog to use or %NULL to use the default title.
 * @palette_name: Initial palette name.
 *
 * Creates a new #GtkWidget that completely controls the selection of
 * a palette.  This widget is suitable for placement in a table in a
 * plug-in dialog.
 *
 * Returns: A #GtkWidget that you can use in your UI.
 *
 * Since: PICMAN 2.4
 */
GtkWidget *
picman_palette_select_button_new (const gchar *title,
                                const gchar *palette_name)
{
  GtkWidget *button;

  if (title)
    button = g_object_new (PICMAN_TYPE_PALETTE_SELECT_BUTTON,
                           "title",        title,
                           "palette-name", palette_name,
                           NULL);
  else
    button = g_object_new (PICMAN_TYPE_PALETTE_SELECT_BUTTON,
                           "palette-name", palette_name,
                           NULL);

  return button;
}

/**
 * picman_palette_select_button_get_palette:
 * @button: A #PicmanPaletteSelectButton
 *
 * Retrieves the name of currently selected palette.
 *
 * Returns: an internal copy of the palette name which must not be freed.
 *
 * Since: PICMAN 2.4
 */
const gchar *
picman_palette_select_button_get_palette (PicmanPaletteSelectButton *button)
{
  PicmanPaletteSelectButtonPrivate *priv;

  g_return_val_if_fail (PICMAN_IS_PALETTE_SELECT_BUTTON (button), NULL);

  priv = PICMAN_PALETTE_SELECT_BUTTON_GET_PRIVATE (button);
  return priv->palette_name;
}

/**
 * picman_palette_select_button_set_palette:
 * @button: A #PicmanPaletteSelectButton
 * @palette_name: Palette name to set; %NULL means no change.
 *
 * Sets the current palette for the palette select button.
 *
 * Since: PICMAN 2.4
 */
void
picman_palette_select_button_set_palette (PicmanPaletteSelectButton *button,
                                        const gchar             *palette_name)
{
  PicmanSelectButton *select_button;

  g_return_if_fail (PICMAN_IS_PALETTE_SELECT_BUTTON (button));

  select_button = PICMAN_SELECT_BUTTON (button);

  if (select_button->temp_callback)
    {
      picman_palettes_set_popup (select_button->temp_callback, palette_name);
    }
  else
    {
      gchar *name;
      gint   num_colors;

      if (palette_name && *palette_name)
        name = g_strdup (palette_name);
      else
        name = picman_context_get_palette ();

      if (picman_palette_get_info (name, &num_colors))
        picman_palette_select_button_callback (name, FALSE, button);

      g_free (name);
    }
}


/*  private functions  */

static void
picman_palette_select_button_finalize (GObject *object)
{
  PicmanPaletteSelectButtonPrivate *priv;

  priv = PICMAN_PALETTE_SELECT_BUTTON_GET_PRIVATE (object);

  g_free (priv->palette_name);
  priv->palette_name = NULL;

  g_free (priv->title);
  priv->title = NULL;

  G_OBJECT_CLASS (picman_palette_select_button_parent_class)->finalize (object);
}

static void
picman_palette_select_button_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  PicmanPaletteSelectButton        *button;
  PicmanPaletteSelectButtonPrivate *priv;

  button = PICMAN_PALETTE_SELECT_BUTTON (object);
  priv = PICMAN_PALETTE_SELECT_BUTTON_GET_PRIVATE (button);

  switch (property_id)
    {
    case PROP_TITLE:
      priv->title = g_value_dup_string (value);
      break;
    case PROP_PALETTE_NAME:
      picman_palette_select_button_set_palette (button,
                                              g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_palette_select_button_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  PicmanPaletteSelectButton        *button;
  PicmanPaletteSelectButtonPrivate *priv;

  button = PICMAN_PALETTE_SELECT_BUTTON (object);
  priv = PICMAN_PALETTE_SELECT_BUTTON_GET_PRIVATE (button);

  switch (property_id)
    {
    case PROP_TITLE:
      g_value_set_string (value, priv->title);
      break;
    case PROP_PALETTE_NAME:
      g_value_set_string (value, priv->palette_name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_palette_select_button_callback (const gchar *palette_name,
                                     gboolean     dialog_closing,
                                     gpointer     user_data)
{
  PicmanPaletteSelectButton        *button;
  PicmanPaletteSelectButtonPrivate *priv;
  PicmanSelectButton               *select_button;

  button = PICMAN_PALETTE_SELECT_BUTTON (user_data);

  priv = PICMAN_PALETTE_SELECT_BUTTON_GET_PRIVATE (button);
  select_button = PICMAN_SELECT_BUTTON (button);

  g_free (priv->palette_name);
  priv->palette_name = g_strdup (palette_name);

  gtk_label_set_text (GTK_LABEL (priv->label), palette_name);

  if (dialog_closing)
    select_button->temp_callback = NULL;

  g_signal_emit (button, palette_button_signals[PALETTE_SET], 0,
                 palette_name, dialog_closing);
  g_object_notify (G_OBJECT (button), "palette-name");
}

static void
picman_palette_select_button_clicked (PicmanPaletteSelectButton *button)
{
  PicmanPaletteSelectButtonPrivate *priv;
  PicmanSelectButton               *select_button;

  priv = PICMAN_PALETTE_SELECT_BUTTON_GET_PRIVATE (button);
  select_button = PICMAN_SELECT_BUTTON (button);

  if (select_button->temp_callback)
    {
      /*  calling picman_palettes_set_popup() raises the dialog  */
      picman_palettes_set_popup (select_button->temp_callback,
                               priv->palette_name);
    }
  else
    {
      select_button->temp_callback =
        picman_palette_select_new (priv->title, priv->palette_name,
                                 picman_palette_select_button_callback,
                                 button);
    }
}

static void
picman_palette_select_drag_data_received (PicmanPaletteSelectButton *button,
                                        GdkDragContext          *context,
                                        gint                     x,
                                        gint                     y,
                                        GtkSelectionData        *selection,
                                        guint                    info,
                                        guint                    time)
{
  gint   length = gtk_selection_data_get_length (selection);
  gchar *str;

  if (gtk_selection_data_get_format (selection) != 8 || length < 1)
    {
      g_warning ("%s: received invalid palette data", G_STRFUNC);
      return;
    }

  str = g_strndup ((const gchar *) gtk_selection_data_get_data (selection),
                   length);

  if (g_utf8_validate (str, -1, NULL))
    {
      gint     pid;
      gpointer unused;
      gint     name_offset = 0;

      if (sscanf (str, "%i:%p:%n", &pid, &unused, &name_offset) >= 2 &&
          pid == picman_getpid () && name_offset > 0)
        {
          gchar *name = str + name_offset;

          picman_palette_select_button_set_palette (button, name);
        }
    }

  g_free (str);
}

static GtkWidget *
picman_palette_select_button_create_inside (PicmanPaletteSelectButton *palette_button)
{
  GtkWidget                      *button;
  GtkWidget                      *hbox;
  GtkWidget                      *image;
  PicmanPaletteSelectButtonPrivate *priv;

  priv = PICMAN_PALETTE_SELECT_BUTTON_GET_PRIVATE (palette_button);

  gtk_widget_push_composite_child ();

  button = gtk_button_new ();

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_container_add (GTK_CONTAINER (button), hbox);

  image = gtk_image_new_from_stock (PICMAN_STOCK_PALETTE, GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  priv->label = gtk_label_new (priv->palette_name);
  gtk_box_pack_start (GTK_BOX (hbox), priv->label, TRUE, TRUE, 4);

  gtk_widget_show_all (button);

  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (picman_palette_select_button_clicked),
                            palette_button);

  gtk_drag_dest_set (GTK_WIDGET (button),
                     GTK_DEST_DEFAULT_HIGHLIGHT |
                     GTK_DEST_DEFAULT_MOTION |
                     GTK_DEST_DEFAULT_DROP,
                     &target, 1,
                     GDK_ACTION_COPY);

  g_signal_connect_swapped (button, "drag-data-received",
                            G_CALLBACK (picman_palette_select_drag_data_received),
                            palette_button);

  gtk_widget_pop_composite_child ();

  return button;
}
