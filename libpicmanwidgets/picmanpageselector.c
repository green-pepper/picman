/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpageselector.c
 * Copyright (C) 2005 Michael Natterer <mitch@picman.org>
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

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "picmanwidgetstypes.h"

#include "picmanpageselector.h"
#include "picmanpropwidgets.h"
#include "picmanstock.h"
#include "picmanwidgets.h"
#include "picman3migration.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmanpageselector
 * @title: PicmanPageSelector
 * @short_description: A widget to select pages from multi-page things.
 *
 * Use this for example for specifying what pages to import from
 * a PDF or PS document.
 **/


enum
{
  SELECTION_CHANGED,
  ACTIVATE,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_N_PAGES,
  PROP_TARGET
};

enum
{
  COLUMN_PAGE_NO,
  COLUMN_THUMBNAIL,
  COLUMN_LABEL,
  COLUMN_LABEL_SET
};


typedef struct
{
  gint                    n_pages;
  PicmanPageSelectorTarget  target;

  GtkListStore           *store;
  GtkWidget              *view;

  GtkWidget              *count_label;
  GtkWidget              *range_entry;

  GdkPixbuf              *default_thumbnail;
} PicmanPageSelectorPrivate;

#define PICMAN_PAGE_SELECTOR_GET_PRIVATE(obj) \
  ((PicmanPageSelectorPrivate *) ((PicmanPageSelector *) (obj))->priv)


static void   picman_page_selector_finalize          (GObject          *object);
static void   picman_page_selector_get_property      (GObject          *object,
                                                    guint             property_id,
                                                    GValue           *value,
                                                    GParamSpec       *pspec);
static void   picman_page_selector_set_property      (GObject          *object,
                                                    guint             property_id,
                                                    const GValue     *value,
                                                    GParamSpec       *pspec);

static void   picman_page_selector_selection_changed (GtkIconView      *icon_view,
                                                    PicmanPageSelector *selector);
static void   picman_page_selector_item_activated    (GtkIconView      *icon_view,
                                                    GtkTreePath      *path,
                                                    PicmanPageSelector *selector);
static gboolean picman_page_selector_range_focus_out (GtkEntry         *entry,
                                                    GdkEventFocus    *fevent,
                                                    PicmanPageSelector *selector);
static void   picman_page_selector_range_activate    (GtkEntry         *entry,
                                                    PicmanPageSelector *selector);
static gint   picman_page_selector_int_compare       (gconstpointer     a,
                                                    gconstpointer     b);
static void   picman_page_selector_print_range       (GString          *string,
                                                    gint              start,
                                                    gint              end);

static GdkPixbuf * picman_page_selector_add_frame    (GtkWidget        *widget,
                                                    GdkPixbuf        *pixbuf);


G_DEFINE_TYPE (PicmanPageSelector, picman_page_selector, GTK_TYPE_BOX)

#define parent_class picman_page_selector_parent_class

static guint selector_signals[LAST_SIGNAL] = { 0 };


static void
picman_page_selector_class_init (PicmanPageSelectorClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize     = picman_page_selector_finalize;
  object_class->get_property = picman_page_selector_get_property;
  object_class->set_property = picman_page_selector_set_property;

  klass->selection_changed   = NULL;
  klass->activate            = NULL;

  /**
   * PicmanPageSelector::selection-changed:
   * @widget: the object which received the signal.
   *
   * This signal is emitted whenever the set of selected pages changes.
   *
   * Since: PICMAN 2.4
   **/
  selector_signals[SELECTION_CHANGED] =
    g_signal_new ("selection-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanPageSelectorClass, selection_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * PicmanPageSelector::activate:
   * @widget: the object which received the signal.
   *
   * The "activate" signal on PicmanPageSelector is an action signal. It
   * is emitted when a user double-clicks an item in the page selection.
   *
   * Since: PICMAN 2.4
   */
  selector_signals[ACTIVATE] =
    g_signal_new ("activate",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (PicmanPageSelectorClass, activate),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  widget_class->activate_signal = selector_signals[ACTIVATE];

  /**
   * PicmanPageSelector:n-pages:
   *
   * The number of pages of the document to open.
   *
   * Since: PICMAN 2.4
   **/
  g_object_class_install_property (object_class, PROP_N_PAGES,
                                   g_param_spec_int ("n-pages", NULL, NULL,
                                                     0, G_MAXINT, 0,
                                                     PICMAN_PARAM_READWRITE));

  /**
   * PicmanPageSelector:target:
   *
   * The target to open the document to.
   *
   * Since: PICMAN 2.4
   **/
  g_object_class_install_property (object_class, PROP_TARGET,
                                   g_param_spec_enum ("target", NULL, NULL,
                                                      PICMAN_TYPE_PAGE_SELECTOR_TARGET,
                                                      PICMAN_PAGE_SELECTOR_TARGET_LAYERS,
                                                      PICMAN_PARAM_READWRITE));

  g_type_class_add_private (object_class, sizeof (PicmanPageSelectorPrivate));
}

static void
picman_page_selector_init (PicmanPageSelector *selector)
{
  PicmanPageSelectorPrivate *priv;
  GtkWidget               *vbox;
  GtkWidget               *sw;
  GtkWidget               *hbox;
  GtkWidget               *hbbox;
  GtkWidget               *button;
  GtkWidget               *label;
  GtkWidget               *combo;

  selector->priv = G_TYPE_INSTANCE_GET_PRIVATE (selector,
                                                PICMAN_TYPE_PAGE_SELECTOR,
                                                PicmanPageSelectorPrivate);

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  priv->n_pages = 0;
  priv->target  = PICMAN_PAGE_SELECTOR_TARGET_LAYERS;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (selector),
                                  GTK_ORIENTATION_VERTICAL);

  gtk_box_set_spacing (GTK_BOX (selector), 12);

  /*  Pages  */

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_box_pack_start (GTK_BOX (selector), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
  gtk_widget_show (sw);

  priv->store = gtk_list_store_new (4,
                                    G_TYPE_INT,
                                    GDK_TYPE_PIXBUF,
                                    G_TYPE_STRING,
                                    G_TYPE_BOOLEAN);

  priv->view = gtk_icon_view_new_with_model (GTK_TREE_MODEL (priv->store));
  gtk_icon_view_set_text_column (GTK_ICON_VIEW (priv->view),
                                 COLUMN_LABEL);
  gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (priv->view),
                                   COLUMN_THUMBNAIL);
  gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (priv->view),
                                    GTK_SELECTION_MULTIPLE);
  gtk_container_add (GTK_CONTAINER (sw), priv->view);
  gtk_widget_show (priv->view);

  g_signal_connect (priv->view, "selection-changed",
                    G_CALLBACK (picman_page_selector_selection_changed),
                    selector);
  g_signal_connect (priv->view, "item-activated",
                    G_CALLBACK (picman_page_selector_item_activated),
                    selector);

  /*  Count label  */

  priv->count_label = gtk_label_new (_("Nothing selected"));
  gtk_misc_set_alignment (GTK_MISC (priv->count_label), 0.0, 0.5);
  picman_label_set_attributes (GTK_LABEL (priv->count_label),
                             PANGO_ATTR_STYLE, PANGO_STYLE_ITALIC,
                             -1);
  gtk_box_pack_start (GTK_BOX (vbox), priv->count_label, FALSE, FALSE, 0);
  gtk_widget_show (priv->count_label);

  /*  Select all button & range entry  */

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (selector), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  hbbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (hbox), hbbox, FALSE, FALSE, 0);
  gtk_widget_show (hbbox);

  button = gtk_button_new_with_mnemonic (_("Select _All"));
  gtk_box_pack_start (GTK_BOX (hbbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (picman_page_selector_select_all),
                            selector);

  priv->range_entry = gtk_entry_new ();
  gtk_widget_set_size_request (priv->range_entry, 80, -1);
  gtk_box_pack_end (GTK_BOX (hbox), priv->range_entry, TRUE, TRUE, 0);
  gtk_widget_show (priv->range_entry);

  g_signal_connect (priv->range_entry, "focus-out-event",
                    G_CALLBACK (picman_page_selector_range_focus_out),
                    selector);
  g_signal_connect (priv->range_entry, "activate",
                    G_CALLBACK (picman_page_selector_range_activate),
                    selector);

  label = gtk_label_new_with_mnemonic (_("Select _range:"));
  gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), priv->range_entry);

  /*  Target combo  */

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (selector), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("Open _pages as"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  combo = picman_prop_enum_combo_box_new (G_OBJECT (selector), "target", -1, -1);
  gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);
  gtk_widget_show (combo);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);

  priv->default_thumbnail = gtk_widget_render_icon (GTK_WIDGET (selector),
                                                    GTK_STOCK_FILE,
                                                    GTK_ICON_SIZE_DND,
                                                    NULL);
}

static void
picman_page_selector_finalize (GObject *object)
{
  PicmanPageSelectorPrivate *priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (object);

  if (priv->default_thumbnail)
    g_object_unref (priv->default_thumbnail);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_page_selector_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanPageSelectorPrivate *priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_N_PAGES:
      g_value_set_int (value, priv->n_pages);
      break;
    case PROP_TARGET:
      g_value_set_enum (value, priv->target);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_page_selector_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanPageSelector        *selector = PICMAN_PAGE_SELECTOR (object);
  PicmanPageSelectorPrivate *priv     = PICMAN_PAGE_SELECTOR_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_N_PAGES:
      picman_page_selector_set_n_pages (selector, g_value_get_int (value));
      break;
    case PROP_TARGET:
      priv->target = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/*  public functions  */

/**
 * picman_page_selector_new:
 *
 * Creates a new #PicmanPageSelector widget.
 *
 * Returns: Pointer to the new #PicmanPageSelector widget.
 *
 * Since: PICMAN 2.4
 **/
GtkWidget *
picman_page_selector_new (void)
{
  return g_object_new (PICMAN_TYPE_PAGE_SELECTOR, NULL);
}

/**
 * picman_page_selector_set_n_pages:
 * @selector: Pointer to a #PicmanPageSelector.
 * @n_pages:  The number of pages.
 *
 * Sets the number of pages in the document to open.
 *
 * Since: PICMAN 2.4
 **/
void
picman_page_selector_set_n_pages (PicmanPageSelector *selector,
                                gint              n_pages)
{
  PicmanPageSelectorPrivate *priv;

  g_return_if_fail (PICMAN_IS_PAGE_SELECTOR (selector));
  g_return_if_fail (n_pages >= 0);

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  if (n_pages != priv->n_pages)
    {
      GtkTreeIter iter;
      gint        i;

      if (n_pages < priv->n_pages)
        {
          for (i = n_pages; i < priv->n_pages; i++)
            {
              gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (priv->store),
                                             &iter, NULL, n_pages);
              gtk_list_store_remove (priv->store, &iter);
            }
        }
      else
        {
          for (i = priv->n_pages; i < n_pages; i++)
            {
              gchar *text;

              text = g_strdup_printf (_("Page %d"), i + 1);

              gtk_list_store_append (priv->store, &iter);
              gtk_list_store_set (priv->store, &iter,
                                  COLUMN_PAGE_NO,   i,
                                  COLUMN_THUMBNAIL, priv->default_thumbnail,
                                  COLUMN_LABEL,     text,
                                  COLUMN_LABEL_SET, FALSE,
                                  -1);

              g_free (text);
            }
        }

      priv->n_pages = n_pages;

      g_object_notify (G_OBJECT (selector), "n-pages");
    }
}

/**
 * picman_page_selector_get_n_pages:
 * @selector: Pointer to a #PicmanPageSelector.
 *
 * Returns: the number of pages in the document to open.
 *
 * Since: PICMAN 2.4
 **/
gint
picman_page_selector_get_n_pages (PicmanPageSelector *selector)
{
  PicmanPageSelectorPrivate *priv;

  g_return_val_if_fail (PICMAN_IS_PAGE_SELECTOR (selector), 0);

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  return priv->n_pages;
}

/**
 * picman_page_selector_set_target:
 * @selector: Pointer to a #PicmanPageSelector.
 * @target:   How to open the selected pages.
 *
 * Since: PICMAN 2.4
 **/
void
picman_page_selector_set_target (PicmanPageSelector       *selector,
                               PicmanPageSelectorTarget  target)
{
  PicmanPageSelectorPrivate *priv;

  g_return_if_fail (PICMAN_IS_PAGE_SELECTOR (selector));
  g_return_if_fail (target <= PICMAN_PAGE_SELECTOR_TARGET_IMAGES);

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  if (target != priv->target)
    {
      priv->target = target;

      g_object_notify (G_OBJECT (selector), "target");
    }
}

/**
 * picman_page_selector_get_target:
 * @selector: Pointer to a #PicmanPageSelector.
 *
 * Returns: How the selected pages should be opened.
 *
 * Since: PICMAN 2.4
 **/
PicmanPageSelectorTarget
picman_page_selector_get_target (PicmanPageSelector *selector)
{
  PicmanPageSelectorPrivate *priv;

  g_return_val_if_fail (PICMAN_IS_PAGE_SELECTOR (selector),
                        PICMAN_PAGE_SELECTOR_TARGET_LAYERS);

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  return priv->target;
}

/**
 * picman_page_selector_set_page_thumbnail:
 * @selector: Pointer to a #PicmanPageSelector.
 * @page_no: The number of the page to set the thumbnail for.
 * @thumbnail: The thumbnail pixbuf.
 *
 * Sets the thumbnail for given @page_no. A default "page" icon will
 * be used if no page thumbnail is set.
 *
 * Since: PICMAN 2.4
 **/
void
picman_page_selector_set_page_thumbnail (PicmanPageSelector *selector,
                                       gint              page_no,
                                       GdkPixbuf        *thumbnail)
{
  PicmanPageSelectorPrivate *priv;
  GtkTreeIter              iter;

  g_return_if_fail (PICMAN_IS_PAGE_SELECTOR (selector));
  g_return_if_fail (thumbnail == NULL || GDK_IS_PIXBUF (thumbnail));

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  g_return_if_fail (page_no >= 0 && page_no < priv->n_pages);

  gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (priv->store),
                                 &iter, NULL, page_no);

  if (! thumbnail)
    {
      thumbnail = g_object_ref (priv->default_thumbnail);
    }
  else
    {
      thumbnail = picman_page_selector_add_frame (GTK_WIDGET (selector),
                                                thumbnail);
    }

  gtk_list_store_set (priv->store, &iter,
                      COLUMN_THUMBNAIL, thumbnail,
                      -1);
  g_object_unref (thumbnail);
}

/**
 * picman_page_selector_get_page_thumbnail:
 * @selector: Pointer to a #PicmanPageSelector.
 * @page_no: The number of the page to get the thumbnail for.
 *
 * Returns: The page's thumbnail, or %NULL if none is set. The returned
 *          pixbuf is owned by #PicmanPageSelector and must not be
 *          unref'ed when no longer needed.
 *
 * Since: PICMAN 2.4
 **/
GdkPixbuf *
picman_page_selector_get_page_thumbnail (PicmanPageSelector *selector,
                                       gint              page_no)
{
  PicmanPageSelectorPrivate *priv;
  GdkPixbuf               *thumbnail;
  GtkTreeIter              iter;

  g_return_val_if_fail (PICMAN_IS_PAGE_SELECTOR (selector), NULL);

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  g_return_val_if_fail (page_no >= 0 && page_no < priv->n_pages, NULL);

  gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (priv->store),
                                 &iter, NULL, page_no);
  gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter,
                      COLUMN_THUMBNAIL, &thumbnail,
                      -1);

  if (thumbnail)
    g_object_unref (thumbnail);

  if (thumbnail == priv->default_thumbnail)
    return NULL;

  return thumbnail;
}

/**
 * picman_page_selector_set_page_label:
 * @selector: Pointer to a #PicmanPageSelector.
 * @page_no:  The number of the page to set the label for.
 * @label:    The label.
 *
 * Sets the label of the specified page.
 *
 * Since: PICMAN 2.4
 **/
void
picman_page_selector_set_page_label (PicmanPageSelector *selector,
                                   gint              page_no,
                                   const gchar      *label)
{
  PicmanPageSelectorPrivate *priv;
  GtkTreeIter              iter;
  gchar                   *tmp;

  g_return_if_fail (PICMAN_IS_PAGE_SELECTOR (selector));

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  g_return_if_fail (page_no >= 0 && page_no < priv->n_pages);

  if (! label)
    tmp = g_strdup_printf (_("Page %d"), page_no + 1);
  else
    tmp = (gchar *) label;

  gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (priv->store),
                                 &iter, NULL, page_no);
  gtk_list_store_set (priv->store, &iter,
                      COLUMN_LABEL,     tmp,
                      COLUMN_LABEL_SET, label != NULL,
                      -1);

  if (! label)
    g_free (tmp);
}

/**
 * picman_page_selector_get_page_label:
 * @selector: Pointer to a #PicmanPageSelector.
 * @page_no: The number of the page to get the thumbnail for.
 *
 * Returns: The page's label, or %NULL if none is set. This is a newly
 *          allocated string that should be g_free()'d when no longer
 *          needed.
 *
 * Since: PICMAN 2.4
 **/
gchar *
picman_page_selector_get_page_label (PicmanPageSelector *selector,
                                   gint              page_no)
{
  PicmanPageSelectorPrivate *priv;
  GtkTreeIter              iter;
  gchar                   *label;
  gboolean                 label_set;

  g_return_val_if_fail (PICMAN_IS_PAGE_SELECTOR (selector), NULL);

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  g_return_val_if_fail (page_no >= 0 && page_no < priv->n_pages, NULL);

  gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (priv->store),
                                 &iter, NULL, page_no);
  gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter,
                      COLUMN_LABEL,     &label,
                      COLUMN_LABEL_SET, &label_set,
                      -1);

  if (! label_set)
    {
      g_free (label);
      label = NULL;
    }

  return label;
}

/**
 * picman_page_selector_select_all:
 * @selector: Pointer to a #PicmanPageSelector.
 *
 * Selects all pages.
 *
 * Since: PICMAN 2.4
 **/
void
picman_page_selector_select_all (PicmanPageSelector *selector)
{
  PicmanPageSelectorPrivate *priv;

  g_return_if_fail (PICMAN_IS_PAGE_SELECTOR (selector));

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  gtk_icon_view_select_all (GTK_ICON_VIEW (priv->view));
}

/**
 * picman_page_selector_unselect_all:
 * @selector: Pointer to a #PicmanPageSelector.
 *
 * Unselects all pages.
 *
 * Since: PICMAN 2.4
 **/
void
picman_page_selector_unselect_all (PicmanPageSelector *selector)
{
  PicmanPageSelectorPrivate *priv;

  g_return_if_fail (PICMAN_IS_PAGE_SELECTOR (selector));

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  gtk_icon_view_unselect_all (GTK_ICON_VIEW (priv->view));
}

/**
 * picman_page_selector_select_page:
 * @selector: Pointer to a #PicmanPageSelector.
 * @page_no: The number of the page to select.
 *
 * Adds a page to the selection.
 *
 * Since: PICMAN 2.4
 **/
void
picman_page_selector_select_page (PicmanPageSelector *selector,
                                gint              page_no)
{
  PicmanPageSelectorPrivate *priv;
  GtkTreeIter              iter;
  GtkTreePath             *path;

  g_return_if_fail (PICMAN_IS_PAGE_SELECTOR (selector));

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  g_return_if_fail (page_no >= 0 && page_no < priv->n_pages);

  gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (priv->store),
                                 &iter, NULL, page_no);
  path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->store), &iter);

  gtk_icon_view_select_path (GTK_ICON_VIEW (priv->view), path);

  gtk_tree_path_free (path);
}

/**
 * picman_page_selector_unselect_page:
 * @selector: Pointer to a #PicmanPageSelector.
 * @page_no: The number of the page to unselect.
 *
 * Removes a page from the selection.
 *
 * Since: PICMAN 2.4
 **/
void
picman_page_selector_unselect_page (PicmanPageSelector *selector,
                                  gint              page_no)
{
  PicmanPageSelectorPrivate *priv;
  GtkTreeIter              iter;
  GtkTreePath             *path;

  g_return_if_fail (PICMAN_IS_PAGE_SELECTOR (selector));

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  g_return_if_fail (page_no >= 0 && page_no < priv->n_pages);

  gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (priv->store),
                                 &iter, NULL, page_no);
  path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->store), &iter);

  gtk_icon_view_unselect_path (GTK_ICON_VIEW (priv->view), path);

  gtk_tree_path_free (path);
}

/**
 * picman_page_selector_page_is_selected:
 * @selector: Pointer to a #PicmanPageSelector.
 * @page_no: The number of the page to check.
 *
 * Returns: %TRUE if the page is selected, %FALSE otherwise.
 *
 * Since: PICMAN 2.4
 **/
gboolean
picman_page_selector_page_is_selected (PicmanPageSelector *selector,
                                     gint              page_no)
{
  PicmanPageSelectorPrivate *priv;
  GtkTreeIter              iter;
  GtkTreePath             *path;
  gboolean                 selected;

  g_return_val_if_fail (PICMAN_IS_PAGE_SELECTOR (selector), FALSE);

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  g_return_val_if_fail (page_no >= 0 && page_no < priv->n_pages, FALSE);

  gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (priv->store),
                                 &iter, NULL, page_no);
  path = gtk_tree_model_get_path (GTK_TREE_MODEL (priv->store), &iter);

  selected = gtk_icon_view_path_is_selected (GTK_ICON_VIEW (priv->view),
                                             path);

  gtk_tree_path_free (path);

  return selected;
}

/**
 * picman_page_selector_get_selected_pages:
 * @selector: Pointer to a #PicmanPageSelector.
 * @n_selected_pages: Returns the number of selected pages.
 *
 * Returns: A sorted array of page numbers of selected pages. Use g_free() if
 *          you don't need the array any longer.
 *
 * Since: PICMAN 2.4
 **/
gint *
picman_page_selector_get_selected_pages (PicmanPageSelector *selector,
                                       gint             *n_selected_pages)
{
  PicmanPageSelectorPrivate *priv;
  GList                   *selected;
  GList                   *list;
  gint                    *array;
  gint                     i;

  g_return_val_if_fail (PICMAN_IS_PAGE_SELECTOR (selector), NULL);
  g_return_val_if_fail (n_selected_pages != NULL, NULL);

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  selected = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (priv->view));

  *n_selected_pages = g_list_length (selected);
  array = g_new0 (gint, *n_selected_pages);

  for (list = selected, i = 0; list; list = g_list_next (list), i++)
    {
      gint *indices = gtk_tree_path_get_indices (list->data);

      array[i] = indices[0];
    }

  qsort (array, *n_selected_pages, sizeof (gint),
         picman_page_selector_int_compare);

  g_list_free_full (selected, (GDestroyNotify) gtk_tree_path_free);

  return array;
}

/**
 * picman_page_selector_select_range:
 * @selector: Pointer to a #PicmanPageSelector.
 * @range: A string representing the set of selected pages.
 *
 * Selectes the pages described by @range. The range string is a
 * user-editable list of pages and ranges, e.g. "1,3,5-7,9-12,14".
 * Note that the page numbering in the range string starts with 1,
 * not 0.
 *
 * Invalid pages and ranges will be silently ignored, duplicate and
 * overlapping pages and ranges will be merged.
 *
 * Since: PICMAN 2.4
 **/
void
picman_page_selector_select_range (PicmanPageSelector *selector,
                                 const gchar      *range)
{
  PicmanPageSelectorPrivate  *priv;
  gchar                   **ranges;

  g_return_if_fail (PICMAN_IS_PAGE_SELECTOR (selector));

  priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);

  if (! range)
    range = "";

  g_signal_handlers_block_by_func (priv->view,
                                   picman_page_selector_selection_changed,
                                   selector);

  picman_page_selector_unselect_all (selector);

  ranges = g_strsplit (range, ",", -1);

  if (ranges)
    {
      gint i;

      for (i = 0; ranges[i] != NULL; i++)
        {
          gchar *range = g_strstrip (ranges[i]);
          gchar *dash;

          dash = strchr (range, '-');

          if (dash)
            {
              gchar *from;
              gchar *to;
              gint   page_from = -1;
              gint   page_to   = -1;

              *dash = '\0';

              from = g_strstrip (range);
              to   = g_strstrip (dash + 1);

              if (sscanf (from, "%i", &page_from) != 1 && strlen (from) == 0)
                page_from = 1;

              if (sscanf (to, "%i", &page_to) != 1 && strlen (to) == 0)
                page_to = priv->n_pages;

              if (page_from > 0        &&
                  page_to   > 0        &&
                  page_from <= page_to &&
                  page_from <= priv->n_pages)
                {
                  gint page_no;

                  page_from = MAX (page_from, 1) - 1;
                  page_to   = MIN (page_to, priv->n_pages) - 1;

                  for (page_no = page_from; page_no <= page_to; page_no++)
                    picman_page_selector_select_page (selector, page_no);
                }
            }
          else
            {
              gint page_no;

              if (sscanf (range, "%i", &page_no) == 1 &&
                  page_no >= 1                        &&
                  page_no <= priv->n_pages)
                {
                  picman_page_selector_select_page (selector, page_no - 1);
                }
            }
        }

      g_strfreev (ranges);
    }

  g_signal_handlers_unblock_by_func (priv->view,
                                     picman_page_selector_selection_changed,
                                     selector);

  picman_page_selector_selection_changed (GTK_ICON_VIEW (priv->view), selector);
}

/**
 * picman_page_selector_get_selected_range:
 * @selector: Pointer to a #PicmanPageSelector.
 *
 * Returns: A newly allocated string representing the set of selected
 *          pages. See picman_page_selector_select_range() for the
 *          format of the string.
 *
 * Since: PICMAN 2.4
 **/
gchar *
picman_page_selector_get_selected_range (PicmanPageSelector *selector)
{
  gint    *pages;
  gint     n_pages;
  GString *string;

  g_return_val_if_fail (PICMAN_IS_PAGE_SELECTOR (selector), NULL);

  string = g_string_new ("");

  pages = picman_page_selector_get_selected_pages (selector, &n_pages);

  if (pages)
    {
      gint range_start, range_end;
      gint last_printed;
      gint i;

      range_start  = pages[0];
      range_end    = pages[0];
      last_printed = -1;

      for (i = 1; i < n_pages; i++)
        {
          if (pages[i] > range_end + 1)
            {
              picman_page_selector_print_range (string,
                                              range_start, range_end);

              last_printed = range_end;
              range_start = pages[i];
            }

          range_end = pages[i];
        }

      if (range_end != last_printed)
        picman_page_selector_print_range (string, range_start, range_end);

      g_free (pages);
    }

  return g_string_free (string, FALSE);
}


/*  private functions  */

static void
picman_page_selector_selection_changed (GtkIconView      *icon_view,
                                      PicmanPageSelector *selector)
{
  PicmanPageSelectorPrivate *priv = PICMAN_PAGE_SELECTOR_GET_PRIVATE (selector);
  GList                   *selected;
  gint                     n_selected;
  gchar                   *range;

  selected = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (priv->view));
  n_selected = g_list_length (selected);
  g_list_free_full (selected, (GDestroyNotify) gtk_tree_path_free);

  if (n_selected == 0)
    {
      gtk_label_set_text (GTK_LABEL (priv->count_label),
                          _("Nothing selected"));
    }
  else if (n_selected == 1)
    {
      gtk_label_set_text (GTK_LABEL (priv->count_label),
                          _("One page selected"));
    }
  else
    {
      gchar *text;

      if (n_selected == priv->n_pages)
        text = g_strdup_printf (ngettext ("%d page selected",
                                          "All %d pages selected", n_selected),
                                n_selected);
      else
        text = g_strdup_printf (ngettext ("%d page selected",
                                          "%d pages selected",
                                          n_selected),
                                n_selected);

      gtk_label_set_text (GTK_LABEL (priv->count_label), text);
      g_free (text);
    }

  range = picman_page_selector_get_selected_range (selector);
  gtk_entry_set_text (GTK_ENTRY (priv->range_entry), range);
  g_free (range);

  gtk_editable_set_position (GTK_EDITABLE (priv->range_entry), -1);

  g_signal_emit (selector, selector_signals[SELECTION_CHANGED], 0);
}

static void
picman_page_selector_item_activated (GtkIconView      *icon_view,
                                   GtkTreePath      *path,
                                   PicmanPageSelector *selector)
{
  g_signal_emit (selector, selector_signals[ACTIVATE], 0);
}

static gboolean
picman_page_selector_range_focus_out (GtkEntry         *entry,
                                    GdkEventFocus    *fevent,
                                    PicmanPageSelector *selector)
{
  picman_page_selector_range_activate (entry, selector);

  return FALSE;
}

static void
picman_page_selector_range_activate (GtkEntry         *entry,
                                   PicmanPageSelector *selector)
{
  picman_page_selector_select_range (selector, gtk_entry_get_text (entry));
}

static gint
picman_page_selector_int_compare (gconstpointer a,
                                gconstpointer b)
{
  return *(gint*)a - *(gint*)b;
}

static void
picman_page_selector_print_range (GString *string,
                                gint     start,
                                gint     end)
{
  if (string->len != 0)
    g_string_append_c (string, ',');

  if (start == end)
    g_string_append_printf (string, "%d", start + 1);
  else
    g_string_append_printf (string, "%d-%d", start + 1, end + 1);
}

static void
draw_frame_row (GdkPixbuf *frame_image,
                gint       target_width,
                gint       source_width,
                gint       source_v_position,
                gint       dest_v_position,
                GdkPixbuf *result_pixbuf,
                gint       left_offset,
                gint       height)
{
  gint remaining_width = target_width;
  gint h_offset        = 0;

  while (remaining_width > 0)
    {
      gint slab_width = (remaining_width > source_width ?
                         source_width : remaining_width);

      gdk_pixbuf_copy_area (frame_image,
                            left_offset, source_v_position,
                            slab_width, height,
                            result_pixbuf,
                            left_offset + h_offset, dest_v_position);

      remaining_width -= slab_width;
      h_offset += slab_width;
    }
}

/* utility to draw the middle section of the frame in a loop */
static void
draw_frame_column (GdkPixbuf *frame_image,
                   gint       target_height,
                   gint       source_height,
                   gint       source_h_position,
                   gint       dest_h_position,
                   GdkPixbuf *result_pixbuf,
                   gint       top_offset, int width)
{
  gint remaining_height = target_height;
  gint v_offset         = 0;

  while (remaining_height > 0)
    {
      gint slab_height = (remaining_height > source_height ?
                          source_height : remaining_height);

      gdk_pixbuf_copy_area (frame_image,
                            source_h_position, top_offset,
                            width, slab_height,
                            result_pixbuf,
                            dest_h_position, top_offset + v_offset);

      remaining_height -= slab_height;
      v_offset += slab_height;
    }
}

static GdkPixbuf *
stretch_frame_image (GdkPixbuf *frame_image,
                     gint       left_offset,
                     gint       top_offset,
                     gint       right_offset,
                     gint       bottom_offset,
                     gint       dest_width,
                     gint       dest_height)
{
  GdkPixbuf *pixbuf;
  gint       frame_width, frame_height;
  gint       target_width,  target_frame_width;
  gint       target_height, target_frame_height;

  frame_width  = gdk_pixbuf_get_width  (frame_image);
  frame_height = gdk_pixbuf_get_height (frame_image );

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
                           dest_width, dest_height);
  gdk_pixbuf_fill (pixbuf, 0);

  target_width = dest_width - left_offset - right_offset;
  target_height = dest_height - top_offset - bottom_offset;
  target_frame_width  = frame_width - left_offset - right_offset;
  target_frame_height = frame_height - top_offset - bottom_offset;

  left_offset   += MIN (target_width / 4, target_frame_width / 4);
  right_offset  += MIN (target_width / 4, target_frame_width / 4);
  top_offset    += MIN (target_height / 4, target_frame_height / 4);
  bottom_offset += MIN (target_height / 4, target_frame_height / 4);

  target_width = dest_width - left_offset - right_offset;
  target_height = dest_height - top_offset - bottom_offset;
  target_frame_width  = frame_width - left_offset - right_offset;
  target_frame_height = frame_height - top_offset - bottom_offset;

  /* draw the left top corner  and top row */
  gdk_pixbuf_copy_area (frame_image,
                        0, 0, left_offset, top_offset,
                        pixbuf, 0,  0);
  draw_frame_row (frame_image, target_width, target_frame_width,
                  0, 0,
                  pixbuf,
                  left_offset, top_offset);

  /* draw the right top corner and left column */
  gdk_pixbuf_copy_area (frame_image,
                        frame_width - right_offset, 0,
                        right_offset, top_offset,

                        pixbuf,
                        dest_width - right_offset,  0);
  draw_frame_column (frame_image, target_height, target_frame_height, 0, 0,
                     pixbuf, top_offset, left_offset);

  /* draw the bottom right corner and bottom row */
  gdk_pixbuf_copy_area (frame_image,
                        frame_width - right_offset, frame_height - bottom_offset,
                        right_offset, bottom_offset,
                        pixbuf,
                        dest_width - right_offset, dest_height - bottom_offset);
  draw_frame_row (frame_image, target_width, target_frame_width,
                  frame_height - bottom_offset, dest_height - bottom_offset,
                  pixbuf, left_offset, bottom_offset);

  /* draw the bottom left corner and the right column */
  gdk_pixbuf_copy_area (frame_image,
                        0, frame_height - bottom_offset,
                        left_offset, bottom_offset,
                        pixbuf,
                        0,  dest_height - bottom_offset);
  draw_frame_column (frame_image, target_height, target_frame_height,
                     frame_width - right_offset, dest_width - right_offset,
                     pixbuf, top_offset, right_offset);

  return pixbuf;
}

#define FRAME_LEFT   2
#define FRAME_TOP    2
#define FRAME_RIGHT  4
#define FRAME_BOTTOM 4

static GdkPixbuf *
picman_page_selector_add_frame (GtkWidget *widget,
                              GdkPixbuf *pixbuf)
{
  GdkPixbuf *frame;
  gint       width, height;

  width  = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);

  frame = g_object_get_data (G_OBJECT (widget), "frame");

  if (! frame)
    {
      frame = gtk_widget_render_icon (widget,
                                      PICMAN_STOCK_FRAME,
                                      GTK_ICON_SIZE_DIALOG, NULL);
      g_object_set_data_full (G_OBJECT (widget), "frame", frame,
                              (GDestroyNotify) g_object_unref);
    }

  frame = stretch_frame_image (frame,
                               FRAME_LEFT,
                               FRAME_TOP,
                               FRAME_RIGHT,
                               FRAME_BOTTOM,
                               width  + FRAME_LEFT + FRAME_RIGHT,
                               height + FRAME_TOP  + FRAME_BOTTOM);

  gdk_pixbuf_copy_area (pixbuf, 0, 0, width, height,
                        frame, FRAME_LEFT, FRAME_TOP);

  return frame;
}
