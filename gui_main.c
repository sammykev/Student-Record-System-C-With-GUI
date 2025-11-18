#include <gtk/gtk.h>
#include "student_logic.h"
#include <stdlib.h>

typedef struct {
    GtkWidget *name_entry;
    GtkWidget *roll_entry;
    GtkWidget *marks_entry;
} AddStudentWidgets;

/*
 * FIXED: Changed argument type from GtkWidget* to GtkWindow*
 * to match what we pass to it (the parent window).
 */
static void show_message_dialog(GtkWindow *parent, const gchar *message) {
    GtkWidget *dialog = gtk_message_dialog_new(parent,
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s",
        message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void update_display_window(GtkWidget *text_view, StudentList *list) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GString *s = g_string_new("");

    if (list->count == 0) {
        g_string_append(s, "No student records to display.\n");
    } else {
        g_string_append_printf(s, "%-20s %-10s %-10s %-10s\n", "Name", "Roll No", "Marks", "Status");
        g_string_append(s, "------------------------------------------------------------\n");
        for (int i = 0; i < list->count; i++) {
            g_string_append_printf(s, "%-20s %-10d %-10.2f %-10s\n",
                   list->students[i].name,
                   list->students[i].roll,
                   list->students[i].marks,
                   (list->students[i].marks > 40) ? "Passed" : "Failed");
        }
    }
    gtk_text_buffer_set_text(buffer, s->str, -1);
    g_string_free(s, TRUE);
}

static void on_display_clicked(GtkWidget *widget, gpointer data) {
    StudentList *list = (StudentList*)data;
    
    GtkWidget *display_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(display_window), "All Students");
    gtk_window_set_default_size(GTK_WINDOW(display_window), 500, 400);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(display_window), scrolled_window);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    
    // FIXED: Replaced deprecated font override with modern CSS provider
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "textview { font-family: monospace; font-size: 14px; }", -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(text_view);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    update_display_window(text_view, list);
    gtk_widget_show_all(display_window);
}

static void on_add_clicked(GtkWidget *widget, gpointer data) {
    StudentList *list = (StudentList*)data;
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *grid;
    AddStudentWidgets *widgets = g_slice_new(AddStudentWidgets);

    // Use the parent window correctly
    GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_toplevel(widget));

    dialog = gtk_dialog_new_with_buttons("Add New Student",
                                         parent_window,
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "_OK", GTK_RESPONSE_ACCEPT,
                                         "_Cancel", GTK_RESPONSE_REJECT,
                                         NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10); // Add some padding
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    widgets->name_entry = gtk_entry_new();
    widgets->roll_entry = gtk_entry_new();
    widgets->marks_entry = gtk_entry_new();

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Name:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets->name_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Roll No:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets->roll_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Marks:"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets->marks_entry, 1, 2, 1, 1);
    
    gtk_widget_show_all(grid);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_ACCEPT) {
        const gchar *name = gtk_entry_get_text(GTK_ENTRY(widgets->name_entry));
        const gchar *roll_str = gtk_entry_get_text(GTK_ENTRY(widgets->roll_entry));
        const gchar *marks_str = gtk_entry_get_text(GTK_ENTRY(widgets->marks_entry));

        int roll = atoi(roll_str);
        float marks = atof(marks_str);

        if (addStudent(list, name, roll, marks)) {
            show_message_dialog(GTK_WINDOW(dialog), "Student added successfully.");
        } else {
            gchar *error_msg = g_strdup_printf("Error: Student with roll %d already exists.", roll);
            show_message_dialog(GTK_WINDOW(dialog), error_msg);
            g_free(error_msg);
        }
    }

    gtk_widget_destroy(dialog);
    g_slice_free(AddStudentWidgets, widgets);
}

static void on_save_clicked(GtkWidget *widget, gpointer data) {
    StudentList *list = (StudentList*)data;
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    
    if (saveToFile(list)) {
        show_message_dialog(parent, "Records saved successfully.");
    } else {
        show_message_dialog(parent, "Error saving records to file.");
    }
}

static void on_load_clicked(GtkWidget *widget, gpointer data) {
    StudentList *list = (StudentList*)data;
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));

    if (loadFromFile(list)) {
        show_message_dialog(parent, "Records loaded successfully.");
    } else {
        show_message_dialog(parent, "Error loading records from file.");
    }
}

int main(int argc, char *argv[]) {
    StudentList list;
    initList(&list);

    gtk_init(&argc, &argv);

    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *add_button;
    GtkWidget *display_button;
    GtkWidget *load_button;
    GtkWidget *save_button;
    GtkWidget *quit_button;
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Student Record System (GUI)");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    add_button = gtk_button_new_with_label("Add Student");
    display_button = gtk_button_new_with_label("Display All Students");
    load_button = gtk_button_new_with_label("Load from File");
    save_button = gtk_button_new_with_label("Save to File");
    quit_button = gtk_button_new_with_label("Quit");

    gtk_box_pack_start(GTK_BOX(vbox), add_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), display_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), load_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), save_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), quit_button, TRUE, TRUE, 5);

    g_signal_connect(add_button, "clicked", G_CALLBACK(on_add_clicked), &list);
    g_signal_connect(display_button, "clicked", G_CALLBACK(on_display_clicked), &list);
    g_signal_connect(load_button, "clicked", G_CALLBACK(on_load_clicked), &list);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), &list);
    
    g_signal_connect(quit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();
    
    freeList(&list);
    return 0;
}