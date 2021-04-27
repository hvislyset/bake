#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <curl/curl.h>

#include "../include/target.h"

BakeTargetList *new_target_list()
{
    BakeTargetList *target_list = calloc(3, sizeof(BakeTargetList));

    if (!target_list)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    target_list->size = TARGET_LIST_INITIAL_SIZE;
    target_list->targets = calloc(target_list->size, sizeof(BakeTargetList));

    if (!target_list->targets)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    return target_list;
}

void target_list_add(BakeTargetList *target_list, const char *name)
{
    BakeTarget *target = find_target(target_list, name);

    if (!target)
    {
        target = new_target(name);

        if (target_list->count == 0)
        {
            target->is_default_target = true;
        }

        target->key = target_list->count;
        target_list->targets[target_list->count++] = target;
    }
}

void free_target_list(BakeTargetList *target_list)
{
    for (size_t i = 0; i < target_list->size; i++)
    {
        BakeTarget *target = target_list->targets[i];

        if (target)
        {
            free_target(target);
        }
    }

    free(target_list->targets);
    free(target_list);
}

BakeTarget *new_target(const char *name)
{
    BakeTarget *target = calloc(9, sizeof(BakeTarget));

    if (!target)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    target->name = strdup(name);
    target->actions_size = ACTIONS_INITIAL_SIZE;
    target->actions = calloc(target->actions_size, sizeof(char *));

    if (!target->actions)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    target->dependencies_size = DEPENDENCIES_INITIAL_SIZE;
    target->dependencies = calloc(target->dependencies_size, sizeof(BakeTarget));

    if (!target->dependencies)
    {
        fprintf(stderr, "No memory\n");

        exit(EXIT_FAILURE);
    }

    return target;
}

BakeTarget *find_target(BakeTargetList *target_list, const char *name)
{
    for (size_t i = 0; i < target_list->size; i++)
    {
        BakeTarget *target = target_list->targets[i];

        if (target && strcmp(target->name, name) == 0)
        {
            return target;
        }
    }

    return NULL;
}

BakeTarget *find_default_target(BakeTargetList *target_list)
{
    for (size_t i = 0; i < target_list->size; i++)
    {
        BakeTarget *target = target_list->targets[i];

        if (target && target->is_default_target)
        {
            return target;
        }
    }

    return NULL;
}

void add_target_dependencies(BakeTargetList *target_list, BakeTarget *target, size_t dependencies_count, char **dependencies)
{
    for (size_t i = 0; i < dependencies_count; i++)
    {
        BakeTarget *existing_target = find_target(target_list, dependencies[i]);

        if (!existing_target)
        {
            target_list_add(target_list, dependencies[i]);

            target->dependencies[target->dependencies_count++] = find_target(target_list, dependencies[i]); // TODO: WE JUST CREATED IT, SO SURELY WE SHOULDN'T HAVE TO SCAN THE WHOLE LIST
        }
        else
        {
            target->dependencies[target->dependencies_count++] = existing_target;
        }
    }
}

void free_target(BakeTarget *target)
{
    free(target->name);

    for (size_t i = 0; i < target->actions_count; i++)
    {
        free(target->actions[i]);
    }

    free(target->actions);
    free(target->dependencies);
    free(target);
}

void add_target_action(BakeTarget *target, const char *action)
{
    target->actions[target->actions_count++] = strdup(action);
}

bool target_requires_rebuilding(BakeTarget *target)
{
    struct stat buf = {0};

    stat(target->name, &buf);

    if (target->dependencies_count == 0)
    {
        return true;
    }

    for (size_t dep = 0; dep < target->dependencies_count; dep++)
    {
        BakeTarget *dependency = target->dependencies[dep];
        struct stat dep_buf = {0};

        if (strncmp(dependency->name, "file://", 7) == 0 || strncmp(dependency->name, "http://", 7) == 0 || strncmp(dependency->name, "https://", 8) == 0)
        {
            CURL *curl = curl_easy_init();
            long *filetime;

            if (curl)
            {
                curl_easy_setopt(curl, CURLOPT_URL, dependency->name);
                curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
                curl_easy_setopt(curl, CURLOPT_FILETIME, 1L);

                CURLcode res = curl_easy_perform(curl);

                if (res != CURLE_OK)
                {
                    fprintf(stderr, "Error accessing external resource\n");

                    exit(EXIT_FAILURE);
                }
                else
                {
                    curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);

                    if (res == CURLE_OK)
                    {
                        time_t file_time = (time_t)filetime;

                        if (file_time > buf.st_mtime)
                        {
                            curl_easy_cleanup(curl);

                            return true;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Error accessing external resource\n");

                        exit(EXIT_FAILURE);
                    }
                }

                curl_easy_cleanup(curl);

                return false;
            }
        }

        if (stat(dependency->name, &dep_buf) == 0)
        {
            if (dep_buf.st_mtime > buf.st_mtime)
            {
                return true;
            }
        }
        else
        {
            return true;
        }
    }

    return false;
}