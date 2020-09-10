#include <ncurses.h>
#include <locale.h>

int main(void) {
  initscr();
  noecho();
  curs_set(0);
  mvprintw(12, 30, "Hello World");
  while(true) {
    int ch = getch();
    if (ch == 'q') break;
  }
  endwin();
}

