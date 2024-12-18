#include <gtk/gtk.h>

#include "system_info.h"
#include "processes.h"
#include "resources.h"
#include "file_systems.h"

// Global for builder
GtkBuilder *builder;

/*
 * Callback function for when the page switches to any 1 of the 4
 */
void on_stack_switched(GtkStack *stack) {
  const gchar *visible_child_name = gtk_stack_get_visible_child_name(stack);
  g_print("Switched to Page: %s\n", visible_child_name);
  
  if (builder == NULL) return;

  if (strcmp(visible_child_name, "System") == 0) {
    // Update the menu_bar to have the 'System' options
    update_system_info(); 
  } else if (strcmp(visible_child_name, "Processes") == 0) {
    display_processes();
    // Update the view_menu_bar to have the 'Processes' options
    show_processes_view_menu(builder);
  } else if (strcmp(visible_child_name, "Resources") == 0) {
    update_resources();
    // Update the menu_bar to have the 'Resources' options
  } else if (strcmp(visible_child_name, "File Systems") == 0) {
    // Update the menu_bar to have the 'File Systems' options
    refresh_file_systems();
  }
}

/*
 * The main function builds the GUIs from task-manager.ui
 * and signal handlers are connected to the appropriate widgets
 */
int main(int argc, char **argv) {
  GtkWidget *window;
  GObject *button;
  GError *error = NULL;

  gtk_init(&argc, &argv);

  /* Init builder from task-manager.ui */
  builder = gtk_builder_new();
  if (gtk_builder_add_from_file(builder, "./task-manager.ui", &error) == 0) {
    g_printerr("Error loading file: %s\n", error->message);
    g_clear_error(&error);
    return -1;
  }

  /* Init the UI elements for each page */
  init_system_info(builder);
  init_processes(builder);
  init_resources(builder);
  init_file_systems(builder);

  /* Connect signal handlers to stack (page) switches */
  GtkStack *stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
  g_signal_connect(stack, "notify::visible-child-name", G_CALLBACK(on_stack_switched), NULL);

  /* Connect signal handlers to widgets */
  window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(window, "destroy", G_CALLBACK(cleanup_system_info), NULL);
  g_signal_connect(window, "destroy", G_CALLBACK(cleanup_resources), NULL);

  button = gtk_builder_get_object(builder, "p_end_process_button");
  g_signal_connect(button, "clicked", G_CALLBACK(kill_process), NULL);

  g_signal_connect(p_tree_view, "button-press-event", G_CALLBACK(on_process_actions_button_press), NULL);

  gtk_widget_show_all(window);
  gtk_main();

  return 0;
}
/* int main() */