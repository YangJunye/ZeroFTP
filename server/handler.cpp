//
// Created by 杨俊晔 on 04/12/2016.
//

#include "handler.h"
#include "../common/const.h"
#include "../common/util.h"
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <fstream>
#include <arpa/inet.h>
#include <string.h>

using namespace std;

Handler::Handler(pthread_t *p_thread, int id, int fd, unsigned int ip) : p_client_thread(p_thread), local_ip(ip),
                                                                         client_id(id),
                                                                         client_fd(fd) {
    init_users();
    is_passive = false;
    is_logined = false;
    curr_dir = "./";
    cout << "Client id : " << client_id << endl;
}

void Handler::process() {
    int res = send_response(SERVER_READY, "FTP Server is ready.");
    if (res < 0) {
        cout << "Client " << client_id << " quit!" << endl;
        return;
    }
    string str, cmd, args;
    while (true) {
        char buf[256];
        int length = recv(client_fd, buf, 256, 0);
        if (length <= 0) {
            cout << "Client " << client_id << " quit!" << endl;
            return;
        }
        if (buf[length - 2] == '\r' && buf[length - 1] == '\n') {
            buf[length - 2] = '\0';
            str = string(buf);
            parse_command(str, cmd, args);
            int is_end = exec(cmd, args);
            if (is_end == 1) {
                cout << "Client " << client_id << " quit!" << endl;
                break;
            }
        } else {
            cout << "Bad Command." << endl;
        }
    }
}

int Handler::send_response(int code, const std::string &msg) {
    string str = to_string(code) + ' ' + msg;
    int res = send(client_fd, (str + "\r\n").c_str(), str.length() + 2, 0);
    return res;
}

int Handler::exec(std::string &cmd, std::string &args) {
    cout << "Client " << client_id << " : [" << cmd << ' ' << args << ']' << endl;
    if (cmd == "USER") {
        if (need_login) {
            send_response(RIGHT_USERNAME, "User name okay, need password.");
            username = args;
        } else {
            send_response(LOGIN_SUCCESS, "User logged in, proceed.");
        }
    } else if (cmd == "PASS") {
        if (users[username] == "" || users[username] != args) {
            is_logined = false;
            send_response(NOT_LOGINED, "Not logged in.");
        } else {
            is_logined = true;
            send_response(LOGIN_SUCCESS, "User logged in, proceed.");
        }
    } else if (cmd == "SYST") {
        send_response(SYSTEM_TYPE, "UNIX Type: L8");
    } else if (cmd == "PWD") {
        if (!is_logined) {
            send_response(NOT_LOGINED, "Not logged in.");
        } else {
            string dir = curr_dir.substr(1, curr_dir.size() - 1).c_str();
            send_response(PATHNAME_CREATED, "\"" + dir + "\" is current directory.");
        }
    } else if (cmd == "TYPE") {
        send_response(COMMAND_OK, "Binary mode");
    } else if (cmd == "FEAT") {
        string str = to_string(211) + '-' + "Features:";
        send(client_fd, (str + "\r\n").c_str(), str.length() + 2, 0);
        write(client_fd, "EPRT\r\n", strlen("EPRT\r\n"));
        send_response(211, "End");
    } else if (cmd == "PASV") {
        if (!is_logined) {
            send_response(NOT_LOGINED, "Not logged in.");
        } else {
            int ret = enter_pasv_mode();
            if (ret < 0) {
                cout << "Enter PASV Mode Error." << endl;
                return 1;
            }
        }
    } else if (cmd == "LIST") {
        if (!is_logined) {
            send_response(NOT_LOGINED, "Not logged in.");
        } else {
            if (handle_ls(curr_dir) < 0) {
                cout << "Send List Data Error" << endl;
            }
        }
    } else if (cmd == "CWD") {
        if (!is_logined) {
            send_response(NOT_LOGINED, "Not logged in.");
        } else {
            handle_cd(args);
        }
    } else if (cmd == "RETR") {
        if (!is_logined) {
            send_response(NOT_LOGINED, "Not logged in.");
        } else {
            handle_get(args);
        }
    } else if (cmd == "STOR") {
        if (!is_logined) {
            send_response(NOT_LOGINED, "Not logged in.");
        } else {
            handle_put(args);
        }
    } else if (cmd == "QUIT") {
        send_response(GOODBYE, "Goodbye, closing session.");
        return 1;
    } else {
        send_response(NOT_IMPLEMENTED, "Not implemented.");
    }
    return 0;
}

int Handler::enter_pasv_mode() {
    if (is_passive)
        ::close(data_listen_fd);
    if (init_data_listen_fd() < 0) {
        return -1;
    }
    int port = get_listen_port();
    if (port < 0)
        return -1;
    char addr_port_buf[128];

    int addr = local_ip;

    sprintf(addr_port_buf, "(%d,%d,%d,%d,%d,%d)",
            (addr >> 24) & 0xFF,
            (addr >> 16) & 0xFF,
            (addr >> 8) & 0xFF,
            addr & 0xFF,
            (port >> 8) & 0xFF,
            port & 0xFF);
    send_response(ENTER_PASSIVE, "Entering Passive Mode " + string(addr_port_buf));
    is_passive = true;
    return 0;
}

Handler::~Handler() {
    ::close(client_fd);
    delete p_client_thread;
}

int Handler::get_listen_port() {
    sockaddr *local_addr_ptr = (sockaddr *) &local_addr;
    socklen_t addrlen = sizeof(local_addr);
    memset(&local_addr, 0, addrlen);
    if (getsockname(data_listen_fd, local_addr_ptr, &addrlen))
        return -1;
    int port = ntohs(local_addr.sin_port);
    return port;
}

int Handler::init_data_listen_fd() {
    data_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (data_listen_fd < 0)
        return -1;
    sockaddr_in svrdata_addr;
    sockaddr *svrdata_addr_ptr = (sockaddr *) &svrdata_addr;
    memset(&svrdata_addr, 0, sizeof(svrdata_addr));
    svrdata_addr.sin_family = AF_INET;
    svrdata_addr.sin_addr.s_addr = INADDR_ANY;
    svrdata_addr.sin_port = htons(0);
    int optval = 1;
    if (setsockopt(data_listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &optval, sizeof(optval)))
        return -1;
    if (::bind(data_listen_fd, svrdata_addr_ptr, sizeof(svrdata_addr)) < 0)
        return -1;
    if (listen(data_listen_fd, 5))
        return -1;
    return 0;
}

int Handler::handle_ls(string &path) {
    data_conn_fd = get_data_fd();
    if (data_conn_fd < 0) {
        cout << "Data Connection Accept Error" << endl;
        return 0;
    }
    int pipe_fd[2];
    if (pipe(pipe_fd))
        return -1;
    pid_t chpid = fork();
    if (chpid < 0)
        return -1;
    if (!chpid) {
        close(pipe_fd[0]);
        if (dup2(pipe_fd[1], fileno(stdout)) < 0)
            return -1;
        if (dup2(pipe_fd[1], fileno(stderr)) < 0)
            return -1;
        execl("/bin/ls", "ls", "-l", path.c_str(), NULL);
        exit(0);
    }

    close(pipe_fd[1]);
    int buf_max_size = 1024;
    char buf[buf_max_size + 1];
    while (true) {
        int size = read(pipe_fd[0], buf, buf_max_size);
        buf[size] = '\0';
        if (size <= 0) {
            int waiting = waitpid(chpid, NULL, 0);
            if (waiting < 0)
                return -1;
            if (waiting == chpid)
                break;
        } else {
            send_with_crlf(string(buf));
        }
    }
    close(pipe_fd[0]);
    close(data_conn_fd);
    send_response(CLOSE_DATA_CONNECTION, "Transfer complete.");
    return 0;
}

int Handler::get_data_fd() {
    if (!is_passive) {
        send_response(CANNOT_OPEN_DATA_CONNECTION, "Not in PASV mode");
        return -1;
    }
    socklen_t sin_size = sizeof(sockaddr_in);
    sockaddr_in data_addr;
    int data_fd = accept(data_listen_fd, (sockaddr *) &data_addr, &sin_size);
    if (data_fd < 0)
        return -1;
    ::close(data_listen_fd);
    is_passive = false;
    send_response(OPENING_DATAPORT, "Data Connection Open");
    return data_fd;
}

int Handler::send_with_crlf(const std::string &str) {
    int last_pos = 0;
    for (int i = 1; i < str.size(); ++i) {
        if (str[i - 1] != '\r' && str[i] == '\n') {
            string data = str.substr(last_pos, i - last_pos) + "\r\n";
            send(data_conn_fd, data.c_str(), data.length(), 0);
            last_pos = i + 1;
        }
    }
    if (last_pos < str.size()) {
        string data = str.substr(last_pos, str.size() - last_pos);
        send(data_conn_fd, data.c_str(), data.length(), 0);
    }
    return 0;
}


string Handler::parse_path(std::string &args) {
    if (args[0] == '/')
        return "." + args;
    if (args == ".")
        return "./";
    if (args[0] == '.' && args[1] == '/')
        return curr_dir + args.substr(2, args.size() - 2);
    if (args[0] == '.' && args[1] == '.' && args[2] == '/') {
        if (curr_dir == "./")
            return curr_dir;
        int pos = curr_dir.substr(0, curr_dir.size() - 1).rfind('/');
        return curr_dir.substr(0, pos + 1) + args.substr(3, args.size() - 3);
    }
    return curr_dir + args;
}

int Handler::send_file(FILE *file) {
    int buf_max_size = 1024;
    char buf[buf_max_size + 1];
    while (true) {
        int size = fread(buf, 1, buf_max_size, file);
        if (size <= 0)
            break;
        buf[size] = '\0';
        if (send(data_conn_fd, buf, size, 0) < 0) {
            cout << "Send Error" << endl;
            return -1;
        }
    }
    return 0;
}

int Handler::handle_get(string &args) {
    string path = parse_path(args);
    FILE *file = fopen(path.c_str(), "rb");
    if (!file) {
        send_response(REQUESTED_ACTION_NOT_TAKEN, "Fail to Open Input File");
        return 0;
    }
    data_conn_fd = get_data_fd();
    if (data_conn_fd < 0) {
        cout << "Data Connection Accept Error" << endl;
        return 0;
    }
    cout << "Transfering..." << endl;
    if (send_file(file) < 0) {
        cout << "Send File Data Error" << endl;
        return 0;
    }
    cout << "Transfer Completed." << endl;
    fclose(file);
    close(data_conn_fd);
    send_response(CLOSE_DATA_CONNECTION, "Data Connection Closed.");
    return 0;
}

int Handler::recv_file(FILE *file) {
    int buf_max_size = 1024;
    char buf[buf_max_size + 1];
    while (true) {
        int size = recv(data_conn_fd, buf, buf_max_size, 0);
        if (size <= 0) break;
        buf[size] = '\0';
        if (fwrite(buf, 1, size, file) != size)
            break;
    }
    return 0;
}

int Handler::handle_put(std::string &args) {
    string path = parse_path(args);
    FILE *file = fopen(path.c_str(), "wb");
    if (!file) {
        send_response(REQUESTED_ACTION_NOT_TAKEN, "Fail to Open Output File");
        return 0;
    }
    data_conn_fd = get_data_fd();
    if (data_conn_fd < 0) {
        cout << "Data Connection Accept Error" << endl;
        return 0;
    }
    cout << "Transfering..." << endl;
    if (recv_file(file) < 0) {
        cout << "Receive File Data Error" << endl;
        return 0;
    }
    cout << "Transfer Completed." << endl;
    close(data_conn_fd);
    fclose(file);
    send_response(CLOSE_DATA_CONNECTION, "Data Connection Closed.");
    return 0;
}

int Handler::handle_cd(std::string &args) {
    string path = parse_path(args);
    if (opendir(path.c_str())) {
        send_response(ACTION_DONE, "Directory successfully changed.");
        if (path[path.size() - 1] == '/')
            curr_dir = path;
        else curr_dir = path + '/';
    } else {
        send_response(ACTION_FAILED, "Failed to change directory.");
    }
    return 0;
}

void Handler::init_users() {
    fstream file;
    char line[300], username[100], password[100];
    file.open("users.txt", ios::in);
    while (!file.eof()) {
        file.getline(line, 300, '\n');
        sscanf(line, "%s %s\n", username, password);
        users[string(username)] = string(password);
    }
    need_login = users.size() > 0;
}
