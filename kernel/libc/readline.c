#include <libc.h>


#define BUFMEM 10
#define BUFLEN 1024

static char buf[BUFLEN];

struct buff {
    char buf[BUFLEN];
    int size;
    pok_bool_t head;
    pok_bool_t tail;
};

/*
 * TRUE if we have more than 10 written commands
 */

pok_bool_t more_than_2_in_MEM = FALSE;

struct buff buffers[BUFMEM];

int current_buf_number = -1;       //Current number in buffers that we have printed on the screen

int number_of_last_buf = -1;          //Position in buf memory of the last written buf

void buf_transfer(char *buf1,char *buf2,int j){
    memset(buf2, 0, BUFLEN);
    for (int i = 0; i <= j; i++){
        buf2[i] = buf1[i];
    }
}

int upper_number_in_mem(int mem){
    if (buffers[mem].tail) 
        return mem;
    if (mem > 0)
        return mem - 1;
    return BUFMEM - 1;
}

int lower_number_in_mem(int mem){
    if (buffers[mem].head)
        return mem;
    return (mem + 1) % BUFMEM;
}

void clear_console(int lenght){
    printf("%c", '\r'); 
    for (int j = 0; j <= lenght + 1; j++)
        printf(" ");
}
void update_console(char * buf,int end,const char * prompt){
    clear_console(end+strlen(prompt));
    printf("%c", '\r');
    printf("%s", prompt);
    for (int j = 0; j < end; j++){
        printf("%c", buf[j]);
    }
}



char *
readline(const char *prompt)
{
    
    if (prompt != NULL)
        printf("%s", prompt);
    int i = 0;
    int c = 0;  
    while (1) {
        c = getchar();
        if (c < 0) {
            printf("read error\n");
            return NULL;
        }  else if (c == '\x7f' || c == '\b') {//if c=backspace
            if (i > 0) i--;         
            update_console(buf,i,prompt);
        }  else if (c == '[') { //if c=up arrow it consist of 2 simbols - '[A' or down arrow -'[B'
            buf[i] = c;               
            printf("%c",buf[i]);
            c = getchar();
            if (c != 'A' && c !='B') { //Check that second simbol != 'A' or 'B'
                    i++;
                    if (c == '\r'  || c == '\n' ){//if c=Enter or end of string
                        printf("\n");
                        if (number_of_last_buf != -1){
                            int prev=number_of_last_buf;
                            number_of_last_buf = (number_of_last_buf + 1) % BUFMEM;
                            current_buf_number = BUFMEM;
                            if (more_than_2_in_MEM){
                                buffers[(number_of_last_buf + 1) % BUFMEM].tail=buffers[number_of_last_buf].tail | buffers[(number_of_last_buf + 1) % BUFMEM].tail;
                                buffers[prev].tail=FALSE;
                                buffers[number_of_last_buf].tail=FALSE;
                            }
                            buffers[prev].head=FALSE;
                            if (number_of_last_buf > 1)
                                more_than_2_in_MEM=TRUE;
                            }else{
                                number_of_last_buf = (number_of_last_buf + 1) % BUFMEM;
                                current_buf_number = BUFMEM;
                                buffers[0].tail=TRUE;
                            }
                        buf[i] = 0;
                        buf_transfer(buf,buffers[number_of_last_buf].buf,i - 1);
                        buffers[number_of_last_buf].size = i - 1;
                        buffers[number_of_last_buf].head=TRUE;
                        return buf;
                    }
                    buf[i++] = c;
                    printf("%c",buf[i - 1]);
                    continue;
            }
            if (c == 'A' && current_buf_number != -1){
                if (current_buf_number == BUFMEM){
                    buf_transfer(buffers[number_of_last_buf].buf,buf,buffers[number_of_last_buf].size);
                    clear_console(i + strlen(prompt));
                    i = buffers[number_of_last_buf].size + 1;
                    update_console(buf,i,prompt);
                    current_buf_number = number_of_last_buf;
                }else{    
                    buf_transfer(buffers[upper_number_in_mem(current_buf_number)].buf,buf,buffers[upper_number_in_mem(current_buf_number)].size);
                    clear_console(i + strlen(prompt));
                    i = buffers[upper_number_in_mem(current_buf_number)].size + 1;
                    update_console(buf,i,prompt);
                    current_buf_number = upper_number_in_mem(current_buf_number);
            }
            }else if (c == 'B' && current_buf_number != -1 && current_buf_number != BUFMEM){
                buf_transfer(buffers[lower_number_in_mem(current_buf_number)].buf,buf,buffers[lower_number_in_mem(current_buf_number)].size);
                clear_console(i + strlen(prompt));
                i = buffers[lower_number_in_mem(current_buf_number)].size + 1;
                update_console(buf,i,prompt);
                current_buf_number = lower_number_in_mem(current_buf_number);
            }
            clear_console(i + strlen(prompt));
            update_console(buf,i,prompt);
        }  else if (c >= ' ' && i < BUFLEN - 1) {//if c=simbol
            printf("%c",c);
            buf[i++] = c;
        }  else if (c == '\r'  || c == '\n' ) {//if c=Enter or end of string
            printf("\n");
            if (number_of_last_buf != -1){
                int prev=number_of_last_buf;
                number_of_last_buf = (number_of_last_buf + 1) % BUFMEM;
                current_buf_number = BUFMEM;
                if (more_than_2_in_MEM){
                    buffers[(number_of_last_buf + 1) % BUFMEM].tail=buffers[number_of_last_buf].tail | buffers[(number_of_last_buf + 1) % BUFMEM].tail;
                    buffers[prev].tail=FALSE;
                    buffers[number_of_last_buf].tail=FALSE;
                }
                buffers[prev].head=FALSE;
                if (number_of_last_buf > 1)
                    more_than_2_in_MEM=TRUE;
            }else{
                number_of_last_buf = (number_of_last_buf + 1) % BUFMEM;
                current_buf_number = BUFMEM;
                buffers[0].tail=TRUE;
            }
            buf[i] = 0;
            buf_transfer(buf,buffers[number_of_last_buf].buf,i - 1);
            buffers[number_of_last_buf].size = i - 1;
            buffers[number_of_last_buf].head=TRUE;
            return buf;
        }
    }
}

