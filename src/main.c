#include "log.h"
#include <stdlib.h>
#include "mongoose.h"
#include "cat-api.h"

static void serveRandomCat(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {

        // First, search for a cat
        struct mg_mgr *mgr = (struct mg_mgr *) fn_data;
        char *catUrl;
        getRandomCatUrl(mgr, &catUrl);
        log_info("Serving cat: ");
        log_info(catUrl);

        // Then get's the image to pass into the body
        char *catBytes;
        int catLen = 0;
        getCatBytesFromUrl(mgr, catUrl, &catBytes, &catLen);

        // Finally, send it back
        mg_printf(
                c, "%s",
                "HTTP/1.1 200 OK\r\n"
                "Cache-Control: no-cache\r\n"
                "Pragma: no-cache\r\nExpires: Thu, 01 Dec 1994 16:00:00 GMT\r\n");
        mg_printf(c,
                  "Content-Type: image/png\r\n"
                  "Content-Length: %lu\r\n\r\n",
                  (unsigned long) catLen);
        mg_send(c, catBytes, catLen);
        // And free the remaining memory
        free(catBytes);
        free(catUrl);
    }

}

int main() {
    char *apiKey = getenv("API_KEY");
    if (!apiKey) {
        log_error("No API key provided, aborting !");
        exit(-1);
    }
    log_info("Started App");
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "0.0.0.0:8081", serveRandomCat, &mgr);
    for (;;) mg_mgr_poll(&mgr, 1000);
}
