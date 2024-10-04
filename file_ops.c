#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>
#include "common.h"

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void handle_event(struct inotify_event *i) {
    int ecode = 0;
    if (i->mask & IN_MODIFY) {
        printf("File modified: %s\n", i->name);
        ecode = FILE_UPDATED;
    }
    if (i->mask & IN_CREATE) {
        printf("File created: %s\n", i->name);
        ecode = FILE_CREATED;
    }
    if (i->mask & IN_DELETE) {
        printf("File deleted: %s\n", i->name);
        ecode = FILE_DELETED;
    }
    send_data_over_mqtt(ecode);
}

// Function to be executed by user login thread
void* thread_file_operations(void* VarPath)
{

    const char *path = (char *)VarPath;

    int inotify_fd = inotify_init();
    if (inotify_fd == -1) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    int wd = inotify_add_watch(inotify_fd, path, IN_MODIFY | IN_CREATE | IN_DELETE);
    if (wd == -1) {
        perror("inotify_add_watch");
        close(inotify_fd);
        exit(EXIT_FAILURE);
    }

    printf("Monitoring %s for file modifications...\n", path);

    char buf[BUF_LEN] __attribute__((aligned(8)));
    ssize_t numRead;

    while (1) {
        numRead = read(inotify_fd, buf, BUF_LEN);
        if (numRead == 0) {
            fprintf(stderr, "read() from inotify fd returned 0!\n");
            exit(EXIT_FAILURE);
        }

        if (numRead == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        for (char *ptr = buf; ptr < buf + numRead;) {
            struct inotify_event *event = (struct inotify_event *) ptr;
            handle_event(event);
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }

    close(wd);
    close(inotify_fd);
        pthread_exit(NULL);
    return 0;
}
~       