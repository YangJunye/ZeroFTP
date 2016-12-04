//
// Created by 杨俊晔 on 03/12/2016.
//

#include <termios.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstdlib>
#include "util.h"

using namespace std;

string get_filename(const string &path) {
    int pos = path.rfind('/');
    if (pos == string::npos || pos == path.size() - 1)
        return path;
    return path.substr(pos + 1, path.size() - pos - 1);
}

void hide_stdin() {
    termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void show_stdin() {
    termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void strip(string &str) {
    int lpos = str.find_first_not_of(" \t");
    int rpos = str.find_last_not_of(" \t");
    if (lpos == string::npos && rpos == string::npos) {
        str = "";
        return;
    }
    str = str.substr(lpos, rpos - lpos + 1);
}

void parse_command(string &str, string &cmd, string &args) {
    int pos = str.find_first_of(" ");
    if (pos == string::npos) {
        cmd = str;
        args = "";
    } else {
        cmd = str.substr(0, pos);
        args = str.substr(pos + 1, str.length() - pos - 1);
    }
}

string get_ip() {
    char ip[256];
    char host[256] = {0};
    if (gethostname(host, sizeof(host)) < 0)
        return "";
    struct hostent *hp;
    if ((hp = gethostbyname(host)) == NULL)
        return "";
    strcpy(ip, inet_ntoa(*(struct in_addr *) hp->h_addr));
    return string(ip);
}

unsigned int parse_ip(const string &ip) {
    if (ip == "")
        return 0;
    string ipstr = ip + '.';
    int h[4];
    int count = 0, lastsep = -1;
    for (int i = 0; i < ipstr.size(); ++i)
        if (ipstr[i] == '.') {
            string tmp = ipstr.substr(lastsep + 1, i - lastsep - 1);
            h[count] = atoi(tmp.c_str());
            lastsep = i;
            ++count;
        }
    unsigned ret = h[0];
    for (int i = 1; i < 4; ++i)
        ret = (ret << 8) + h[i];
    return ret;
}