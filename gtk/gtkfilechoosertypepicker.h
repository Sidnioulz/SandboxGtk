/* -*- Mode: C; h-file-style: "gnu"; tab-width: 8 -*- */
/* LibSandboxUtils - a fork of GTK
 * gtkfilechoosertypepicker.h: File type selector
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

#ifndef __GTK_FILE_CHOOSER_TYPE_PICKER_H__
#define __GTK_FILE_CHOOSER_TYPE_PICKER_H__

#include <glib-object.h>
#include "gtkgrid.h"

G_BEGIN_DECLS

#define GTK_TYPE_FILE_CHOOSER_TYPE_PICKER		(gtk_file_chooser_type_picker_get_type ())
#define GTK_FILE_CHOOSER_TYPE_PICKER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_FILE_CHOOSER_TYPE_PICKER, GtkFileChooserTypePicker))
#define GTK_FILE_CHOOSER_TYPE_PICKER_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_FILE_CHOOSER_TYPE_PICKER, GtkFileChooserTypePicker const))
#define GTK_FILE_CHOOSER_TYPE_PICKER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_FILE_CHOOSER_TYPE_PICKER, GtkFileChooserTypePickerClass))
#define GTK_IS_FILE_CHOOSER_TYPE_PICKER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_FILE_CHOOSER_TYPE_PICKER))
#define GTK_IS_FILE_CHOOSER_TYPE_PICKER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_FILE_CHOOSER_TYPE_PICKER))
#define GTK_FILE_CHOOSER_TYPE_PICKER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_FILE_CHOOSER_TYPE_PICKER, GtkFileChooserTypePickerClass))

typedef struct _GtkFileChooserTypePicker	GtkFileChooserTypePicker;
typedef struct _GtkFileChooserTypePickerClass	GtkFileChooserTypePickerClass;
typedef struct _GtkFileChooserTypePickerPrivate	GtkFileChooserTypePickerPrivate;

struct _GtkFileChooserTypePicker
{
	GtkGrid parent;

	GtkFileChooserTypePickerPrivate *priv;
};

struct _GtkFileChooserTypePickerClass
{
	GtkGridClass parent_class;

};

GType
gtk_file_chooser_type_picker_get_type         (void) G_GNUC_CONST;

GtkWidget *
gtk_file_chooser_type_picker_new              (void);

void
gtk_file_chooser_type_picker_add_extension    (GtkFileChooserTypePicker *picker,
                                               const gchar              *file_type,
                                               const gchar              *extension);

gboolean
gtk_file_chooser_type_picker_remove_extension (GtkFileChooserTypePicker *picker,
                                               const gchar              *file_type,
                                               const gchar              *extension);
void
gtk_file_chooser_type_picker_clear_extensions (GtkFileChooserTypePicker *picker);

void
gtk_file_chooser_type_picker_set_force_valid_extension (GtkFileChooserTypePicker *picker,
                                                        gboolean                  force_valid);

gboolean
gtk_file_chooser_type_picker_get_force_valid_extension (GtkFileChooserTypePicker *picker);

gboolean
gtk_file_chooser_type_picker_select_extension (GtkFileChooserTypePicker *picker,
                                               const gchar              *extension);

gboolean
gtk_file_chooser_type_picker_select_extension_from_path (GtkFileChooserTypePicker *picker,
                                                         const gchar              *path,
                                                         const gchar             **extension);

void
gtk_file_chooser_type_picker_unselect_extension (GtkFileChooserTypePicker *picker);

gchar *
gtk_file_chooser_type_picker_get_selected_extension (GtkFileChooserTypePicker *picker);

gboolean
gtk_file_chooser_type_picker_set_current_extension_full (GtkFileChooserTypePicker *picker,
                                                         const gchar              *file_type,
                                                         const gchar              *extension);

guint
gtk_file_chooser_type_picker_get_extension_count (GtkFileChooserTypePicker *selector);

gchar *
gtk_file_chooser_type_picker_get_current_extension (GtkFileChooserTypePicker *picker);

gchar *
gtk_file_chooser_type_picker_get_current_file_type (GtkFileChooserTypePicker *picker);

gboolean
gtk_file_chooser_type_picker_set_current_extension (GtkFileChooserTypePicker *picker,
                                                    const gchar              *extension);

gboolean
gtk_file_chooser_type_picker_set_current_file_type (GtkFileChooserTypePicker *picker,
                                                    const gchar              *file_type);

gchar *
gtk_file_chooser_type_picker_extract_filename (const gchar *path);

gchar *
gtk_file_chooser_type_picker_extract_extension (const gchar *path);

G_END_DECLS

#endif /* __GTK_FILE_CHOOSER_TYPE_PICKER_H__ */

/* ex:set ts=8 noet: */
