#ifndef RESOURCES_H
#define RESOURCES_H

#include <gtk/gtk.h>

// Initialize the resources UI components
void init_resources(GtkBuilder *builder);

// Update resource information
void update_resources();

// Cleanup resources
void cleanup_resources();

#endif // RESOURCES_H