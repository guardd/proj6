/*
 * CS252 Project 6 Group 27 
 *
 * File Systems file.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <sys/wait.h>
#include "file_systems.h"

/* GTK UI Globals */
GtkTreeStore *fs_tree_store;
GtkTreeView  *fs_tree_view;
GtkTreeSelection *fs_selection;
GtkTreeViewColumn *fs_file_system_col;
GtkTreeViewColumn *fs_mount_col;
GtkTreeViewColumn *fs_total_col;
GtkTreeViewColumn *fs_used_col;
GtkTreeViewColumn *fs_available_col;
GtkTreeViewColumn *fs_percent_used_col;
GtkCellRenderer *fs_file_system_col_renderer;
GtkCellRenderer *fs_mount_col_renderer;
GtkCellRenderer *fs_total_col_renderer;
GtkCellRenderer *fs_used_col_renderer;
GtkCellRenderer *fs_available_col_renderer;
GtkCellRenderer *fs_percent_used_prog_renderer;

/*
 * Initializes the file_systems.h Gtk UI globals from a builder
 */
/* fs_tree_store format:
0 filesystem
1 mount
2 total_raw
3 total
4 used_raw
5 used
6 available_raw
7 available
8 percent_used_raw
9 percent_used
 */
void init_file_systems(GtkBuilder *builder) {
  fs_tree_store = GTK_TREE_STORE(gtk_builder_get_object(builder, "fs_tree_store"));
  fs_tree_view  = GTK_TREE_VIEW(gtk_builder_get_object(builder, "fs_tree_view"));
  fs_selection  = GTK_TREE_SELECTION(gtk_builder_get_object(builder, "fs_selection"));

  fs_file_system_col  = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "file_system_col"));
  fs_mount_col        = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "mount_col"));
  fs_total_col        = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "total_col"));
  fs_used_col         = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "used_col"));
  fs_available_col    = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "available_col"));
  fs_percent_used_col = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "percent_used_col"));

  fs_file_system_col_renderer   = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "file_system_col_renderer"));
  fs_mount_col_renderer         = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "mount_col_renderer"));
  fs_total_col_renderer         = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "total_col_renderer"));
  fs_used_col_renderer          = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "used_col_renderer"));
  fs_available_col_renderer     = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "available_col_renderer"));
  fs_percent_used_prog_renderer = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "percent_used_prog_renderer"));

  gtk_tree_view_column_add_attribute(fs_file_system_col, fs_file_system_col_renderer, "text", 0);
  gtk_tree_view_column_add_attribute(fs_mount_col, fs_mount_col_renderer, "text", 1);
  gtk_tree_view_column_add_attribute(fs_total_col, fs_total_col_renderer, "text", 3);
  gtk_tree_view_column_add_attribute(fs_used_col, fs_used_col_renderer, "text", 5);
  gtk_tree_view_column_add_attribute(fs_available_col, fs_available_col_renderer, "text", 7);
  gtk_tree_view_column_add_attribute(fs_percent_used_col, fs_percent_used_prog_renderer, "value", 8);
  gtk_tree_view_column_add_attribute(fs_percent_used_col, fs_percent_used_prog_renderer, "text", 9);
}
/* void init_file_systems(GtkBuilder* builder) */

void fs_show_error_dialog(const char* message, const char* header_message) {
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_OK, "%s", message);
  gtk_window_set_title(GTK_WINDOW(dialog), header_message);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}
/* void fs_show_error_dialog() */

void fs_format_memory_size(size_t size_in_kb, char *output_buffer, size_t buffer_size) {
  char *units[] = {"KiB", "MiB", "GiB", "TiB"};
  int index = 0;
  double size = (double)size_in_kb;
  while (size >= 1024 && index < 3) {
    size /= 1024;
    index++;
  }
  snprintf(output_buffer, buffer_size, "%.2f %s", size, units[index]);
}
/* void fs_format_memory_size() */

/*
 * Refreshes the file systems page
 */
void refresh_file_systems() {
  // Clear the tree store
  gtk_tree_store_clear(fs_tree_store);
  // Execlp the get_file_systems script
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return;
  }
  if (pid == 0) {
    execlp("./scripts/get_file_systems", "get_file_systems", NULL);
    perror("execlp");
    exit(-1);
  } else {
    waitpid(pid, NULL, 0);
  }

  // Open the file
  FILE *file = fopen("./tmp/s.txt", "r");
  if (!file) {
    fs_show_error_dialog("Internal file error!\nCould not read File Systems", "Open Files Error");
    return;
  }

  // Parse the file
  GtkTreeIter iter;
  char line[1024];
  while(fgets(line, sizeof(line), file)) {
    char file_system_buf[32];
    char mount_buf[256];
    char total_buf[16];
    char used_buf[16];
    char avail_buf[16];
    char perc_used_buf[16];
    int fields_read = sscanf(line, "%s %s %s %s %s %s", file_system_buf,
                              total_buf, used_buf, avail_buf,
                              perc_used_buf, mount_buf);
    (void) fields_read;

    // Parse amounts
    size_t fs_total_val = (size_t) atoi(total_buf);
    size_t fs_used_val = (size_t) atoi(used_buf);
    size_t fs_avail_val = (size_t) atoi(avail_buf);
    fs_format_memory_size(fs_total_val, total_buf, sizeof(total_buf));
    fs_format_memory_size(fs_used_val, used_buf, sizeof(used_buf));
    fs_format_memory_size(fs_avail_val, avail_buf, sizeof(avail_buf));
    double perc_used_val = (double) fs_used_val / (double) fs_total_val;
    snprintf(perc_used_buf, sizeof(perc_used_buf), "%.2f%%", perc_used_val);
    // g_print("%s", line);
    gtk_tree_store_append(fs_tree_store, &iter, NULL);
    gtk_tree_store_set(fs_tree_store, &iter,
                        0, file_system_buf, 1, mount_buf,
                        2, fs_total_val, 3, total_buf,
                        4, fs_used_val, 5, used_buf,
                        6, fs_avail_val, 7, avail_buf,
                        8, perc_used_val * 100.0, 9, perc_used_buf, -1); // % used
  }
  fclose(file);
}