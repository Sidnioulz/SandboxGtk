/* -*- Mode: C; c-file-style: "gnu"; tab-width: 8 -*- */
/* LibSandboxUtils - a fork of GTK
 * gtkfilechoosertypepicker.c: File type selector 
 * Copyright (C) 2014, Steve Dodier-Lazaro, <sidnioulz@gmail.com>.
 *
 * Originally a copy of gedit-highlight-mode-selector from Gedit
 * (Copyright (C) 2013 - Ignacio Casal Quinteiro)
 *
 * This library is free software; you can redistribute it and/or
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <string.h>
#include "gtkfilechoosertypepicker.h"
#include "gtkmarshalers.h"
#include "gtktreemodelfilter.h"
#include "gtktreeselection.h"
#include "gtktreestore.h"

enum
{
	COLUMN_FILE_TYPE,
	COLUMN_EXTENSION,
	N_COLUMNS
};

struct _GtkFileChooserTypePickerPrivate
{
	GtkWidget *treeview;
	GtkWidget *entry;
	GtkTreeStore *treestore;
	GtkTreeModelFilter *treemodelfilter;
	GtkTreeSelection *treeview_selection;

	gboolean force_valid_extension;
  gchar   *current_file_type;
  gchar   *current_extension;
  guint    extension_count;
};

/* Signals */
enum
{
	EXTENSION_CHANGED,
	FORCE_VALID_EXTENSION_SET,
	EXTENSION_COUNT_CHANGED,
	N_SIGNALS
};
static guint signals[N_SIGNALS] = { 0 };

/* Properties */
enum
{
  PROP_ZERO,
  PROP_FORCE_VALID_EXTENSION,
  PROP_CURRENT_FILE_TYPE,
  PROP_CURRENT_EXTENSION,
  PROP_EXTENSION_COUNT,
  N_PROPERTIES
};
static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GtkFileChooserTypePicker, gtk_file_chooser_type_picker, GTK_TYPE_GRID)

static void
gtk_file_chooser_type_picker_set_property (GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  GtkFileChooserTypePicker *selector = GTK_FILE_CHOOSER_TYPE_PICKER (object);

  switch (property_id)
  {
    case PROP_FORCE_VALID_EXTENSION:
      gtk_file_chooser_type_picker_set_force_valid_extension (selector, g_value_get_boolean (value));
      break;

    case PROP_CURRENT_FILE_TYPE:
      gtk_file_chooser_type_picker_set_current_file_type (selector, g_value_get_string (value));
      break;

    case PROP_CURRENT_EXTENSION:
      gtk_file_chooser_type_picker_set_current_extension (selector, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
gtk_file_chooser_type_picker_get_property (GObject    *object,
                                           guint       property_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
  GtkFileChooserTypePicker *selector = GTK_FILE_CHOOSER_TYPE_PICKER (object);

  switch (property_id)
  {
    case PROP_FORCE_VALID_EXTENSION:
      g_value_set_boolean (value, 
          gtk_file_chooser_type_picker_get_force_valid_extension (selector));
      break;

    case PROP_CURRENT_FILE_TYPE:
      g_value_take_string (value,
          gtk_file_chooser_type_picker_get_current_file_type (selector));
      break;

    case PROP_CURRENT_EXTENSION:
      g_value_take_string (value,
          gtk_file_chooser_type_picker_get_current_extension (selector));
      break;

    case PROP_EXTENSION_COUNT:
      g_value_set_uint (value,
          gtk_file_chooser_type_picker_get_extension_count (selector));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
gtk_file_chooser_type_picker_dispose (GObject *obj)
{
	g_return_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (obj));

	G_OBJECT_CLASS (gtk_file_chooser_type_picker_parent_class)->dispose (obj);
}

static void
gtk_file_chooser_type_picker_finalize (GObject *obj)
{
	g_return_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (obj));
  GtkFileChooserTypePicker        *selector = GTK_FILE_CHOOSER_TYPE_PICKER (obj);
	GtkFileChooserTypePickerPrivate *priv     = selector->priv;

  if (priv->current_file_type)
  {
    g_free (priv->current_file_type);
    priv->current_file_type = NULL;
  }

  if (priv->current_extension)
  {
    g_free (priv->current_extension);
    priv->current_extension = NULL;
  }

  priv->extension_count = 0;

	G_OBJECT_CLASS (gtk_file_chooser_type_picker_parent_class)->finalize (obj);
}

static void
gtk_file_chooser_type_picker_class_init (GtkFileChooserTypePickerClass *klass)
{
	GtkWidgetClass *widget_class  = GTK_WIDGET_CLASS (klass);
  GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);

	signals[EXTENSION_CHANGED] =
		g_signal_new ("extension-changed",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		              0,
		              NULL, NULL,
		              _gtk_marshal_VOID__STRING_STRING,
		              G_TYPE_NONE,
		              2,
		              G_TYPE_STRING,
		              G_TYPE_STRING);

	signals[FORCE_VALID_EXTENSION_SET] =
		g_signal_new ("force-valid-extension-set",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		              0,
		              NULL, NULL,
		              _gtk_marshal_VOID__BOOLEAN,
		              G_TYPE_NONE,
		              1,
		              G_TYPE_BOOLEAN);

	signals[EXTENSION_COUNT_CHANGED] =
		g_signal_new ("extension-count-changed",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		              0,
		              NULL, NULL,
		              g_cclosure_marshal_VOID__UINT,
		              G_TYPE_NONE,
		              1,
		              G_TYPE_UINT);

  gobject_class->set_property = gtk_file_chooser_type_picker_set_property;
  gobject_class->get_property = gtk_file_chooser_type_picker_get_property;
  gobject_class->dispose = gtk_file_chooser_type_picker_dispose;
  gobject_class->finalize = gtk_file_chooser_type_picker_finalize;

  obj_properties[PROP_FORCE_VALID_EXTENSION] =
    g_param_spec_boolean ("force-valid-extension",
                          "Force Valid Extension",
                          "Sets whether verifying the validity of a given extension will fail if that extension is not defined in the selector",
                          FALSE /* default value */,
                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_CURRENT_FILE_TYPE] =
    g_param_spec_string  ("current-file-type",
                          "File Type of Currently Active Row",
                          "The file type currently active in the selector",
                          NULL /* default value */,
                          G_PARAM_READWRITE);

  obj_properties[PROP_CURRENT_EXTENSION] =
    g_param_spec_string  ("current-extension",
                          "Extension of Currently Active Row",
                          "The extension currently active in the selector",
                          NULL /* default value */,
                          G_PARAM_READWRITE);


  obj_properties[PROP_EXTENSION_COUNT] =
    g_param_spec_uint    ("extension-count",
                          "Number of Extensions Currently Added",
                          "The number of extensions added to this selector",
                          0, G_MAXUINT,
                          0 /* default value */,
                          G_PARAM_READABLE);

  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);

	/* Bind class to template */
	gtk_widget_class_set_template_from_resource (widget_class,
	                                             "/org/mupuf/sandboxutils/ui/gtkfilechoosertypepicker.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GtkFileChooserTypePicker, treeview);
	gtk_widget_class_bind_template_child_private (widget_class, GtkFileChooserTypePicker, entry);
	gtk_widget_class_bind_template_child_private (widget_class, GtkFileChooserTypePicker, treestore);
	gtk_widget_class_bind_template_child_private (widget_class, GtkFileChooserTypePicker, treemodelfilter);
	gtk_widget_class_bind_template_child_private (widget_class, GtkFileChooserTypePicker, treeview_selection);
}

//TODO refactor the "Guess from Extension" line and the setters/getters of current extension and name to graciously handle setting invalid extensions and display Guess from Extension only when force_valid is turned off.
static gboolean
visible_func (GtkTreeModel *model,
              GtkTreeIter  *iter,
              GtkFileChooserTypePicker *selector)
{
	const gchar *entry_text = NULL;
	gchar       *name       = NULL;
	gchar       *ext        = NULL;
	gchar       *name_lower = NULL;
	gchar       *ext_lower  = NULL;
	gchar       *text_lower = NULL;
	gboolean     visible    = FALSE;

	entry_text = gtk_entry_get_text (GTK_ENTRY (selector->priv->entry));
	if (*entry_text == '\0')
		return TRUE;

	text_lower = g_utf8_strdown (entry_text, -1);

	gtk_tree_model_get (model, iter,
	                    COLUMN_FILE_TYPE, &name,
	                    COLUMN_EXTENSION, &ext,
	                    -1);
  if (name)
  {
	  name_lower = g_utf8_strdown (name, -1);
	  
	  if (strstr (name_lower, text_lower) != NULL)
		  visible |= TRUE;

	  g_free (name);
	  g_free (name_lower);
  }

  if (ext)
  {
	  ext_lower = g_utf8_strdown (ext, -1);
	  
	  if (strstr (ext_lower, text_lower) != NULL)
		  visible |= TRUE;

	  g_free (ext);
  	g_free (ext_lower);
  }
  else if (selector->priv->force_valid_extension)
  {
    /* This is the "Guess from extension" line which should be hidden when
     * forcing valid extensions. Doesn't seem to work when no text is typed
     * in the filter entry */
    visible = FALSE;
  }

	g_free (text_lower);

	return visible;
}

static void
on_entry_changed (GtkEntry     *entry,
                  GtkFileChooserTypePicker *selector)
{
	GtkTreeIter iter;

	gtk_tree_model_filter_refilter (selector->priv->treemodelfilter);

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (selector->priv->treemodelfilter), &iter))
	{
		gtk_tree_selection_select_iter (selector->priv->treeview_selection, &iter);
	}
}

static gboolean
move_selection (GtkFileChooserTypePicker *selector,
                gint                      howmany)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	gint *indices;
	gint ret = FALSE;

	if (!gtk_tree_selection_get_selected (selector->priv->treeview_selection, NULL, &iter) &&
	    !gtk_tree_model_get_iter_first (GTK_TREE_MODEL (selector->priv->treemodelfilter), &iter))
	{
		return FALSE;
	}

	path = gtk_tree_model_get_path (GTK_TREE_MODEL (selector->priv->treemodelfilter), &iter);
	indices = gtk_tree_path_get_indices (path);

	if (indices)
	{
		gint num;
		gint idx;
		GtkTreePath *new_path;

		idx = indices[0];
		num = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (selector->priv->treemodelfilter), NULL);

		if ((idx + howmany) < 0)
		{
			idx = 0;
		}
		else if ((idx + howmany) >= num)
		{
			idx = num - 1;
		}
		else
		{
			idx = idx + howmany;
		}

		new_path = gtk_tree_path_new_from_indices (idx, -1);
		gtk_tree_selection_select_path (selector->priv->treeview_selection, new_path);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (selector->priv->treeview),
		                              new_path, NULL, TRUE, 0.5, 0);
		gtk_tree_path_free (new_path);

		ret = TRUE;
	}

	gtk_tree_path_free (path);

	return ret;
}

static gboolean
on_entry_key_press_event (GtkWidget    *entry,
                          GdkEventKey  *event,
                          GtkFileChooserTypePicker *selector)
{
	if (event->keyval == GDK_KEY_Down)
	{
		return move_selection (selector, 1);
	}
	else if (event->keyval == GDK_KEY_Up)
	{
		return move_selection (selector, -1);
	}
	else if (event->keyval == GDK_KEY_Page_Down)
	{
		return move_selection (selector, 5);
	}
	else if (event->keyval == GDK_KEY_Page_Up)
	{
		return move_selection (selector, -5);
	}

	return FALSE;
}

static void
on_row_activated (GtkTreeView  *tree_view,
                  GtkTreePath  *path,
                  GtkTreeViewColumn        *column,
                  GtkFileChooserTypePicker *selector)
{
	g_return_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector));

	GtkFileChooserTypePickerPrivate *priv = selector->priv;
	gchar *type = NULL;
	gchar *ext = NULL;
	GtkTreeIter iter;

  if (path) {
	  if (gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->treemodelfilter), &iter, path))
	  {
		  gtk_tree_model_get (GTK_TREE_MODEL (priv->treemodelfilter), &iter,
		                      COLUMN_FILE_TYPE, &type,
		                      COLUMN_EXTENSION, &ext,
		                      -1);

      if (priv->current_file_type)
        g_free (priv->current_file_type);
      priv->current_file_type = type;

      if (priv->current_extension)
        g_free (priv->current_extension);
      priv->current_extension = ext;
      
      g_signal_emit (G_OBJECT (selector), signals[EXTENSION_CHANGED], 0, type, ext);
	  }
  }
}

static void
activate_store_row (GtkFileChooserTypePicker *selector,
                    GtkTreeIter               iter)
{
	g_return_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector));

	GtkFileChooserTypePickerPrivate *priv = selector->priv;
	gchar                           *type = NULL;
	gchar                           *ext  = NULL;

  gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), &iter,
                      COLUMN_FILE_TYPE, &type,
                      COLUMN_EXTENSION, &ext,
                      -1);

  g_return_if_fail (type || ext);

  if (priv->current_file_type)
    g_free (priv->current_file_type);
  priv->current_file_type = type;

  if (priv->current_extension)
    g_free (priv->current_extension);
  priv->current_extension = ext;
      
  g_signal_emit (G_OBJECT (selector), signals[EXTENSION_CHANGED], 0, type, ext);
}

static const gchar *
get_guess_from_extension_label ()
{
  return _("Guess from extension");
}

static void
add_guess_from_extension_row (GtkTreeStore *store)
{
  GtkTreeIter iter;

  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      COLUMN_FILE_TYPE, get_guess_from_extension_label (),
                      COLUMN_EXTENSION, NULL,
                      -1);
}

static void
gtk_file_chooser_type_picker_init (GtkFileChooserTypePicker *selector)
{
	GtkFileChooserTypePickerPrivate *priv;
	GtkTreeIter                      iter;

	selector->priv = gtk_file_chooser_type_picker_get_instance_private (selector);
	priv = selector->priv;

	gtk_widget_init_template (GTK_WIDGET (selector));

	gtk_tree_model_filter_set_visible_func (priv->treemodelfilter,
	                                        (GtkTreeModelFilterVisibleFunc)visible_func,
	                                        selector,
	                                        NULL);

	g_signal_connect (priv->entry, "changed",
	                  G_CALLBACK (on_entry_changed), selector);
	g_signal_connect (priv->entry, "key-press-event",
	                  G_CALLBACK (on_entry_key_press_event), selector);
	g_signal_connect (priv->treeview, "row-activated",
	                  G_CALLBACK (on_row_activated), selector);

  gtk_tree_view_column_set_title (gtk_tree_view_get_column (GTK_TREE_VIEW (priv->treeview), 0),
                                  _("File Type"));
  gtk_tree_view_column_set_title (gtk_tree_view_get_column (GTK_TREE_VIEW (priv->treeview), 1),
                                  _("Extension"));

	/* Populate tree model */
  add_guess_from_extension_row (priv->treestore);

	/* select first item if any */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (selector->priv->treemodelfilter), &iter))
	{
		gtk_tree_selection_select_iter (selector->priv->treeview_selection, &iter);
	}
}

void
gtk_file_chooser_type_picker_add_extension    (GtkFileChooserTypePicker *selector,
                                               const gchar              *file_type,
                                               const gchar              *extension)
{
	g_return_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector));
	g_return_if_fail (file_type != NULL);
	g_return_if_fail (extension != NULL);

	GtkFileChooserTypePickerPrivate *priv = selector->priv;
	GtkTreeIter                      iter;

  gint n = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (priv->treemodelfilter), NULL);
  gtk_tree_store_insert (priv->treestore, &iter, NULL, n-1);
//	gtk_tree_store_append (priv->treestore, &iter, NULL);

  priv->extension_count++;
	gtk_tree_store_set (priv->treestore, &iter,
	                    COLUMN_FILE_TYPE, file_type,
	                    COLUMN_EXTENSION, extension,
	                    -1);
  g_signal_emit (G_OBJECT (selector), signals[EXTENSION_COUNT_CHANGED], 0, priv->extension_count);
}

static gboolean
_gtk_file_chooser_type_picker_remove_extension (GtkFileChooserTypePicker *selector,
                                                const gchar              *file_type,
                                                const gchar              *extension,
                                                gboolean                  seek_null_ext)
{
	g_return_val_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector), FALSE);
	g_return_val_if_fail (file_type || extension, FALSE);

	GtkFileChooserTypePickerPrivate *priv = selector->priv;
	GtkTreeIter                      iter;

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->treemodelfilter), &iter))
	{
		do
		{
			gchar *iter_type;
			gchar *iter_extension;

			gtk_tree_model_get (GTK_TREE_MODEL (priv->treemodelfilter),
			                    &iter,
			                    COLUMN_FILE_TYPE, &iter_type,
			                    COLUMN_EXTENSION, &iter_extension,
			                    -1);

      gboolean equal = TRUE;
      equal &= !file_type || !g_strcmp0 (file_type, iter_type);
 
      // Internally we sometimes actively want to match the null extension, but
      // the API offers to search only by filetype with a NULL extension param.
      if (!seek_null_ext)
        equal &= !extension || !g_strcmp0 (extension, iter_extension);
      else
        equal &= (!extension && !iter_extension) || !g_strcmp0 (extension, iter_extension);

      g_free (iter_type);
      g_free (iter_extension);

			if (equal)
			{
			  GtkTreeIter storeiter;
			  gtk_tree_model_filter_convert_iter_to_child_iter (priv->treemodelfilter, &storeiter, &iter); 

        gboolean removed = gtk_tree_store_remove (priv->treestore, &storeiter);
        if (removed)
        {
			    priv->extension_count--;
		      g_signal_emit (G_OBJECT (selector), signals[EXTENSION_COUNT_CHANGED], 0, priv->extension_count);
        }
		    return removed;
			}
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->treemodelfilter), &iter));
	}

  return FALSE;
}

gboolean
gtk_file_chooser_type_picker_remove_extension (GtkFileChooserTypePicker *selector,
                                               const gchar              *file_type,
                                               const gchar              *extension)
{
  return _gtk_file_chooser_type_picker_remove_extension (selector, file_type, extension, FALSE);
}

void
gtk_file_chooser_type_picker_clear_extensions (GtkFileChooserTypePicker *selector)
{
	g_return_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector));
	GtkFileChooserTypePickerPrivate *priv = selector->priv;
	gtk_tree_store_clear (priv->treestore);
  priv->extension_count = 0;
  g_signal_emit (G_OBJECT (selector), signals[EXTENSION_COUNT_CHANGED], 0, priv->extension_count);
}

GtkWidget *
gtk_file_chooser_type_picker_new ()
{
	return g_object_new (GTK_TYPE_FILE_CHOOSER_TYPE_PICKER, NULL);
/*
	return g_object_new (GTK_TYPE_FILE_CHOOSER_TYPE_PICKER,
	                     PROP_FORCE_VALID_EXTENSION, FALSE,
	                     PROP_CURRENT_FILE_TYPE, NULL,
	                     PROP_CURRENT_EXTENSION, NULL,
	                     NULL);
*/
}

void
gtk_file_chooser_type_picker_set_force_valid_extension (GtkFileChooserTypePicker *selector,
                                                        gboolean                  force_valid)
{
	GtkFileChooserTypePickerPrivate *priv = selector->priv;

  priv->force_valid_extension = force_valid;
  gtk_tree_model_filter_refilter (priv->treemodelfilter);

  g_signal_emit (G_OBJECT (selector), signals[FORCE_VALID_EXTENSION_SET], 0, force_valid);
}

gboolean
gtk_file_chooser_type_picker_get_force_valid_extension (GtkFileChooserTypePicker *selector)
{
  return selector->priv->force_valid_extension;
}

gboolean
gtk_file_chooser_type_picker_select_extension (GtkFileChooserTypePicker *selector,
                                               const gchar              *extension)
{
	GtkTreeIter iter;

	g_return_val_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector), FALSE);

	if (extension == NULL)
	{
		return FALSE;
	}

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (selector->priv->treemodelfilter), &iter))
	{
		do
		{
			gchar *iter_extension;

			gtk_tree_model_get (GTK_TREE_MODEL (selector->priv->treemodelfilter),
			                    &iter,
			                    COLUMN_EXTENSION, &iter_extension,
			                    -1);

			if (iter_extension != NULL)
			{
				gboolean equal = (g_strcmp0 (iter_extension, extension) == 0);

				g_free (iter_extension);

				if (equal)
				{
					GtkTreePath *path;

					path = gtk_tree_model_get_path (GTK_TREE_MODEL (selector->priv->treemodelfilter), &iter);

					gtk_tree_selection_select_iter (selector->priv->treeview_selection, &iter);
					gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (selector->priv->treeview),
					                              path, NULL, TRUE, 0.5, 0);
					gtk_tree_path_free (path);
					return TRUE;
				}
			}
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (selector->priv->treemodelfilter), &iter));
	}

	return FALSE;
}

gchar *
gtk_file_chooser_type_picker_extract_extension (const gchar *path)
{
  if (path == NULL)
    return NULL;

  const gchar *folder_pos;
  gchar       *last_dot_pos  = NULL;

  folder_pos = g_strrstr (path, G_DIR_SEPARATOR_S);
  if (!folder_pos)
    folder_pos = path;

  last_dot_pos = g_strrstr (folder_pos, ".");

  if (last_dot_pos)
  {
    gboolean hidden = FALSE;

    if (last_dot_pos <= folder_pos + 1)
    {
      hidden = TRUE;
    }

    if (!hidden)
    {
      return g_strdup (last_dot_pos);
    }
  }

  return NULL;
}

gchar *
gtk_file_chooser_type_picker_extract_filename (const gchar *path)
{
  if (path == NULL)
    return NULL;

  const gchar *folder_pos;
  gchar       *last_dot_pos  = NULL;

  folder_pos = g_strrstr (path, G_DIR_SEPARATOR_S);
  if (!folder_pos)
    folder_pos = path;

  last_dot_pos = g_strrstr (folder_pos, ".");

  if (last_dot_pos)
  {
    gboolean hidden = FALSE;

    if (last_dot_pos <= folder_pos + 1)
    {
      hidden = TRUE;
    }

    if (!hidden)
    {
      return g_strndup (path, last_dot_pos - path);
    }
  }

  return g_strdup (path);
}

//TODO document code as much as widget is documented

gboolean
gtk_file_chooser_type_picker_select_extension_from_path (GtkFileChooserTypePicker *selector,
                                                         const gchar              *path,
                                                         const gchar             **return_ext)
{
	g_return_val_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector), FALSE);
	g_return_val_if_fail (path, FALSE);
  if (return_ext)
    g_return_val_if_fail (*return_ext == NULL, FALSE);

	GtkFileChooserTypePickerPrivate *priv           = selector->priv;
  gchar                           *path_ext       = NULL;
  gint                             match_counter  = 0;
  gint                             prefix_counter = 0;
  gboolean                         valid;
  GtkTreeIter                      iter;
  gchar                           *iter_ext       = NULL;
  gchar                           *activate_type  = NULL;
  gchar                           *activate_ext   = NULL;

  path_ext = gtk_file_chooser_type_picker_extract_extension (path);

  valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->treemodelfilter), &iter);
  while (valid)
  {
    gtk_tree_model_get (GTK_TREE_MODEL (priv->treemodelfilter), &iter,
                        COLUMN_EXTENSION, &iter_ext, -1);

    if (path_ext && g_strcmp0 (iter_ext, path_ext) == 0)
    {
      if (match_counter == 0) /* only keep the first one */
      {
        gtk_tree_model_get (GTK_TREE_MODEL (priv->treemodelfilter), &iter,
                      COLUMN_FILE_TYPE, &activate_type,
                      COLUMN_EXTENSION, &activate_ext, -1);
      }

      ++match_counter;
    }
    else if (path_ext && iter_ext) /* g_str_has_prefix complains about NULL */
    {
      if (g_str_has_prefix (iter_ext, path_ext))
        ++prefix_counter;
    }

    if (iter_ext)
      g_free (iter_ext);

    valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->treemodelfilter), &iter);
  }

  if (prefix_counter > 0 && match_counter != 0)
  {
    gtk_entry_set_text (GTK_ENTRY (selector->priv->entry), path_ext);
  }
  else
  {
    gtk_entry_set_text (GTK_ENTRY (selector->priv->entry), "");
  }

  if (match_counter == 0)
  {
    gtk_file_chooser_type_picker_set_current_extension_full (selector, get_guess_from_extension_label (), NULL);
  }
  else
  {
    gtk_file_chooser_type_picker_set_current_extension_full (selector, activate_type, activate_ext);

    if (activate_type)
      g_free (activate_type);
    if (activate_ext)
      g_free (activate_ext);
  }

  if (return_ext)
    *return_ext = path_ext;
  else
    g_free (path_ext);

  return TRUE;
}

void
gtk_file_chooser_type_picker_unselect_extension (GtkFileChooserTypePicker *selector)
{
	g_return_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector));

  gtk_tree_selection_unselect_all (selector->priv->treeview_selection);
}

gchar *
gtk_file_chooser_type_picker_get_selected_extension (GtkFileChooserTypePicker *selector)
{
	g_return_val_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector), NULL);

	GtkFileChooserTypePickerPrivate *priv = selector->priv;
	gchar                           *ext  = NULL;
	GtkTreeIter                      iter;

	if (gtk_tree_selection_get_selected (priv->treeview_selection, NULL, &iter))
	{
		gtk_tree_model_get (GTK_TREE_MODEL (priv->treemodelfilter), &iter,
		                    COLUMN_EXTENSION, &ext,
		                    -1);
	}

  return ext;
}

gboolean
gtk_file_chooser_type_picker_set_current_extension_full (GtkFileChooserTypePicker *selector,
                                                         const gchar              *file_type,
                                                         const gchar              *extension)
{
	GtkTreeIter iter;
	gboolean equal = FALSE;

	g_return_val_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector), FALSE);

	if (!extension && !file_type)
	{
		return FALSE;
	}
	
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (selector->priv->treestore), &iter))
	{
		do
		{
		  gchar *iter_file_type;
			gchar *iter_extension;

			gtk_tree_model_get (GTK_TREE_MODEL (selector->priv->treestore),
			                    &iter,
			                    COLUMN_FILE_TYPE, &iter_file_type,
			                    COLUMN_EXTENSION, &iter_extension,
			                    -1);

			if (iter_extension || iter_file_type)
			{
				equal = !extension || (g_strcmp0 (iter_extension, extension) == 0);
			  equal &= !file_type || (g_strcmp0 (iter_file_type, file_type) == 0);

				if (equal)
				{
          activate_store_row (selector, iter);
				}
			}

			if (iter_extension)
				g_free (iter_extension);

			if (iter_file_type)
				g_free (iter_file_type);
		}
		while (!equal && gtk_tree_model_iter_next (GTK_TREE_MODEL (selector->priv->treestore), &iter));
	}

	return equal;
}

gchar *
gtk_file_chooser_type_picker_get_current_file_type (GtkFileChooserTypePicker *selector)
{
	g_return_val_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector), NULL);

  return g_strdup (selector->priv->current_file_type);
}

guint
gtk_file_chooser_type_picker_get_extension_count (GtkFileChooserTypePicker *selector)
{
	g_return_val_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector), 0);

  return selector->priv->extension_count;
}

gchar *
gtk_file_chooser_type_picker_get_current_extension (GtkFileChooserTypePicker *selector)
{
	g_return_val_if_fail (GTK_IS_FILE_CHOOSER_TYPE_PICKER (selector), NULL);

  return g_strdup (selector->priv->current_extension);
}

gboolean
gtk_file_chooser_type_picker_set_current_extension (GtkFileChooserTypePicker *selector,
                                                    const gchar              *extension)
{
  return gtk_file_chooser_type_picker_set_current_extension_full (selector, NULL, extension);
}

gboolean
gtk_file_chooser_type_picker_set_current_file_type (GtkFileChooserTypePicker *selector,
                                                    const gchar              *file_type)
{
  return gtk_file_chooser_type_picker_set_current_extension_full (selector, file_type, NULL);
}
