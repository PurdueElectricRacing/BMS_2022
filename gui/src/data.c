#include "data.h"

module_t        modules[MODULE_COUNT];
cells_t         cells[MODULE_COUNT];
faults_t        faults;
debug_t         debug;
app_widgets_usr *wdgts_data;

extern bool run;

void dataInit(app_widgets_usr* wdgts)
{
    wdgts_data = wdgts;
    srand(1000);
    memset(cells->voltsb, 0, sizeof(cells->voltsb));
    memset(cells->volts, 0, sizeof(cells->volts));
    memset(cells->tempsb, 0, sizeof(cells->tempsb));
    memset(cells->temps, 0, sizeof(cells->temps));
}

static void min()
{
    // Locals
    uint8_t i;
    uint8_t j;
    uint32_t vmin;
    uint32_t tmin;

    vmin = tmin = 65535;

    for (i = 0; i < MODULE_COUNT; i++)
    {
        for (j = 0; j < CELL_COUNT; j++)
        {
            if (cells->volts[j] < vmin)
            {
                vmin = cells->volts[j];
            }
        }

        for (j = 0; j < CELL_COUNT; j++)
        {
            if (cells->temps[j] < tmin)
            {
                tmin = cells->temps[j];
            }
        }

        modules[i].min_volts = vmin;
        modules[i].min_temp = tmin;

        vmin = tmin = 65535;
    }
}

static void max()
{
    // Locals
    uint8_t i;
    uint8_t j;
    uint16_t vmax;
    uint16_t tmax;

    vmax = tmax = 0;

    for (i = 0; i < MODULE_COUNT; i++)
    {
        for (j = 0; j < CELL_COUNT; j++)
        {
            if (cells->volts[j] > vmax)
            {
                vmax = cells->volts[j];
            }
        }

        for (j = 0; j < CELL_COUNT; j++)
        {
            if (cells->temps[j] > tmax)
            {
                tmax = cells->temps[j];
            }
        }

        modules[i].max_volts = vmax;
        modules[i].max_temp = tmax;

        vmax = tmax = 0;
    }
}

static void avg()
{
    // Locals
    uint8_t i;
    uint32_t avgv;
    uint32_t avgt;

    avgv = avgt = 0;

    for (i = 0; i < CELL_COUNT; i++)
    {
        avgv += cells->volts[i];
        avgt += cells->temps[i];
    }

    modules[0].avg_volts = ((float) avgv / CELL_COUNT);
    modules[0].avg_temp = ((float) avgt / CELL_COUNT);
}

static void agregate()
{
    // Locals
    uint8_t i;
    uint32_t sum;

    sum = 0;

    for (i = 0; i < CELL_COUNT; i++)
    {
        sum += cells->volts[i];
    }

    modules[0].mod_volts = sum;
}

static void varUpdate()
{
    // Locals
    uint8_t i;

    if (run == 1)
    {
        min();
        max();
        avg();
        agregate();
        for (i = 0; i < CELL_COUNT; i++)
        {
            if (cells->voltsb[i] == 0)
            {
                cells->voltsb[i] = 4100 + (rand() % 100);
            }
            else
            {
                cells->volts[i] = cells->voltsb[i] + (rand() % 2);
            }

            if (cells->tempsb[i] == 0)
            {
                cells->tempsb[i] = 39500 + (rand() % 500);
            }
            else
            {
                cells->temps[i] = cells->tempsb[i] + (rand() % 20);
            }
        }
    }
}

gboolean bgLoop(void* nothing)
{
    // Locals
    static int i;
    char* buffer[CELL_COUNT][10];
    char* min_volts[MODULE_COUNT][10];
    char* min_temps[MODULE_COUNT][10];
    char* max_volts[MODULE_COUNT][10];
    char* max_temps[MODULE_COUNT][10];
    char  avg_volts[10];
    char  avg_temp[10];
    char  mod_volts[10];

    varUpdate();

    for (i = 0; i < CELL_COUNT; i++)
    {
        sprintf((char*) &buffer[i][0], "%.3f V", ((float) cells->volts[i]) / 1000);
    }

    for (i = 0; i < MODULE_COUNT; i++)
    {
        sprintf((char*) &min_volts[0][0], "%.3f V", ((float) modules[i].min_volts) / 1000);
    }

    for (i = 0; i < MODULE_COUNT; i++)
    {
        sprintf((char*) &min_temps[0][0], "%.2f °C", ((float) modules[i].min_temp) / 1000);
    }

    for (i = 0; i < MODULE_COUNT; i++)
    {
        sprintf((char*) &max_volts[0][0], "%.3f V", ((float) modules[i].max_volts) / 1000);
    }

    for (i = 0; i < MODULE_COUNT; i++)
    {
        sprintf((char*) &max_temps[0][0], "%.2f °C", ((float) modules[i].max_temp) / 1000);
    }

    sprintf(avg_volts, "%.3f V", ((float) modules[0].avg_volts) / 1000);
    sprintf(avg_temp, "%.2f °C", ((float) modules[0].avg_temp) / 1000);
    sprintf(mod_volts, "%.2f V", ((float) modules[0].mod_volts) / 1000);

    if (run == true)
    {
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv0), (gchar*) buffer[0]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv1), (gchar*) buffer[1]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv2), (gchar*) buffer[2]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv3), (gchar*) buffer[3]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv4), (gchar*) buffer[4]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv5), (gchar*) buffer[5]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv6), (gchar*) buffer[6]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv7), (gchar*) buffer[7]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv8), (gchar*) buffer[8]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv9), (gchar*) buffer[9]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv10), (gchar*) buffer[10]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_min_volts), (gchar*) min_volts[0]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_min_temp), (gchar*) min_temps[0]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_max_volts), (gchar*) max_volts[0]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_max_temp), (gchar*) max_temps[0]);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_avg_volts), (gchar*) avg_volts);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_avg_temp), (gchar*) avg_temp);
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_mod_volts), (gchar*) mod_volts);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv0), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv1), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv2), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv3), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv4), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv5), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv6), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv7), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv8), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv9), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_cv10), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_min_volts), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_min_volts), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_max_volts), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_max_temp), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_avg_volts), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_avg_temp), "NULL");
        gtk_label_set_text(GTK_LABEL(wdgts_data->label_mod_volts), "NULL");
    }

    return G_SOURCE_CONTINUE;
}