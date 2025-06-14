#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>

int split(const char *input, char **key, char **value, const char equal) {
  const char *eq = strchr(input, equal);
  if (!eq) return 0;

  size_t key_len = eq - input;
  size_t value_len = strlen(eq + 1);

  *key = malloc(key_len + 1);
  *value = malloc(value_len + 1);

  strncpy(*key, input, key_len);
  (*key)[key_len] = '\0';
  strcpy(*value, eq + 1);

  return 1;
}

char *read_input() {
  char *input = readline("caldesh> ");
  if (*input)
    add_history(input);
  return input;
}

char **parse_command(char *buf) {
  char **out = NULL;
  char *token;
  char *input_copy = strdup(buf);
  char *rest = input_copy;
  int count = 0;

  while ((token = strtok_r(rest, " ", &rest))) {
    out = realloc(out, sizeof(char *) * (count + 1));
    out[count++] = strdup(token);
  }

  out = realloc(out, sizeof(char *) * (count));
  out[count] = NULL;

  free(input_copy);
  return out;
}

int execute_command_child(char **cmd) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  } else if (pid == 0) { // Child process
    execvp(cmd[0], cmd);
    perror("execvp failed");
    exit(EXIT_FAILURE);
  } else { // Parent process
    int status;
    waitpid(pid, &status, 0);
  }
  return 0;
}

int execute_command(char **cmd) {
  int ret_code = 0;
  if (!strncmp(cmd[0], "cd", 2)) { // cd
    if (cmd[2]) {
      fprintf(stderr, "cd: too many arguments\n");
      return 1;
    }

    if (cmd[1]) {
      if (chdir(cmd[1]) != 0)
        perror("cd");
    } else {
      if (chdir(getenv("HOME")) != 0)
        perror("cd");
    }
  } else if (!strncmp(cmd[0], "exit", 4)) { // exit
    if (cmd[2]) {
      fprintf(stderr, "exit: too many arguments\n");
      return 1;
    }

    int code = EXIT_SUCCESS;
    if (cmd[1]) {
      char *endptr;
      code = strtol(cmd[1], &endptr, 10);
      if (*endptr != '\0') {
        fprintf(stderr, "exit: numeric argument required\n");
        code = EXIT_FAILURE;
      } else
        exit(code);
    }
  } else if (!strncmp(cmd[0], "pwd", 3)) { // pwd
    if (cmd[1]) {
      fprintf(stderr, "pwd: too many arguments\n");
      return 1;
    }

    char *cwd;
    char buffer[PATH_MAX + 1];

    cwd = getcwd(buffer, sizeof(buffer));
    if (cwd != NULL)
      printf("%s\n", cwd);
    else
      perror("pwd: getcwd failed");
  } else if (!strncmp(cmd[0], "export", 6)) { // export
    if (cmd[2]) {
      fprintf(stderr, "export: too many arguments\n");
      return 1;
    }

    char *name, *value;
    split(cmd[1], &name, &value, '=');

    if (setenv(name, value, 1))
      perror("export");
    printf("%s\n", getenv(name));
  } else if (!strncmp(cmd[0], "unset", 5)) { // unset
    if (cmd[2]) {
      fprintf(stderr, "unset: too many arguments\n");
      return 1;
    }

    if (unsetenv(cmd[1]))
      perror("unset");
  } else if (!strncmp(cmd[0], "history", 7)) { // history
    if (cmd[1]) {
      fprintf(stderr, "history: too many arguments\n");
      return 1;
    }

    HIST_ENTRY **list = history_list();
    if (list)
      for (int i = 0; list[i]; i++)
        printf("%d: %s\n", i + history_base, list[i]->line);
  } else // no built-in
    ret_code = execute_command_child(cmd);
  return ret_code;
}

int main(int argc, char *argv[]) {
  while (1) {
    char *buf = read_input();
    char **cmd = parse_command(buf);
    execute_command(cmd);

    // Cleanup
    int i = 0;
    free(cmd);
    free(buf);
  }
  return 0;
}
