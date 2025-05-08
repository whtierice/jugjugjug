/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "../csapp.h"

int is_valid_int(char *s)
{
  int len = strlen(s);

  if (len > 10)
    return 0;
  if (len < 10)
    return 1;

  return strcmp(s, "2147483647") <= 0;
}

int main(void)
{
  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0;

  int max_int = ((1 << 31) - 1);
  /* Extract the two arguments */
  if ((buf = getenv("QUERY_STRING")) != NULL)
  {
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(arg1, buf);
    strcpy(arg2, p + 1);

    // 정수 최댓값 체크
    int val_check1 = is_valid_int(arg1);
    int val_check2 = is_valid_int(arg2);

    if (val_check1 < 1 || val_check2 < 1)
    {
      sprintf(content, "The number is out of range. Please check the input value.\r \n");
      printf("Content-type: text/html\r\n");
      printf("Content-length: %d\r\n", (int)strlen(content));
      printf("\r\n");
      printf("%s", content);
      fflush(stdout);
    
      exit(0);
    }

    n1 = atoi(strchr(arg1, '=') + 1);
    n2 = atoi(strchr(arg2, '=') + 1);
  }
  /* Make the response body */
  sprintf(content, "A=%d\r\n", n1);
  sprintf(content + strlen(content), "B=%d\r\n<p>", n2);
  sprintf(content + strlen(content), "Welcome to add.com: ");
  sprintf(content + strlen(content), "THE Internet addition portal.\r\n<p>");
  sprintf(content + strlen(content), "The answer is: %d + %d = %d\r\n<p>",
          n1, n2, n1 + n2);
  sprintf(content + strlen(content), "Thanks for visiting!\r\n");

  /* Generate the HTTP response */
  printf("Content-type: text/html\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("\r\n");
  printf("%s", content);
  fflush(stdout);

  exit(0);
}
/* $end adder */
