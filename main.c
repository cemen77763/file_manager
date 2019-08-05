#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <curses.h>
#include <string.h>
#include <dirent.h>

void sig_winch(int signo);
void menu(WINDOW *leftWindow, WINDOW *rightWindow);
void file_manager(WINDOW *leftWindow, WINDOW *rightWindow, char *leftPath, char *rightPath);
void info(WINDOW *leftWindow, WINDOW *rightWindow, char *leftPath, char *rightPath);

int main(){
	WINDOW *leftWindow;
	WINDOW *rightWindow;
	char leftPath[255], rightPath[255];

	initscr();
	signal(SIGWINCH, sig_winch);
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(TRUE);
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_GREEN, COLOR_BLUE);

	leftWindow = newwin(20, 40, 0, 0);
	rightWindow = newwin(20, 80, 0, 40);

	leftPath[0] = '.'; leftPath[1] = '\0';
	rightPath[0] = '.'; rightPath[1] = '\0';

	menu(leftWindow, rightWindow);
	file_manager(leftWindow, rightWindow, leftPath, rightPath);

	//delwin(leftWindow); delwin(rightWindow);
	endwin();
	exit(EXIT_SUCCESS);
}

void sig_winch(int signo){
	struct winsize size;
	ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
	resizeterm(size.ws_row, size.ws_col);
}

void menu(WINDOW *leftWindow, WINDOW *rightWindow){
	WINDOW *lname, *rname;

	move(20,0);      
    hline(ACS_CKBOARD,100); 
    move(21, 0);
    printw("F1 to quit");
	refresh();

	wbkgd(leftWindow, COLOR_PAIR(1));
	wbkgd(rightWindow, COLOR_PAIR(1));
	box(leftWindow, '|', '*');
	box(rightWindow, '|', '*');

	lname = derwin(leftWindow, 20, 25, 0, 0);
	wbkgd(lname, COLOR_PAIR(1));
	box(lname, '|', '*');
	wrefresh(lname);

	rname = derwin(rightWindow, 20, 25, 0, 0);
	wbkgd(rname, COLOR_PAIR(1));
	box(rname, '|', '*');
	wrefresh(rname);

	wmove(leftWindow, 0, 11);
	wprintw(leftWindow, "NAME");
	wmove(leftWindow, 0, 31);
	wprintw(leftWindow, "SIZE");
	wrefresh(leftWindow);

	wmove(rightWindow, 0, 11);
	wprintw(rightWindow, "NAME");
	wmove(rightWindow, 0, 31);
	wprintw(rightWindow, "SIZE");
	wrefresh(rightWindow);
}

void info(WINDOW *leftWindow, WINDOW *rightWindow, char *leftPath, char *rightPath){
	struct stat sb;
	struct dirent **namelist = NULL;
	int n = -1, i = 1;

	n = scandir(rightPath, &namelist, NULL, alphasort);
	while(i < n){
		wmove(leftWindow, i, 1);
		wprintw(leftWindow, namelist[i]->d_name);
		free(namelist[i - 1]);
		i++;
	}
	wrefresh(leftWindow);
	free(namelist);

	i = 1;
	n = scandir(leftPath, &namelist, NULL, alphasort);
	while(i < n){
		wmove(rightWindow, i, 1);
		wprintw(rightWindow, namelist[i]->d_name);
		free(namelist[i - 1]);
		i++;
	}
	wrefresh(rightWindow);
	free(namelist);
}

void file_manager(WINDOW *leftWindow, WINDOW *rightWindow, char *leftPath, char *rightPath){
	int working = 1;
	int ch, num = 1;

	info(leftWindow, rightWindow, leftPath, rightPath);

	while(working){
		ch = getch();
		switch(ch){
			case KEY_F(1):{
				working = 0;
				break;
			}
			case 10:{
				if (num == 1){
					leftPath[0] = '.'; leftPath[1] = '.'; leftPath[2] = '\0';
					chdir(leftPath);
				}
				else{
					rightPath[0] = '.'; rightPath[1] = '.'; rightPath[2] = '\0';
					chdir(rightPath);
				}
				clear();
				menu(leftWindow, rightWindow);
				info(leftWindow, rightWindow, leftPath, rightPath);
			}
			case KEY_STAB:{
				if (num == 1)
					num = 2;
				else num = 1;
			}
		}
	}
}