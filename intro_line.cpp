#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <algorithm>
using namespace std;

class Cursor;

class Line {
  public:
    Line* prev;
    Line* next;
    string content;
    short width;
    int number;
    void calc_width(Cursor*);
};

class Cursor {
  public:
    int y, x;
    int ymax, xmax;
    int line_num;
    short width_num;
    int w_offset;
    Line* line;
    Line* head;
    void set_line(Line*);
    void cmove(int, int);
};

void moveforward(Cursor* cursor);
void movebackward(Cursor* cursor);
void moveup(Cursor* cursor, vector<int> &line_alloc);
void movedown(Cursor* cursor, vector<int> &line_alloc);
void insert_line(Cursor* cursor, vector<int> &line_alloc);
void insert_ch(Cursor* cursor, vector<int> &line_alloc, int c);
void backspace(Cursor* cursor, vector<int> &line_alloc);
void save(Line* head, string path);

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

void Line::calc_width(Cursor* cursor) {
  this->width = this->content.size() / (cursor->xmax - 3) + 1;
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

Line* init_lines(string path) {
  ifstream ifs(path);
  if (ifs.fail()) {
    cout << "error" << endl;
    return NULL;
  }
  Line* head = new Line;
  head->number = 1;
  string tmp_str;
  if (getline(ifs, tmp_str)) {
    string s(tmp_str);
    head->content = s;
  }
  Line* tail = head;
  while(getline(ifs, tmp_str)) {
    Line* next = new Line;
    string s(tmp_str);
    next->content = s;
    next->prev = tail;
    next->next = NULL;
    next->number = tail->number+1;
    tail->next = next;
    tail = next;
  }
  head->prev = NULL;
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
    if (now->content.size() == 0) {
      now->width = 1;
    } else {
      now->width = (now->content.size() + xmax - 4)/(xmax - 3); // 整数切り上げ
    }
    if (now->width == 0) {
      cerr << "error: width_calc at " << now->number << endl;
    }
    now = now->next;
  }
}

void alloc_reflesh(Line* head, vector<int> &line_alloc, int s) {
  Line* now = head;
  fill(line_alloc.begin(), line_alloc.end(), -1);
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

void endprocess(Line* head) {
  Line* now = head;
  while(now != NULL) {
    cout << now->content << endl;
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
  cursor->width_num = 0;
  cursor->w_offset = 0;
  cursor->line = head;
  cursor->head = head;
  return cursor;
}

void cursor_set(Cursor* cursor, vector<int> line_alloc) {
  for(int i = 0; i < line_alloc.size(); i++) {
    if(line_alloc[i] == cursor->line_num) {
      cursor->width_num = cursor->w_offset / (cursor->xmax - 3);
      cursor->cmove(i + cursor->w_offset / (cursor->xmax - 3), 3 + cursor->w_offset % (cursor->xmax - 3));
      return;
    } 
  }
  throw;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    cout << "error: select file" << endl;
    return 1;
  }
  string filepath = argv[1];
  ifstream ifs(filepath);
  if (ifs.fail()) {
    cout << ifs.fail() << endl;
    cout << "error: no such file :" << filepath << endl;
    return 1;
  }
  int x,y;
  int xmax, ymax;
  WINDOW* win = initscr();
  noecho();
  nl();
  curs_set(1);
  keypad(win, true);
  start_color();
  init_mycolors();
  getmaxyx(win, ymax, xmax);
  vector<int> line_alloc(ymax-2);
  WINDOW* subwindow = subwin(win, 3, xmax, ymax-3, 0);
  waddstr(subwindow, "This is subwindow");
  Line* head = init_lines(filepath);
  width_calc(head, win);
  alloc_reflesh(head, line_alloc, 0);
  scr_reflesh(head, win, line_alloc);
  wmove(subwindow, 2, 0);
  Cursor* cursor = cursor_init(ymax-2, xmax, head);
  int last_pushed = 0;
  while(true) {
    getyx(win, y, x);
    alloc_reflesh(cursor->head, line_alloc, 0);
    scr_reflesh(cursor->head, win, line_alloc);
    move(y, x);

    // reflesh subwindow
    wmove(subwindow, 1, 0);
    wclrtoeol(subwindow);
    mvwprintw(subwindow, 1, 0, "y: %d, x: %d, ymax:%d, xmax: %d, pushed: %d", cursor->y, cursor->x, ymax, xmax, last_pushed);
    mvwprintw(subwindow, 2, 0, "Cursor, line_num: %d, width_num: %d, w_offset: %d", cursor->line_num, cursor->width_num, cursor->w_offset);
    wrefresh(subwindow);

    int c = getch();
    if(c == 127 || c == 263) {
      backspace(cursor, line_alloc);
    } else if(c == 10) {
      insert_line(cursor, line_alloc);
    } else if (c == 6 || c == KEY_RIGHT) {
      moveforward(cursor);
    } else if(c == 2 || c == KEY_LEFT) {
      movebackward(cursor);
    } else if(c == 14 || c == KEY_DOWN) {
      movedown(cursor, line_alloc);
    } else if(c == 16 || c == KEY_UP) {
      moveup(cursor, line_alloc);
    } else if(c == 4) {
      endwin();
      endprocess(head);
      return 0;
    } else if(c == 23) {
      save(head, filepath);
    } else {
      insert_ch(cursor, line_alloc, c);
    }
    last_pushed = c;
  }
  endwin();
}

void scroll_down(Cursor* cursor, vector<int> &line_alloc, int n) {
  while(n > 0) {
    if(cursor->head->next == NULL) {
      break;
    } else {
      n -= cursor->head->width;
      cursor->head = cursor->head->next;
    }
  }
  alloc_reflesh(cursor->head, line_alloc, 0);
}

void scroll_up(Cursor* cursor, vector<int> &line_alloc, int n) {
  while(n > 0) {
    if(cursor->head->prev == NULL) {
      break;
    } else {
      n -= cursor->head->width;
      cursor->head = cursor->head->prev;
    }
  }
  alloc_reflesh(cursor->head, line_alloc, 0);
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

void moveup(Cursor* cursor, vector<int> &line_alloc) {
  if(cursor->line->prev == NULL) {
  } else {
    if(cursor->y == 0) {
      scroll_up(cursor, line_alloc, 1);
    }
    cursor->line_num--;
    cursor->line = cursor->line->prev;
    cursor->w_offset = min(cursor->w_offset, (int)cursor->line->content.size());
    cursor_set(cursor, line_alloc);
  }
}

void movedown(Cursor* cursor, vector<int> &line_alloc) {
  if(cursor->line->next == NULL) {
  } else {
    if(line_alloc[cursor->ymax-1] == cursor->line->number) {
      scroll_down(cursor, line_alloc, cursor->line->next->width);
    }
    cursor->line_num++;
    cursor->line = cursor->line->next;
    cursor->w_offset = min(cursor->w_offset, (int)cursor->line->content.size());
    cursor_set(cursor, line_alloc);
  }
}

void insert_line(Cursor* cursor, vector<int> &line_alloc) {
  Line* now_line = cursor->line;
  Line* new_line = new Line;
  new_line->content = now_line->content.substr(cursor->w_offset);
  now_line->content.erase(cursor->w_offset);
  now_line->calc_width(cursor);
  new_line->calc_width(cursor);
  new_line->next = now_line->next;
  new_line->prev = now_line;
  if(now_line->next != NULL) {
    now_line->next->prev = new_line;
  }
  now_line->next = new_line;
  renumber(now_line);
  alloc_reflesh(now_line, line_alloc, cursor->y - cursor->width_num);
  if(cursor->y == cursor->ymax - 3) {
    scroll_down(cursor, line_alloc, 1);
  }
  cursor->line_num++;
  cursor->w_offset = 0;
  cursor->line = new_line;
  cursor_set(cursor, line_alloc);
}

void delete_line(Cursor* cursor, vector<int> &line_alloc) {
  if(cursor->line->prev == NULL) {
  } else {
    Line* now_line = cursor->line;
    Line* prev_line = cursor->line->prev;
    cursor->w_offset = prev_line->content.size();
    if(now_line->next != NULL) {
      now_line->next->prev = prev_line;
    }
    prev_line->next = now_line->next;
    prev_line->content = prev_line->content + now_line->content;
    renumber(prev_line);
    cursor->line_num--;
    cursor->line = prev_line;
    cursor->line->calc_width(cursor);
    cursor_set(cursor, line_alloc);
    delete now_line;
  }
}


void insert_ch(Cursor* cursor, vector<int> &line_alloc, int c) {
  string s(1, (char)c); 
  cursor->line->content.insert(cursor->w_offset, s);
  cursor->w_offset++;
  cursor->line->calc_width(cursor);
  cursor_set(cursor, line_alloc);
}

void delete_ch(Cursor* cursor, vector<int> &line_alloc) {
  cursor->line->content.erase(cursor->w_offset, 1);
  cursor->line->calc_width(cursor);
  cursor_set(cursor, line_alloc);
}

void backspace(Cursor* cursor, vector<int> &line_alloc) {
  if(cursor->w_offset == 0) {
    delete_line(cursor, line_alloc);
    scroll_up(cursor, line_alloc, 1);
  } else {
    cursor->line->content.erase(cursor->w_offset - 1, 1);
    cursor->w_offset--;
    cursor->line->calc_width(cursor);
    cursor_set(cursor, line_alloc);
  }
}

void save(Line* head, string path) {
  ofstream ofs(path);
  if (ofs.fail()) {
    cerr << "error: cannot open savefile";
    throw;
  }
  Line* now = head;
  while (now != NULL) {
    ofs << now->content << endl;
    now = now->next;
  }
}