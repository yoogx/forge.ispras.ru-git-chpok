
#include <libc.h>

#define BUFLEN 1024
static char buf[BUFLEN];

char *
readline(const char *prompt)
{
	if (prompt != NULL)
		printf("%s", prompt);

	int i = 0;
	int c = 0;	
	while (1) {
		c = getchar();
//		printf("%d\n",c);
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
		}  else if (c >= ' ' && i < BUFLEN-1) {//if c=simbol
			printf("%c",c);
			buf[i++] = c;
		} else if (c == '\r' /*='\r'*/ || c == '\n' /*='\n'*/) {//if c=Enter or end of string
			printf("\n");
			buf[i] = 0;
			return buf;
		}
	}
}

