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
    short width;
    int number;
};

Line* getLine(Line* head, int number) {
  Line* now = head;
  while(now != NULL) {
    if(now->number == number) {
      return now;
    }
    now = now->next;
  }
  return NULL;
}

void init_mycolors() {
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
}

Line* init_lines(int n) {
  Line* head = new Line;
  head->content = "hello";
  head->number = 1;
  Line* prev = new Line;
  head->next = prev;
  prev->prev = head;
  prev->content = "hello--------------------------------------------------------------------------";
  prev->number = 2;
  for(int i = 0; i < n - 2; i++) {
    Line* now = new Line;
    now->content = "hello";
    now->number = i+3;
    prev->next = now;
    now->prev = prev;
    now->next = NULL;
    prev = now;
  }
  return head;
}

void width_calc(Line* head, WINDOW* win) {
  int xmax, ymax;
  getmaxyx(win, ymax, xmax);
  Line* now = head;
  while(now != NULL) {
    now->width = (now->content.size() + xmax - 4)/(xmax - 3); // 整数切り上げ
    now = now->next;
  }
}

void alloc_reflesh(Line* head, vector<int> &line_alloc) {
  Line* now = head;
  fill(line_alloc.begin(), line_alloc.end(), -1);
  int s = 0;
  while(now != NULL && s < line_alloc.size()) {
   for(int i=0; i<now->width; i++) {
     line_alloc[s] = now->number;
     s++;
     if(s >= line_alloc.size()) {
       break;
     }
   }
   now = now->next;
  }
}


void scr_reflesh(Line* head, WINDOW* win, vector<int> &line_alloc) {
  int x, y;
  int xmax, ymax;
  getmaxyx(win, ymax, xmax);
  // 行番号を出力
  int tmp = 0;
  for(int i=0; i<ymax; i++) {
    if(line_alloc[i] > 0 && line_alloc[i] != tmp) {
      attrset(COLOR_PAIR(1));
      mvprintw(i, 0, "%2d", line_alloc[i]);
      attrset(0);
      tmp = line_alloc[i];
    }
  }
  // 行内容を出力
  Line* now = head;
  move(0, 0);
  while(now != NULL) {
    getyx(win, y, x);
    for(int w=0; w<now->width; w++) {
      mvaddnstr(y+w, 3, now->content.c_str()+(xmax-3)*w, xmax-3);
    }
    printw(" %d", now->width);
    move(y+now->width, 0);
    now = now->next;
  }
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
  Line* head = init_lines(20);
  width_calc(head, win);
  alloc_reflesh(head, line_alloc);
  scr_reflesh(head, win, line_alloc);
  wmove(subwindow, 2, 0);
  for (int i=0; i<ymax; i++) {
    wprintw(subwindow, "%d ", line_alloc[i]);
  }
  move(0, 0);
  Line* now = head;
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

