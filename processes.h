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
} process;

/*
 * FOR FUTURE REFERENCE:
 * cat /proc/<pid>/maps outputs the info needed for memory maps
 * ls -l /proc/<pid>/fd outputs the info needed for open files
 */

#endif // PROCESSES_H