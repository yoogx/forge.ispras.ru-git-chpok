#include <bsp.h>
#include <libc.h>


#define BUFMEM 10
#define BUFLEN 1024

static char buf[BUFLEN];

struct buff {
    char buf[BUFLEN];
    int size;
};

/*
 * TRUE if we have more than 10 written commands
 */

static pok_bool_t more_than_BUFMEM = FALSE;

static struct buff previous_buffers[BUFMEM];

static int current_buf_number = 0;       //Current number in previous_buffers that we have printed on the screen

static int number_of_last_buf = 0;          //Position in buf memory of the last written buf

static void buf_transfer(char *buf1,char *buf2,int j){
    memset(buf2, 0, BUFLEN);
    for (int i = 0; i <= j; i++){
        buf2[i] = buf1[i];
    }
}

static int left_number_in_mem(int mem){
    if (mem != 0) 
        return mem - 1;
    if (more_than_BUFMEM)
        return BUFMEM - 1;
    return mem;
}

static int right_number_in_mem(int mem){
    return (mem + 1) % BUFMEM;
}

static void clear_console(int lenght){
    printf("%c", '\r'); 
    for (int j = 0; j <= lenght + 1; j++)
        printf(" ");
}
static void update_console(char * buf,int end,const char * prompt){
    clear_console(end+strlen(prompt));
    printf("%c", '\r');
    printf("%s", prompt);
    for (int j = 0; j < end; j++){
        printf("%c", buf[j]);
    }
}



char *
readline2(const char *prompt)
{
    
    if (prompt != NULL)
        printf("%s", prompt);
    int i = 0;
    int c = 0;  
    while (1) {
        c = getchar2();
        if (c < 0) {
            printf("read error\n");
            return NULL;
        }  else if (c == '\x7f' || c == '\b') {//if c=backspace
            if (i > 0) i--;         
            update_console(buf,i,prompt);
        }  else if (c == '[') { //if c=up arrow it consist of 2 simbols - '[A' or down arrow -'[B'
            buf[i] = c;               
            printf("%c",buf[i]);
            c = getchar2();
            if (c != 'A' && c !='B') { //Check that second simbol != 'A' or 'B'
                    i++;
                    if (c == '\r'  || c == '\n' ){//if c=Enter or end of string
                            printf("\n");
                            buf[i] = 0;
                            buf_transfer(buf,previous_buffers[number_of_last_buf].buf,i - 1);
                            previous_buffers[number_of_last_buf].size = i - 1;
                            if (number_of_last_buf + 1 == BUFMEM) more_than_BUFMEM = TRUE;
                            number_of_last_buf = (number_of_last_buf + 1) % BUFMEM;
                            current_buf_number = number_of_last_buf;
                            return buf;
                    }
                    buf[i++] = c;
                    printf("%c",buf[i - 1]);
                    continue;
            }
            if (c == 'A'){
                buf_transfer(previous_buffers[left_number_in_mem(current_buf_number)].buf,buf,previous_buffers[left_number_in_mem(current_buf_number)].size);
                clear_console(i + strlen(prompt));
                i = previous_buffers[left_number_in_mem(current_buf_number)].size + 1;
                update_console(buf,i,prompt);
                current_buf_number = left_number_in_mem(current_buf_number);
            }else if (c == 'B'){
                buf_transfer(previous_buffers[right_number_in_mem(current_buf_number)].buf,buf,previous_buffers[right_number_in_mem(current_buf_number)].size);
                clear_console(i + strlen(prompt));
                i = previous_buffers[right_number_in_mem(current_buf_number)].size + 1;
                update_console(buf,i,prompt);
                if (more_than_BUFMEM || current_buf_number + 2 < number_of_last_buf)
                    current_buf_number = right_number_in_mem(current_buf_number);
            }
        }  else if (c >= ' ' && i < BUFLEN - 1) {//if c=simbol
            printf("%c",c);
            buf[i++] = c;
        }  else if (c == '\r'  || c == '\n' ) {//if c=Enter or end of string
            printf("\n");
            buf[i] = 0;
            buf_transfer(buf,previous_buffers[number_of_last_buf].buf,i - 1);
            previous_buffers[number_of_last_buf].size = i - 1;
            if (number_of_last_buf + 1 == BUFMEM) more_than_BUFMEM = TRUE;
            number_of_last_buf = (number_of_last_buf + 1) % BUFMEM;
            current_buf_number = number_of_last_buf;
            return buf;
        }
    }
}

