#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define RELEASE_FILE "/usr/share/zyphor-whats-new/release-notes.md"

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *scrolled;
    GtkWidget *text_view;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Zyphor What's New");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 500);

    scrolled = gtk_scrolled_window_new();
    gtk_window_set_child(GTK_WINDOW(window), scrolled);

    text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), text_view);

    FILE *file = fopen(RELEASE_FILE, "r");
    if (file)
    {
        char buffer[1024];
        GtkTextBuffer *buffer_text = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

        while (fgets(buffer, sizeof(buffer), file))
        {
            gtk_text_buffer_insert_at_cursor(buffer_text, buffer, -1);
        }

        fclose(file);
    }
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.zyphor.whatsnew", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}