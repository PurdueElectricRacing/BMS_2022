#include <gtk-3.0/gtk/gtk.h>
#include <stdlib.h>
#include "main.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "connect.h"
#include "data.h"

app_widgets_usr wdgts;
GtkBuilder* builder;
bool run;

int main(int argc, char *argv[])
{
    GtkWidget  *window;

    run = 0;
    wdgts.i = 0;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "glade/window_main.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, NULL);

    initItems();
    dataInit(&wdgts);
    connInit(&wdgts);

    if(checkOnlyProcess())
    {
        return EXIT_FAILURE;
    }

    gdk_threads_add_timeout(1000, bgLoop, NULL);

    g_object_unref(builder);

    gtk_widget_show(window);
    gtk_main();

    return EXIT_SUCCESS;
}

static void hardFault(uint8_t type)
{
    // Locals
    GtkDialogFlags flags;
    GtkWidget*     dialog;

    flags = GTK_DIALOG_DESTROY_WITH_PARENT;

    switch (type)
    {
        case PROCESS_OPEN:
            dialog = gtk_message_dialog_new(NULL, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error: Cannot open GUI with another instance running");
            break;

        case BAD_HOME:
            dialog = gtk_message_dialog_new(NULL, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error: Improper home directory set");
            break;
        
        default:
            dialog = gtk_message_dialog_new(NULL, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error: Something went wrong");
    }

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

int checkOnlyProcess()
{
    // Locals
    FILE*  fptr;
    char*  dir;
    size_t dirlen;
    char*  fpath;
    char   buff[9];
    int    ret;

    ret = 0;

    sprintf(buff, "%s", "bms_open");
    dir = getenv("HOME");
    if (dir == NULL || dir[0] != '/')
    {
        hardFault(BAD_HOME);
    }

    dirlen = strlen(dir);
    fpath = malloc(sizeof(dirlen + 3));
    
    if (fpath == NULL)
    {
        hardFault(255);
    }

    sprintf(fpath, "%s%s", dir, "/1");
    fptr = fopen(fpath, "r");
    
    if (fptr == NULL)
    {
        fptr = fopen(fpath, "wb+");

        if (fptr == NULL)
        {
            hardFault(255);
        }
        else
        {
            fwrite(buff, sizeof(char), sizeof("bms_open"), fptr);
        }
    }
    else
    {
        hardFault(PROCESS_OPEN);
        ret = -1;
    }

    fclose(fptr);
    return ret;
}

void initItems()
{
    wdgts.label_conn_can = GTK_WIDGET(gtk_builder_get_object(builder, "label_conn_can"));
    wdgts.label_status_can = GTK_WIDGET(gtk_builder_get_object(builder, "label_status_can"));
    wdgts.label_cv0 = GTK_WIDGET(gtk_builder_get_object(builder, "label_cv0"));
    wdgts.label_cv1 = GTK_WIDGET(gtk_builder_get_object(builder, "label_cv1"));
    wdgts.label_cv2 = GTK_WIDGET(gtk_builder_get_object(builder, "label_cv2"));
    wdgts.label_cv3 = GTK_WIDGET(gtk_builder_get_object(builder, "label_cv3"));
    wdgts.label_cv4 = GTK_WIDGET(gtk_builder_get_object(builder, "label_cv4"));
    wdgts.label_cv5 = GTK_WIDGET(gtk_builder_get_object(builder, "label_cv5"));
    wdgts.label_cv6 = GTK_WIDGET(gtk_builder_get_object(builder, "label_cv6"));
    wdgts.label_cv7 = GTK_WIDGET(gtk_builder_get_object(builder, "label_cv7"));
    wdgts.label_cv8 = GTK_WIDGET(gtk_builder_get_object(builder, "label_cv8"));
    wdgts.label_cv9 = GTK_WIDGET(gtk_builder_get_object(builder, "label_cv9"));
    wdgts.label_cv10 = GTK_WIDGET(gtk_builder_get_object(builder, "label_cv10"));
    wdgts.label_min_volts = GTK_WIDGET(gtk_builder_get_object(builder, "label_min_volts"));
    wdgts.label_min_temp = GTK_WIDGET(gtk_builder_get_object(builder, "label_min_temp"));
    wdgts.label_max_volts = GTK_WIDGET(gtk_builder_get_object(builder, "label_max_volts"));
    wdgts.label_max_temp = GTK_WIDGET(gtk_builder_get_object(builder, "label_max_temp"));
    wdgts.label_avg_volts = GTK_WIDGET(gtk_builder_get_object(builder, "label_avg_volts"));
    wdgts.label_avg_temp = GTK_WIDGET(gtk_builder_get_object(builder, "label_avg_temp"));
    wdgts.label_mod_volts = GTK_WIDGET(gtk_builder_get_object(builder, "label_mod_volts"));
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

void btn_callback(GtkMenuItem *menuitem, app_widgets *app_wdgts)
{
    // Locals
    char buffer[10];

    sprintf(buffer, "%d", ++wdgts.i);

    gtk_label_set_text(GTK_LABEL(wdgts.label_test), "Fuck");
    gtk_label_set_text(GTK_LABEL(wdgts.label_count), buffer);
}

void on_window_main_destroy(GtkMenuItem *menuitem, app_widgets *app_wdgts)
{
    // Locals
    FILE*  fptr;
    char*  dir;
    size_t dirlen;
    char*  fpath;
    char   buff[9];

    sprintf(buff, "%s", "bms_open");
    dir = getenv("HOME");
    if (dir == NULL || dir[0] != '/')
    {
        hardFault(BAD_HOME);
    }

    dirlen = strlen(dir);
    fpath = malloc(sizeof(dirlen + 3));
    
    if (fpath == NULL)
    {
        hardFault(255);
    }

    sprintf(fpath, "%s%s", dir, "/1");

    fptr = fopen(fpath, "r");
    
    if (fptr != NULL)
    {
        fclose(fptr);
        remove(fpath);
    }
    else
    {
        fclose(fptr);
    }

    gtk_main_quit();
}