#include "therun.h"
#include "timer.h"
#include <jansson.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>

/**
 * Converts a time in "milliseconds" (microseconds) to a json float in milliseconds.
 * 
 * This should be put in timer.c later probably
 *
 * @param microseconds Time to convert.
 */
json_t* time_to_ms(int64_t microseconds) {
    if (microseconds == LLONG_MAX) { 
        return json_null(); 
    }
    double milliseconds = (double)microseconds / 1000.0;
    return json_real(milliseconds);
}

char* build_therun_live_payload(ls_timer *timer) {
    const char* therun_key = getenv("LIBRESPLIT_THERUN_KEY");
    json_t *root = json_object();

    json_t *metadata = json_object();
    // This is all to split split file title to game and category for API. Should be added as fields into
    // JSON at some point, alongside platform, region and emulator maybe?
    const char *game_title = timer->game->title;
    char *pipe_pos = strchr(game_title, '|');
    if (pipe_pos != NULL) {
        size_t game_len = pipe_pos - game_title;
        while (game_len > 0 && game_title[game_len - 1] == ' ') {
            game_len--;
        }
        json_object_set_new(metadata, "game", json_stringn(game_title, game_len));
        const char *category_start = pipe_pos + 1;
        while (*category_start == ' ') {
            category_start++;
        }
        json_object_set_new(metadata, "category", json_string(category_start));
    } else {
        json_object_set_new(metadata, "game", json_string(game_title));
        json_object_set_new(metadata, "category", json_string(""));
    }
    json_object_set_new(metadata, "platform", json_string(""));
    json_object_set_new(metadata, "region", json_string(""));
    json_object_set_new(metadata, "emulator", json_false());
    json_object_set_new(metadata, "variables", json_string("")); //Empty in my dumps
    json_object_set_new(root, "metadata", metadata);

    json_t *runData = json_array();
    for (int i = 0; i < timer->game->split_count; i++) {
        json_t *segment = json_object();
        json_object_set_new(segment, "name", json_string(timer->game->split_titles[i]));
        json_object_set_new(segment, "splitTime", time_to_ms(timer->split_times[i]));
        json_object_set_new(segment, "pbSplitTime", time_to_ms(timer->best_splits[i])); //is this correct? Subject to test out
        json_object_set_new(segment, "bestPossible", time_to_ms(timer->best_segments[i]));
        json_object_set_new(segment, "comparisons", json_array()); //empty for now, this contains Personal Best, Best Segments, Average Segments fields in LiveSplit dumps
        json_array_append_new(runData, segment);
    }
    json_object_set_new(root, "runData", runData);

    json_object_set_new(root, "currentTime", time_to_ms(timer->time));
    json_object_set_new(root, "currentSplitName", json_string(timer->game->split_titles[timer->curr_split]));
    json_object_set_new(root, "currentSplitIndex", json_integer(timer->curr_split));
    json_object_set_new(root, "timingMethod", json_integer(0)); //NYI, 0 in my dumps, maybe says either its RTA or IGT
    json_object_set_new(root, "currentDuration", time_to_ms(timer->time)); //NYI, Time with pauses, for now just time
    json_object_set_new(root, "startTime", json_integer(0)); //NYI
    json_object_set_new(root, "endTime", json_integer(0)); //NYI
    json_object_set_new(root, "uploadKey", json_string(therun_key));
    json_object_set_new(root, "isPaused", json_false()); //NYI, for now false
    json_object_set_new(root, "isGameTimePaused", json_false()); //NYI, for now false
    json_object_set_new(root, "gameTimePauseTime", json_null()); //NYI, for now null
    json_object_set_new(root, "totalPauseTime", json_null()); //NYI, for now null
    json_object_set_new(root, "currentPauseTime", json_null()); //NYI, for now null
    json_object_set_new(root, "timePausedAt", json_integer(0)); //NYI, for now 0
    json_object_set_new(root, "wasJustResumed", json_false()); //NYI, for now false
    json_object_set_new(root, "currentComparison", json_string("Personal Best")); //NYI, for now PB

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

void therun_trigger_update(ls_timer *timer) {
    char *payload = build_therun_live_payload(timer);
    if (!payload) return;

    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://dspc6ekj2gjkfp44cjaffhjeue0fbswr.lambda-url.eu-west-1.on.aws/");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "[therun.gg] FAILED: %s\n", curl_easy_strerror(res));
        } else {
            fprintf(stderr, "[therun.gg] OK!\n");
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(payload);
    }
}