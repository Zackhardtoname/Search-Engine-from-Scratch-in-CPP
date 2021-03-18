//
// Created by youh on 3/17/21.
//

#include <iostream>
#include <ctime>
#include <mqueue.h>
#include <cassert>
#include "../defs.h"

static constexpr long TARGET_SIZE = 2 * GB;

/// This dummy index worker takes a message from the parsed document message
/// queue, and store all the contents into file with this compact format:
/// [len_msg i32][msg][len_msg i32][msg]...
/// About 2 GB (configurable) a piece.
int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "usage: " << argv[0] << " [target size in KB]" << std::endl;
        return 1;
    }
    long targetSize;
    targetSize = atol(argv[1]);
    std::cout << "targeting dump size " << targetSize << " KB" << std::endl;
    assert(targetSize > 0 && "invalid target size");
    targetSize *= KB;

    mqd_t parsedDocsQueue = mq_open(PARSED_DOCS_QUEUE, O_RDONLY);
    assert(parsedDocsQueue != -1 && "mq_open() failed");

    while (true) {
        // get current time
        time_t timestamp = std::time(nullptr);
        tm *timeRepr = std::gmtime(&timestamp);
        char filename[1000];
        strftime(filename, 1000, "%Y_%m_%d_%H_%M_%S.dump", timeRepr);

        // open dump file
        FILE *f = fopen(filename, "a"); // append
        long curSize = 0;

        char buffer[sizeof(int32_t) + PARSED_DOCS_QUEUE_MAX_MSGSIZE];
        auto *header = reinterpret_cast<int32_t *>(buffer);
        // if still within target dump size
        while (curSize < TARGET_SIZE) {
            // get a message from mq
            ssize_t msgSize = mq_receive(parsedDocsQueue, buffer, PARSED_DOCS_QUEUE_MAX_MSGSIZE, nullptr);
            assert(msgSize != -1 && "mq_receive() failed");
            *header = static_cast<int32_t>(msgSize);

            // write to dump file
            size_t written = fwrite(buffer, 1, sizeof(int32_t) + msgSize, f);
            assert(written == sizeof(int32_t) + msgSize && "fwrite() failed");
            curSize += written;
        }

        // close file and open the next one
        fclose(f);
    }
}

