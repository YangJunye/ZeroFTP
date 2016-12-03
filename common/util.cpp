//
// Created by 杨俊晔 on 03/12/2016.
//

#include <termios.h>
#include <unistd.h>
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