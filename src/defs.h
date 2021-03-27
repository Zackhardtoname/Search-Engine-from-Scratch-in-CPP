//
// Created by youh on 3/15/21.
//

#ifndef SCOOBLE_SEARCH_DEFS_H
#define SCOOBLE_SEARCH_DEFS_H

static constexpr long KB = 1024;      // 1 KB
static constexpr long MB = 1024 * KB; // 1 MB
static constexpr long GB = 1024 * MB; // 1 GB

static constexpr const char *FRONTIER_MQ = "/frontier";
static constexpr const char *NEW_LINKS_QUEUE = "/links";
static constexpr const char *PARSED_DOCS_QUEUE = "/docs";
static constexpr int MAX_URL_LEN = 1000; // TODO: consider making these smaller
static constexpr int MAX_QUEUE_SIZE = 256; // TODO: consider changing this size
static constexpr long PARSED_DOCS_QUEUE_MAX_MSGSIZE = 4 * MB; // 4 MB

#endif // SCOOBLE_SEARCH_DEFS_H
