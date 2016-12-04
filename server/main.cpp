//
// Created by 杨俊晔 on 02/12/2016.
//

#include <cstdlib>
#include "server.h"

int main(int argc, char **argv) {
    Server server;
    if (argc == 1) {
        server.run();
    } else if (argc == 2) {
        server.run(atoi(argv[1]));
    }
    return 0;
}
