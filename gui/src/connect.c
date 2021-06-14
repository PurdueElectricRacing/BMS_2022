#include "connect.h"

app_widgets_usr *wdgts_conn;

extern bool run;

void connInit(app_widgets_usr* wdgts)
{
    wdgts_conn = wdgts;

    gtk_label_set_text(GTK_LABEL(wdgts_conn->label_status_can), "Disconnected");
}

void btn_connect_call()
{
    if (run == false)
    {
        run = true;
        gtk_label_set_text(GTK_LABEL(wdgts_conn->label_conn_can), "500k");
        gtk_label_set_text(GTK_LABEL(wdgts_conn->label_status_can), "Connected");
    }
    else
    {
        run = false;
        gtk_label_set_text(GTK_LABEL(wdgts_conn->label_conn_can), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_conn->label_status_can), "Disconnected");
    }
}