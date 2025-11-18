#include <gtk/gtk.h>
#include "student_logic.h"
#include <stdlib.h>
#include <string.h>

/* --- Structs --- */
typedef struct {
    GtkWidget *name_entry;
    GtkWidget *roll_entry;
    GtkWidget *marks_entry;
} StudentEntryWidgets;

/* --- Helper: Show Message --- */
static void show_message(GtkWindow *parent, const gchar *message) {
    GtkWidget *dialog = gtk_message_dialog_new(parent,
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s",
        message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* --- Helper: Get Integer Input (for Roll No) --- */
// Returns -1 if cancelled, otherwise returns the integer entered
static int pop_up_input_dialog(GtkWindow *parent, const char *title, const char *prompt) {
    GtkWidget *dialog, *content_area, *entry, *label;
    int result = -1;

    dialog = gtk_dialog_new_with_buttons(title, parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_OK", GTK_RESPONSE_ACCEPT,
        "_Cancel", GTK_RESPONSE_REJECT, NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new(prompt);
    gtk_container_add(GTK_CONTAINER(content_area), label);
    
    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
        if (strlen(text) > 0) {
            result = atoi(text);
        }
    }
    gtk_widget_destroy(dialog);
    return result;
}

/* --- Display Logic --- */
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

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(display_window), scrolled);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    
    // Modern font setting
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "textview { font-family: monospace; font-size: 14px; }", -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(text_view);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    gtk_container_add(GTK_CONTAINER(scrolled), text_view);
    update_display_window(text_view, list);
    gtk_widget_show_all(display_window);
}

/* --- Add Student --- */
static void on_add_clicked(GtkWidget *widget, gpointer data) {
    StudentList *list = (StudentList*)data;
    GtkWidget *dialog, *grid;
    StudentEntryWidgets *w = g_slice_new(StudentEntryWidgets);
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));

    dialog = gtk_dialog_new_with_buttons("Add Student", parent, GTK_DIALOG_MODAL, "_OK", GTK_RESPONSE_ACCEPT, "_Cancel", GTK_RESPONSE_REJECT, NULL);
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), grid);
    
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    w->name_entry = gtk_entry_new();
    w->roll_entry = gtk_entry_new();
    w->marks_entry = gtk_entry_new();

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Name:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), w->name_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Roll No:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), w->roll_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Marks:"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), w->marks_entry, 1, 2, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(w->name_entry));
        int roll = atoi(gtk_entry_get_text(GTK_ENTRY(w->roll_entry)));
        float marks = atof(gtk_entry_get_text(GTK_ENTRY(w->marks_entry)));

        if (addStudent(list, name, roll, marks)) {
            show_message(GTK_WINDOW(dialog), "Student added successfully.");
        } else {
            show_message(GTK_WINDOW(dialog), "Error: Roll number already exists.");
        }
    }
    gtk_widget_destroy(dialog);
    g_slice_free(StudentEntryWidgets, w);
}

/* --- Remove Student --- */
static void on_remove_clicked(GtkWidget *widget, gpointer data) {
    StudentList *list = (StudentList*)data;
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    
    int roll = pop_up_input_dialog(parent, "Remove Student", "Enter Roll Number to remove:");
    if (roll != -1) {
        if (removeStudent(list, roll)) {
            show_message(parent, "Student removed successfully.");
        } else {
            show_message(parent, "Student not found.");
        }
    }
}

/* --- Search Student --- */
static void on_search_clicked(GtkWidget *widget, gpointer data) {
    StudentList *list = (StudentList*)data;
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));

    int roll = pop_up_input_dialog(parent, "Search Student", "Enter Roll Number to search:");
    if (roll != -1) {
        int idx = searchStudent(list, roll);
        if (idx != -1) {
            gchar *info = g_strdup_printf("Found!\nName: %s\nRoll: %d\nMarks: %.2f", 
                list->students[idx].name, list->students[idx].roll, list->students[idx].marks);
            show_message(parent, info);
            g_free(info);
        } else {
            show_message(parent, "Student not found.");
        }
    }
}

/* --- Average Marks --- */
static void on_avg_clicked(GtkWidget *widget, gpointer data) {
    StudentList *list = (StudentList*)data;
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));

    float avg = getAverageMarks(list);
    gchar *msg = g_strdup_printf("Class Average Marks: %.2f", avg);
    show_message(parent, msg);
    g_free(msg);
}

/* --- Sort Students --- */
static void on_sort_clicked(GtkWidget *widget, gpointer data) {
    StudentList *list = (StudentList*)data;
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Sort Records", parent, 
        GTK_DIALOG_MODAL, "Ascending (Low->High)", 1, "Descending (High->Low)", 0, "Cancel", -1, NULL);
    
    GtkWidget *label = gtk_label_new("Choose sort order by marks:");
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label);
    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (response == 1) {
        sortStudents(list, 1); // Ascending
        show_message(parent, "Sorted Ascending.");
    } else if (response == 0) {
        sortStudents(list, 0); // Descending
        show_message(parent, "Sorted Descending.");
    }
}

/* --- Modify Student --- */
static void on_modify_clicked(GtkWidget *widget, gpointer data) {
    StudentList *list = (StudentList*)data;
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));

    // 1. Ask for Roll Number
    int roll = pop_up_input_dialog(parent, "Modify Record", "Enter Roll Number to modify:");
    if (roll == -1) return;

    int idx = searchStudent(list, roll);
    if (idx == -1) {
        show_message(parent, "Student not found.");
        return;
    }

    // 2. Show Dialog to enter new details
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Modify Details", parent, GTK_DIALOG_MODAL, "_Update", GTK_RESPONSE_ACCEPT, "_Cancel", GTK_RESPONSE_REJECT, NULL);
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    GtkWidget *name_entry = gtk_entry_new();
    GtkWidget *marks_entry = gtk_entry_new();
    
    // Pre-fill current name
    gtk_entry_set_text(GTK_ENTRY(name_entry), list->students[idx].name);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("New Name:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("New Marks (-1 to keep):"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), marks_entry, 1, 1, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *newName = gtk_entry_get_text(GTK_ENTRY(name_entry));
        const char *marksStr = gtk_entry_get_text(GTK_ENTRY(marks_entry));
        float newMarks = (strlen(marksStr) > 0) ? atof(marksStr) : -1;

        modifyStudent(list, roll, newName, newMarks);
        show_message(parent, "Record Updated.");
    }
    gtk_widget_destroy(dialog);
}

/* --- Load/Save --- */
static void on_save_clicked(GtkWidget *widget, gpointer data) {
    if (saveToFile((StudentList*)data)) 
        show_message(GTK_WINDOW(gtk_widget_get_toplevel(widget)), "Saved!");
    else 
        show_message(GTK_WINDOW(gtk_widget_get_toplevel(widget)), "Error saving.");
}

static void on_load_clicked(GtkWidget *widget, gpointer data) {
    if (loadFromFile((StudentList*)data)) 
        show_message(GTK_WINDOW(gtk_widget_get_toplevel(widget)), "Loaded!");
    else 
        show_message(GTK_WINDOW(gtk_widget_get_toplevel(widget)), "Error loading.");
}

/* --- Main --- */
int main(int argc, char *argv[]) {
    StudentList list;
    initList(&list);

    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Student System");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 500); // Taller window
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Helper macro to add buttons quickly
    #define ADD_BTN(label, callback) \
        GtkWidget *btn_##callback = gtk_button_new_with_label(label); \
        gtk_box_pack_start(GTK_BOX(vbox), btn_##callback, TRUE, TRUE, 2); \
        g_signal_connect(btn_##callback, "clicked", G_CALLBACK(callback), &list);

    ADD_BTN("1. Add Student", on_add_clicked);
    ADD_BTN("2. Display All", on_display_clicked);
    ADD_BTN("3. Modify Record", on_modify_clicked);
    ADD_BTN("4. Remove Record", on_remove_clicked);
    ADD_BTN("5. Search by Roll No", on_search_clicked);
    ADD_BTN("6. Save to File", on_save_clicked);
    ADD_BTN("7. Load from File", on_load_clicked);
    ADD_BTN("8. Calculate Average", on_avg_clicked);
    ADD_BTN("9. Sort Records", on_sort_clicked);

    GtkWidget *quit_btn = gtk_button_new_with_label("0. Quit");
    gtk_box_pack_start(GTK_BOX(vbox), quit_btn, TRUE, TRUE, 2);
    g_signal_connect(quit_btn, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();
    
    freeList(&list);
    return 0;
}