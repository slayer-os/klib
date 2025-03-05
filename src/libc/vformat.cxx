#include <stdarg.h>
#include <libc/string.h>
#include <libc/intx.h>

void vsprintf(char *buffer, const char *format, va_list args) {
    size_t b_pos=0;
    size_t f_pos=0;
    bool ext_prefix = false;
    bool v_long = false;
    bool v_quad = false;
    while (format[f_pos]) {
        if (format[f_pos] == '%') {
            f_pos++;
            if (format[f_pos] == '#') { ext_prefix = true; f_pos++; }
            if (format[f_pos] == 'l') { v_long = true; f_pos++; }
            if (format[f_pos] == 'q') { v_quad = true; f_pos++; }
            if (format[f_pos] == '%') {
                buffer[b_pos] = '%';
            }
            else if (format[f_pos] == 'd') {
                s64 i = 0;
                if (v_long) { i = va_arg(args, s64); }
                else { i = va_arg(args, int); }
                char str[32];
                itoa(i, str, 10);
                memcpy(buffer+b_pos, str, strlen(str));
                b_pos += strlen(str)-1;
            }
            else if (format[f_pos] == 'x') {
                if (v_quad) {
                    /*u512 v = va_arg(args, u512);*/
                    /*char str[128];*/
                    /*quadtoa_hex(str, v);*/
                    /*memcpy(buffer+b_pos, str, strlen(str));*/
                    /*b_pos += strlen(str)-1;*/
                } else {
                  s64 i = 0;
                  if (v_long) { i = va_arg(args, s64); }
                  else { i = va_arg(args, int); }
                  char str[32];
                  if (ext_prefix) { str[0] = '0'; str[1]='x'; }
                  itoa_hex(i, str + (ext_prefix ? 2 : 0));
                  memcpy(buffer+b_pos, str, strlen(str));
                  b_pos += strlen(str)-1;
                }
            }
            else if (format[f_pos] == 'p') {
                s64 i = va_arg(args, s64);
                char str[32];
                str[0] = '0';
                str[1] = 'x';
                itoa_hex(i, str+2);
                memcpy(buffer+b_pos, str, strlen(str));
                b_pos += strlen(str)-1;
            }
            else if (format[f_pos] == 's') {
                char *s = va_arg(args, char *);
                if (!s) {
                    s = (char*)"(null)";
                }
                else if (s[0] == 0) {
                    s = (char*)"(empty)";
                }
                memcpy(buffer+b_pos, s, strlen(s));
                b_pos += strlen(s)-1;
            }
            else if (format[f_pos] == 'a') {
                // for dates and times which need to be exactly 2 digits
                char str[32];
                u8 off = 0;
                int i = va_arg(args, int);
                if (i < 10) {
                    str[0] = '0';
                    off = 1;
                }
                itoa(i, str+off, 10);
                memcpy(buffer+b_pos, str, strlen(str));
                b_pos += strlen(str)-1;
            }
            else if (format[f_pos] == 'c') {
                char c = va_arg(args, int);
                buffer[b_pos] = c;
            }
        } else {
            buffer[b_pos] = format[f_pos];
        }
        b_pos++;
        f_pos++;
    }
    buffer[b_pos] = 0;
}

void sprintf(char *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
}
