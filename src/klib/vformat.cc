#include <stdarg.h>
#include <klib/string.h>
#include <klib/intx.h>

void pad_string(char *dest, const char *src, int width, bool zero_pad, bool has_prefix) {
    int src_len = strlen(src);
    
    if (width <= src_len) {
        strcpy(dest, src);
        return;
    }
    
    int pad_len = width - src_len;
    int dest_pos = 0;
    
    if (has_prefix && zero_pad) {
        dest[dest_pos++] = src[0]; // '0'
        dest[dest_pos++] = src[1]; // 'x'
        for (int i = 0; i < pad_len; i++) {
            dest[dest_pos++] = '0';
        }
        strcpy(dest + dest_pos, src + 2);
    } else if (has_prefix && !zero_pad) {
        for (int i = 0; i < pad_len; i++) {
            dest[dest_pos++] = ' ';
        }
        strcpy(dest + dest_pos, src);
    } else if (!has_prefix && zero_pad) {
        for (int i = 0; i < pad_len; i++) {
            dest[dest_pos++] = '0';
        }
        strcpy(dest + dest_pos, src);
    } else {
        for (int i = 0; i < pad_len; i++) {
            dest[dest_pos++] = ' ';
        }
        strcpy(dest + dest_pos, src);
    }
}

void quadtoa_hex(char *buffer, u512 n) {
  char tmp[NLIMBS_512 * 16 + 1];
  int pos = 0;
  for (int i = NLIMBS_512 - 1; i >= 0; i--) {
      u64 limb = n.limbs[i];
      for (int j = 15; j >= 0; j--) {
          u8 nibble = (limb >> (j * 4)) & 0xF;
          tmp[pos++] = nibble < 10 ? '0' + nibble : 'a' + nibble - 10;
      }
  }
  tmp[pos] = '\0';
  char *p = tmp;
  while (*p == '0' && *(p + 1) != '\0') {
      p++;
  }
  strcpy(buffer, p);
}

void vsprintf(char *buffer, const char *format, va_list args) {
    usize b_pos=0;
    usize f_pos=0;
    bool ext_prefix = false;
    bool v_long = false;
    bool v_quad = false;
    bool zero_pad = false;
    int width = 0;
    while (format[f_pos]) {
        if (format[f_pos] == '%') {
            f_pos++;
            ext_prefix = false;
            v_long = false;
            v_quad = false;
            zero_pad = false;
            width = 0;
            
            if (format[f_pos] == '#') { ext_prefix = true; f_pos++; }
            if (format[f_pos] == '0') { zero_pad = true; f_pos++; }
            
            while (format[f_pos] >= '0' && format[f_pos] <= '9') {
                width = width * 10 + (format[f_pos] - '0');
                f_pos++;
            }
            
            if (format[f_pos] == 'l') { 
                v_long = true; 
                f_pos++; 
                if (format[f_pos] == 'l') {
                    f_pos++; 
                }
            }
            if (format[f_pos] == 'q') { v_quad = true; f_pos++; }
            if (format[f_pos] == '%') {
                buffer[b_pos] = '%';
            }
            else if (format[f_pos] == 'd') {
                s64 i = 0;
                if (v_long) { i = va_arg(args, s64); }
                else { i = va_arg(args, int); }
                char str[64];
                char temp_str[32];
                itoa(i, temp_str, 10);
                
                if (width > 0) {
                    pad_string(str, temp_str, width, zero_pad, false);
                } else {
                    strcpy(str, temp_str);
                }
                
                memcpy(buffer+b_pos, str, strlen(str));
                b_pos += strlen(str)-1;
            }
            else if (format[f_pos] == 'u') {
                u64 i = 0;
                if (v_long) { i = va_arg(args, u64); }
                else { i = va_arg(args, unsigned int); }
                char str[64];
                char temp_str[32];
                
                if (i == 0) {
                    strcpy(temp_str, "0");
                } else {
                    char reversed[32];
                    int pos = 0;
                    while (i > 0) {
                        reversed[pos++] = '0' + (i % 10);
                        i /= 10;
                    }
                    for (int j = 0; j < pos; j++) {
                        temp_str[j] = reversed[pos - 1 - j];
                    }
                    temp_str[pos] = '\0';
                }
                
                if (width > 0) {
                    pad_string(str, temp_str, width, zero_pad, false);
                } else {
                    strcpy(str, temp_str);
                }
                
                memcpy(buffer+b_pos, str, strlen(str));
                b_pos += strlen(str)-1;
            }
            else if (format[f_pos] == 'x') {
                if (v_quad) {
                    u512 v = va_arg(args, u512);
                    char str[128];
                    char temp_str[128];
                    if (ext_prefix) { temp_str[0] = '0'; temp_str[1]='x'; }
                    quadtoa_hex(temp_str + (ext_prefix ? 2 : 0), v);
                    
                    if (width > 0) {
                        pad_string(str, temp_str, width, zero_pad, ext_prefix);
                    } else {
                        strcpy(str, temp_str);
                    }
                    
                    memcpy(buffer+b_pos, str, strlen(str));
                    b_pos += strlen(str)-1;
                } else {
                  s64 i = 0;
                  if (v_long) { i = va_arg(args, s64); }
                  else { i = va_arg(args, int); }
                  char str[64];
                  char temp_str[32];
                  if (ext_prefix) { temp_str[0] = '0'; temp_str[1]='x'; }
                  itoa_hex(i, temp_str + (ext_prefix ? 2 : 0));
                  
                  if (width > 0) {
                      pad_string(str, temp_str, width, zero_pad, ext_prefix);
                  } else {
                      strcpy(str, temp_str);
                  }
                  
                  memcpy(buffer+b_pos, str, strlen(str));
                  b_pos += strlen(str)-1;
                }
            }
            else if (format[f_pos] == 'p') {
                s64 i = va_arg(args, s64);
                char str[64];
                char temp_str[32];
                temp_str[0] = '0';
                temp_str[1] = 'x';
                itoa_hex(i, temp_str+2);
                
                if (width > 0) {
                    pad_string(str, temp_str, width, zero_pad, true);
                } else {
                    strcpy(str, temp_str);
                }
                
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
                char str[64];
                char temp_str[32];
                u8 off = 0;
                int i = va_arg(args, int);
                
                int actual_width = (width > 0) ? width : 2;
                
                if (i < 10 && actual_width >= 2) {
                    temp_str[0] = '0';
                    off = 1;
                }
                itoa(i, temp_str+off, 10);
                
                if (actual_width > (int)strlen(temp_str)) {
                    pad_string(str, temp_str, actual_width, true, false);
                } else {
                    strcpy(str, temp_str);
                }
                
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
