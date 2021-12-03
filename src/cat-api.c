#include "cat-api.h"
#include <stdio.h>
#include "mongoose.h"
#include "mjson.h"

// Max size of an URL, including the qs
#define MAX_URL_SIZE 2048
#define CAT_SEARCH_URL "https://api.thecatapi.com/v1/images/search"

static struct request {
    /** Event loop control. When turned to true, tell the request manager
     * to stop pooling for result
     */
    bool done;
    /** # of bytes allocated to bodybuffer*/
    int bodyBufferLen;
    /** Called url for this request. Somehow you can't get it back normally **/
    const char *calledUrl;
    /** handler's return value **/
    char **bodyBuffer;

    /** Request handler **/
    void (*handler)(struct request *rt, struct mg_http_message *hm);
};


/**
 * Allocate memory and copy a given value to the dynamic part of a request struct
 * @param rt request
 * @param value value to copy in the request body
 * @param size # of bytes to copy, not accounting for \0
 */
static inline void setResponseBufferValue(struct request *rt, const char *value, int size) {
    *(rt->bodyBuffer) = malloc(sizeof(char) * size);
    if (!(*(rt->bodyBuffer))) {
        perror("Memory allocation error. Aborting");
        exit(-1);
    }
    memcpy(*(rt->bodyBuffer), value, size);
}


/**
 * Parse the JSON response body from the Cat API and sends back the URL of the first cat found
 * @param response resquest's response
 * @param catBuffer buffer to store the cat url into
 * @param bodyLen length of the cat url
 */
static void parseFirstCatUrl(struct mg_http_message *response, const char *catUrl) {
    // Don't forget the NULL after the body
    char *responseBuffer = malloc(sizeof(char) * response->body.len + 1);
    if (!responseBuffer) {
        perror("Memory allocation error. Aborting");
        exit(-1);
    }
    sprintf(responseBuffer, "%.*s", (int) response->body.len, response->body.ptr);
    const char *cb;
    int len;
    if (!mjson_find(responseBuffer, strlen(responseBuffer), "$[0].url", &cb, &len)) {
        // The OS should handle this. But hey, this might be embedded code
        free(responseBuffer);
        perror("No cats were returned. API down ? Aborting");
        exit(-1);
    }
    // There is a slight possibility that mjson sends back wrong info
    char *url = strtok(cb, ",");

    // Remove extra quotes from the string
    // remove first character -> " by incrementing the pointer
    url++;
    // remove last char -> " by replacing it with the NULL terminaison
    url[strlen(url) - 1] = 0;
    // Sanity check, preventing to write after the allocated memory buffer
    if (strlen(url) + 1 > MAX_URL_SIZE) {
        free(responseBuffer);
        perror("URL is bigger that the inteneded size. Aborting");
        exit(-1);
    }

    strcpy(catUrl, url);
    free(responseBuffer);
}

/**
 * The the first cat returned by the cAT API and sends back its url
 * @param rt
 * @param hm
 */
static void findCatUrl(struct request *rt, struct mg_http_message *hm) {
    // Retrieve the cat URI from the JSON response
    const char catUrl[MAX_URL_SIZE];
    parseFirstCatUrl(hm, &catUrl);

    // Sends back the found URI
    rt->bodyBufferLen = strlen(catUrl) + 1;
    setResponseBufferValue(rt, catUrl, rt->bodyBufferLen);
}


/**
 * Copies the raw image data into the request's response
 * @param rt
 * @param hm
 */
static inline void sendBackImageData(struct request *rt, struct mg_http_message *hm) {
    // Sends back the image data
    setResponseBufferValue(rt, hm->body.ptr, hm->body.len);
    rt->bodyBufferLen = (int) hm->body.len;
}

/**
 * Handle any mongoose request
 * @param c
 * @param ev
 * @param ev_data
 * @param fn_data
 */
static void initRequestWithHandler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct request *rt = (struct request *) fn_data;

    switch (ev) {
        case MG_EV_CONNECT:
            // Connected to server. Extract host name from URL
            struct mg_str host = mg_url_host(rt->calledUrl);

            // If s_url is https://, tell client connection to use TLS
            if (mg_url_is_ssl(rt->calledUrl)) {
                struct mg_tls_opts opts = {.ca = "ca.pem", .srvname = host};
                mg_tls_init(c, &opts);
            }
            char *apiKey = getenv("API_KEY");
            // Send request
            mg_printf(c,
                      "GET %s HTTP/1.0\r\n"
                      "Host: %.*s\r\n"
                      "x-api-key: %s\r\n"
                      "\r\n",
                      mg_url_uri(rt->calledUrl), (int) host.len, host.ptr, apiKey);
            break;
        case MG_EV_HTTP_MSG:
            struct mg_http_message *hm = (struct mg_http_message *) ev_data;

            // Handle request with the user-defined custom handler
            rt->handler(rt, hm);
            // Request is handled, we can close the connection
            c->is_closing = 1;
            rt->done = true;
            break;
        case MG_EV_ERROR:
            printf("ERROR");
            rt->done = true;  // Error, tell event loop to stop
        default:
            // Pass
            break;
    }
}


/**
 * Query the Cat API, return the first cat image url found.
 * /!\ You'll have to free the url
 * @param manager Mongoose connection manager
 * @param catUrl Out parameter, cat url to fill in.
 */
void getRandomCatUrl(struct mg_mgr *manager, char **catUrl) {
    struct request rt = {false, 0, CAT_SEARCH_URL, catUrl, findCatUrl};
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_connect(manager, CAT_SEARCH_URL, initRequestWithHandler, &rt);
    while (!rt.done) mg_mgr_poll(manager, 1000);
    mg_mgr_free(&mgr);

}

/**
 * Given an image url, return it's raw images bytes
 * /!\ You'll have to free the images bytes
 * @param manager Mongoose connection manager
 * @param url image URL
 * @param bytes Out parameter, image bytes
 * @param catLen Out parameter, Image length
 */
void getCatBytesFromUrl(struct mg_mgr *manager, const char *url, char **bytes, int *catLen) {
    struct request rt = {false, catLen, url, bytes, sendBackImageData};
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_connect(&mgr, url, initRequestWithHandler, &rt);
    while (!rt.done) mg_mgr_poll(&mgr, 1000);
    mg_mgr_free(&mgr);
    *catLen = rt.bodyBufferLen;

}