#ifndef DAEMONIZE_H
#define DAEMONIZE_H

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


// based on https://github.com/jirihnidek/daemon/blob/master/src/daemon.c
void daemonize(const char* rootDir = nullptr, const char* pid_file_name = nullptr)
{
  int pid_fd = -1;
  pid_t pid = 0;
  int fd;

  /* Fork off the parent process */
  pid = fork();

  /* An error occurred */
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  /* Success: Let the parent terminate */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* On success: The child process becomes session leader */
  if (setsid() < 0) {
    exit(EXIT_FAILURE);
  }

  /* Ignore signal sent from child to parent process */
  signal(SIGCHLD, SIG_IGN);

  /* Fork off for the second time*/
  pid = fork();

  /* An error occurred */
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  /* Success: Let the parent terminate */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* Set new file permissions */
  umask(0);

  /* Change the working directory to the root directory */
  /* or another appropriated directory */
  if (rootDir == nullptr)
    chdir("/");
  else
    chdir(rootDir);

  /* Close all open file descriptors */
  for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--)
  {
    close(fd);
  }

  /* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
  stdin = fopen("/dev/null", "r");
  stdout = fopen("/dev/null", "w+");
  stderr = fopen("/dev/null", "w+");

  /* Try to write PID of daemon to lockfile */
  if (pid_file_name != nullptr)
  {
    char str[256];
    pid_fd = open(pid_file_name, O_RDWR|O_CREAT, 0640);
    if (pid_fd < 0)
    {
      /* Can't open lockfile */
      exit(EXIT_FAILURE);
    }

    if (lockf(pid_fd, F_TLOCK, 0) < 0)
    {
      /* Can't lock file */
      exit(EXIT_FAILURE);
    }

    /* Get current PID */
    sprintf(str, "%d\n", getpid());
    /* Write PID to lockfile */
    write(pid_fd, str, strlen(str));
  }
}
#endif // DAEMONIZE_H
