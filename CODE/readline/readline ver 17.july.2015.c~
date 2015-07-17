/* We need functions:
*	-printf()
*	-getchar()
*/

//FIXME add include

#define BUFLEN 1024
static char buf[BUFLEN];

char *
readline(const char *prompt)
{
	int i, c;

	if (prompt != NULL)
		printf("%s", prompt);

	i = 0;
	while (1) {
		c = getchar();
		if (c < 0) {
			printf("read error\n");
			return NULL;
		}  else if (c >= ' ' && i < BUFLEN-1) {//if c=simbol
			printf("%c",c);
			buf[i++] = c;
		} else if (c == '\n' || c == NULL) {//if c=Enter or end of string
			printf("\n");
			buf[i] = 0;
			return buf;
		}
	}
}

