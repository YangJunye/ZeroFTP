//
// Created by 杨俊晔 on 04/12/2016.
//

#ifndef ZEROFTP_HANDLER_H
#define ZEROFTP_HANDLER_H

#include <pthread.h>
#include <string>
#include <netinet/in.h>
#include <map>

class Handler {
private:
    pthread_t *p_client_thread;
    int client_id;
    int client_fd;
    int data_listen_fd;
    int data_conn_fd;
    bool is_passive;
    bool need_login;
    bool is_logined;
    sockaddr_in local_addr;
    std::string curr_dir;
    std::string username;
    std::map<std::string, std::string> users;

    int send_response(int code, const std::string &msg);

    int exec(std::string &cmd, std::string &args);

    int enter_pasv_mode();

    int get_listen_port();

    int init_data_listen_fd();

    int get_data_fd();

    int send_with_crlf(const std::string &str);

    std::string parse_path(std::string &args);

    int send_file(FILE *file);

    int recv_file(FILE *file);

    void init_users();

public:
    void process();

    Handler(pthread_t *p_thread, int id, int fd);

    ~Handler();

    int handle_ls(std::string &dir);

    int handle_get(std::string &args);

    int handle_put(std::string &args);

    int handle_cd(std::string &args);

};


#endif //ZEROFTP_HANDLER_H
