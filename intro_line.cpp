#include <ncurses.h>
#include <iostream>
#include <string>
#include <cassert>
#include <vector>
using namespace std;

class Line {
  public:
    Line* prev;
    Line* next;
    string content;
    short width;
    int number;
};

class Cursor {
  public:
    int y, x;
    int ymax, xmax;
    int line_num;
    short width_num;
    int w_offset;
    Line* line;
    void set_line(Line*);
    void cmove(int, int);
};

void moveforward(Cursor* cursor);
void movebackward(Cursor* cursor);
void moveup(Cursor* cursor, vector<int> line_alloc);
void movedown(Cursor* cursor, vector<int> line_alloc);

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

void Cursor::set_line(Line* head) {
  this->line = getLine(head, this->line_num);
}

void Cursor::cmove(int y, int x) {
  this->y = y;
  this->x = x;
  move(y, x);
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

void renumber(Line* now_line) { // 信頼できるnumberを持ったLine*をもらう
  Line* now = now_line->next;
  int number = now_line->number;
  while(now != NULL) {
    now->number = ++number;
    now = now->next;
  }
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
  erase();
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
    move(y+now->width, 0);
    now = now->next;
  }
}

Cursor* cursor_init(int ymax, int xmax, Line* head) {
  move(0, 3);
  Cursor* cursor = new Cursor;
  cursor->y = 0;
  cursor->x = 3;
  cursor->ymax = ymax;
  cursor->xmax = xmax;
  cursor->line_num = 1;
  cursor->width_num = 1;
  cursor->w_offset = 0;
  cursor->line = head;
  return cursor;
}

void cursor_set(Cursor* cursor, vector<int> line_alloc) {
  for(int i = 0; i < line_alloc.size(); i++) {
    if(line_alloc[i] == cursor->line_num) {
      cursor->cmove(i + cursor->w_offset / (cursor->xmax - 3), 3 + cursor->w_offset % (cursor->xmax - 3));
      return;
    } 
  }
  throw;
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
  Cursor* cursor = cursor_init(ymax, xmax, head);
  while(true) {
    getyx(win, y, x);
    alloc_reflesh(head, line_alloc);
    scr_reflesh(head, win, line_alloc);
    move(y, x);

    // reflesh subwindow
    wmove(subwindow, 1, 0);
    wclrtoeol(subwindow);
    mvwprintw(subwindow, 1, 0, "y: %d, x: %d, ymax:%d, xmax: %d", y, x, ymax, xmax);
    mvwprintw(subwindow, 2, 0, "Cursor, line_num: %d, width_num: %d, w_offset: %d", cursor->line_num, cursor->width_num, cursor->w_offset);
    wrefresh(subwindow);
    
    int c = getch();
    if(c == 127) {
      addch('\b');
      addch(' ');
      addch('\b');
    } else if(c == 10) {
      addch('\n');
    } else if (c == 6) {
      moveforward(cursor);
    } else if(c == 2) {
      movebackward(cursor);
    } else if(c == 14) {
      movedown(cursor, line_alloc);
    } else if(c == 16) {
      moveup(cursor, line_alloc);
    } else {
      addch(c);
    }
  }
  endwin();
}

void moveforward(Cursor* cursor) {
  if(cursor->w_offset == cursor->line->content.size()) {
  } else {
    cursor->w_offset++;
    if(cursor->w_offset % (cursor->xmax - 3) == 0) {
      cursor->width_num++;
      cursor->cmove(cursor->y + 1, 3);
    } else {
      cursor->cmove(cursor->y, cursor->x + 1);
    }
  }
}

void movebackward(Cursor* cursor) {
  if(cursor->w_offset == 0) {
  } else {
    cursor->w_offset--;
    if(cursor->w_offset % (cursor->xmax -3) == cursor->xmax - 4) {
      cursor->width_num--;
      cursor->cmove(cursor->y - 1, cursor->xmax - 1);
    } else {
      cursor->cmove(cursor->y, cursor->x - 1);
    }
  }
}

void moveup(Cursor* cursor, vector<int> line_alloc) {
  if(cursor->y == 0) {
  } else if(cursor->line->prev == NULL) {
  } else {
    cursor->line_num--;
    cursor->line = cursor->line->prev;
    cursor->w_offset = 0;
    cursor_set(cursor, line_alloc);
  }
}

void movedown(Cursor* cursor, vector<int> line_alloc) {
  if(cursor->y == cursor->ymax - 3) {
  } else if(cursor->line->next == NULL) {
  } else {
    cursor->line_num++;
    cursor->line = cursor->line->next;
    cursor->w_offset = 0;
    cursor_set(cursor, line_alloc);
  }
}

void insert_line(Cursor* cursor) {
  Line* now_line = cursor->line;
  Line* new_line = new Line;
  new_line->next = now_line->next;
  new_line->prev = now_line;
  if(cursor->line->next != NULL) {
    now_line->next->prev = new_line;
  }
  now_line->next = new_line;
  renumber(now_line);
}

