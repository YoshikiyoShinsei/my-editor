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
    short width;
};

void init_mycolors() {
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
}

/*void complement(vector<int> &line_alloc) {
//  assert(line_alloc[0] != -1);
  for (int i = 1; i < line_alloc.size(); i++) {
    if (line_alloc[i] == -1) {
      line_alloc[i] = line_alloc[i-1];
    }
  }
}*/

/*void init_alloc(vector<int> &line_alloc) {
  for (int i=0; i<line_alloc.size(); i++) {
    line_alloc[i] = -1;
  }
}*/

Line* init_lines(int n) {
  Line* head = new Line;
  head->content = "hello";
  Line* prev = new Line;
  head->next = prev;
  prev->prev = head;
  prev->content = "hello--------------------------------------------------------------------------";
  for(int i = 0; i < n - 2; i++) {
    Line* now = new Line;
    now->content = "hello";
    prev->next = now;
    now->prev = prev;
    now->next = NULL;
    prev = now;
  }
  return head;
}

/*void show_lines(Line* head, WINDOW* win, vector<int> &line_alloc) {
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
    line_alloc[y] = i;
    move(y+1, 4);
    now = now->next;
    i++;
  }
//  complement(line_alloc);
}*/

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
  int l = 1;
  while(now != NULL && s < line_alloc.size()) {
   for(int i=0; i<now->width; i++) {
     line_alloc[s] = l;
     s++;
     if(s >= line_alloc.size()) {
       break;
     }
   }
   l++;
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
  Line* lines = init_lines(20);
  /*try {
  show_lines(lines, win, line_alloc);
  } catch(const char* error) {
    endwin();
    cout << error << endl;
    return EXIT_FAILURE;
  }*/
  width_calc(lines, win);
  alloc_reflesh(lines, line_alloc);
  scr_reflesh(lines, win, line_alloc);
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


