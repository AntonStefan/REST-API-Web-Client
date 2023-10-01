#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *auth)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if(auth != NULL){
        sprintf(line, "Authorization: Bearer %s", auth);
        compute_message(message, line);
    }

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    //Se adauga cookie-urile

    // Add cookies, according to the protocol format
    if (cookies != NULL) {
        char* list = malloc(BUFLEN * cookies_count);
        
        strcpy(list, cookies[0]);
        if(cookies_count > 1){
            strcat(list, "; "); 
        }
        for(int i=1; i < cookies_count-1; i++){
            strcat(list, cookies[i]);
            strcat(list, "; ");
        }    
        if(cookies_count > 1){
            strcat(list, cookies[cookies_count-1]);
        } 
        sprintf(line,"Cookie: %s",list);
        compute_message(message, line);
        free(list);
    }


    // Step 4: add final new line
    compute_message(message, "");

    // Free allocated data
    free(line);

    // Return the computed message
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
                            char** cookies, int cookies_count, char *auth)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);


    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    // Add the Content Type header
   	sprintf(line, "Content-Type: %s", content_type);
	compute_message(message, line);

    // Add the Content Length header
	sprintf(line, "Content-Length: %ld", strlen(body_data));
	compute_message(message, line);

    // Step 4 (optional): add cookies
    if (cookies_count != 0) {
   	sprintf(line, "Cookie: ");
		strcat(message, line);
		for(int i = 0 ; i < cookies_count - 1; i++) {
		   sprintf(line, "%s", cookies[i]);
		   strcat(message, line);
		   strcat(message, "; ");
		}
		sprintf(line, "%s", cookies[cookies_count - 1]);
		compute_message(message, line);
	}

	if (auth != NULL) {
		sprintf(line, "Authorization: Bearer %s", auth);
		compute_message(message, line);
	}
    // Step 5: add new line at end of header


    // Step 6: add the actual payload data
    compute_message(message, "");
    compute_message(message, body_data);

    free(line);
    return message;
}


// Computes a DELETE request given a host, an url, the content type 

char *compute_delete_request(char *host, char *url, char* content_type, 
                             char **cookies, int cookies_count, char* auth)
{
    
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Method name, URL and protocol type
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);

    // Authorization header
    if(auth != NULL){
        sprintf(line, "Authorization: Bearer %s", auth);
        compute_message(message, line);
    }
    
    // Add the Content Type header
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    // Host header
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add cookies
    if (cookies != NULL) {
        char* list = malloc(BUFLEN * cookies_count);
        
        strcpy(list, cookies[0]);
        if(cookies_count > 1){
            strcat(list, "; "); 
        }
        for(int i=1; i < cookies_count-1; i++){
            strcat(list, cookies[i]);
            strcat(list, "; ");
        }    
        if(cookies_count > 1){
            strcat(list, cookies[cookies_count-1]);
        } 
        sprintf(line,"Cookie: %s",list);
        compute_message(message, line);
        free(list);
    }
    // Add a new line at end of header
    compute_message(message, "");
    
    // Free allocated data
    free(line); 

    // Return the computed message
    return message;
}