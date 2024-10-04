#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include <sys/stat.h>

#define LOG_FILE "/var/log/auth.log"
#define FAILED_LOGIN_MSG "Failed password"


void check_failed_logins(FILE *fp) {
        if (!fp) {
                perror("Failed to open log file");
                exit(EXIT_FAILURE);
        }

        char line[1024];
        while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, FAILED_LOGIN_MSG)) {
                        printf("Failed login attempt detected:\n%s\n", line);
                        send_data_over_mqtt(FAILED_LOGIN);
                }
        }

        fclose(fp);
}

// Function to be executed by user login thread
void* thread_failed_user_login(void* threadid)
{
        printf("Monitoring for failed login attempts...\n");
        struct stat st;
        int size =0;
        int offset=0;
        while (1)
        {
                FILE *log_file = fopen(LOG_FILE, "r");
                stat(LOG_FILE, &st);
                size = st.st_size;
                if(offset==0)
                {
                        //check_failed_logins(log_file);
                        offset=size;
                }
                else if(offset<size)
                {
                        fseek(log_file, offset, SEEK_SET);
                        check_failed_logins(log_file);
                        offset=size;
                }
                sleep(10);  // Check the log file every 10 seconds
        }

        /*    while (1) {
              check_failed_logins();
              sleep(10);  // Check the log file every 10 seconds
              }
              */
        pthread_exit(NULL);
        return 0;
}