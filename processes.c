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
      char proc_id[16];
      char proc_mem[16];
      snprintf(perc_cpu, sizeof(perc_cpu), "%.2f%%", proc->percent_cpu);
      snprintf(proc_id, sizeof(perc_cpu), "%d", proc->pid);
      snprintf(proc_mem, sizeof(perc_cpu), "%.2f MiB", (float) proc->resident_memory / (1024.0));
      gtk_tree_store_append(p_tree_store, &iter, NULL);
      gtk_tree_store_set(p_tree_store, &iter, 0, proc->name,
                                              1, get_process_state_string(proc->state),
                                              2, perc_cpu,
                                              3, proc_id,
                                              4, proc_mem, -1);
      i++;
    }
  } else if (process_display_view == 'T') { // Tree view
    // Map from PID to Gtk Iters in a hash table
    GHashTable *pid_to_iter = g_hash_table_new(g_int_hash, g_int_equal);
    int i = 0;
    while (current_processes[i] != NULL) {
      process *proc = current_processes[i];
      char perc_cpu[16];
      char proc_id[16];
      char proc_mem[16];
      snprintf(perc_cpu, sizeof(perc_cpu), "%.2f%%", proc->percent_cpu);
      snprintf(proc_id, sizeof(proc_id), "%d", proc->pid);
      snprintf(proc_mem, sizeof(proc_mem), "%.2f MiB", (float)proc->resident_memory / (1024.0));

      GtkTreeIter *parent = NULL;

      // If the process has a parent, find its GtkTreeIter in the hash table
      if (proc->ppid != 0) { // Child process
        GtkTreeIter *parent_lookup = g_hash_table_lookup(pid_to_iter, &proc->ppid);
        if (parent_lookup) {
          parent = parent_lookup;
        }
      }

      // Append the current process to the TreeStore
      GtkTreeIter *current_iter = g_new(GtkTreeIter, 1);
      gtk_tree_store_append(p_tree_store, current_iter, parent); // NULL parent at root
      gtk_tree_store_set(p_tree_store, current_iter,
                          0, proc->name,
                          1, get_process_state_string(proc->state),
                          2, perc_cpu,
                          3, proc_id,
                          4, proc_mem,
                          -1);

      // Add the current processes iter to the hash table
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
  gchar *process_name, *process_state, *process_cpu, *process_id, *process_mem;

  // Retrieve the selected row from p_selection
  if (gtk_tree_selection_get_selected(p_selection, &model, &iter)) {
      // Extract data from the selected row
      gtk_tree_model_get(model, &iter, 0, &process_name, 1, &process_state,
                         2, &process_cpu, 3, &process_id, 4, &process_mem, -1);
  } else {
    printf("Nothing is selected!\n");
    return -1;
  }
  int pid = atoi((char*) process_id);

  // Free the extracted strings
  g_free(process_name);
  g_free(process_state);
  g_free(process_cpu);
  g_free(process_id);
  g_free(process_mem);

  return pid;
}

void stop_process() {
  g_print("Stopping a process: %d!\n", get_selected_process_id());
}

void continue_process() {
  g_print("Continuing a process: %d!\n", get_selected_process_id());
}

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
    waitpid(pid, NULL, 0);
  }
  display_processes();
}

void show_memory_maps() {
  g_print("Showing memory maps: %d!\n", get_selected_process_id());
}

void show_open_files() {
  g_print("Showing open files: %d!\n", get_selected_process_id());
}

void show_properties() {
  g_print("Showing properties: %d!\n", get_selected_process_id());
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