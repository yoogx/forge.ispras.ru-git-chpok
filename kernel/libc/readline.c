#include <bsp.h>
#include <libc.h>

#define BUFLEN 1024
static char buf[BUFLEN];

static char previous_buf[BUFLEN];
static int previous_size=0;

void buf_transfer(char *buf1,char *buf2,int j){
    for (int i=0;i<=j;i++){
        buf2[i]=buf1[i];
    }
}


char *
readline(const char *prompt)
{
    
	if (prompt != NULL)
		printf("%s", prompt);
	int i = 0;
	long int c = 0;	
	while (1) {
		c = getchar();
		if (c < 0) {
			printf("read error\n");
			return NULL;
		}  else if (c == '\x7f' || c == '\b') {//if c=backspace
			if (i > 0) i--;			
			printf("%c", '\r');	
			for (int j=0; j<=i+strlen(prompt); j++){
				printf(" ");
			}
			printf("%c", '\r');
			printf("%s", prompt);
			for (int j=0; j<i; j++){
				printf("%c", buf[j]);
			}
        }  else if (c == '[') { //if c=up arrow it consist of 2 simbols - '[A'
            buf[i]=c;       		
   			printf("%c",buf[i]);
            c = getchar();
            if (c != 'A') { //Check that second simbol = 'A'
                    i++;
                    if (c == '\r'  || c == '\n' ){
                            printf("\n");
                            buf[i] = 0;
                            buf_transfer(buf,previous_buf,i-1);
                            previous_size=i-1;
			                return buf;
                    }
                    buf[i++]=c;
        			printf("%c",buf[i-1]);
                    continue;
            } 
            buf_transfer(previous_buf,buf,previous_size);


			printf("%c", '\r');	
			for (int j=0; j<=i+strlen(prompt); j++){
				printf(" ");
			}
			printf("%c", '\r');
            i=previous_size+1;
			printf("%s", prompt);
			for (int j=0; j<i; j++){
				printf("%c", buf[j]);
			}
		}  else if (c >= ' ' && i < BUFLEN-1) {//if c=simbol
			printf("%c",c);
            buf[i++] = c;
		}  else if (c == '\r'  || c == '\n' ) {//if c=Enter or end of string
			printf("\n");
			buf[i] = 0;
            buf_transfer(buf,previous_buf,i-1);
            previous_size=i-1;
			return buf;
		}
	}
}

