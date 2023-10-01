#ifndef _HELPERS_
#define _HELPERS_

#define LOGIN       2
#define ENTER_LIB   3
#define BOOKS       4
#define BOOK        5
#define ADD_BOOK    6
#define DEL         7
#define LOGOUT      8
#define EXIT_SV     9
#define TYPE        14
#define VAR_LEN     50
#define BOOK_DET    100
#define BUFLEN      4096
#define LINELEN     1000

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

#endif