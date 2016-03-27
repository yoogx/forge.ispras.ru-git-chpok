/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

#include <config.h>

#if defined (POK_NEEDS_DEBUG) || defined (POK_NEEDS_INSTRUMENTATION) || defined (POK_NEEDS_COVERAGE_INFOS)

#include <types.h>
#include <libc.h>
#include <stdarg.h>
#include <bsp_common.h>

static const char digits[] = "0123456789abcdef";

#define BUF_SIZE 128

/*
 * some types
 */

struct s_file
{
    char   buff[BUF_SIZE];
    size_t pos;
};

struct s_sprintf
{
    char  *ptr;
    size_t size;
};

typedef void (*t_putc)(int val, void *out);

/*
 * buffered I/O
 */

static void buf_flush(struct s_file *file)
{
    pok_cons_write (file->buff, file->pos);
    file->pos = 0;
}

static struct s_file* init_buffered_output(void)
{
    static struct s_file res;

    res.pos = 0;
    return &res;
}

static void buf_putc(int c, void *out)
{
    if (c < 0) 
        return;

    struct s_file * file = out;
    file->buff[file->pos++] = c;

    if (file->pos == BUF_SIZE)
        buf_flush(file);
}

static void sprintf_putc(int c, void *out)
{
    if (c < 0) 
        return;

    struct s_sprintf *out_s = out;

    switch (out_s->size)
    {
        case 1:
            c = 0;
        default:
            *(out_s->ptr++) = c;
             out_s->size--;
        case 0:
            break;
    }
}

static void close_buffered_output(struct s_file *file)
{
    buf_flush(file);
}

static void print_num(t_putc putc,
                      void *out,
                      unsigned long long value, 
                      unsigned base, 
                      int pad,
                      int neg,
                      int pad_with_zero)
{
    unsigned char digit_str[32]; //64-bit number in octal base is 32 digits long
    int size = 0;
    do {
        unsigned mod = value%base;
        value /= base;
        digit_str[size] = digits[mod];
        size++;

    } while (value);

    if (pad_with_zero) {
        if (neg) {
            putc('-', out);
            pad--;
        }
        while (size < pad--)
            putc('0', out);

    } else {
        if (neg)
            pad--;

        while (size < pad--)
            putc(' ', out);

        if (neg)
            putc('-', out);
    }

    while (size--)
        putc(digit_str[size], out);
}

/* DON'T USE floating point register in kernel
static void print_float(t_putc putc,
                        void *out,
                        long double value, 
                        int pad, 
                        int precision, 
                        int neg,
                        int pad_with_zero)
{
    long long floor = value;
    long double fractional = 0;

    print_num(putc, out, floor, 10, pad-(precision+1), neg, pad_with_zero);

    putc('.', out);

    fractional = (value - floor);
    for (int i=0; i<precision; i++)
        fractional *= 10;
    print_num(putc, out, fractional, 10, precision, 0, 1);
}
*/

/*
 * finally, printf
 */
//XXX for now precision is used only for floating points
const char * handle_fmt(t_putc putc, void * out, const char* format, va_list *args)
{
    unsigned l = 0;
    int pad = 0;
    int pad_with_zero = 0; // if 1 than add zeros for padding
    int precision = 0;
    int was_dot = 0; //if 1 than there were '.' in fmt before
    while (*format) {
        switch (*format) {
            case '0':
                if (was_dot)
                    precision *= 10;
                else if (pad == 0)
                    pad_with_zero = 1;
                else
                    pad *= 10;
                format++;
                break;
            case '1'...'9':
                if (was_dot)
                    precision = *format - '0' + precision*10;
                else
                    pad = *format - '0' + pad*10;
                format++;
                break;
            case 'l':
                l++;
                format++;
                break;
           case '.':
                was_dot = 1;
                format++;
                break;
           case 'z':		// size_t modifier
                l = 0;
                format++;
                break;
           case 's':
                {
                    //TODO implement l!=0 case
                    const char *s = va_arg(*args, const char *);
                    if (!s)
                        s = (const char *)"(null)";
                    int len = strlen(s);

                    if (precision != 0 && precision<len)
                        len = precision;

                    while (len < pad--)
                        putc(' ', out);
                    while (*s && len-- ) 
                        putc(*s++, out);
                    return ++format;
                }
           case 'p':
                putc('0', out);
                putc('x', out);
           case 'x':
                {
                    long long value;

                    if (l == 2)
                        value = va_arg(*args, unsigned long long);
                    else if (l == 1)
                        value = va_arg(*args, unsigned long);
                    else 
                        value = va_arg(*args, unsigned );

                    print_num(putc, out, value, 16, pad, 0, pad_with_zero);
                    return ++format;
                }
           case 'd':
           case 'i':
                {
                    long long value;
                    if (l == 2)
                        value = va_arg(*args, long long);
                    else if (l == 1)
                        value = va_arg(*args, long);
                    else 
                        value = va_arg(*args, int);
                    int neg = value < 0;
                    if (neg)
                        value = -value;
                    print_num(putc, out, value, 10, pad, neg, pad_with_zero);
                    return ++format;
                }
            case 'u':
                {
                    long long value;
                    if (l == 2)
                        value = va_arg(*args, unsigned long long);
                    else if (l == 1)
                        value = va_arg(*args, unsigned long);
                    else 
                        value = va_arg(*args, unsigned );

                    print_num(putc, out, value, 10, pad, 0, pad_with_zero);
                    return ++format;
                }
            /* DON'T USE floating point register in kernel
            case 'f':
                {
                    long double value;
                    if (l == 2)
                        value = va_arg(*args, long double);
                    else
                        value = va_arg(*args, double);
                    int neg = value < 0;
                    if (neg)
                        value = -value;
                    if (precision == 0)
                        precision = 6;
                    print_float(putc, out, value, pad, precision, neg, pad_with_zero);
                    return ++format;
                }
            */
            case 'c':
                //TODO implement l!=0 case
                putc(va_arg(*args, unsigned ), out);
                return ++format;
            case '%':
                putc('%', out);
                return ++format;
            case 0:
                return format;
            default:
                putc('%', out);
                putc(*format, out);
                return ++format;
        }
    }
    return format;
}

void vprintf(t_putc putc, void *out, const char* format, va_list *args)
{
    while(*format) {
        if (*format == '%') {
            format++;
            format = handle_fmt(putc, out, format, args);
        }
        else
            putc(*format++, out);
    }
}

//Why no void?!
int printf(const char *format, ...)
{
    va_list args;
    struct s_file* out_file = init_buffered_output();

    va_start(args, format);
    vprintf (buf_putc, out_file, format, &args);
    va_end(args);

    close_buffered_output(out_file);
    return 0;
}

void snprintf(char *dst, unsigned size, const char *format, ...)
{
    struct s_sprintf out;
    out.ptr = dst;
    out.size = size;

    va_list args;
    va_start(args, format);
    vprintf(sprintf_putc, &out, format, &args);
    sprintf_putc('\0', &out);
    va_end(args);
}


#endif
