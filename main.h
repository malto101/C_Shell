#ifndef MAIN_H
#define MAIN_H

#define ESC "\x1b"
#define CURSOR_LEFT ESC "[D"
#define CURSOR_RIGHT ESC "[C"
#define CLEAR_FROM_CURSOR ESC "[K"
#define CLEAR_ENTIRE_LINE ESC "[2K"
#define CARRIAGE_RETURN "\r"
#define CURSOR_RIGHT_ONE ESC "[1C"

#define INITIAL_BUFFER_SIZE 1024
#define BUFFER_INCREMENT 1024
#define INITIAL_TOKEN_SIZE 64
#define TOKEN_INCREMENT 2
#define PRINTABLE_MIN 32
#define PRINTABLE_MAX 126
#define BACKSPACE '\x7f'
#define ENTER_CR '\r'
#define ENTER_LF '\n'
#define ESCAPE '\x1b'

#endif