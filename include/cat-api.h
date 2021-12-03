#pragma once

#include "mongoose.h"

/**
 * Query the Cat API, return the first cat image url found.
 * /!\ You'll have to free the url
 * @param manager Mongoose connection manager
 * @param catUrl Out parameter, cat url to fill in.
 */
extern void getRandomCatUrl(struct mg_mgr *manager, char **catUrl);

/**
 * Given an image url, return it's raw images bytes
 * /!\ You'll have to free the images bytes
 * @param manager Mongoose connection manager
 * @param url image URL
 * @param bytes Out parameter, image bytes
 * @param catLen Out parameter, Image length
 */
extern void getCatBytesFromUrl(struct mg_mgr *manager, const char *url, char **bytes, int *catLen);