#include <fcntl.h>
#include <unistd.h>


int
rec_get(fpath, data, size, pos)
  char *fpath;
  void *data;
  int size, pos;
{
  int fd;
  int ret;

  ret = -1;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    if (lseek(fd, (off_t) (size * (pos-1)), SEEK_SET) >= 0)
    {
      if (read(fd, data, size) == size)
      {
	close(fd);
	ret = 0;
	return 0;
      }
    }
    close(fd);
  }
  return ret;
}
