/*
 * CS252 Project 6 Group 27 
 *
 * Processes file.
 *
 */
#include <gtk/gtk.h>
#include "processes.h"

/* Gtk UI Globals */ 
GtkTreeStore *p_tree_store;
GtkTreeView  *p_tree_view;
GtkTreeSelection *p_selection;
GtkTreeViewColumn *p_name_col;
GtkTreeViewColumn *p_status_col;
GtkTreeViewColumn *p_cpu_percent_col;
GtkTreeViewColumn *p_id_col;
GtkTreeViewColumn *p_memory_col;
GtkCellRenderer *p_name_col_renderer;
GtkCellRenderer *p_status_col_renderer;
GtkCellRenderer *p_cpu_percent_col_renderer;
GtkCellRenderer *p_id_col_renderer;
GtkCellRenderer *p_memory_col_renderer;

/*
 * void p_init_ui(GtkBuilder* builder)
 * Initializes all the processes.h Gtk UI globals from a builder
 */
void p_init_ui(GtkBuilder *builder) {
  // Init the globals in processes.h from the GtkBuilder in main
  p_tree_store = GTK_TREE_STORE(gtk_builder_get_object(builder, "p_tree_store"));
  p_tree_view  = GTK_TREE_VIEW(gtk_builder_get_object(builder, "p_tree_view"));
  p_selection  = GTK_TREE_SELECTION(gtk_builder_get_object(builder, "p_selection"));

  p_name_col        = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "p_name_col"));
  p_status_col      = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "p_status_col"));
  p_cpu_percent_col = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "p_cpu_percent_col"));
  p_id_col          = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "p_id_col"));
  p_memory_col      = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "p_memory_col"));

  p_name_col_renderer        = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "p_name_col_renderer"));
  p_status_col_renderer      = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "p_status_col_renderer"));
  p_cpu_percent_col_renderer = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "p_cpu_percent_col_renderer"));
  p_id_col_renderer          = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "p_id_col_renderer"));
  p_memory_col_renderer      = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "p_memory_col_renderer"));

  // Connect the column elements with their respective renderers
  gtk_tree_view_column_add_attribute(p_name_col, p_name_col_renderer, "text", 0);
  gtk_tree_view_column_add_attribute(p_status_col, p_status_col_renderer, "text", 1);
  gtk_tree_view_column_add_attribute(p_cpu_percent_col, p_cpu_percent_col_renderer, "text", 2);
  gtk_tree_view_column_add_attribute(p_id_col, p_id_col_renderer, "text", 3);
  gtk_tree_view_column_add_attribute(p_memory_col, p_memory_col_renderer, "text", 4);
}
/* void p_init_ui(GtkBuilder* builder) */

