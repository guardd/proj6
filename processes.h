#ifndef PROCESSES_H
#define PROCESSES_H

#include <gtk/gtk.h>

/*
 * p_state enum maps output of first char in STAT
 * column output from 'ps' to a p_state
 */
typedef enum p_state {
  RUNNING     = 'R',
  SLEEPING    = 'S',
  UNINT_SLEEP = 'D',
  ZOMBIE      = 'Z',
  STOPPED     = 'T',
  TRACED      = 't',
  DEAD        = 'X',
  IDLE        = 'I',
  PARKED      = 'P'
} p_state;

/* 
 * process struct to hold individual process information
 */
typedef struct process {
  int pid;
  int ppid;
  char* name;
  char* user;
  p_state state;
  int virtual_memory;
  int resident_memory;
  char* cpu_time;
  float percent_cpu;
  float percent_mem;
  char* start_date;
  struct process *parent;
  struct process **children;
} process;

/* Gtk UI Globals for Processes Page */
extern GtkTreeStore *p_tree_store;
extern GtkTreeView  *p_tree_view;
extern GtkTreeSelection *p_selection;
extern GtkTreeViewColumn *p_name_col;
extern GtkTreeViewColumn *p_status_col;
extern GtkTreeViewColumn *p_cpu_percent_col;
extern GtkTreeViewColumn *p_id_col;
extern GtkTreeViewColumn *p_memory_col;
extern GtkCellRenderer *p_name_col_renderer;
extern GtkCellRenderer *p_status_col_renderer;
extern GtkCellRenderer *p_cpu_percent_col_renderer;
extern GtkCellRenderer *p_id_col_renderer;
extern GtkCellRenderer *p_memory_col_renderer;

/* Other globals */
extern struct process **current_processes;

/* Function headers */
void p_init_ui(GtkBuilder *builder);
void refresh_current_processes(char mode); // 'a' for all, 'u' for user
void refresh_process_file(char mode);
void print_processes();
void display_processes();
void on_menu_item_activated(gpointer user_data);
void show_process_actions(GdkEventButton *event);
void on_process_actions_button_press(GtkWidget *tree_view, GdkEventButton *event);

#endif // PROCESSES_H

/*
 * FOR FUTURE REFERENCE:
 * cat /proc/<pid>/maps outputs the info needed for memory maps
 * ls -l /proc/<pid>/fd outputs the info needed for open files
 */