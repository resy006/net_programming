#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#define _GNU_SOURCE

#define DELIMS " \r\t\n"
#define MAX 1024

int tokenization(char *str, char *token_s[]) {
    int token_count = 0;
    char *token = strtok(str, DELIMS);
    while (token != NULL) {
        token_s[token_count++] = token;
        token = strtok(NULL, DELIMS);
    }

    token_s[token_count] = NULL;
    return token_count;
}

int redir_in(char *tokens[], int idx) {
    int fd, i;
    if (tokens[idx]) {
        if (!tokens[idx + 1]) {
            return -1;
        } else {
            if ((fd = open(tokens[idx + 1], O_RDONLY)) == -1) { 
                perror(tokens[idx + 1]);
                return -1;
            }
        }
        dup2(fd, STDIN_FILENO);
        close(fd);

        for (i = idx; tokens[i] != NULL; i++) {
            tokens[i] = tokens[i + 2];
        }
        tokens[i] = NULL;
    }
    return 0;
}

int redir_out(char *tokens[], int idx, int flag) {
    int i, fd;
    if (tokens[idx]) {
        if (!tokens[idx + 1]) {
            return -1;
        } else {
            if (flag == 0) {
                if ((fd = open(tokens[idx + 1], O_WRONLY | O_CREAT | O_TRUNC, 0664)) == -1) {
                    perror("file open error\n");
                    return -1;
                }
            } else {
                if ((fd = open(tokens[idx + 1], O_WRONLY | O_CREAT | O_APPEND, 0664)) == -1) {
                    perror("file open error\n");
                    return -1;
                }
            }
        }
        dup2(fd, STDOUT_FILENO); 
        close(fd);

        for (i = idx; tokens[i] != NULL; i++) {
            tokens[i] = tokens[i + 2];
        }
        tokens[i] = NULL;
    }
    return 0;
}

int redirection(char *tokens[]) {
    int i;
    int flag, out_flag = 0;
    for (i = 0; tokens[i] != NULL; i++) {
        if (!strcmp(tokens[i], ">")) {
            flag = 0;
            break;
        }
        if (!strcmp(tokens[i], "<")) {
            flag = 1;
            break;
        }
        if (!strcmp(tokens[i], ">>")) {
            flag = 0;
            out_flag = 1;
            break;
        }
    }

    if (flag == 0) {
        if (redir_out(tokens, i, out_flag) == -1) {
            return -1;
        }
    } else {
        if (redir_in(tokens, i) == -1) {
            return -1;
        }
    }
    return 0;
}

void menu() {
    printf("redirection, cd, cat, ....\n");
}

bool run(char *input) {
    int redir_check = 0;
    int input_count = 0;
    char *token[300];
    pid_t pid;
    if ((tokenization(input, token)) == 0) {
        return true;
    }

    if (!strcmp(token[0], "exit")) {
        return false;
    }

    if (!strcmp(token[0], "clear")) {
        system("clear");
    }

    if ((pid = fork()) < 0) {
        perror("fork() error");
        return false;
    } else if (pid == 0) {
        if (!redirection(token)) {
            execvp(token[0], token);
            perror("execvp() error");
            return false;
        }
    }
    wait(NULL);

    return true;
}

int main() {
    char input[MAX];
    while (1) {
        printf("$ ");                                   
        fgets(input, MAX - 1, stdin);
        input[strcspn(input, "\n")] = '\0';

        if (run(input) == false) {
            break;
        }
    }
    return 0;
}
