#include <ncurses.h>
#include <iostream>
#include <string>
#include <cassert>
#include <vector>
using namespace std;

void moveforward(int y, int x, int ymax, int xmax);
void movebackward(int y, int x, int ymax, int xmax);

class Line {
  public:
    Line* prev;
    Line* next;
    string content;
    int ch_num;
};

void init_mycolors() {
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
}

void complement(vector<int> &line_alloc) {
//  assert(line_alloc[0] != -1);
  for (int i = 1; i < line_alloc.size(); i++) {
    if (line_alloc[i] == -1) {
      line_alloc[i] = line_alloc[i-1];
    }
  }
}

void init_alloc(vector<int> &line_alloc) {
  for (int i=0; i<line_alloc.size(); i++) {
    line_alloc[i] = -1;
  }
}

Line* init_lines(int n) {
  Line* head = new Line;
  head->content = "hello";
  Line* prev = new Line;
  head->next = prev;
  prev->prev = head;
  prev->content = "hello--------------------------------------------------------------------------";
  for(int i = 0; i < n-1; i++) {
    Line* now = new Line;
    now->content = "hello";
    prev->next = now;
    now->prev = prev;
    now->next = NULL;
    prev = now;
  }
  return head;
}

void show_lines(Line* head, WINDOW* win, vector<int> &line_alloc) {
  init_alloc(line_alloc);
  Line* now = head;
  int x, y;
  int i = 1;
  move(0, 0);
  while(now->next != NULL) {
    getyx(win, y, x);
    line_alloc[y] = i;
    attrset(COLOR_PAIR(1));
    mvprintw(y, 0, "%2d", i);
    attrset(0);
    mvaddstr(y, 3, now->content.c_str());
    getyx(win, y, x);
    move(y+1, 4);
    now = now->next;
    i++;
  }
  complement(line_alloc);
}

int main() {
  int x,y;
  int xmax, ymax;
  WINDOW* win = initscr();
  noecho();
  nl();
  curs_set(1);
  start_color();
  init_mycolors();
  getmaxyx(win, ymax, xmax);
  vector<int> line_alloc(ymax);
  WINDOW* subwindow = subwin(win, 3, xmax, ymax-3, 0);
  waddstr(subwindow, "This is subwindow");
  Line* lines = init_lines(20);
  try {
  show_lines(lines, win, line_alloc);
  } catch(const char* error) {
    endwin();
    cout << error << endl;
    return EXIT_FAILURE;
  }
  wmove(subwindow, 2, 0);
  for (int i=0; i<ymax; i++) {
    wprintw(subwindow, "%d ", line_alloc[i]);
  }
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


