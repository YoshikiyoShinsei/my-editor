#include <termios.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

struct termios orig_termios;

void reset_terminal_mode()
{
  tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
  struct termios new_termios;

  /* take two copies - one for new, one for later */
  tcgetattr(0, &orig_termios);
  memcpy(&new_termios, &orig_termios, sizeof(new_termios));

  /*v register clearnup handler, and set the new terminal mode */
  atexit(reset_terminal_mode);
  cfmakeraw(&new_termios);
  tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
  struct timeval tv = { 0L, 0L };
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(0, &fds);
  return select(1, &fds, NULL, NULL, &tv);
}

int getch()
{
  int r;
  unsigned char c;
  if ((r = read(0, &c, sizeof(c))) < 0) {
    return r;
  } else {
    return c;
  }
}

int main(int argc, char *argv[])
{
  set_conio_terminal_mode();
  char buf;

  while(true) {
     /* do some work */
    if (kbhit()) {
      buf = getch();
      if ((int)buf == 127) {
	std::cout << '\b' << ' ' << '\b' << std::flush;
      } else if ((int)buf == 13) {
	std::cout << '\r' << std::endl;
      } else if ((int)buf == 3) {
	break;
      } else {
	std::cout << buf << std::flush;
      }
    }
  }
}

