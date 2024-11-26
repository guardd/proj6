#include <gtk/gtk.h>

#include "system_info.h"
#include "processes.h"
#include "resources.h"
#include "file_systems.h"

/*
 * Some dummy functions to showcase button signal handling
 */
void print_system() {
  g_print("System Button has been clicked!\n");
}

void clear_tree_store() {
  gtk_tree_store_clear(p_tree_store); // temp -- remove later
}

void print_resources() {
  g_print("Resources Button has been clicked!\n");
}

void print_file_systems() {
  g_print("File Systems Button has been clicked!\n");
}

void on_stack_switched(GtkStack *stack) {
  const gchar *visible_child_name = gtk_stack_get_visible_child_name(stack);
  g_print("Switched to Page: %s\n", visible_child_name);
  display_processes();
}
/* Dummy functions */

/*
 * The main function builds the GUIs from task-manager.ui
 * and signal handlers are connected to the appropriate widgets
 */
int main(int argc, char **argv) {
  GtkBuilder *builder;
  GtkWidget *window;
  GObject *button;
  GError *error = NULL;

  gtk_init(&argc, &argv);

  /* Init builder from task-manager.ui */
  builder = gtk_builder_new();
  if (gtk_builder_add_from_file(builder, "task-manager.ui", &error) == 0) {
    g_printerr("Error loading file: %s\n", error->message);
    g_clear_error(&error);
    return -1;
  }
  /* Init the UI elements for the processes page */
  p_init_ui(builder);

  /* Connect signal handlers to stack (page) switches */
  GtkStack *stack = GTK_STACK(gtk_builder_get_object(builder, "stack"));
  g_signal_connect(stack, "notify::visible-child-name", G_CALLBACK(on_stack_switched), NULL);

  /* Connect signal handlers to widgets */
  window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  button = gtk_builder_get_object(builder, "system_button");
  g_signal_connect(button, "clicked", G_CALLBACK(print_system), NULL);

  button = gtk_builder_get_object(builder, "resources_button");
  g_signal_connect(button, "clicked", G_CALLBACK(print_resources), NULL);

  button = gtk_builder_get_object(builder, "file_systems_button");
  g_signal_connect(button, "clicked", G_CALLBACK(print_file_systems), NULL);

  button = gtk_builder_get_object(builder, "p_end_process_button");
  g_signal_connect(button, "clicked", G_CALLBACK(clear_tree_store), NULL);

  button = gtk_builder_get_object(builder, "p_refresh_process_button");
  g_signal_connect(button, "clicked", G_CALLBACK(display_processes), NULL);


  g_signal_connect(p_tree_view, "button-press-event", G_CALLBACK(on_process_actions_button_press), NULL);

  gtk_widget_show_all(window);
  gtk_main();

  return 0;
}
/* int main() */