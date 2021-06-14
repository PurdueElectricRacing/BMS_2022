#ifndef _MAIN_H_
#define _MAIN_H_

// Structures
typedef struct {
    GtkWidget *w_txtvw_main;            // Pointer to text view object
    GtkWidget *w_dlg_file_choose;       // Pointer to file chooser dialog box
    GtkTextBuffer *textbuffer_main;     // Pointer to text buffer
} app_widgets;

typedef struct {
    GtkWidget* label_conn_can;
    GtkWidget* label_status_can;
    GtkWidget* label_conn_status;
    GtkWidget* label_conn_type;
    GtkWidget* label_min_volts;
    GtkWidget* label_min_temp;
    GtkWidget* label_max_volts;
    GtkWidget* label_max_temp;
    GtkWidget* label_avg_volts;
    GtkWidget* label_avg_temp;
    GtkWidget* label_mod_volts;
    GtkWidget* label_cv0;
    GtkWidget* label_cv1;
    GtkWidget* label_cv2;
    GtkWidget* label_cv3;
    GtkWidget* label_cv4;
    GtkWidget* label_cv5;
    GtkWidget* label_cv6;
    GtkWidget* label_cv7;
    GtkWidget* label_cv8;
    GtkWidget* label_cv9;
    GtkWidget* label_cv10;
    GtkWidget* label_test;
    GtkWidget* label_count;
    int i;
} app_widgets_usr;

// Function prototypes
void initItems();
void btn_callback(GtkMenuItem *menuitem, app_widgets *app_wdgts);

#endif