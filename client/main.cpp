//
// Created by 杨俊晔 on 02/12/2016.
//

#include <cstdlib>
#include "client.h"

int main(int argc, char **argv) {
    Client client;
    if (argc == 1) {
        client.run();
    } else if (argc == 2) {
        client.run(argv[1]);
    } else if (argc == 3) {
        client.run(argv[1], atoi(argv[2]));
    }
//    client.run("59.66.159.73", 21);
    return 0;
}
