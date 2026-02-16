#include "timer.h"
#include <jansson.h>

char* build_therun_live_payload(ls_timer *timer) {
    printf("tesrt");
    json_t *root = json_object();
    json_t *metadata = json_object();
    json_object_set_new(metadata, "game", json_string(timer->game->title));
    json_object_set_new(root, "metadata", metadata);

    char *payload = json_dumps(root, 0);
    json_decref(root);
    fprintf(stderr, payload);

    return payload;
}