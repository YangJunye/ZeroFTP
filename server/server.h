//
// Created by 杨俊晔 on 02/12/2016.
//

#ifndef ZEROFTP_SERVER_H
#define ZEROFTP_SERVER_H


class Server {
private:
    int client_id;
    int server_fd;

    void init();

public:
    Server();

    void accept();

    bool check_status();

    void run();

    void loop();
};


#endif //ZEROFTP_SERVER_H
