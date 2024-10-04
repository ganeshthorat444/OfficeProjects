#include <stdio.h>
#include <stdlib.h>
#include <utmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "common.h"

#define WTMP_FILE "/var/log/wtmp"
#define POLL_INTERVAL 10 // Polling interval in seconds

void print_login_entry(struct utmp *entry) {
    printf("New login detected:\n");
    printf("User: %s\n", entry->ut_user);
    printf("Terminal: %s\n", entry->ut_line);
    printf("Host: %s\n", entry->ut_host);
    printf("Time: %s", ctime(&(entry->ut_tv.tv_sec)));
    send_data_over_mqtt(SUCCESSFUL_LOGIN);
}

void* thread_user_login(void *param)
{
    int fd = open(WTMP_FILE, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    off_t last_position = lseek(fd, 0, SEEK_END); // Start by reading from the end of the file

    while (1) {
        struct utmp entry;
        off_t current_position = lseek(fd, 0, SEEK_CUR);

        // Move file pointer to the last read position
        lseek(fd, last_position, SEEK_SET);

        while (read(fd, &entry, sizeof(struct utmp)) == sizeof(struct utmp)) {
            if (entry.ut_type == USER_PROCESS) {
                print_login_entry(&entry);
            }
        }

        // Update the last read position
        last_position = lseek(fd, 0, SEEK_CUR);

        // Sleep before the next poll
        sleep(POLL_INTERVAL);
    }


    close(fd);
    return EXIT_SUCCESS;
}
