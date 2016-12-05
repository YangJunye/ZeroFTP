//
// Created by 杨俊晔 on 02/12/2016.
//

#ifndef ZEROFTP_SERVER_H
#define ZEROFTP_SERVER_H


#include <string>
#include <map>

class Server {
private:
    int client_id;
    int server_fd;
    unsigned int host;

    void init_socket(int port);

public:
    Server();

    void accept();

    bool check_status();

    void run();

    void run(int port);

    void loop();
};


#endif //ZEROFTP_SERVER_H
