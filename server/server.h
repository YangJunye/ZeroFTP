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
    std::map<std::string, std::string> users;

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
