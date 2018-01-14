#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void
f_cat(fpath, msg)
  char *fpath;
  char *msg;
{
  int fd;

  if ((fd = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0600)) >= 0)
  {
    strcat(msg,"\n");
    write(fd, msg, strlen(msg));
    close(fd);
  }
}