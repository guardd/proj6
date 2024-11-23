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
/* p_state */

/* 
 * Use the following command in execlp to parse from its output:
 * ps -eo pid,ppid,comm,user,stat,vsz,rss,time,lstart,%cpu,%mem
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
  char* start_date;
  float percent_cpu;
  float percent_mem;
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

void p_init_ui(GtkBuilder *builder);

#endif // PROCESSES_H

/*
 * FOR FUTURE REFERENCE:
 * cat /proc/<pid>/maps outputs the info needed for memory maps
 * ls -l /proc/<pid>/fd outputs the info needed for open files
 */