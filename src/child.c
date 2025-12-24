#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 256

void sys_write_string(const char *str);
void sys_write_int(int n);

void sys_write_string(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    write(STDOUT_FILENO, str, len);
}

void sys_write_int(int n) {
    char buffer[32];
    int i = 30;
    int is_negative = 0;

    if (n == 0) {
        buffer[i--] = '0';
    } else {
        if (n < 0) {
            is_negative = 1;
            n = -n;
        }

        while (n != 0) {
            buffer[i--] = (n % 10) + '0';
            n /= 10;
        }

        if (is_negative) {
            buffer[i--] = '-';
        }
    }

    write(STDOUT_FILENO, &buffer[i + 1], 30 - i);
}

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    long total_sum = 0;
    char *start_ptr = buffer;
    char *end_ptr;
    int current_number;

    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1)) > 0) {
        
        buffer[bytes_read] = '\0';
        start_ptr = buffer;

        while (*start_ptr != '\0') {
            
            while (*start_ptr != '\0' && (*start_ptr == ' ' || *start_ptr == '\n' || *start_ptr == '\t')) {
                start_ptr++;
            }

            if (*start_ptr == '\0') {
                break;
            }

            current_number = strtol(start_ptr, &end_ptr, 10);

            if (end_ptr != start_ptr) {
                total_sum += current_number;
                start_ptr = end_ptr;
            } else {
                start_ptr++;
            }
        }
    }

    if (bytes_read == -1) {
        sys_write_string("Child: Error reading from STDIN_FILENO.\n");
        exit(EXIT_FAILURE);
    }

    sys_write_string("The sum of numbers is: ");
    sys_write_int(total_sum);
    write(STDOUT_FILENO, "\n", 1);

    exit(EXIT_SUCCESS);
}
