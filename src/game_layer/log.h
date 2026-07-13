#ifndef LOG_H
#define LOG_H

enum Log_Level {
	LOG_ERROR,
	LOG_MSG,
};

enum Error_Type log_message(enum Log_Level l, const char *message);

#endif //LOG_H
