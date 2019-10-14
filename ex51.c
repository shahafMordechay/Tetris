#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>

#define ERROR "Error in system call\n"

char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    return (buf);
}

int main() {
    int fd[2];

    if (pipe(fd) < 0) {
        write(2, ERROR, sizeof(ERROR));
        _exit(1);
    }

    pid_t pid;

    if ((pid = fork()) < 0) {
        close(fd[0]);
        close(fd[1]);
        write(2, ERROR, sizeof(ERROR));
        _exit(1);
    }

    if (pid == 0) { // child
        char *args[] = {"./Draw.out", NULL};
        dup2(fd[0], 0);
        execvp("./Draw.out", args);

        // if failed
        close(fd[0]);
        close(fd[1]);
        write(2, ERROR, sizeof(ERROR));
        _exit(1);

    } else {    // father
        char c = '\0';
        while (c != 'q') {
            c = getch();
            write(fd[1], &c, 1);
            kill(pid, SIGUSR2);
        }

        close(fd[0]);
        close(fd[1]);
    }

    return 0;
}
