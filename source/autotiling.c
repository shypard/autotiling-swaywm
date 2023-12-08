#define _GNU_SOURCE

#include <cJSON.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <swayipc.h>
#include <time.h>

/* Global variables */
int running = 1;
#define GET_TREE_OUTPUT_LEN 64000
#define SLEEP_TIME_NS 1000 * 1000 * 10 // 0.5 milliseconds

/* signal handler */
void signal_handler(int signal_num) { running = 0; }

/* split windows */
void split(cJSON* json)
{
    cJSON* current = NULL;

    if ((current = cJSON_GetObjectItemCaseSensitive(json, "container")) == NULL)
        return;

    /* check if fullscreen */
    cJSON* fullscreen =
        cJSON_GetObjectItemCaseSensitive(current, "fullscreen_mode");
    if (fullscreen != NULL && cJSON_IsTrue(fullscreen)) return;

    /* get window dimensions */
    cJSON* rect = cJSON_GetObjectItemCaseSensitive(current, "rect");
    if (rect == NULL || !cJSON_IsObject(rect)) return;

    cJSON* w = cJSON_GetObjectItemCaseSensitive(rect, "width");
    cJSON* h = cJSON_GetObjectItemCaseSensitive(rect, "height");

    if (w == NULL || !cJSON_IsNumber(w) || h == NULL || !cJSON_IsNumber(h))
        return;

    /* set new split mode depending on window dimensions */
    char* split_mode = "splitv";
    if (w->valueint > h->valueint) split_mode = "splith";

    /* pass split mode to sway */
    swayipc_send_command(split_mode, strlen(split_mode));
}

int main(void)
{
    cJSON*          json  = NULL;
    struct timespec ts    = {0, SLEEP_TIME_NS}; // 100 milliseconds
    enum event_type sub[] = {SWAY_EVENT_WINDOW, SWAY_EVENT_WORKSPACE};

    /* register signal handlers */
    signal(SIGINT, signal_handler);  // SIGINT: Ctrl+C
    signal(SIGTERM, signal_handler); // SIGTERM: Termination signal

    /* initialize swayipc */
    swayipc_init();

    /* subscribe to WINDOW and WORKSPACE events */
    swayipc_subscribe(sub, 1);

    /* loop until swayipc is closed */
    while (running) {
        /* sleep a few milliseconds to avoid 100% CPU usage */
        nanosleep(&ts, NULL);

        /* get last event from event_queue */
        event_s* last_event = swayipc_get_event();

        /* if no event is available, continue */
        if (last_event == NULL) continue;

        if (last_event->type == SWAY_EVENT_WINDOW ||
            last_event->type == SWAY_EVENT_WORKSPACE) {
            /* parse JSON data */
            if ((json = cJSON_Parse(last_event->data)) == NULL) {
                printf("autotiling: error before: [%s]\n", cJSON_GetErrorPtr());
                continue;
            }

            /* split windows */
            split(json);
        }
    }

    cJSON_Delete(json);

    /* shutdown swayipc */
    swayipc_shutdown();

    return 0;
}
