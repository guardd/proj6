#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <gtk/gtk.h>

// Structure to hold system information
typedef struct {
    char* os_release;
    char* kernel_version;
    char* memory_total;
    char* processor_info;
    char* disk_space;
} SystemInfo;

// Function declarations
void init_system_info(GtkBuilder *builder);
void update_system_info();
void cleanup_system_info();

#endif // SYSTEM_INFO_H