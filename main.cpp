/******BEGIN LICENSE BLOCK*******
 * The MIT License
 *
 * Copyright (c) 2009 Max Hodak
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ********END LICENSE BLOCK*********/

#include <iostream>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <ftw.h>

#include "config.h"

#define DEBUG

using namespace std;

int g_die = 0;

void signal_handler(int sig) {
  switch(sig) {
    case SIGINT:
    case SIGQUIT:
    case SIGHUP:
    case SIGTERM:
      g_die = 1;
      syslog(LOG_WARNING, "Received signal %s.", strsignal(sig));
      break;
    default:
      syslog(LOG_WARNING, "Unhandled signal %s", strsignal(sig));
      break;
  }
}

static int callback(const char *fpath, const struct stat *sb, int typeflag) {
  string fpathnew = "";
  int move_file = 0;
  struct stat stat_file;
  if (typeflag == FTW_F) {
    stat(fpath, &stat_file);
    if(stat_file.st_size < 10485760 /* 10 mb */) {
      if(rand() % 10 > 9){
        move_file = 1;
        fpathnew = "foo/bar";
      } else { move_file = 0; }
      if(move_file){
        syslog(LOG_ALERT, "Sorting file: %s -> %s", fpath, fpathnew.c_str());
      }      
    }
  }
  return 0;
}

int main (int argc, char * const argv[]) {

#if defined(DEBUG)
  int daemonize = 0;
#else
  int daemonize = 1;
#endif

  int logflag = 0;
  static int ch;
  string config_file = "";

  signal(SIGHUP,  signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGINT,  signal_handler);
  signal(SIGQUIT, signal_handler);

  while ((ch = getopt(argc, argv, "nlf:")) != -1) {
    switch (ch) {
      case 'n':
        daemonize = 0;
        break;
      case 'l':
        logflag = 1;
        break;
      case 'f':
        config_file = optarg;
        break;
    }
  }

  NeatConf neat_conf;
  if("" != config_file){
    // neat_conf = luaConf->parse(config_file);
  } else {
    neat_conf.monitor_paths.push_back("~/Desktop");
    neat_conf.sorted_paths.push_back("~/Documents");
    neat_conf.sorted_paths.push_back("~/Pictures");
  }

#if defined(DEBUG)
  setlogmask(LOG_UPTO(LOG_DEBUG));
  openlog("neatd", LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
#else
  setlogmask(LOG_UPTO(LOG_INFO));
  openlog("neatd", LOG_CONS, LOG_USER);
#endif

  pid_t pid, sid;

  if(1 == daemonize){
    syslog(LOG_NOTICE, "Daemon starting up");
    pid = fork();
    if (pid < 0) {
      exit(EXIT_FAILURE);
    }
    if (pid > 0) {
      exit(EXIT_SUCCESS);
    }
    umask(0);
    sid = setsid();
    if (sid < 0) {
      exit(EXIT_FAILURE);
    }
    if ((chdir("/")) < 0) {
      exit(EXIT_FAILURE);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }
  while (0 == g_die) {
    syslog(LOG_NOTICE, "Scanning ~/Desktop...");
    ftw("/Users/maxhodak/Desktop", callback, 1);
    sleep(120);
  }
  exit(EXIT_SUCCESS);
}