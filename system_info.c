/*
 * CS252 Project 6 Group 27 
 *
 * System Info file.
 *
 */

#include "system_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>

// Global variables
static GtkLabel *os_label = NULL;
static GtkLabel *kernel_label = NULL;
static GtkLabel *memory_label = NULL;
static GtkLabel *processor_label = NULL;
static GtkLabel *disk_label = NULL;
static SystemInfo current_info = {NULL};

// Helper function to get OS release info
static char* get_os_release() {
    FILE* file = fopen("/etc/os-release", "r");
    if (!file) return g_strdup("Unknown");
    
    char buffer[1024];
    char *name = NULL;
    char *version = NULL;
    char *tmp;
    
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strncmp(buffer, "NAME=", 5) == 0) {
            tmp = strdup(buffer + 5);
            if (tmp) {
                name = g_strdup(g_strstrip(tmp));
                free(tmp);
            }
        }
        if (strncmp(buffer, "VERSION_ID=", 11) == 0) {
            tmp = strdup(buffer + 11);
            if (tmp) {
                version = g_strdup(g_strstrip(tmp));
                free(tmp);
            }
        }
    }
    fclose(file);
    
    char *result;
    if (name && version) {
        // Remove quotes if present
        g_strdelimit(name, "\"", ' ');
        g_strdelimit(version, "\"", ' ');
        result = g_strdup_printf("%s %s", g_strstrip(name), g_strstrip(version));
    } else {
        result = g_strdup("Unknown");
    }
    
    g_free(name);
    g_free(version);
    return result;
}

// Helper function to get processor info
static char* get_processor_info() {
    FILE* file = fopen("/proc/cpuinfo", "r");
    if (!file) return g_strdup("Unknown");
    
    char buffer[1024];
    char *model_name = NULL;
    int cpu_cores = 0;
    
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strncmp(buffer, "model name", 10) == 0) {
            char* colon = strchr(buffer, ':');
            if (colon && !model_name) {
                model_name = g_strdup(g_strstrip(colon + 1));
            }
        }
        if (strncmp(buffer, "processor", 9) == 0) {
            cpu_cores++;
        }
    }
    fclose(file);
    
    char *result;
    if (model_name) {
        result = g_strdup_printf("%s × %d", model_name, cpu_cores);
    } else {
        result = g_strdup_printf("Unknown × %d", cpu_cores);
    }
    
    g_free(model_name);
    return result;
}

// Helper function to get memory info
static char* get_memory_info() {
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        unsigned long total_mb = si.totalram * si.mem_unit / (1024 * 1024);
        return g_strdup_printf("%.1f GiB", total_mb / 1024.0);
    }
    return g_strdup("Unknown");
}

// Helper function to get disk space
static char* get_disk_space() {
    struct statvfs vfs;
    if (statvfs("/", &vfs) == 0) {
        unsigned long available = (vfs.f_bavail * vfs.f_frsize) / (1024 * 1024 * 1024);
        return g_strdup_printf("%lu GiB", available);
    }
    return g_strdup("Unknown");
}

void init_system_info(GtkBuilder *builder) {
    // Get label widgets
    os_label = GTK_LABEL(gtk_builder_get_object(builder, "os_label"));
    kernel_label = GTK_LABEL(gtk_builder_get_object(builder, "kernel_label"));
    memory_label = GTK_LABEL(gtk_builder_get_object(builder, "memory_label"));
    processor_label = GTK_LABEL(gtk_builder_get_object(builder, "processor_label"));
    disk_label = GTK_LABEL(gtk_builder_get_object(builder, "disk_label"));
    
    // Initialize all pointers to NULL
    current_info.os_release = NULL;
    current_info.kernel_version = NULL;
    current_info.memory_total = NULL;
    current_info.processor_info = NULL;
    current_info.disk_space = NULL;
    
    // Initial update
    update_system_info();
}

void update_system_info() {
    char *new_os_release = NULL;
    char *new_kernel_version = NULL;
    char *new_memory_total = NULL;
    char *new_processor_info = NULL;
    char *new_disk_space = NULL;
    
    struct utsname un;
    if (uname(&un) == 0) {
        new_kernel_version = g_strdup(un.release);
    } else {
        new_kernel_version = g_strdup("Unknown");
    }
    
    new_os_release = get_os_release();
    new_memory_total = get_memory_info();
    new_processor_info = get_processor_info();
    new_disk_space = get_disk_space();
    
    // Free old values before assigning new ones
    g_free(current_info.os_release);
    g_free(current_info.kernel_version);
    g_free(current_info.memory_total);
    g_free(current_info.processor_info);
    g_free(current_info.disk_space);
    
    // Assign new values
    current_info.os_release = new_os_release;
    current_info.kernel_version = new_kernel_version;
    current_info.memory_total = new_memory_total;
    current_info.processor_info = new_processor_info;
    current_info.disk_space = new_disk_space;
    
    // Update labels if they exist
    if (os_label)
        gtk_label_set_text(os_label, current_info.os_release);
    if (kernel_label)
        gtk_label_set_text(kernel_label, current_info.kernel_version);
    if (memory_label)
        gtk_label_set_text(memory_label, current_info.memory_total);
    if (processor_label)
        gtk_label_set_text(processor_label, current_info.processor_info);
    if (disk_label)
        gtk_label_set_text(disk_label, current_info.disk_space);
}

void cleanup_system_info() {
    // Free allocated memory
    g_free(current_info.os_release);
    g_free(current_info.kernel_version);
    g_free(current_info.memory_total);
    g_free(current_info.processor_info);
    g_free(current_info.disk_space);
    
    // Set all pointers to NULL after freeing
    current_info.os_release = NULL;
    current_info.kernel_version = NULL;
    current_info.memory_total = NULL;
    current_info.processor_info = NULL;
    current_info.disk_space = NULL;
}