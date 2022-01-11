#include <curses.h>
#include "client.h"
#include "dentry.h"

class Panel {
	static const int field_num = 6;
	static const int field_width[field_num + 1];
public:
	// curses lib
	WINDOW * panel_win;
	unsigned height, width;
	char * buffer;
	unsigned cursor_row;
	unsigned visual_start_row;

	// data
	dircomb_t dirdata;
	dirvec_t active_vec;

	Panel();
	~Panel();

	int add_mdirent(mdirent_t * md, unsigned into = 0);
	void update_data();
	void update_draw();
	void input_loop();
	void cursor_down();
	void cursor_up();

	// interaction
	void pn_download();
	void pn_upload();
	void pn_cd();
};

const int Panel::field_width[field_num + 1] = {
	0, 
	12, // row number
	12 + 40, // file name (if select, then invert the highlight)
	12 + 40 + 24, // local time
	12 + 40 + 24 + 7, // compare the two times
	12 + 40 + 24 + 7 + 24, // remote time
};

// exported functions
void start_ui_loop() {
	Panel panel;

	cl_start();
	eprintf("repoabs : %s\n", cl_params_obj.repoabs);
	eprintf("prefix : %s\n", cl_params_obj.REPO_PREFIX);
	eprintf("absrel : %s\n", cl_params_obj.absrel);

	panel.update_data();
	panel.update_draw();
	panel.input_loop();
}

Panel::Panel() {
	// ncurses initialize
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	refresh();
	getmaxyx(stdscr, height, width);
	if (width < field_width[field_num])
		errExit("Please make window wider!");
	buffer = new char[width];

	this->panel_win = newwin(this->height, this->width, 0, 0);
	wborder(this->panel_win, 0, 0, 0, 0, 0, 0, 0, 0);
	wrefresh(this->panel_win);
	keypad(stdscr, true);

	eprintf("creating panel width %d height %d\n", width, height);

	// data member initialize 
	visual_start_row = 0;
	cursor_row = 0;
}

Panel::~Panel() {
	delwin(panel_win);
	endwin();
}

int Panel::add_mdirent(mdirent_t * md, unsigned into) {
	active_vec.arr.emplace_back(md);
	return 1;
}

void Panel::update_data() {
	dirdata = _cl_ls();
	active_vec.arr.clear();
	active_vec.arr.insert(active_vec.arr.end(), dirdata.loc.arr.begin(), dirdata.loc.arr.end());
	active_vec.arr.insert(active_vec.arr.end(), dirdata.rem.arr.begin(), dirdata.rem.arr.end());
	active_vec.arr.insert(active_vec.arr.end(), dirdata.sync.arr.begin(), dirdata.sync.arr.end());
}

void Panel::update_draw() {
	int line = 0; // the graphical row 
	int row = visual_start_row; // the data row
	// int fn = field_width.size() - 1; // the number of fields

	wclear(panel_win);

	eprintf("active_vec length %lu\n", active_vec.arr.size());

	while (line < height && row < active_vec.arr.size()) {
		if (row == cursor_row) {
			wattron(panel_win, A_STANDOUT);
		}

		// row number
		snprintf(buffer, field_width[1] - field_width[0], "%i", row);
		wmove(panel_win, line, field_width[0]);
		wprintw(panel_win, buffer);

		snprintf(buffer, field_width[2] - field_width[1], "%s", active_vec.arr[row]->m_name);
		wmove(panel_win, line, field_width[1]);
		wprintw(panel_win, buffer);

		prtime(buffer, &active_vec.arr[row]->m_mtime_loc, field_width[3] - field_width[2]);
		wmove(panel_win, line, field_width[2]);
		wprintw(panel_win, buffer);

		if (active_vec.arr[row]->m_mtime_loc && active_vec.arr[row]->m_mtime_rem) {
			if (active_vec.arr[row]->m_mtime_loc < active_vec.arr[row]->m_mtime_rem) 
				strcpy(buffer, "REM");
			if (active_vec.arr[row]->m_mtime_loc > active_vec.arr[row]->m_mtime_rem)
				strcpy(buffer, "LOC");
			if (active_vec.arr[row]->m_mtime_loc == active_vec.arr[row]->m_mtime_rem)
				strcpy(buffer, "===");
			wmove(panel_win, line, field_width[3] + 2);
			wprintw(panel_win, buffer);
		}

		prtime(buffer, &active_vec.arr[row]->m_mtime_rem, field_width[5] - field_width[4]);
		wmove(panel_win, line, field_width[4]);
		wprintw(panel_win, buffer);

		if (row == cursor_row) {
			wattroff(panel_win, A_STANDOUT);
		}

		line += 1;
		row += 1;
	}

	wrefresh(panel_win);
}

void Panel::cursor_up() {
	if (cursor_row <= 0)
		return;
	cursor_row--;

	visual_start_row = std::min(visual_start_row, cursor_row);
	eprintf("visual_start_row is %u\n", visual_start_row);
	update_draw();
}

void Panel::cursor_down() {
	if (cursor_row >= active_vec.arr.size() - 1)
		return;
	cursor_row++;

	visual_start_row = std::max(visual_start_row, cursor_row - std::min(cursor_row, height - 1));
	eprintf("visual_start_row is %u\n", visual_start_row);
	update_draw();
}

void Panel::pn_download() {
	
}

void Panel::pn_upload() {

}

void Panel::pn_cd() {

}

void Panel::input_loop() {
	wchar_t ch;
	while (1) {
		ch = getch();
		switch (ch) 
		{
		case KEY_DOWN: // go down an entry
			cursor_down();
			break;
		case KEY_UP: // go up an entry
			cursor_up();
			break;
		case '\n': // go into a directory
			pn_cd();
			break;
		case 'd':
			pn_download();
			break;
		case 'u':
			pn_upload();
		case 'q':
			return;
		default:
			break;
		}
	}
}
