#ifndef CONTROL_SERVER_H
#define CONTROL_SERVER_H

extern char AUDIO_CONTROL_SOCKET_PATH[];

#define CLIENT_QUEUED 1
#define CLIENT_NOT_QUEUED 0

#define AUDIO_INPUT_REQUEST 1
#define AUDIO_OUTPUT_REQUEST 2

// Control commands
#define CONTROL_GET_COMMAND 3
#define CONTROL_SET_COMMAND 4

// Response codes
#define RESPONSE_OK 200
#define RESPONSE_ERROR 400
#define RESPONSE_UNKNOWN_VARIABLE 404

// Functions
void *audio_control_server_thread(void *arg);
void handle_control_client(int client_sock);

#endif // CONTROL_SERVER_H
