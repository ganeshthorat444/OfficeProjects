#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "common.h"
#include <mosquitto.h>

#define HOST "localhost"
#define PORT 3003
#define KEEPALIVE 0
#define TOPIC "event/deviceNotification"
#define MESSAGE "Hello, MQTT from C!"

#define ALERT_TYPE_CRITICAL     "Critical"
#define ALERT_TYPE_WARNING      "Warning"
#define ALERT_TYPE_INFO         "Information"

#define MAX_PACKET_SIZE 256

static struct mosquitto *mosq;

int init_mqtt()
{
    int rc;

    mosquitto_lib_init();
    mosq = mosquitto_new("CClientPublisher", true, NULL);
    if(!mosq){
        fprintf(stderr, "Error: Unable to create Mosquitto client.\n");
        return 1;
    }

    rc = mosquitto_connect(mosq, HOST, PORT, KEEPALIVE);
    if(rc != MOSQ_ERR_SUCCESS){
        fprintf(stderr, "Unable to connect (%d): %s\n", rc, mosquitto_strerror(rc));
        return rc;
    }
    return 0;
}



int send_data_over_mqtt(int ecode)
{
        int rc;
        // Create a JSON object
        cJSON *root = cJSON_CreateObject();
        switch(ecode)
        {
        case USB_CONNECTED:
                cJSON_AddStringToObject(root, "eventcode", "USBC");
                cJSON_AddStringToObject(root, "eventmessage", "USB has been plugged in to the system");
                cJSON_AddStringToObject(root, "eventseverity", ALERT_TYPE_CRITICAL);
                break;
        case USB_DISCONNECTED:
                cJSON_AddStringToObject(root, "eventcode", "USBD");
                cJSON_AddStringToObject(root, "eventmessage", "USB has been ejected from the system");
                cJSON_AddStringToObject(root, "eventseverity", ALERT_TYPE_CRITICAL);
                break;
        case SUCCESSFUL_LOGIN:
                cJSON_AddStringToObject(root, "eventcode", "SLOGIN");
                cJSON_AddStringToObject(root, "eventmessage", "Successful Login");
                cJSON_AddStringToObject(root, "eventseverity", ALERT_TYPE_INFO);
                break;
        case FAILED_LOGIN:
                cJSON_AddStringToObject(root, "eventcode", "FLOGIN");
                cJSON_AddStringToObject(root, "eventmessage", "Login Failed");
                cJSON_AddStringToObject(root, "eventseverity", ALERT_TYPE_WARNING);
                break;
        case FILE_CREATED:
                cJSON_AddStringToObject(root, "eventcode", "FCREATE");
                cJSON_AddStringToObject(root, "eventmessage", "File has been created");
                cJSON_AddStringToObject(root, "eventseverity", ALERT_TYPE_CRITICAL);
                break;
        case FILE_UPDATED:
                cJSON_AddStringToObject(root, "eventcode", "FEDIT");
                cJSON_AddStringToObject(root, "eventmessage", "File has been edited");
                cJSON_AddStringToObject(root, "eventseverity", ALERT_TYPE_WARNING);
                break;
        case FILE_DELETED:
                cJSON_AddStringToObject(root, "eventcode", "FDELETE");
                cJSON_AddStringToObject(root, "eventmessage", "File has been deleted from the system");
                cJSON_AddStringToObject(root, "eventseverity", ALERT_TYPE_CRITICAL);
                break;
        }
        // Add some key-value pairs

        // Convert to string
        char *string = cJSON_Print(root);
        printf("%s\n", string);

        rc = mosquitto_publish(mosq, NULL, TOPIC, strlen(string), string, 0, false);
        if(rc != MOSQ_ERR_SUCCESS)
        {
                fprintf(stderr, "Unable to publish (%d): %s\n", rc, mosquitto_strerror(rc));
                return 1;
        }

        return 0;
}

int main(int argc, char* argv[])
{
        if (argc < 2)
        {
                fprintf(stderr, "Usage: %s <path_to_monitor>\n", argv[0]);
                exit(EXIT_FAILURE);
        }

    const char *path = argv[1];

    pthread_t tuser_login={0}, tuser_failed_login={0}, tfile_operations={0}, tusb_connection={0};
    int rc;
    long t;

        rc = pthread_create(&tuser_login, NULL, thread_user_login, (void *)t);
        if (rc)
        {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
        }

        rc = pthread_create(&tuser_failed_login, NULL, thread_failed_user_login, (void *)t);
        if (rc)
        {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
        }
        rc = pthread_create(&tusb_connection, NULL, thread_usb_connection_disconnection, (void *)t);
        if (rc)
        {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
        }
        rc = pthread_create(&tfile_operations, NULL, thread_file_operations, (void *)path);
        if (rc)
        {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
        }

        rc = init_mqtt();
        if(rc)
        {
                printf("ERROR; return code from init_mqtt is %d\n", rc);
                exit(-1);
        }

        //send_data_over_mqtt(MESSAGE);

        // Wait for all threads to complete
        pthread_join(tuser_login, NULL);
        pthread_join(tuser_failed_login, NULL);
        pthread_join(tusb_connection, NULL);
        pthread_join(tfile_operations, NULL);


        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();

        while (1)
        {
        }

    printf("All threads completed.\n");
    pthread_exit(NULL);
}
           