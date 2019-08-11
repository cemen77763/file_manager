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
#include <pthread.h>

short selectedR, selectedL;

struct pthrData{
	char filename1[1000];
	char filename2[1000];
	struct stat sb;
	WINDOW *win;
};

void sig_winch(int signo);

void menu(WINDOW *leftWindow, WINDOW *rightWindow);

void fillpath(char *buff, char *path);

void nonfolder(char *path);

void takefilename(char *buff, char *filename, char *path);

void* funcforcoping(void *thread_data);

void* inputcoping(void *thread_data);

void file_manager(WINDOW *leftWindow, WINDOW *rightWindow, char *leftPath, char *rightPath);

void info(WINDOW *leftWindow, WINDOW *rightWindow, char *leftPath, char *rightPath);

int main(){
	WINDOW *leftWindow;
	WINDOW *rightWindow;
	char leftPath[1000], rightPath[1000];

	initscr();
	signal(SIGWINCH, sig_winch);
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(TRUE);
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_GREEN, COLOR_YELLOW);
	init_pair(3, COLOR_WHITE, COLOR_BLACK);

	leftWindow = newwin(20, 40, 0, 0);
	rightWindow = newwin(20, 80, 0, 40);

	leftPath[0] = '.'; leftPath[1] = '/'; leftPath[2] = '\0';
	rightPath[0] = '.'; rightPath[1] = '/'; rightPath[2] = '\0';

	menu(leftWindow, rightWindow);
	file_manager(leftWindow, rightWindow, leftPath, rightPath);

	delwin(leftWindow); delwin(rightWindow);
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
    move(22, 0);
    printw("F2 to change window");    
    move(23, 0);
    printw("F3 to copy file");
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

void fillpath(char *buff, char *path){
	int i = 1, j = 0;

	while(path[j] != '\0')
		j++;
	if (path[j - 1] != '/'){
		path[j] = '/';
		j++;
	}
	
	while (buff[i] != ' '){
		path[j] = buff[i];
		j++;
		i++;
	}
	path[j] = '\0';
	return;
}

void nonfolder(char *path){
	int i = 0;
	while(path[i] != '\0')
		i++;
	while(path[i] != '/'){
		i--;
	}
	path[i] = '\0';
}

void takefilename(char *buff, char *filename, char *path){
	int i = 0, j = 1;
	while(path[i] != '\0'){
		filename[i] = path[i];
		i++;
	}
	if (i > 2){
		filename[i] = '/'; 
		i++;
	}
	while(buff[j] != ' '){
		filename[j + i - 1] = buff[j];
		j++;
	}
	filename[j + i - 1] = '\0';
}

void info(WINDOW *leftWindow, WINDOW *rightWindow, char *leftPath, char *rightPath){
	wclear(leftWindow); wclear(rightWindow);
	menu(leftWindow, rightWindow);

	char buff[100], filename[1000];
	struct stat sb;
	struct dirent **namelist = NULL;
	int n = -1, i = 1;

	n = scandir(leftPath, &namelist, NULL, alphasort);
	if (n == -1){
		nonfolder(leftPath);
		n = scandir(leftPath, &namelist, NULL, alphasort);
	}
	if ((selectedL + 1) >= n) selectedL = 0;
	while(i < n){
		if (i == (selectedL + 1)) wattron(leftWindow, COLOR_PAIR(2));
		wmove(leftWindow, i, 1);
		wprintw(leftWindow, namelist[i]->d_name);

		wmove(leftWindow, i, 0);
		winstr(leftWindow, buff);
		takefilename(buff, filename, leftPath);
		lstat(filename, &sb);
		wmove(leftWindow, i, 26);
		wprintw(leftWindow, "%lldkb", sb.st_size/1024);

		free(namelist[i - 1]);
		if (i == (selectedL + 1)) wattron(leftWindow, COLOR_PAIR(1));
		i++;
	}
	wrefresh(leftWindow);
	free(namelist);

	i = 1;	
	n = scandir(rightPath, &namelist, NULL, alphasort);
	if (n == -1){
		nonfolder(rightPath);
		n = scandir(rightPath, &namelist, NULL, alphasort);
	}
	if ((selectedR + 1) >= n) selectedR = 0;
	while(i < n){
		if (i == (selectedR + 1)) wattron(rightWindow, COLOR_PAIR(2));
		wmove(rightWindow, i, 1);
		wprintw(rightWindow, namelist[i]->d_name);

		wmove(leftWindow, i, 0);
		winstr(rightWindow, buff);
		takefilename(buff, filename, rightPath);
		lstat(filename, &sb);
		wmove(rightWindow, i, 26);
		wprintw(rightWindow, "%lldkb", sb.st_size/1024);

		free(namelist[i - 1]);
		if (i == (selectedR + 1)) wattron(rightWindow, COLOR_PAIR(1));
		i++;
	}
	wrefresh(rightWindow);
	free(namelist);
}

void* funcforcoping(void *thread_data){
	struct pthrData *data = (struct pthrData*)thread_data;

	FILE *fp1, *fp2;
	char buff[1024];
	int i = 0;

	fp1 = fopen(data->filename1, "r");
	if (fp1 == NULL) pthread_exit(EXIT_SUCCESS);

	fp2 = fopen(data->filename2, "w");
	if (fp2 == NULL) pthread_exit(EXIT_SUCCESS);

	while (1){
		if (fread(buff, 1, 1024, fp1) == 0) break;
		fwrite(buff, 1, 1024, fp2);
	}

	fclose(fp1);
	fclose(fp2);
	pthread_exit(EXIT_SUCCESS);
}

void* inputcoping(void *thread_data){
	struct pthrData *data = (struct pthrData*)thread_data;
	data->sb.st_size = 0;
	long long size;
	long long proc;
	float perc = 0;

	if (lstat(data->filename1, &(data->sb)) == -1)
		pthread_exit(EXIT_SUCCESS);
	size = data->sb.st_size;

	while (perc < 100) {
		if (lstat(data->filename2, &(data->sb)) == -1){
			data->sb.st_size = 0;
			continue;
		}
		perc = ((float)data->sb.st_size/(float)size)*100;

		wattron(data->win, COLOR_PAIR(3));
		wmove(data->win, 16, 1);
		wprintw(data->win, "Coping %f percent", perc); 
		wmove(data->win, 17, 1);
		wprintw(data->win, "Current file %lld to %lld", size/(1024*1024), data->sb.st_size/(1024*1024));
		wrefresh(data->win);
		refresh();
	} 

	wattron(data->win, COLOR_PAIR(1));
	wmove(data->win, 18, 1);
	wprintw(data->win, "I'm done");
	pthread_exit(EXIT_SUCCESS);
}

void file_manager(WINDOW *leftWindow, WINDOW *rightWindow, char *leftPath, char *rightPath){
	int working = 1;
	int ch, num = 1;
	char buff[30], filename1[1000], filename2[1000];
	pthread_t tid[2];

	struct pthrData thread_data;

	selectedL = 0; selectedR = 0;

	info(leftWindow, rightWindow, leftPath, rightPath);

	while(working){
		ch = getch();
		switch(ch){
			case KEY_F(1):{
				working = 0;
				break;
			}
			case KEY_UP:{
				if ((num == 1) && (selectedL > 0)) selectedL--;
				else if (selectedR > 0) selectedR--;
				info(leftWindow, rightWindow, leftPath, rightPath);
				break;
			}
			case KEY_DOWN:{
				if (num == 1) selectedL++;
				else selectedR++;
				info(leftWindow, rightWindow, leftPath, rightPath);
				break;
			}
			case 10:{
				if (num == 1){
					wmove(leftWindow, selectedL + 1, 0);
					winstr(leftWindow, buff);
					fillpath(buff, leftPath);
				}
				else{
					wmove(rightWindow, selectedR + 1, 0);
					winstr(rightWindow, buff);
					fillpath(buff, rightPath);
				}
				clear();
				info(leftWindow, rightWindow, leftPath, rightPath);
				break;
			}
			case KEY_F(2):{
				if (num == 1)
					num = 2;
				else num = 1;
				break;
			}
			case KEY_F(3):{
				if (num == 1){
					wmove(leftWindow, selectedL + 1, 0);
					winstr(leftWindow, buff);
					takefilename(buff, thread_data.filename1, leftPath);
					takefilename(buff, thread_data.filename2, rightPath);
					thread_data.win = rightWindow;

					pthread_create(&(tid[0]), NULL, funcforcoping, &(thread_data));
					pthread_create(&(tid[1]), NULL, inputcoping, &(thread_data));
				} else{
					wmove(rightWindow, selectedR + 1, 0);
					winstr(rightWindow, buff);
					takefilename(buff, thread_data.filename1, rightPath);
					takefilename(buff, thread_data.filename2, leftPath);
					thread_data.win = leftWindow;

					pthread_create(&(tid[0]), NULL, funcforcoping, &(thread_data));
					pthread_create(&(tid[1]), NULL, inputcoping, &(thread_data));
				}
				pthread_join(tid[0], NULL);
				pthread_join(tid[1], NULL);
				//info(leftWindow, rightWindow, leftPath, rightPath);
				break;
			}
			default: break;
		}
	}
}