//
// Created by 杨俊晔 on 02/12/2016.
//

#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include "client.h"
#include "../common/const.h"

using namespace std;

void Client::run() {
    loop();
    close();
}

void Client::run(const char *host) {
    connect(host, 21);
    if (is_connected)
        login();
    loop();
    close();
}

void Client::run(const char *host, int port) {
    connect(host, port);
    if (is_connected)
        login();
    loop();
    close();
}

void Client::connect(const char *host, int port) {
    this->host = string(host);
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(port);
    int ret = ::connect(client_fd, (struct sockaddr *) &addr, sizeof(addr));
    if (ret < 0) {
        cout << "Can't connect to " + this->host << endl;
        return;
    }
    Response res = get_response();
    if (res.code == SERVER_READY) {
        is_connected = true;
    } else cout << res.msg;
}

void Client::close() {
    ::close(client_fd);
}

void Client::login(const string &username, const string &password) {
    send_command("USER " + username);
    get_response();

    send_command("PASS " + password);
    get_response();
}

void Client::do_pwd() {
    if (!is_connected) {
        cout << "Not connected." << endl;
        return;
    }
    send_command("PWD");
    get_response();
}

void Client::send_command(const string &str) {
    int res = send(client_fd, (str + "\r\n").c_str(), str.length() + 2, 0);
    if (res < 0) {
        cout << "Server quit!" << endl;
        exit(0);
    }
}

Response Client::get_response() {
    char recv_buf[256];
    int length = recv(client_fd, recv_buf, 256, 0);
    if (length < 0) {
        cout << "Server error. Quit." << endl;
        exit(0);
    }
    recv_buf[length - 2] = '\0';
    Response res;
    res.msg = string(recv_buf);
    int pos = res.msg.find(' ');
    int code = char_to_code(recv_buf[pos - 3], recv_buf[pos - 2], recv_buf[pos - 1]);
    res.code = code;
    cout << res.msg << endl;
    return res;
}

void Client::do_goodbye() {
    if (!is_connected) {
        exit(0);
    }
    send_command("QUIT");
    get_response();
}

void Client::do_ls() {
    if (!is_connected) {
        cout << "Not connected." << endl;
        return;
    }
    send_command("PASV");
    Response res = get_response();
    if (res.code != ENTER_PASSIVE)
        return;
    get_pasv_ip_port(res);
    int data_conn_fd = get_data_conn();
    if (data_conn_fd < 0)
        return;
    send_command("LIST");
    res = get_response();
    if (res.code == OPENING_DATAPORT || res.code == TRANSFERING) {
        int buf_max_size = 1024;
        char buf[buf_max_size + 1];
        string data;
        while (true) {
            int size = recv(data_conn_fd, buf, buf_max_size, 0);
            if (size <= 0) break;
            buf[size] = '\0';
            data += string(buf);
        }
        cout << data << endl;
    }
    get_response();
    ::close(data_conn_fd);
}

void Client::do_cd(string &args) {
    if (!is_connected) {
        cout << "Not connected." << endl;
        return;
    }
    send_command("CWD " + args);
    get_response();
}

int Client::get_data_conn() {
    int data_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in data_addr;
    memset(&data_addr, 0, sizeof(data_addr));
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = inet_addr(host.c_str());
    data_addr.sin_port = htons(port);
    if (data_fd < 0) {
        cout << "Init Data Socket Error" << endl;
        return -1;
    }
    if (::connect(data_fd, (struct sockaddr *) &data_addr, sizeof(struct sockaddr))) {
        cout << "Connect Error" << endl;
        return -1;
    }
    return data_fd;
}

void Client::get_pasv_ip_port(Response &res) {
    string &msg = res.msg;
    int lpos = msg.find('(') + 1;
    int rpos = msg.rfind(')');
    int count = 0, last_sep = -1;
    for (int i = lpos; i <= rpos; ++i) {
        if (msg[i] == ',' || msg[i] == ')') {
            msg[i] = '.';
            ++count;
            if (count == 4)
                host = msg.substr(lpos, i - lpos);
            else if (count == 5)
                port = atoi(msg.substr(last_sep + 1, i - last_sep - 1).c_str());
            else if (count == 6) {
                int tmp = atoi(msg.substr(last_sep + 1, i - last_sep - 1).c_str());
                port = (port << 8) + tmp;
            }
            last_sep = i;
        }
    }
}


int Client::get_command(string &cmd, string &args) {
    string str;
    if (!getline(cin, str))
        return -1;
    strip(str);
    int pos = str.find_first_of(" ");
    if (pos == string::npos) {
        cmd = str;
        args = "";
    } else {
        cmd = str.substr(0, pos);
        args = str.substr(pos + 1, str.size() - pos - 1);
        strip(cmd);
        strip(args);
    }
    for (int i = 0; i < cmd.size(); ++i)
        if (cmd[i] >= 'A' && cmd[i] <= 'Z')
            cmd[i] += 'a' - 'A';
    return 0;
}

void Client::loop() {
    string cmd, args;
    while (true) {
        cout << "ftp> ";
        get_command(cmd, args);
        if (cmd == "pwd") {
            do_pwd();
        } else if (cmd == "ls" || cmd == "dir") {
            do_ls();
        } else if (cmd == "bye" || cmd == "quit" || cmd == "exit") {
            do_goodbye();
            break;
        } else if (cmd == "cd") {
            do_cd(args);
        } else if (cmd == "get") {
            do_get(args);
        } else if (cmd == "put") {
            do_put(args);
        } else if (cmd == "ftp") {
            do_ftp(args);
        } else if (cmd == "user") {
            do_user(args);
        } else if (cmd == "?" || cmd == "help") {
            show_help();
        } else {
            cout << "Invalid command." << endl;
            continue;
        }
    }
}


bool Client::login() {
    string username;
    cout << "Username: ";
    if (!getline(cin, username))
        return false;
    strip(username);
    return do_user(username);
}

int Client::do_get(string &args) {
    if (!is_connected) {
        cout << "Not connected." << endl;
        return -1;
    }
    string filename = get_filename(args);
    FILE *wfile = fopen(filename.c_str(), "wb");
    if (!wfile) {
        cout << "Open output file Error!" << endl;
        return 0;
    }
    send_command("PASV");
    Response res = get_response();
    if (res.code != ENTER_PASSIVE)
        return 0;
    get_pasv_ip_port(res);
    int data_conn_fd = get_data_conn();
    if (data_conn_fd < 0)
        return 0;
    send_command("RETR " + args);
    res = get_response();
    if (res.code == 550) {
        unlink(filename.c_str());
        return 0;
    }
    if (res.code == OPENING_DATAPORT || res.code == TRANSFERING) {
        int buf_max_size = 1024;
        char buf[buf_max_size + 1];
        while (true) {
            int size = recv(data_conn_fd, buf, buf_max_size, 0);
            if (size <= 0)
                break;
            buf[size] = '\0';
            if (fwrite(buf, 1, size, wfile) != size)
                break;
        }
        fclose(wfile);
    }
    get_response();
    ::close(data_conn_fd);
    return 0;
}

void Client::show_help() {
    static string commands[] = {"get", "put", "ls", "dir", "user", "pasv", "cd", "pwd", "ftp", "bye", "quit", "exit",
                                "help", "?"};
    cout << "Commands are:" << endl;
    for (auto str : commands) {
        cout << str << "\t\t";
    }
    cout << endl;
}

int Client::do_put(string &args) {
    if (!is_connected) {
        cout << "Not connected." << endl;
        return -1;
    }
    FILE *rfile = fopen(args.c_str(), "rb");
    if (!rfile) {
        cout << "Open Input file Error!" << endl;
        return 0;
    }

    send_command("PASV");
    Response res = get_response();
    if (res.code != ENTER_PASSIVE)
        return 0;
    get_pasv_ip_port(res);
    int data_conn_fd = get_data_conn();
    if (data_conn_fd < 0)
        return 0;
    send_command("STOR " + get_filename(args));
    res = get_response();
    if (res.code == 553)
        return 0;
    if (res.code == OPENING_DATAPORT || res.code == TRANSFERING) {
        int buf_max_size = 1024;
        char buf[buf_max_size + 1];
        while (true) {
            int size = fread(buf, 1, buf_max_size, rfile);
            if (size <= 0) break;
            buf[size] = '\0';
            if (send(data_conn_fd, buf, size, 0) < 0) {
                cout << "Send Error" << endl;
                return -1;
            }
        }
        fclose(rfile);
        ::close(data_conn_fd);
    }
    get_response();
    return 0;
}

void Client::do_ftp(string &args) {
    int pos = args.find_first_of(" ");
    if (pos == string::npos) {
        host = args;
        port = 21;
    } else {
        host = args.substr(0, pos);
        port = atoi((args.substr(pos + 1, args.size() - pos - 1)).c_str());
        strip(host);
    }
    connect(host.c_str(), port);
    if (is_connected)
        login();
}

bool Client::do_user(std::string &args) {
    send_command("USER " + args);
    Response res = get_response();
    if (res.code == LOGIN_SUCCESS) {
        return true;
    }
    if (res.code != RIGHT_USERNAME) {
        return false;
    }
    string password;
    cout << "Password: ";
    hide_stdin();
    if (!getline(cin, password))
        return false;
    show_stdin();
    cout << endl;
    strip(password);
    send_command("PASS " + password);
    res = get_response();
    return res.code == LOGIN_SUCCESS;
}


