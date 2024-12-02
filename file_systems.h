#ifndef FILE_SYSTEMS_H
#define FILE_SYSTEMS_H

/* Gtk UI Globals for File Systems Page */
extern GtkTreeStore *fs_tree_store;
extern GtkTreeView  *fs_tree_view;
extern GtkTreeSelection *fs_selection;
extern GtkTreeViewColumn *fs_file_system_col;
extern GtkTreeViewColumn *fs_mount_col;
extern GtkTreeViewColumn *fs_total_col;
extern GtkTreeViewColumn *fs_used_col;
extern GtkTreeViewColumn *fs_available_col;
extern GtkTreeViewColumn *fs_percent_used_col;
extern GtkCellRenderer *fs_file_system_col_renderer;
extern GtkCellRenderer *fs_mount_col_renderer;
extern GtkCellRenderer *fs_total_col_renderer;
extern GtkCellRenderer *fs_used_col_renderer;
extern GtkCellRenderer *fs_available_col_renderer;
extern GtkCellRenderer *fs_percent_used_prog_renderer;

// functions
void init_file_systems(GtkBuilder *builder);
void refresh_file_systems();

#endif // FILE_SYSTEMS_H