#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define SERVER "34.254.242.81"
#define PORT 8080
#define PTYPE "application/json"
#define SIZE_INPUT 100


typedef enum {
	CMD_INVALID = -1,
	CMD_REGISTER = 0,
	CMD_LOGIN,
	CMD_ENTER_LIBRARY,
	CMD_GET_BOOKS,
	CMD_GET_BOOK,
	CMD_ADD_BOOK,
	CMD_DELETE_BOOK,
	CMD_LOGOUT,
	CMD_EXIT
} Command;


// function to create and send message
char* create_and_send_msg(int sockfd, char *request, char *url, char *content, char **cookies, int cookies_count, char *auth) {
    char *message, *server_resp;
    
    if (strcmp(request, "GET") == 0) {
        message = compute_get_request(SERVER, url, NULL, cookies, cookies_count, auth);
    } else if (strcmp(request, "POST") == 0) {
        message = compute_post_request(SERVER, url, PTYPE, content, cookies, cookies_count, auth);
    } else if (strcmp(request, "DELETE") == 0) {
        message = compute_delete_request(SERVER, url, NULL, cookies, cookies_count, auth);
    }

    send_to_server(sockfd, message);
    server_resp = receive_from_server(sockfd);

    return server_resp;
}

// function to open a connection to the server
int establish_connection() {
    int sockfd;
    sockfd = open_connection(SERVER, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Socket error.");
    }
    return sockfd;
}

// function to send a GET message
char *get_send_msg(char *url, char **cookies, int cookies_count, char *auth) {
    int sockfd = establish_connection();
    char *response = create_and_send_msg(sockfd, "GET", url, NULL, cookies, cookies_count, auth);
    close_connection(sockfd);
    return response;
}

// function to send a POST message
char *post_send_msg(char *url, char *content, char **cookies, int cookies_count, char *auth) {
    int sockfd = establish_connection();
    char *response = create_and_send_msg(sockfd, "POST", url, content, cookies, cookies_count, auth);
    close_connection(sockfd);
    return response;
}

// function to send a DELETE message
char *delete_send_msg(char *url, char **cookies, int cookies_count, char *auth) {
    int sockfd = establish_connection();
    char *response = create_and_send_msg(sockfd, "DELETE", url, NULL, cookies, cookies_count, auth);
    close_connection(sockfd);
    return response;
}

// functie de inregistrare a unui nou user
void user_registration() {
	char new_username[SIZE_INPUT], new_password[SIZE_INPUT];
	JSON_Value *root_value;
	JSON_Object *root_object;
	char *server_response;

	printf("username=");
	fgets(new_username, SIZE_INPUT, stdin);
	new_username[strlen(new_username) - 1] = '\0';

	printf("password=");
	fgets(new_password, SIZE_INPUT, stdin);
	new_password[strlen(new_password) - 1] = '\0';

	// initialize JSON object
	root_value = json_value_init_object();
	root_object = json_value_get_object(root_value);

	json_object_set_string(root_object, "username", new_username);
	json_object_set_string(root_object, "password", new_password);

	// send the POST request
	server_response = post_send_msg("/api/v1/tema/auth/register", json_serialize_to_string(root_value), NULL, 0, NULL);

	 // Find the start of the JSON substring
    char *json_start = strchr(server_response, '{');
    if (json_start != NULL) {
        // Create a new string to store the JSON substring
        char json_string[strlen(json_start) + 1];
        strcpy(json_string, json_start);

        root_value = json_parse_string(json_string);
	}
	root_object = json_value_get_object(root_value);
	const char *msg_verify = json_object_dotget_string(root_object, "error");

	if (msg_verify != NULL) {
		printf("Error: %s\n", msg_verify);
	} else {
		printf("Congratulations %s, your new account has been created.\n", new_username);
	}
}

// functie user login
void user_login(char **session_cookies, int *cookies_counter) {
	char login_username[SIZE_INPUT], login_password[SIZE_INPUT];
	JSON_Value *root_value;
	JSON_Object *root_object;
	char *server_response;

	if (*cookies_counter > 0) {
		printf("Error: You are already logged in!\n");
		return;
	}

	printf("username=");
	fgets(login_username, SIZE_INPUT, stdin);
	login_username[strlen(login_username) - 1] = '\0';

	printf("password=");
	fgets(login_password, SIZE_INPUT, stdin);
	login_password[strlen(login_password) - 1] = '\0';

	// initialize JSON object
	root_value = json_value_init_object();
	root_object = json_value_get_object(root_value);

	json_object_set_string(root_object, "username", login_username);
	json_object_set_string(root_object, "password", login_password);

	// send the POST request
	server_response = post_send_msg("/api/v1/tema/auth/login", json_serialize_to_string(root_value), session_cookies, *cookies_counter, NULL);

	// Find the start of the JSON substring
    char *json_start = strchr(server_response, '{');
    if (json_start != NULL) {
        // Create a new string to store the JSON substring
        char json_string[strlen(json_start) + 1];
        strcpy(json_string, json_start);

        root_value = json_parse_string(json_string);
	}
	root_object = json_value_get_object(root_value);
	const char *error_message = json_object_dotget_string(root_object, "error");

	if (error_message != NULL) {
		printf("Error: %s\n", error_message);
	} else {
		printf("You have logged in with username %s!\n", login_username);

		// save the server's returned cookie
		server_response = strstr(server_response, "Set-Cookie");
		char *cookie_start = strchr(server_response, ' ');
		char *cookie_end = strchr(cookie_start, ';');
		size_t cookie_length = cookie_end - cookie_start;

		*session_cookies = (char *)malloc((cookie_length + 1) * sizeof(char));
		strncpy(*session_cookies, cookie_start, cookie_length);
		(*session_cookies)[cookie_length] = '\0';
		(*cookies_counter)++;
	}
}

// functie de acces in biblioteca
void enter_library(char **cookies, int cookies_count, char **token_aut) {
    if (!cookies || cookies_count == 0) {
        printf("Error: Access denied. Please login first.\n");
        return;
    }

    if (*token_aut == NULL) {
        char *response = get_send_msg("/api/v1/tema/library/access", cookies, cookies_count, *token_aut);

        JSON_Value *value = json_parse_string(strstr(response, "{"));
        JSON_Object *object = json_object(value);
        const char *error_message = json_object_dotget_string(object, "error");

        if (error_message != NULL) {
            printf("Error: %s\n", error_message);
        } else {
            object = json_value_get_object(value);
            char *token = (char *)json_object_get_string(object, "token");

            *token_aut = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(*token_aut, token);

            printf("You have gained access to the library.\n");
        }
    } else {
        printf("Error: You already have library access.\n");
    }
}

// functie de intoarce cartile din biblioteca
void get_books(char **cookies, int cookies_count, char *token_aut) {
	// Check if user is logged in
    if (!cookies || cookies_count == 0) {
        printf("Access denied, you need to enter library first.\n");
        return;
    }
	if (token_aut == NULL) {
		printf("Error: Authorization header is missing!, You have to enter library first\n");
		return;
	}

	char *server_response;
	server_response = get_send_msg("/api/v1/tema/library/books", cookies, cookies_count, token_aut);
	
	JSON_Value *parsed_value = json_parse_string(strstr(server_response, "["));
	JSON_Array *book_array = json_value_get_array(parsed_value);
	int book_qty = json_array_get_count(book_array);
    
	if (book_qty > 0) {
    	printf("List of available books (%d):\n", book_qty);
    	int i = 0;
   	 	JSON_Object *current_book = json_array_get_object(book_array, i);
   		while (current_book != NULL) {
        printf("Book ID: %d - Title: %s\n", (int)json_object_dotget_number(current_book, "id"),
            json_object_dotget_string(current_book, "title"));
        i++;
        current_book = json_array_get_object(book_array, i);
    	}
	} else {
        printf("No publications currently available.\n");
    }
}

// functie ce intoarce o anumita carte din biblioteca
void get_book(char **cookies, int cookies_count, char *token_aut) {
	JSON_Value *root_value;
	JSON_Object *root_object;
    if (!cookies || cookies_count == 0) {
        printf("Error: Access denied. You need to enter the library first.\n");
        return;
    }

    if (token_aut == NULL) {
        printf("Error: Authorization header is missing! You have to enter the library first.\n");
        return;
    }

    char id[SIZE_INPUT];
    printf("id=");
    fgets(id, SIZE_INPUT, stdin);
    id[strcspn(id, "\n")] = '\0'; // Remove trailing newline character

    char url[SIZE_INPUT];
    snprintf(url, SIZE_INPUT, "/api/v1/tema/library/books/%s", id);

    char *server_response = get_send_msg(url, cookies, cookies_count, token_aut);

    char *json_start = strchr(server_response, '{');
    if (json_start != NULL) {
        // Create a new string to store the JSON substring
        char json_string[strlen(json_start) + 1];
        strcpy(json_string, json_start);

        root_value = json_parse_string(json_string);
	}

    root_object = json_object(root_value);
    const char *error = json_object_dotget_string(root_object, "error");

    if (error != NULL) {
        printf("Server Error: %s\n", error);
        return;
    }
     // Extract book details
    int bookId = (int)json_object_dotget_number(root_object, "id");
    const char *title = json_object_dotget_string(root_object, "title");
    const char *author = json_object_dotget_string(root_object, "author");
    const char *publisher = json_object_dotget_string(root_object, "publisher");
    const char *genre = json_object_dotget_string(root_object, "genre");
    int pageCount = (int)json_object_dotget_number(root_object, "page_count");

    // Display the book details
    printf("Book found:\n");
    printf("ID: %d\n", bookId);
    printf("Title: %s\n", title);
    printf("Author: %s\n", author);
    printf("Publisher: %s\n", publisher);
    printf("Genre: %s\n", genre);
    printf("Page Count: %d\n", pageCount);
}


void read_input(char* line) {
    fgets(line, SIZE_INPUT, stdin);
    line[strlen(line) - 1] = '\0';
}

// functie ce adauga o noua carte in biblioteca
void add_book(char **cookies, int cookies_count, char *token_aut) {
	// Check if user is logged in
    if (!cookies || cookies_count == 0) {
        printf("Access denied, you need to enter library first.\n");
        return;
    }
	if (token_aut == NULL) {
		printf("Error: Authorization header is missing!, You have to enter library first\n");
		return;
	}
	//	char *line = malloc(SIZE_INPUT * sizeof(char));
	char line[SIZE_INPUT];
	JSON_Value *value = json_value_init_object();
	JSON_Object *object = json_value_get_object(value);

	// citim campurile din stdin si construim obiectul JSON
printf("title=");
read_input(line);
json_object_set_string(object, "title", line);

printf("author=");
read_input(line);
json_object_set_string(object, "author", line);

printf("genre=");
read_input(line);
json_object_set_string(object, "genre", line);

printf("publisher=");
read_input(line);
json_object_set_string(object, "publisher", line);

printf("page_count=");
read_input(line);

	// verific sa fie valid
	if (atoi(line) <= 0) {
		printf("Error: Invalid input.\n");
		return;
	}
	json_object_set_number(object, "page_count", (double)atoi(line));
	
	char *server_response;
	server_response = post_send_msg("/api/v1/tema/library/books", json_serialize_to_string(value),
								 cookies, cookies_count, token_aut);

	// Find the start of the JSON substring
    char *json_start = strchr(server_response, '{');
    if (json_start != NULL) {
        // Create a new string to store the JSON substring
        char json_string[strlen(json_start) + 1];
        strcpy(json_string, json_start);

        value = json_parse_string(json_string);
	}
	object = json_object(value);
	// verific daca a intors o eroare
	const char *check_value = json_object_dotget_string(object, "error");
	if (check_value != NULL) {
		printf("Error: %s\n", check_value);
		return;
	}
	printf("Book successfully added to library.\n");

	//free(line);
}

// functie ce sterge o carte din biblioteca
void book_remove(char **cookies, int cookies_count, char *token_aut) {
	JSON_Value *root_value;
	JSON_Object *root_object;
	// Check if user is logged in
    if (!cookies || cookies_count == 0) {
        printf("Access denied, you need to enter library first.\n");
        return;
    }
	if (token_aut == NULL) {
		printf("Error: Authorization header is missing!, You have to enter library first\n");
		return;
	}
    char id[SIZE_INPUT];
    printf("id=");
	read_input(id);

    char url[SIZE_INPUT];
    snprintf(url, SIZE_INPUT, "/api/v1/tema/library/books/%s", id);

    char *server_response;
	server_response = delete_send_msg(url, cookies, cookies_count, token_aut);

    char *json_start = strchr(server_response, '{');
    if (json_start != NULL) {
        // Create a new string to store the JSON substring
        char json_string[strlen(json_start) + 1];
        strcpy(json_string, json_start);

        root_value = json_parse_string(json_string);
	}
	
    root_object = json_object(root_value);
    const char *error = json_object_dotget_string(root_object, "error");

    if (error != NULL && strncmp(error, "Bad", 3) == 0) {
        printf("Error: Invalid book ID! Please provide a valid integer ID.\n");
        return;
    }

    if (error != NULL) {
        printf("Error: %s\n", error);
        return;
    }

    printf("Book successfully deleted.\n");
}

void user_logout(char **session_cookies, int *cookies_count, char **token_aut) {

	JSON_Value *value;
	JSON_Object *object;
    char *server_response;
	server_response = get_send_msg("/api/v1/tema/auth/logout", session_cookies, *cookies_count, *token_aut);

    char *json_start = strchr(server_response, '{');
    if (json_start != NULL) {
        // Create a new string to store the JSON substring
        char json_string[strlen(json_start) + 1];
        strcpy(json_string, json_start);

        value = json_parse_string(json_string);
	}
	
    object = json_object(value);
    const char *error_message = json_object_dotget_string(object, "error");
    if (error_message != NULL) {
        printf("Error: %s\n", error_message);
        return;
    }

    free(*session_cookies);
    *session_cookies = NULL;
    *cookies_count = 0;
    free(*token_aut);
    *token_aut = NULL;

    printf("Logout successful. You have been logged out.\n");
}

Command str_to_cmd(const char *str) {
	if (strcmp(str, "register") == 0) return CMD_REGISTER;
	if (strcmp(str, "login") == 0) return CMD_LOGIN;
	if (strcmp(str, "enter_library") == 0) return CMD_ENTER_LIBRARY;
	if (strcmp(str, "get_books") == 0) return CMD_GET_BOOKS;
	if (strcmp(str, "get_book") == 0) return CMD_GET_BOOK;
	if (strcmp(str, "add_book") == 0) return CMD_ADD_BOOK;
	if (strcmp(str, "delete_book") == 0) return CMD_DELETE_BOOK;
	if (strcmp(str, "logout") == 0) return CMD_LOGOUT;
	if (strcmp(str, "exit") == 0) return CMD_EXIT;
	return CMD_INVALID;
}

int main(int argc, char *argv[])
{
	char input_message[SIZE_INPUT];
	char *cookies = NULL;
	int cookies_counter = 0;
	char *token_aut = NULL;

	printf("Library Management System\n");
	printf("-------------------------\n");

	while (1) {
		fgets(input_message, SIZE_INPUT, stdin);
		input_message[strlen(input_message) - 1] = '\0'; // removing the newline character at the end

		Command cmd = str_to_cmd(input_message);

		switch (cmd) {
			case CMD_REGISTER:
				user_registration();
				break;
			case CMD_LOGIN:
				user_login(&cookies, &cookies_counter);
				break;
			case CMD_ENTER_LIBRARY:
				enter_library(&cookies, cookies_counter, &token_aut);
				break;
			case CMD_GET_BOOKS:
				get_books(&cookies, cookies_counter, token_aut);
				break;
			case CMD_GET_BOOK:
				get_book(&cookies, cookies_counter, token_aut);
				break;
			case CMD_ADD_BOOK:
				add_book(&cookies, cookies_counter, token_aut);
				break;
			case CMD_DELETE_BOOK:
				book_remove(&cookies, cookies_counter, token_aut);
				break;
			case CMD_LOGOUT:
				user_logout(&cookies, &cookies_counter, &token_aut);
				break;
			case CMD_EXIT:
				printf("Exiting the program...\n");
				goto label;
			default:
				printf("Invalid command.\n");
				break;
		}
	}

	// free the allocated data at the end!
	free(cookies);
	free(token_aut);
	label:
	return 0;
}