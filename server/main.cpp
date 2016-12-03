//
// Created by 杨俊晔 on 02/12/2016.
//

#include "server.h"

int test_main() {
    Server server;
    while (true) {
        server.accept();
        if (!server.check_status())
            break;
    }
    return 0;
}
