/*
 * CS252 Project 6 Group 27 
 *
 * Processes file.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <sys/wait.h>
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

process **current_processes;
char process_display_mode = 'u';
char process_display_view = 't';

char* get_process_state_string(p_state state) {
  switch (state) {
    case RUNNING:
      return "Running";
    case SLEEPING:
      return "Sleeping";
    case UNINT_SLEEP:
      return "Uninterrupted Sleep";
    case ZOMBIE:
      return "Zombie";
    case STOPPED:
      return "Stopped";
    case TRACED:
      return "Traced";
    case DEAD:
      return "Dead";
    case IDLE:
      return "Idle";
    case PARKED:
      return "Parked";
    default:
      return "Unknown";
  }
}

/*
 * Initializes all the processes.h Gtk UI globals from a builder
 */
void init_processes(GtkBuilder *builder) {
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
  gtk_tree_view_column_add_attribute(p_cpu_percent_col, p_cpu_percent_col_renderer, "text", 3);
  gtk_tree_view_column_add_attribute(p_id_col, p_id_col_renderer, "text", 4);
  gtk_tree_view_column_add_attribute(p_memory_col, p_memory_col_renderer, "text", 6);
}
/* void init_processes(GtkBuilder* builder) */

void show_error_dialog(const char* message, const char* header_message) {
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_OK, "%s", message);
  gtk_window_set_title(GTK_WINDOW(dialog), header_message);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}
/* void show_error_dialog() */

void show_process_properties_dialog(struct process *proc) {
  GError *error = NULL;
  GtkBuilder *builder = gtk_builder_new();
  if (gtk_builder_add_from_file(builder, "./process-properties.ui", &error) == 0) {
    g_printerr("Error loading file: %s\n", error->message);
    g_clear_error(&error);
    return;
  }

  // Init the gui
  GtkWidget *dialog = GTK_WIDGET(gtk_builder_get_object(builder, "properties_dialog"));
  GtkTreeStore *tree_store = GTK_TREE_STORE(gtk_builder_get_object(builder, "properties_tree_store"));
  GtkTreeViewColumn *property = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "process_property_col"));
  GtkTreeViewColumn *value    = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "process_value_col"));
  GtkCellRenderer *property_renderer = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "property_col_renderer"));
  GtkCellRenderer *value_renderer    = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "value_col_renderer"));
  gtk_tree_view_column_add_attribute(property, property_renderer, "text", 0);
  gtk_tree_view_column_add_attribute(value, value_renderer, "text", 1);
  GtkWidget *close_button = GTK_WIDGET(gtk_builder_get_object(builder, "properties_close_button"));
  g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_widget_destroy), dialog);
  
  if (proc) {
    char title[64];
    snprintf(title, sizeof(title), "Process %d Properties", proc->pid);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  } else {
    return;
  }

  // Init the table
  GtkTreeIter iter;
  char val[64];
  // Name
  snprintf(val, sizeof(val), "%s", proc->name);
  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter, 0, "Process Name", 1, val, -1);
  // User
  snprintf(val, sizeof(val), "%s", proc->user);
  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter, 0, "User", 1, val, -1);
  // PID
  snprintf(val, sizeof(val), "%d", proc->pid);
  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter, 0, "PID", 1, val, -1);
  // PPID
  snprintf(val, sizeof(val), "%d", proc->ppid);
  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter, 0, "Parent PID", 1, val, -1);
  // State
  snprintf(val, sizeof(val), "%s", get_process_state_string(proc->state));
  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter, 0, "State", 1, val, -1);
  // VSS
  snprintf(val, sizeof(val), "%.2f MiB", (float) proc->virtual_memory / (1024.0));
  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter, 0, "Virtual Memory", 1, val, -1);
  // RSS
  snprintf(val, sizeof(val), "%.2f MiB", (float) proc->resident_memory / (1024.0));
  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter, 0, "Resident Memory", 1, val, -1);
  // CPU Time
  snprintf(val, sizeof(val), "%s", proc->cpu_time);
  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter, 0, "CPU Time", 1, val, -1);
  // %CPU
  snprintf(val, sizeof(val), "%.2f%%", proc->percent_cpu);
  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter, 0, "\% CPU", 1, val, -1);
  // %MEM
  snprintf(val, sizeof(val), "%.2f%%", proc->percent_mem);
  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter, 0, "\% Memory", 1, val, -1);
  // Start Date/Time
  snprintf(val, sizeof(val), "%s", proc->start_date);
  gtk_tree_store_append(tree_store, &iter, NULL);
  gtk_tree_store_set(tree_store, &iter, 0, "Start Date/Time", 1, val, -1);

  // Run the gui as a modal dialog
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  g_object_unref(builder);
}
/* void show_process_properties_dialog() */

void show_open_files_dialog(int pid) {
  GError *error = NULL;
  GtkBuilder *builder = gtk_builder_new();
  if (gtk_builder_add_from_file(builder, "./process-open-files.ui", &error) == 0) {
    g_printerr("Error loading file: %s\n", error->message);
    g_clear_error(&error);
    return;
  }
  // Init the gui
  GtkWidget *dialog = GTK_WIDGET(gtk_builder_get_object(builder, "open_files_dialog"));
  GtkTreeStore *tree_store  = GTK_TREE_STORE(gtk_builder_get_object(builder, "open_files_tree_store"));
  GtkTreeViewColumn *fds    = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "open_files_FD_col"));
  GtkTreeViewColumn *mode   = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "open_files_mode_col"));
  GtkTreeViewColumn *type   = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "open_files_type_col"));
  GtkTreeViewColumn *object = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "open_files_object_col"));
  GtkCellRenderer *fds_renderer    = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "open_files_FD_col_renderer"));
  GtkCellRenderer *mode_renderer   = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "open_files_mode_col_renderer"));
  GtkCellRenderer *type_renderer   = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "open_files_type_col_renderer"));
  GtkCellRenderer *object_renderer = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "open_files_object_col_renderer"));
  gtk_tree_view_column_add_attribute(fds, fds_renderer, "text", 0);
  gtk_tree_view_column_add_attribute(mode, mode_renderer, "text", 1);
  gtk_tree_view_column_add_attribute(type, type_renderer, "text", 2);
  gtk_tree_view_column_add_attribute(object, object_renderer, "text", 3);
  GtkWidget *close_button = GTK_WIDGET(gtk_builder_get_object(builder, "open_files_close_button"));
  g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_widget_destroy), dialog);
  char title[32];
  snprintf(title, sizeof(title), "Process %d Open Files", pid);
  gtk_window_set_title(GTK_WINDOW(dialog), title);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

  // Run the get_open_files script
  pid_t id = fork();
  if (id == -1) {
    perror("fork");
    return;
  }
  char flag[8];
  snprintf(flag, sizeof(flag), "%d", pid);
  if (id == 0) {
    execlp("./scripts/get_open_files", "get_open_files", flag, NULL);
    perror("execlp");
    exit(-1);
  } else {
    waitpid(id, NULL, 0);
  }
  // Check for output in ./tmp/f.txt
  FILE *file = fopen("./tmp/f.txt", "r");
  if (!file) {
    show_error_dialog("Internal file error!", "Open Files Error");
    return;
  } else {
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, 0);
    if (size == 0) {
      show_error_dialog("Permission denied\nor process already terminated!",
                        "Open Files Error");
      return;
    }
  }
  // Parse output and populate tree_store
  GtkTreeIter iter;
  char line[512];
  while (fgets(line, sizeof(line), file)) {
    char fd_mode[16];
    char type_buf[32];
    char object_buf[256];
    int fields_read = sscanf(line, "%s %s %s", fd_mode, type_buf, object_buf);
    (void) fields_read;

    char mode = fd_mode[strlen(fd_mode) - 1];
    fd_mode[strlen(fd_mode) - 1] = '\0';
    int fd = atoi(fd_mode);
    
    const char* mode_buf;
    switch (mode) {
      case 'r':
        mode_buf = "Read Only";
        break;
      case 'w':
        mode_buf = "Write Only";
        break;
      case 'u':
        mode_buf = "Read & Write";
        break;
      default:
        mode_buf = "N/A";
    }
    
    gtk_tree_store_append(tree_store, &iter, NULL);
    gtk_tree_store_set(tree_store, &iter, 0, fd, 1, mode_buf, 2, type_buf, 3, object_buf, -1);
  }
  fclose(file);
  // Run the gui as a modal dialog
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  g_object_unref(builder);
}
/* void show_open_files_dialog(int pid) */

/* memory maps store:
0  filename
1  vm_start_raw
2  vm_start
3  vm_end_raw
4  vm_end
5  vm_size_raw
6  vm_size
7  flags
8  vm_offset_raw
9  vm_offset
10 device_id
11 inode
*/
void format_memory_size(size_t size_in_bytes, char *output_buffer, size_t buffer_size) {
  char *units[] = {"B", "KiB", "MiB", "GiB", "TiB"};
  int index = 0;
  double size = (double)size_in_bytes;
  while (size >= 1024 && index < 4) {
    size /= 1024;
    index++;
  }
  snprintf(output_buffer, buffer_size, "%.2f %s", size, units[index]);
}
void show_memory_maps_dialog(int pid) {
  GError *error = NULL;
  GtkBuilder *builder = gtk_builder_new();
  if (gtk_builder_add_from_file(builder, "./process-memory-maps.ui", &error) == 0) {
    g_printerr("Error loading file: %s\n", error->message);
    g_clear_error(&error);
    return;
  }
  // Init the gui
  GtkWidget *dialog = GTK_WIDGET(gtk_builder_get_object(builder, "memory_maps_dialog"));
  GtkTreeStore *tree_store = GTK_TREE_STORE(gtk_builder_get_object(builder, "memory_maps_tree_store"));
  GtkTreeViewColumn *file_name_col = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "mm_filename_col"));
  GtkTreeViewColumn *vm_start_col  = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "mm_vm_start_col"));
  GtkTreeViewColumn *vm_end_col    = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "mm_vm_end_col"));
  GtkTreeViewColumn *vm_size_col   = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "mm_vm_size_col"));
  GtkTreeViewColumn *flags_col     = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "mm_flags_col"));
  GtkTreeViewColumn *vm_offset_col = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "mm_vm_offset_col"));
  GtkTreeViewColumn *device_id_col = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "mm_device_id_col"));
  GtkTreeViewColumn *inode_col     = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "mm_inode_col"));
  GtkCellRenderer *file_name_col_renderer = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "mm_filename_col_renderer"));
  GtkCellRenderer *vm_start_col_renderer  = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "mm_vm_start_col_renderer"));
  GtkCellRenderer *vm_end_col_renderer    = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "mm_vm_end_col_renderer"));
  GtkCellRenderer *vm_size_col_renderer   = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "mm_vm_size_col_renderer"));
  GtkCellRenderer *flags_col_renderer     = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "mm_flags_col_renderer"));
  GtkCellRenderer *vm_offset_col_renderer = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "mm_vm_offset_col_renderer"));
  GtkCellRenderer *device_id_col_renderer = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "mm_device_id_col_renderer"));
  GtkCellRenderer *inode_col_renderer     = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "mm_inode_col_renderer"));
  gtk_tree_view_column_add_attribute(file_name_col, file_name_col_renderer, "text", 0);
  gtk_tree_view_column_add_attribute(vm_start_col, vm_start_col_renderer, "text", 2);
  gtk_tree_view_column_add_attribute(vm_end_col, vm_end_col_renderer, "text", 4);
  gtk_tree_view_column_add_attribute(vm_size_col, vm_size_col_renderer, "text", 6);
  gtk_tree_view_column_add_attribute(flags_col, flags_col_renderer, "text", 7);
  gtk_tree_view_column_add_attribute(vm_offset_col, vm_offset_col_renderer, "text", 9);
  gtk_tree_view_column_add_attribute(device_id_col, device_id_col_renderer, "text", 10);
  gtk_tree_view_column_add_attribute(inode_col, inode_col_renderer, "text", 11);
  GtkWidget *close_button = GTK_WIDGET(gtk_builder_get_object(builder, "memory_maps_close_button"));
  g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_widget_destroy), dialog);
  char title[32];
  snprintf(title, sizeof(title), "Process %d Memory Maps", pid);
  gtk_window_set_title(GTK_WINDOW(dialog), title);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

  // Execlp
  pid_t id = fork();
  if (id == -1) {
    perror("fork");
    return;
  }
  char flag[8];
  snprintf(flag, sizeof(flag), "%d", pid);
  if (id == 0) {
    execlp("./scripts/get_memory_maps", "get_memory_maps", flag, NULL);
    perror("execlp");
    exit(-1);
  } else {
    waitpid(id, NULL, 0);
  }

  // Open file
  FILE *file = fopen("./tmp/m.txt", "r");
  if (!file) {
    show_error_dialog("Internal file error!", "Memory Maps Error");
    return;
  } else {
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, 0);
    if (size == 0) {
      show_error_dialog("Permission denied\nor process already terminated!",
                        "Memory Maps Error");
      return;
    }
  }

  // Parse file
  GtkTreeIter iter;
  char line[1024];
  while (fgets(line, sizeof(line), file)) {
    char start_end_buf[64];
    char flags_buf[16];
    char offset_buf[24];
    char device_buf[16];
    char inode_buf[16];
    char file_name_buf[256];
    int fields_read = sscanf(line, "%s %s %s %s %s %s", start_end_buf, flags_buf, offset_buf,
                                                        device_buf, inode_buf, file_name_buf);
    (void) fields_read;

    // Parse vm start/end
    char vm_start[24];
    char vm_end[24];
    char *sep = strchr(start_end_buf, '-');
    if (sep != NULL) {
      size_t start_len = sep - start_end_buf;
      if (start_len >= sizeof(vm_start)) {
        start_len = sizeof(vm_start) - 1;
      }
      strncpy(vm_start, start_end_buf, start_len);
      vm_start[start_len] = '\0';
      strncpy(vm_end, sep + 1, sizeof(vm_end) - 1);
      vm_end[sizeof(vm_end) - 1] = '\0';
    } else {
      strcpy(vm_start, "0");
      strcpy(vm_end, "0");
    }
    size_t vm_start_val = (size_t)strtoull(vm_start, NULL, 16);
    size_t vm_end_val = (size_t)strtoull(vm_end, NULL, 16);
    // Parse vm size
    size_t vm_size_val = vm_end_val - vm_start_val;
    char vm_size[16];
    format_memory_size(vm_size_val, vm_size, sizeof(vm_size));
    // Parse vm offset
    size_t vm_offset_val = (size_t)strtoull(offset_buf, NULL, 24);
    // Parse inode
    size_t inode_val = atoi(inode_buf);
    
    // Set row in the tree store
    // g_print("%s", line);
    gtk_tree_store_append(tree_store, &iter, NULL);
    gtk_tree_store_set(tree_store, &iter, 0, file_name_buf,
                      1, vm_start_val, 2, vm_start,
                      3, vm_end_val, 4, vm_end,
                      5, vm_size_val, 6, vm_size,
                      7, flags_buf,
                      8, vm_offset_val, 9, offset_buf,
                      10, device_buf,
                      11, inode_val, -1);

  }
  fclose(file);
  // Run the gui as a modal dialog
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
  g_object_unref(builder);
}
/* void show_memory_maps_dialog(int pid) */

/*
 * Executes the get_running_processes bash script based
 * on the current mode to refresh the file stored in ./tmp/
 */
void refresh_process_file(char mode) {
  const char *flag;
  if (mode == 'a') {
    flag = "-a";
  } else if (mode == 'u') {
    flag = "-u";
  } else {
    return;
  }

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return;
  }
  
  if (pid == 0) {
    if (execlp("./scripts/get_running_processes", "get_running_processes", flag, NULL) == -1) {
      perror("execlp");
      exit(-1);
    }
  } else {
    waitpid(pid, NULL, 0);
  }
}

/*
 * Parses the file inside ./tmp/ to populate process structs
 */
void refresh_current_processes(char mode) {
  // Refresh processes to get most updated running processes
  refresh_process_file(mode);

  // Parse from correct file
  const char* file_name;
  if (mode == 'a') {
    file_name = "./tmp/a.txt";
  } else if (mode == 'u') {
    file_name = "./tmp/u.txt";
  } else {
    return;
  }

  FILE *file = fopen(file_name, "r");
  if (!file) {
    return;
  }

  // Free old current_processes
  if (current_processes) {
    int i = 0;
    while (current_processes[i] != NULL) {
      free(current_processes[i]->name);
      free(current_processes[i]->user);
      free(current_processes[i]->cpu_time);
      free(current_processes[i]->start_date);
      free(current_processes[i]);
      i++;
    }
    free(current_processes);
  }

  // Initialize current_processes
  current_processes = malloc(sizeof(process *) * 32);
  int proc_count = 0;

  char line[1024];
  while (fgets(line, sizeof(line), file)) {
    // printf("%s", line);
    // Init new process
    process *proc = malloc(sizeof(process));

    // Init buf to parse into from sscanf
    char buf[15][16];
    int fields_read = sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
                      buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
                      buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14]);
    char start_date[80];
    snprintf(start_date, sizeof(start_date), "%s %s %s %s %s",
             buf[10], buf[11], buf[12], buf[13], buf[14]);
    if (fields_read != 15) continue;

    // Populate the proc struct
    proc->pid = atoi(buf[0]);
    proc->ppid = atoi(buf[1]);
    proc->name = strdup(buf[2]);
    proc->user = strdup(buf[3]);
    proc->state = (p_state) buf[4][0];
    proc->virtual_memory = atoi(buf[5]);
    proc->resident_memory = atoi(buf[6]);
    proc->cpu_time = strdup(buf[7]);
    proc->percent_cpu = atof(buf[8]);
    proc->percent_mem = atof(buf[9]);
    proc->start_date = strdup(start_date);

    // Set proc to correct location in current_processes & inc proc_count
    current_processes[proc_count++] = proc;

    // Resize current_processes if necessary
    if (proc_count % 32 == 0) {
      current_processes = realloc(current_processes, sizeof(process *) * (proc_count + 32));
    }
  }

  // Null terminate current_processes
  current_processes[proc_count] = NULL;
  fclose(file);
}
/* void refresh_current_processes(char mode) */

/*
 * Displays the currently running processes in a table (t) or Tree (T) view
 */
void display_processes() {
  refresh_current_processes(process_display_mode);
  // Clear the existing tree store
  gtk_tree_store_clear(p_tree_store);

  if (process_display_view == 't') { // table view
    GtkTreeIter iter;
    // Append rows as needed from current_processes
    int i = 0;
    while (current_processes[i] != NULL) {
      process *proc = current_processes[i];
      char perc_cpu[16];
      char proc_mem[16];
      snprintf(perc_cpu, sizeof(perc_cpu), "%.2f%%", proc->percent_cpu);
      snprintf(proc_mem, sizeof(perc_cpu), "%.2f MiB", (float) proc->resident_memory / (1024.0));
      gtk_tree_store_append(p_tree_store, &iter, NULL);
      gtk_tree_store_set(p_tree_store, &iter, 0, proc->name,
                                              1, get_process_state_string(proc->state),
                                              2, proc->percent_cpu, // raw data (in the store)
                                              3, perc_cpu, // formatted data (for the tree_view)
                                              4, proc->pid,
                                              5, proc->resident_memory, // raw data (in the store)
                                              6, proc_mem, -1); // formatted data (for the tree_view)
      i++;
    }
  } else if (process_display_view == 'T') { // Tree view
    // Map from pid to iters in a hash table
    GHashTable *pid_to_iter = g_hash_table_new(g_int_hash, g_int_equal);
    int i = 0;
    while (current_processes[i] != NULL) {
      process *proc = current_processes[i];
      char perc_cpu[16];
      char proc_mem[16];
      snprintf(perc_cpu, sizeof(perc_cpu), "%.2f%%", proc->percent_cpu);
      snprintf(proc_mem, sizeof(proc_mem), "%.2f MiB", (float)proc->resident_memory / (1024.0));

      GtkTreeIter *parent = NULL;
      // If the process has a parent, find its iter in the hash table
      if (proc->ppid != 0) { // Child process
        GtkTreeIter *parent_lookup = g_hash_table_lookup(pid_to_iter, &proc->ppid);
        if (parent_lookup) {
          parent = parent_lookup;
        }
      }

      // Append the current process to the tree_store
      GtkTreeIter *current_iter = g_new(GtkTreeIter, 1);
      gtk_tree_store_append(p_tree_store, current_iter, parent);
      gtk_tree_store_set(p_tree_store, current_iter,
                          0, proc->name,
                          1, get_process_state_string(proc->state),
                          2, proc->percent_cpu, // raw
                          3, perc_cpu, // formatted
                          4, proc->pid,
                          5, proc->resident_memory, // raw
                          6, proc_mem, -1); // formatted

      g_hash_table_insert(pid_to_iter, &proc->pid, current_iter);
      i++;
    }
    g_hash_table_destroy(pid_to_iter);
    gtk_tree_view_expand_all(GTK_TREE_VIEW(p_tree_view));
  }
}
/* void display_processes() */

/*
 * Functions for the process specific actions
 */
int get_selected_process_id() {
  GtkTreeIter iter;
  GtkTreeModel *model;
  int pid;

  // Retrieve the selected row from p_selection
  if (gtk_tree_selection_get_selected(p_selection, &model, &iter)) {
      // Extract data from the selected row
      gtk_tree_model_get(model, &iter, 4, &pid, -1);
  } else {
    printf("Nothing is selected!\n");
    return -1;
  }

  return pid;
}
/* int get_selected_process_id() */

void stop_process() {
  int prod_id = get_selected_process_id();
  if (prod_id < 0) {
    return;
  }
  printf("Stopping a process: %d!\n", prod_id);
  char prod_id_str[16];
  snprintf(prod_id_str, sizeof(prod_id_str), "%d", prod_id);
  
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return;
  }
  
  if (pid == 0) {
    if (execlp("kill", "kill", "-STOP", prod_id_str, NULL) == -1) {
      perror("execlp");
      exit(-1);
    }
  } else {
    int status;
    waitpid(pid, &status, 0);
    if (status != 0) {
      show_error_dialog("\nInsufficient permissions,\nor process already terminated!",
                          "Stop Process Error");
    }
  }
  display_processes();
}
/* void stop_process() */

void continue_process() {
  int prod_id = get_selected_process_id();
  if (prod_id < 0) {
    return;
  }
  printf("Continuing a process: %d!\n", prod_id);
  char prod_id_str[16];
  snprintf(prod_id_str, sizeof(prod_id_str), "%d", prod_id);
  
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return;
  }
  
  if (pid == 0) {
    if (execlp("kill", "kill", "-CONT", prod_id_str, NULL) == -1) {
      perror("execlp");
      exit(-1);
    }
  } else {
    int status;
    waitpid(pid, &status, 0);
    if (status != 0) {
      show_error_dialog("\nInsufficient permissions,\nor process already terminated!",
                          "Continue Process Error");
    }
  }
  display_processes();
}
/* void continue_process() */

void kill_process() {
  int prod_id = get_selected_process_id();
  if (prod_id < 0) {
    return;
  }
  printf("Killing a process: %d!\n", prod_id);
  char prod_id_str[16];
  snprintf(prod_id_str, sizeof(prod_id_str), "%d", prod_id);
  
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return;
  }
  
  if (pid == 0) {
    if (execlp("kill", "kill", "-SIGTERM", prod_id_str, NULL) == -1) {
      perror("execlp");
      exit(-1);
    }
  } else {
    int status;
    waitpid(pid, &status, 0);
    if (status != 0) {
      show_error_dialog("\nInsufficient permissions,\nor process already terminated!",
                          "Kill Process Error");
    }
  }
  display_processes();
}
/* void kill_process() */

void show_memory_maps() {
  int pid = get_selected_process_id();
  g_print("Showing memory maps: %d!\n", pid);
  show_memory_maps_dialog(pid);
}
/* void show_memory_maps() */

void show_open_files() {
  int pid = get_selected_process_id();
  show_open_files_dialog(pid);
}
/* void show_open_files() */

void show_properties() {
  int pid = get_selected_process_id();
  process* proc;
  int i = 0;

  while(current_processes[i] != NULL) {
    if (current_processes[i]->pid == pid) {
      proc = current_processes[i];
      break;
    } else {
      i++;
    }
  }
  if (proc) {
    show_process_properties_dialog(proc);
  } else {
    show_error_dialog("\nUnable to show properties!",
                        "Process Properties Error");
  }
}
/* Functions for the process specific functions */

/*
 * Displays the process specific actions
 */
void show_process_actions(GdkEventButton *event) {
  GtkWidget *menu;
  GtkWidget *menu_item;
  menu = gtk_menu_new();

  menu_item = gtk_menu_item_new_with_label("Stop Process");
  g_signal_connect(menu_item, "activate", G_CALLBACK(stop_process), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

  menu_item = gtk_menu_item_new_with_label("Continue Process");
  g_signal_connect(menu_item, "activate", G_CALLBACK(continue_process), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

  menu_item = gtk_menu_item_new_with_label("Kill Process");
  g_signal_connect(menu_item, "activate", G_CALLBACK(kill_process), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

  menu_item = gtk_menu_item_new_with_label("Memory Maps");
  g_signal_connect(menu_item, "activate", G_CALLBACK(show_memory_maps), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

  menu_item = gtk_menu_item_new_with_label("Open Files");
  g_signal_connect(menu_item, "activate", G_CALLBACK(show_open_files), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

  menu_item = gtk_menu_item_new_with_label("Properties");
  g_signal_connect(menu_item, "activate", G_CALLBACK(show_properties), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

  gtk_widget_show_all(menu);
  gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent*) event);
}
/* void show_process_actions(GdkEventButton *event) */

/*
 * Shows the correct view menu options when the page is switched to Processes
 */
void on_view_all() {
  process_display_mode = 'a';
  display_processes();
}
void on_view_user() {
  process_display_mode = 'u';
  display_processes();
}
void on_view_table() {
  process_display_view = 't';
  display_processes();
}
void on_view_tree() {
  process_display_view = 'T';
  display_processes();
}
void on_refresh_view() {
  display_processes();
}
void destroy_widgets(GtkWidget *widget, gpointer user_data) {
  (void) user_data;
  gtk_widget_destroy(widget);
}
void show_processes_view_menu(GtkBuilder *builder) {
  GtkMenuItem *view_menu_item = GTK_MENU_ITEM(gtk_builder_get_object(builder, "view_menu_bar"));
  GtkWidget *menu_item;

  // Clear the old view menu (if there are children)
  GtkWidget *view_menu = gtk_menu_item_get_submenu(view_menu_item);
  if (view_menu) {
    gtk_container_foreach(GTK_CONTAINER(view_menu), destroy_widgets, NULL);
  } else {
    view_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(view_menu_item, view_menu);
  }

  menu_item = gtk_menu_item_new_with_label("All Processes");
  g_signal_connect(menu_item, "activate", G_CALLBACK(on_view_all), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), menu_item);

  menu_item = gtk_menu_item_new_with_label("User Processes");
  g_signal_connect(menu_item, "activate", G_CALLBACK(on_view_user), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), menu_item);

  menu_item = gtk_menu_item_new_with_label("Table");
  g_signal_connect(menu_item, "activate", G_CALLBACK(on_view_table), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), menu_item);

  menu_item = gtk_menu_item_new_with_label("Tree");
  g_signal_connect(menu_item, "activate", G_CALLBACK(on_view_tree), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), menu_item);

  menu_item = gtk_menu_item_new_with_label("Refresh");
  g_signal_connect(menu_item, "activate", G_CALLBACK(on_refresh_view), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), menu_item);

  gtk_widget_show_all(view_menu);
}
/* void show_processes_view_menu(GtkBuilder *builder) */

/*
 * Callback function for the process specific actions
 */
void on_process_actions_button_press(GtkWidget *tree_view, GdkEventButton *event) {
  if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_SECONDARY) {
    GtkTreePath *path;
    GtkTreeViewColumn *column;

    if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tree_view),
               event->x, event->y, &path, &column, NULL, NULL)) {
      gtk_tree_selection_select_path(p_selection, path);
      show_process_actions(event);
      gtk_tree_path_free(path);
      return;
    }
  }
  return;
}
/* void on_process_actions_button_press(GtkWidget *tree_view, GdkEventButton *event) */

/*
 * Useful print processes function to print
 * contents of current_processes
 */
void print_processes() {
  if (current_processes == NULL) {
    printf("No processes to display.\n");
    return;
  }

  // Iterate through the current_processes array
  int i = 0;
  while (current_processes[i] != NULL) {  // Null-terminated array
    process *proc = current_processes[i];

    // Print the process details
    printf("\n-----PROCESS %d-----\n", proc->pid);
    printf("PID: %d\n", proc->pid);
    printf("PPID: %d\n", proc->ppid);
    printf("Name: %s\n", proc->name ? proc->name : "N/A");
    printf("User: %s\n", proc->user ? proc->user : "N/A");
    printf("State: %s\n", get_process_state_string(proc->state));
    printf("Virtual Memory: %d KB\n", proc->virtual_memory);
    printf("Resident Memory: %d KB\n", proc->resident_memory);
    printf("CPU Time: %s\n", proc->cpu_time ? proc->cpu_time : "N/A");
    printf("CPU Percent: %.2f%%\n", proc->percent_cpu);
    printf("Memory Percent: %.2f%%\n", proc->percent_mem);
    printf("Start Date: %s\n", proc->start_date ? proc->start_date : "N/A");
    printf("--------------------\n");

    i++;
  }
}
/* void print_processes() */