//
// Created by 杨俊晔 on 03/12/2016.
//

#ifndef ZEROFTP_UTIL_H
#define ZEROFTP_UTIL_H

#include <string>

extern std::string get_filename(const std::string &path);

extern void hide_stdin();

extern void show_stdin();

extern void strip(std::string &str);

#endif //ZEROFTP_UTIL_H
