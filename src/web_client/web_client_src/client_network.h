#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H

#define AUDIO_OUTPUT_SOCKET_PATH "ingenic_audio_output"
#define AUDIO_CONTROL_SOCKET_PATH "ingenic_audio_control"

#define AUDIO_OUTPUT_REQUEST 2

// Function declarations
int setup_client_connection(int request_type);
int setup_control_client_connection();
void close_client_connection(int sockfd);

#endif // CLIENT_NETWORK_H
