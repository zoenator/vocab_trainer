//! NOT FINISHED, IGNORE

// #include "db.h"
// #include "glib-object.h"
// #include "paths.h"
// #include "vocab_entry.h"

// #include <gtk/gtk.h>
// #include <stdio.h>

int main(int argc, char **argv)
{
    //     gtk_init(&argc, &argv);

    //     GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    //     GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    //     gtk_container_add(GTK_CONTAINER(window), box);

    //     GtkWidget *stack = gtk_stack_new();
    //     GtkWidget *sidebar = gtk_stack_sidebar_new();
    //     gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(sidebar), GTK_STACK(stack));

    //     gtk_box_pack_start(GTK_BOX(box), sidebar, FALSE, TRUE, 5);
    //     gtk_box_pack_start(GTK_BOX(box), stack, TRUE, TRUE, 5);

    //     GtkWidget *label_stats = gtk_label_new("Stats");
    //     gtk_stack_add_titled(GTK_STACK(stack), label_stats, "stats_page", "Stats");

    //     GtkWidget *scrollable = gtk_scrolled_window_new(NULL, NULL);
    //     GtkWidget *list_box = gtk_list_box_new();

    //     gtk_container_add(GTK_CONTAINER(scrollable), list_box);
    //     gtk_stack_add_titled(GTK_STACK(stack), scrollable, "Vocablist", "Vocablist");

    //     // logic to iterate db

    //     FILE *f = fopen(get_storage_filepath(), "rb");
    //     vocab_entry ve;

    //     while (fread(&ve, sizeof(vocab_entry), 1, f))
    //     {
    //         char buffer[512];
    //         snprintf(buffer,
    //                  sizeof(buffer),
    //                  "%s  | %s ",
    //                  ve.front_text,
    //                  ve.back_text); //! ONly for testing include more information soon (or make it clickable for more infos)
    //         GtkWidget *vocab = gtk_label_new(buffer);
    //         gtk_container_add(GTK_CONTAINER(list_box), vocab);
    //     }

    //     fclose(f);

    //     GtkWidget *label_settings = gtk_label_new("Vocablist");
    //     gtk_stack_add_titled(GTK_STACK(stack), label_settings, "settings", "settings");

    //     g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    //     gtk_widget_show_all(window);
    //     gtk_main();
    return 0;
}