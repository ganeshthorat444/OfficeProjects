#ifndef COMMON_H
#include <cjson/cJSON.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/* Prototypes for C thread functions */
void* thread_user_login(void* threadid);
void* thread_usb_connection_disconnection(void* threadid);
void* thread_failed_user_login(void* threadid);
void* thread_file_operations(void* threadid);

int send_data_over_mqtt(int);

#define SUCCESSFUL_LOGIN 1
#define FAILED_LOGIN 2
#define USB_CONNECTED 3
#define USB_DISCONNECTED 4
#define FILE_CREATED 5
#define FILE_DELETED 6
#define FILE_UPDATED 7



#define ERROR_TYPE_USB                          (0x00000001)
#define ERROR_TYPE_LOGIN                        (0x00000010)
#define ERROR_TYPE_FILE_OPS                     (0x00000100)
#define ERROR_TYPE_NW_PORT_CHANGES              (0x00001000)

#define ERROR_USB_CONNECTION            ERROR_TYPE_USB+1
#define ERROR_USB_DISCONNECTION         ERROR_TYPE_USB+2

#define ERROR_LOGIN_DETECTED            ERROR_TYPE_LOGIN+1
#define ERROR_FAILED_LOGIN_DETECTED     ERROR_TYPE_LOGIN+2

#define ERROR_FILE_OPS_DETECTED         ERROR_TYPE_FILE_OPS+1


#endif //COMMON_H