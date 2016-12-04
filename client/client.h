//
// Created by 杨俊晔 on 02/12/2016.
//

#ifndef ZEROFTP_CLIENT_H
#define ZEROFTP_CLIENT_H

#include <string>
#include "../common/util.h"


struct Command {
    std::string cmd;
    std::string args;
};

class Client {
private:
    bool is_connected;
    int client_fd, data_conn_fd, port;
    std::string host;

    void send_command(const std::string &str);

    Response get_response();

    int get_data_conn();

    void get_pasv_ip_port(Response &res);

    int get_command(std::string &cmd, std::string &args);

    void loop();

public:
    void run();

    void run(const char *host);

    void run(const char *host, int port);

    void connect(const char *host, int port);

    void close();

    bool login();

    void login(const std::string &username, const std::string &password);

    void do_ftp(std::string &args);

    void do_pwd();

    void do_goodbye();

    void do_ls();

    void do_cd(std::string &args);

    int do_get(std::string &args);

    int do_put(std::string &args);

    void show_help();
};


#endif //ZEROFTP_CLIENT_H
