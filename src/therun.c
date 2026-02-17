#include "timer.h"
#include <jansson.h>
#include <time.h>
#include <limits.h>

json_t* time_to_ms(int64_t microseconds) {
    if (microseconds == LLONG_MAX) { 
        return json_null(); 
    }
    double milliseconds = (double)microseconds / 1000.0;
    return json_real(milliseconds);
}

char* build_therun_live_payload(ls_timer *timer) {
    json_t *root = json_object();
    json_t *metadata = json_object();
    json_object_set_new(metadata, "game", json_string(timer->game->title)); //Yeah, we don't have anything else than split title to work with
    json_object_set_new(metadata, "category", json_string(timer->game->title)); //I guess I could like split string on | as a crutch for now
    json_object_set_new(metadata, "platform", json_string(""));
    json_object_set_new(metadata, "region", json_string(""));
    json_object_set_new(metadata, "emulator", json_boolean(false)); //It's a bool in LiveSplit I think
    json_object_set_new(metadata, "variables", json_string("")); //No idea
    json_object_set_new(root, "metadata", metadata);

    json_t *runData = json_array();
    for (int i = 0; i < timer->game->split_count; i++) {
        json_t *segment = json_object();
        json_object_set_new(segment, "name", json_string(timer->game->split_titles[i]));
        json_object_set_new(segment, "splitTime", time_to_ms(timer->split_times[i]));
        json_object_set_new(segment, "pbSplitTime", time_to_ms(timer->best_splits[i])); //is this correct? Subject to test out
        json_object_set_new(segment, "bestPossible", time_to_ms(timer->best_segments[i]));
        // Comparison goes here, whatever it is, like comparison to best time??
        json_array_append_new(runData, segment);
    }
    json_object_set_new(root, "runData", runData);

    json_object_set_new(root, "currentTime", json_integer(0));
    json_object_set_new(root, "currentSplitName", json_integer(0));
    json_object_set_new(root, "currentSplitIndex", json_integer(0));
    json_object_set_new(root, "timingMethod", json_integer(0));
    json_object_set_new(root, "currentDuration", json_integer(0));
    json_object_set_new(root, "startTime", json_integer(0));
    json_object_set_new(root, "endTime", json_integer(0));
    json_object_set_new(root, "uploadKey", json_integer(0));
    json_object_set_new(root, "isPaused", json_integer(0));
    json_object_set_new(root, "isGameTimePaused", json_integer(0));
    json_object_set_new(root, "gameTimePauseTime", json_integer(0));
    json_object_set_new(root, "totalPauseTime", json_integer(0));
    json_object_set_new(root, "currentPauseTime", json_integer(0));
    json_object_set_new(root, "timePausedAt", json_integer(0));
    json_object_set_new(root, "wasJustResumed", json_integer(0));
    json_object_set_new(root, "currentComparison", json_integer(0));

    char *payload = json_dumps(root, 1);

    time_t rawtime;
    struct tm* timeinfo;
    char time_buf[64];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d_%H-%M-%S", timeinfo);
    char filename[PATH_MAX];
    snprintf(filename, sizeof(filename), "build/test-%s.json", time_buf);
    json_dump_file(root, filename, JSON_PRESERVE_ORDER | JSON_INDENT(2));
    json_decref(root);
    fprintf(stderr, payload);

    return payload;
}