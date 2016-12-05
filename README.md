# FTP Client & Server

## Usage

```bash
$ make
$ (sudo) ./ftp-server [port]
$ ./ftp [ip] [port]
```
The default port for server is 2016.
`sudo` is needed when port is less than 1024.
The default port for client is 21.

## Supported Commands

```bash
ls      pwd     cd      get     put
user    dir     quit    exit    bye
help    ?
```
