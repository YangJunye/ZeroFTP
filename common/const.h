//
// Created by 杨俊晔 on 03/12/2016.
//

#ifndef ZEROFTP_CONST_H
#define ZEROFTP_CONST_H

const int TRANSFERING = 125;
const int OPENING_DATAPORT = 150;

const int SERVER_READY = 220;
const int ENTER_PASSIVE = 227;
const int LOGIN_SUCCESS = 230;

const int RIGHT_USERNAME = 331;

#define char_to_code(a, b, c) ((a - '0') * 100 + (b - '0') * 10 + (c - '0'))

#endif //ZEROFTP_CONST_H
