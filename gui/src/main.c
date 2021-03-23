#include <gtk-3.0/gtk/gtk.h>

typedef struct {
    GtkWidget *w_txtvw_main;            // Pointer to text view object
    GtkWidget *w_dlg_file_choose;       // Pointer to file chooser dialog box
    GtkTextBuffer *textbuffer_main;     // Pointer to text buffer
} app_widgets;

int main(int argc, char *argv[])
{
    GtkBuilder *builder;
    GtkWidget  *window;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "glade/window_main.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(builder);

    gtk_widget_show(window);
    gtk_main();

    return 0;
}

void on_menuitm_open_activate(GtkMenuItem *menuitem, app_widgets *app_wdgts)
{

}

void on_menuitm_close_activate(GtkMenuItem *menuitem, app_widgets *app_wdgts)
{

}

void on_menuitm_quit_activate(GtkMenuItem *menuitem, app_widgets *app_wdgts)
{
    gtk_main_quit();
}

void on_window_main_destroy()
{
    gtk_main_quit();
}