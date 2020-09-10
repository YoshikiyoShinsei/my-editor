#include <ncurses.h>
#include <iostream>
#include <string>

void moveforward(int y, int x, int ymax, int xmax);
void movebackward(int y, int x, int ymax, int xmax);

int main() {
  int x,y;
  int xmax, ymax;
  WINDOW* win = initscr();
  noecho();
  nl();
  curs_set(1);
  getmaxyx(win, ymax, xmax);
  WINDOW* subwindow = subwin(win, 3, xmax, ymax-3, 0);
  waddstr(subwindow, "This is subwindow");
  while(true) {
    getyx(win, y, x);
    wmove(subwindow, 1, 0);
    wclrtoeol(subwindow);
    mvwprintw(subwindow, 1, 0, "y: %d, x: %d, ymax:%d, xmax: %d", y, x, ymax, xmax);
    wrefresh(subwindow);
    int c = getch();
    if(c == 127) {
      addch('\b');
      addch(' ');
      addch('\b');
    } else if(c == 10) {
      addch('\n');
    } else if (c == 6) {
      moveforward(y, x, ymax, xmax);
    } else if(c == 2) {
      movebackward(y, x, ymax, xmax);
    } else if(c == 14) {
      move(y+1, x);
    } else if(c == 16) {
      move(y-1, x);
    } else {
      addch(c);
    }
  }
  endwin();
}

void moveforward(int y, int x, int ymax, int xmax) {
  if(x == xmax - 1) {
    move(y+1, 0);
  } else {
    move(y, x+1);
  }
}

void movebackward(int y, int x, int ymax, int xmax) {
  if (x == 0) {
    move(y-1, xmax-1);
  } else {
    move(y, x-1);
  }
}

