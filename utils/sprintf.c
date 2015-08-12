#include "common.h"

char* itoa(int64_t val, char *ostr, int base, int sign)
{
        char str[512];
        char *opts = "0123456789ABCDEF";
        if(base == 16) {
                for(int i = 0; i < 16; i++)
                {
                        str[15 - i] = opts[((val >> (i*4))&0x0F)];
                }
                str[16] = 0;
        }else if(base == 2)
        {
                for(int i = 0; i < 64; i++)
                {
                        str[63 - i] = opts[(val >> i) & 1];
                }
                str[64] = 0;
        }else if(base < 16)
        {
                int pos = 0;
                int negative = 0;
                if(val < 0) {
                        negative = 1;
                        val = -val;
                }
                do {
                        str[pos++] = opts[val % base];
                        val /= base;
                }
                while(val != 0);

                if(sign && negative) str[pos++] = '-';
                str[pos] = 0;
                strrev(str);
        }else{
                str[0] = 0;
        }
        memcpy(ostr, str, strlen(str));
        return ostr + strlen(str);
}

int vsnprintf ( char * str, const char * format, va_list vl )
{
        int n = 0;

        //Parse the format string
        for(size_t i = 0; i < strlen(format); i++)
        {
                if(format[i] == '%' && format[i + 1] != '%') n++;
        }

        //Generate the real string
        for(size_t i = 0; i < strlen(format); i++)
        {
                if(format[i] != '%')
                {
                        *str = format[i];
                        str++;
                }else{
                        i++;
                        int in_format = 1;
                        char padding_char = ' ';
                        int padding_size = 0;
                        int precision = 0;
                        while(in_format) {
                                //Perform a substitution here based on the specifier
                                switch(format[i])
                                {
                                case '0':
                                        padding_char = '0';
                                        break;
                                case '#':
                                        *str++ = '0';
                                        *str++ = 'x';
                                        i++;
                                        break;
                                case '1':
                                case '2':
                                case '3':
                                case '4':
                                case '5':
                                case '6':
                                case '7':
                                case '8':
                                case '9':
                                        while( (format[i] - 0x30) <= 9 && (format[i] - 0x30) >= 0) {
                                                padding_size = (padding_size * 10) + (format[i] - 0x30);
                                                i++;
                                        }
                                        break;
                                case '*':
                                        padding_size = va_arg(vl, int);
                                        break;
                                case '.':
                                        switch (format[++i]) {
                                        case '*':
                                                precision = va_arg(vl, int);
                                                break;
                                        default:
                                                while( (format[i] - 0x30) <= 9 && (format[i] - 0x30) >= 0) {
                                                        padding_size = (padding_size * 10) + (format[i] - 0x30);
                                                        i++;
                                                }
                                                break;
                                        }
                                        break;
                                case 'd':
                                case 'i':
                                {
                                        int len = itoa(va_arg(vl, int), str++, 10, 1) - str;
                                        str += len;
                                        while(len < padding_size)
                                        {
                                                *str++ = padding_char;
                                                len++;
                                        }
                                        in_format = 0;
                                }
                                break;
                                case 'u':
                                        str = itoa(va_arg(vl, int), str, 10, 0);
                                        in_format = 0;
                                        break;
                                case 'b':
                                        str = itoa(va_arg(vl, int), str, 2, 0);
                                        in_format = 0;
                                        break;
                                case 'o':
                                        str = itoa(va_arg(vl, int), str, 8, 0);
                                        in_format = 0;
                                        break;
                                case 'x':
                                case 'X':
                                        str = itoa(va_arg(vl, int), str, 16, 0);
                                        str++;
                                        in_format = 0;
                                        break;
                                case 'f':
                                case 'F':
                                        str++;
                                        in_format = 0;
                                        break;
                                case 'c':
                                        *str++ = va_arg(vl, int) & 0xFF;
                                        in_format = 0;
                                        break;
                                case 's':
                                {
                                        const char *arg = va_arg(vl, char*);
                                        memcpy(str, (void*)arg, strlen(arg));
                                        str += strlen(arg);
                                        in_format = 0;
                                }
                                break;
                                case '%':
                                        *str++ = '%';
                                        in_format = 0;
                                        break;
                                }
                        }
                }
        }

        *str = 0;
        return 0;
}

int sprintf ( char * str, const char * format, ... )
{
        va_list vl;
        va_start(vl, format);
        vsnprintf(str, format, vl);
        va_end(vl);
        return 0;
}