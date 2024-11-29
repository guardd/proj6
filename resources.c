/*
 * CS252 Project 6 Group 27 
 *
 * Resources file.
 *
 */

#include "resources.h"
#include <cairo.h>
#include <math.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HISTORY_SIZE 60  // Store 60 data points for history
#define UPDATE_INTERVAL 1000  // Update every 1000ms (1 second)

// Structure to hold resource data
typedef struct {
    double cpu_usage[HISTORY_SIZE];
    double memory_usage[HISTORY_SIZE];
    double swap_usage[HISTORY_SIZE];
    double network_receive[HISTORY_SIZE];
    double network_send[HISTORY_SIZE];
    int current_index;
} ResourceHistory;

static ResourceHistory history = {0};
static GtkDrawingArea *cpu_graph = NULL;
static GtkDrawingArea *memory_graph = NULL;
static GtkDrawingArea *network_graph = NULL;
static guint update_timeout_id = 0;

// Function to get CPU usage
static double get_cpu_usage() {
    static unsigned long long prev_total = 0, prev_idle = 0;
    
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return 0.0;
    
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    fclose(fp);
    
    unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    unsigned long long current_idle = idle + iowait;
    
    unsigned long long total_diff = total - prev_total;
    unsigned long long idle_diff = current_idle - prev_idle;
    
    prev_total = total;
    prev_idle = current_idle;
    
    if (total_diff == 0) return 0.0;
    return (100.0 * (total_diff - idle_diff)) / total_diff;
}

// Function to get memory usage
static void get_memory_usage(double *mem_usage, double *swap_usage) {
    struct sysinfo si;
    if (sysinfo(&si) != 0) {
        *mem_usage = 0.0;
        *swap_usage = 0.0;
        return;
    }
    
    unsigned long total_mem = si.totalram * si.mem_unit;
    unsigned long used_mem = (si.totalram - si.freeram) * si.mem_unit;
    *mem_usage = 100.0 * used_mem / total_mem;
    
    if (si.totalswap != 0) {
        unsigned long total_swap = si.totalswap * si.mem_unit;
        unsigned long used_swap = (si.totalswap - si.freeswap) * si.mem_unit;
        *swap_usage = 100.0 * used_swap / total_swap;
    } else {
        *swap_usage = 0.0;
    }
}

// Function to get network usage
static void get_network_usage(double *receive, double *send) {
    static unsigned long long prev_receive = 0, prev_send = 0;
    static struct timespec prev_time = {0, 0};
    
    FILE *fp = fopen("/proc/net/dev", "r");
    if (!fp) {
        *receive = 0.0;
        *send = 0.0;
        return;
    }
    
    char line[200];
    unsigned long long total_receive = 0, total_send = 0;
    
    // Skip header lines
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    
    while (fgets(line, sizeof(line), fp)) {
        char iface[20];
        unsigned long long r, s;
        sscanf(line, "%s %llu %*u %*u %*u %*u %*u %*u %*u %llu",
               iface, &r, &s);
        if (strcmp(iface, "lo:") != 0) {  // Skip loopback
            total_receive += r;
            total_send += s;
        }
    }
    fclose(fp);
    
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    
    double time_diff = (current_time.tv_sec - prev_time.tv_sec) +
                      (current_time.tv_nsec - prev_time.tv_nsec) / 1e9;
    
    if (time_diff > 0 && prev_receive != 0) {
        *receive = (total_receive - prev_receive) / time_diff;
        *send = (total_send - prev_send) / time_diff;
    } else {
        *receive = 0.0;
        *send = 0.0;
    }
    
    prev_receive = total_receive;
    prev_send = total_send;
    prev_time = current_time;
}

// Helper function to draw axis labels
// Helper function to draw axis labels
static void draw_axis_labels(cairo_t *cr, G_GNUC_UNUSED int width, int height, const char* y_unit) {
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12.0);
    cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);

    // Y-axis labels with fixed spacing
    for (int i = 0; i <= 4; i++) {
        char label[32];
        double y = (height - 20) * i / 4.0 + 10; // Adjusted position with margins
        snprintf(label, sizeof(label), "%d%s", 100 - (i * 25), y_unit);
        
        cairo_move_to(cr, 5, y + 4);
        cairo_show_text(cr, label);
    }
}

// Helper function to draw a basic graph with grid
static void draw_graph_base(cairo_t *cr, int width, int height) {
    // Clear background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    
    // Draw grid
    cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1);
    cairo_set_line_width(cr, 0.5);
    
    // Horizontal grid lines with margins
    for (int i = 0; i < 5; i++) {
        double y = (height - 20) * i / 4.0 + 10; // Adjusted position with margins
        cairo_move_to(cr, 35, y);
        cairo_line_to(cr, width - 5, y);
    }
    cairo_stroke(cr);
}

static gboolean draw_cpu_graph(GtkWidget *widget, cairo_t *cr, G_GNUC_UNUSED gpointer data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    draw_graph_base(cr, width, height);
    draw_axis_labels(cr, width, height, "%");
    
    // Draw CPU usage line
    cairo_set_source_rgba(cr, 0.2, 0.6, 0.9, 1);
    cairo_set_line_width(cr, 2.0);
    
    int start_idx = (history.current_index + 1) % HISTORY_SIZE;
    for (int i = 0; i < HISTORY_SIZE; i++) {
        int idx = (start_idx + i) % HISTORY_SIZE;
        double x = 35 + (width - 40) * i / (HISTORY_SIZE - 1.0);
        double y = ((height - 20) * (100 - history.cpu_usage[idx]) / 100.0) + 10;
        
        if (i == 0)
            cairo_move_to(cr, x, y);
        else
            cairo_line_to(cr, x, y);
    }
    cairo_stroke(cr);
    
    return FALSE;
}

// Similar adjustments for memory_graph and network_graph
static gboolean draw_memory_graph(GtkWidget *widget, cairo_t *cr, G_GNUC_UNUSED gpointer data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    draw_graph_base(cr, width, height);
    draw_axis_labels(cr, width, height, "%");
    
    cairo_set_source_rgba(cr, 0.9, 0.2, 0.2, 1);
    cairo_set_line_width(cr, 2.0);
    
    int start_idx = (history.current_index + 1) % HISTORY_SIZE;
    for (int i = 0; i < HISTORY_SIZE; i++) {
        int idx = (start_idx + i) % HISTORY_SIZE;
        double x = 35 + (width - 40) * i / (HISTORY_SIZE - 1.0);
        double y = ((height - 20) * (100 - history.memory_usage[idx]) / 100.0) + 10;
        
        if (i == 0)
            cairo_move_to(cr, x, y);
        else
            cairo_line_to(cr, x, y);
    }
    cairo_stroke(cr);
    
    return FALSE;
}

static gboolean draw_network_graph(GtkWidget *widget, cairo_t *cr, G_GNUC_UNUSED gpointer data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    
    draw_graph_base(cr, width, height);
    
    // Find max value for scaling
    double max_value = 1024.0;  // Minimum scale (1 KB/s)
    for (int i = 0; i < HISTORY_SIZE; i++) {
        max_value = fmax(max_value, history.network_receive[i]);
    }
    
    // Round max_value to next power of 2 for nice scaling
    max_value = pow(2, ceil(log2(max_value)));
    
    // Draw network rate labels
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12.0);
    cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
    
    for (int i = 0; i <= 4; i++) {
        char label[32];
        double rate = max_value * (4 - i) / 4.0;
        double y = (height - 20) * i / 4.0 + 10;
        
        if (rate >= 1024*1024)
            snprintf(label, sizeof(label), "%.1fM", rate/(1024*1024));
        else if (rate >= 1024)
            snprintf(label, sizeof(label), "%.1fK", rate/1024);
        else
            snprintf(label, sizeof(label), "%.0f", rate);
            
        cairo_move_to(cr, 5, y + 4);
        cairo_show_text(cr, label);
    }
    
    cairo_set_source_rgba(cr, 0.2, 0.6, 0.9, 1);
    cairo_set_line_width(cr, 2.0);
    
    int start_idx = (history.current_index + 1) % HISTORY_SIZE;
    for (int i = 0; i < HISTORY_SIZE; i++) {
        int idx = (start_idx + i) % HISTORY_SIZE;
        double x = 35 + (width - 40) * i / (HISTORY_SIZE - 1.0);
        double y = ((height - 20) * (1.0 - history.network_receive[idx] / max_value)) + 10;
        
        if (i == 0)
            cairo_move_to(cr, x, y);
        else
            cairo_line_to(cr, x, y);
    }
    cairo_stroke(cr);
    
    return FALSE;
}
// Update function called periodically
static gboolean update_resource_data(G_GNUC_UNUSED gpointer user_data) {
    // Update CPU usage
    history.cpu_usage[history.current_index] = get_cpu_usage();
    
    // Update memory and swap usage
    get_memory_usage(&history.memory_usage[history.current_index],
                    &history.swap_usage[history.current_index]);
    
    // Update network usage
    get_network_usage(&history.network_receive[history.current_index],
                     &history.network_send[history.current_index]);
    
    // Increment index
    history.current_index = (history.current_index + 1) % HISTORY_SIZE;
    
    // Request redraw of graphs
    if (cpu_graph)
        gtk_widget_queue_draw(GTK_WIDGET(cpu_graph));
    if (memory_graph)
        gtk_widget_queue_draw(GTK_WIDGET(memory_graph));
    if (network_graph)
        gtk_widget_queue_draw(GTK_WIDGET(network_graph));
    
    return G_SOURCE_CONTINUE;
}

void init_resources(GtkBuilder *builder) {
    if (!builder) {
        g_warning("Builder is NULL in init_resources");
        return;
    }

    // Get drawing areas
    cpu_graph = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "cpu_graph"));
    if (!cpu_graph) {
        g_warning("Could not find cpu_graph");
        return;
    }

    memory_graph = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "memory_graph"));
    if (!memory_graph) {
        g_warning("Could not find memory_graph");
        return;
    }

    network_graph = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "network_graph"));
    if (!network_graph) {
        g_warning("Could not find network_graph");
        return;
    }

    // Connect drawing signals
    g_signal_connect(G_OBJECT(cpu_graph), "draw", G_CALLBACK(draw_cpu_graph), NULL);
    g_signal_connect(G_OBJECT(memory_graph), "draw", G_CALLBACK(draw_memory_graph), NULL);
    g_signal_connect(G_OBJECT(network_graph), "draw", G_CALLBACK(draw_network_graph), NULL);

    // Start update timer
    update_timeout_id = g_timeout_add(UPDATE_INTERVAL, update_resource_data, NULL);
}

void update_resources() {
    update_resource_data(NULL);
}

void cleanup_resources() {
    if (update_timeout_id > 0) {
        g_source_remove(update_timeout_id);
        update_timeout_id = 0;
    }
}