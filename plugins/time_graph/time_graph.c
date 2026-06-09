#include <cairo.h>
#include <gtk/gtk.h>
#include "plugins/plugin.h"
#include "plugins/plugin_utils.h"

const char plugin_name[] = "Time Comparison Graph";
const char plugin_description[] = "Time Comparison Graph (in a window)";
const char plugin_version[] = "0.1";
const char plugin_author[] = "Tepiloxtl";
const abi_version_t abi_version = 0 << 16 | 1; // v0.1

typedef struct {
    unsigned int total_splits;
    unsigned int completed_splits;
    long long* deltas;
    int* info_flags;
} GraphData;

static GraphData run_data = {0, 0, NULL, NULL};
static GtkWidget* graph_window = NULL;
static GtkWidget* drawing_area = NULL;


static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;
    int center_y = height / 2;

    // Background
    cairo_set_source_rgb(cr, 0.08, 0.08, 0.08);
    cairo_paint(cr);

    // Baseline
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.3);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, 0, center_y);
    cairo_line_to(cr, width, center_y);
    cairo_stroke(cr);

    if (run_data.completed_splits < 1 || run_data.total_splits <= 1) {
        return FALSE;
    }

    // Determine a dynamic vertical scale based on the largest absolute delta seen so far
    long long max_abs_delta = 1000; // default minimum boundary (1 second)
    for (unsigned int i = 0; i < run_data.completed_splits; i++) {
        long long abs_val = llabs(run_data.deltas[i]);
        if (abs_val > max_abs_delta) max_abs_delta = abs_val;
    }
    // Scale factor: pixels per unit of time delta
    double scale = (double)(center_y - 20) / max_abs_delta; 

    double last_x = 0;
    double last_y = center_y;

    for (unsigned int i = 0; i < run_data.completed_splits; i++) {
        double next_x = ((double)(i + 1) / run_data.total_splits) * width;
        // Up (ahead) is -y
        double next_y = center_y - (run_data.deltas[i] * scale);

        cairo_move_to(cr, last_x, last_y);
        cairo_line_to(cr, next_x, next_y);

        if (run_data.info_flags[i] & LS_INFO_BEHIND_TIME) {
            cairo_set_source_rgb(cr, 0.85, 0.25, 0.25);
        } else {
            cairo_set_source_rgb(cr, 0.25, 0.85, 0.25);
        }
        
        cairo_set_line_width(cr, 2.5);
        cairo_stroke(cr);

        last_x = next_x;
        last_y = next_y;
    }

    return FALSE;
}

int on_split(const ls_timer* timer) {
    if (!timer || !timer->game) return 0;

    run_data.total_splits = timer->game->split_count;
    run_data.completed_splits = timer->curr_split;

    run_data.deltas = realloc(run_data.deltas, run_data.completed_splits * sizeof(long long));
    run_data.info_flags = realloc(run_data.info_flags, run_data.completed_splits * sizeof(int));

    for (unsigned int i = 0; i < run_data.completed_splits; i++) {
        run_data.deltas[i] = timer->split_deltas[i];
        run_data.info_flags[i] = timer->split_info[i];
    }

    if (drawing_area) {
        gtk_widget_queue_draw(drawing_area);
    }
    return 0;
}

int on_reset(const ls_timer* timer) {
    run_data.completed_splits = 0;
    if (run_data.deltas) { free(run_data.deltas); run_data.deltas = NULL; }
    if (run_data.info_flags) { free(run_data.info_flags); run_data.info_flags = NULL; }
    
    if (drawing_area) {
        gtk_widget_queue_draw(drawing_area);
    }
    return 0;
}

// Window Management
void init_graph_window()
{
    if (graph_window)
        return; // Prevent multiple windows

    // Create a new top-level window
    graph_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(graph_window), "Comparison graph");
    gtk_window_set_default_size(GTK_WINDOW(graph_window), 400, 250);

    // Create the drawing canvas
    drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(graph_window), drawing_area);

    // Connect the Cairo drawing function
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw_event), NULL);

    // gtk_widget_hide_on_delete hides the window instead of destroying it on close.
    g_signal_connect(G_OBJECT(graph_window), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    // Show the window and its contents
    gtk_widget_show_all(graph_window);
}

static gboolean delayed_window_init(gpointer data) {
    init_graph_window();
    return G_SOURCE_REMOVE; // Tells GTK to remove it from the queue
}

int plug_init(PlugAPI* api)
{
    g_idle_add(delayed_window_init, NULL);

    api->register_event_hook(SPLIT, on_split);
    api->register_event_hook(RESET, on_reset);

    return 0;
}

int plug_shutdown()
{
    if (graph_window) {
        gtk_widget_destroy(graph_window);
        graph_window = NULL;
    }
    return 0;
}