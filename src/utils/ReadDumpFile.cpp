//
// Created by youh on 3/16/21.
//

#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>
#include <cassert>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "usage: " << argv[0] << " [dump file]" << std::endl;
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    for (int i = 0;; i++) {
        char sizeBuf[4];
        switch (fread(sizeBuf, 1, 4, f)) {
            case 0:
                std::cout << "<eof>" << std::endl;
                return 0;
            case 4:
                break;
            default:
                assert(false && "corrupted");
        }
        int32_t lenMsg = *reinterpret_cast<int32_t *>(sizeBuf);
        std::string msg(lenMsg, '\0');
        assert(fread(msg.data(), 1, lenMsg, f) == lenMsg && "corrupted");

        std::cout << "msg " << i << " len = " << lenMsg << " bytes:" << msg << std::endl;
    }
}