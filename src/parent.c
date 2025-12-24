#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_FILE_NAME_LEN 256
#define BUFFER_SIZE  256

void sys_write_string_parent(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    write(STDOUT_FILENO, str, len);
    write(STDOUT_FILENO, "\n", 1);
}

int main() {
    int pipefd[2];
    pid_t pid;
    char filename[MAX_FILE_NAME_LEN];
    int file_fd = -1;

    sys_write_string_parent("Enter the filename to be processed:");

    ssize_t bytes_read = read(STDIN_FILENO, filename, MAX_FILE_NAME_LEN - 1);
    
    if (bytes_read == -1) {
        sys_write_string_parent("Error reading filename from STDIN.");
        exit(EXIT_FAILURE);
    }
    
    if (bytes_read > 0 && filename[bytes_read - 1] == '\n') {
        bytes_read--;
    }
    filename[bytes_read] = '\0';
    
    if (bytes_read == 0) {
        sys_write_string_parent("Filename cannot be empty.");
        exit(EXIT_FAILURE);
    }
    
    if (pipe(pipefd) == -1) {
        sys_write_string_parent("Error creating pipe.");
        exit(EXIT_FAILURE);
    }
    
    file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        sys_write_string_parent("Error opening file.");
        sys_write_string_parent(filename);
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid == -1) {
        sys_write_string_parent("Error creating child process (fork failed).");
        close(file_fd);
        close(pipefd[0]);
        close(pipefd[1]);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        
        close(pipefd[0]);

        if (dup2(file_fd, STDIN_FILENO) == -1) {
            sys_write_string_parent("Error redirecting STDIN to file.");
            close(file_fd);
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }

        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            sys_write_string_parent("Error redirecting STDOUT to pipe.");
            close(file_fd);
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }

        close(file_fd);
        close(pipefd[1]);

        execl("./child", "child", (char *)NULL);

        sys_write_string_parent("Error executing child program (execl failed).");
        exit(EXIT_FAILURE);

    } else {
        
        close(pipefd[1]);
        close(file_fd);

        char buffer[BUFFER_SIZE];
        while ((bytes_read = read(pipefd[0], buffer, BUFFER_SIZE)) > 0) {
            if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
                sys_write_string_parent("Error writing to STDOUT.");
                break;
            }
        }

        if (bytes_read == -1) {
            sys_write_string_parent("Error reading from pipe.");
        }

        close(pipefd[0]);

        if (wait(NULL) == -1 && errno != ECHILD) {
            sys_write_string_parent("Error waiting for child process.");
            exit(EXIT_FAILURE);
        }

        sys_write_string_parent("All done.");
    }

    return 0;
}
