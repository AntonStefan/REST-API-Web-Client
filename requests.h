#ifndef _REQUESTS_
#define _REQUESTS_

/* Computes a GET request given a host, an url, query parameters (optional), 
    a list of cookies (optional) and an authorization token (optional) */
char *compute_get_request(char *host, char *url, char *query_params,
							char **cookies, int cookies_count, char* auth_token);


/* Computes a POST request given a host, an url, the content type, a list of strings for
   body data, a list of cookies (optional) and an authorization token (optional) */
char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
							char** cookies, int cookies_count, char *auth_token);


/* Computes a DELETE request given a host, an url, the content type, 
   a list of cookies (optional) and an authorization token (optional) */
char *compute_delete_request(char *host, char *url, char* content_type, 
                             char** cookies, int cookies_count, char *auth_token);

#endif