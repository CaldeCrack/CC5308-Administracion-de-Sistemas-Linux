#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <linux/limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>

#define HISTORY_FILE ".shell-history"

char prev_dir[PATH_MAX] = "";

char *get_history_path() {
  const char *home = getenv("HOME");
  if (!home) {
    struct passwd *pw = getpwuid(getuid());
    home = pw->pw_dir;
  }

  size_t len = strlen(home) + strlen("/.shell-history") + 1;
  char *path = malloc(len);
  snprintf(path, len, "%s/.shell-history", home);
  return path;
}

int split(const char *input, char **key, char **value, const char equal) {
  const char *eq = strchr(input, equal);
  if (!eq)
    return 0;

  size_t key_len = eq - input;
  size_t value_len = strlen(eq + 1);

  *key = malloc(key_len + 1);
  *value = malloc(value_len + 1);

  strncpy(*key, input, key_len);
  (*key)[key_len] = '\0';
  strcpy(*value, eq + 1);

  return 1;
}

char *read_input(char *history_path) {
  char *input = readline("caldesh> ");
  if (*input) {
    add_history(input);
    write_history(history_path);
  }
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
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));

    if (cmd[1] && !strncmp(cmd[1], "-", 1)) {
      printf("%s\n", prev_dir);

      if (chdir(prev_dir))
        perror("cd");
      else
        strncpy(prev_dir, cwd, PATH_MAX);
    } else {
      const char *target = cmd[1] ? cmd[1] : getenv("HOME");
      if (chdir(target))
        perror("cd");
      else
        strncpy(prev_dir, cwd, PATH_MAX);
    }
  } else if (!strncmp(cmd[0], "exit", 4)) { // exit
    int code = EXIT_SUCCESS;
    if (cmd[1]) {
      char *endptr;
      code = strtol(cmd[1], &endptr, 10);
      if (*endptr != '\0') {
        fprintf(stderr, "exit: numeric argument required\n");
        code = EXIT_FAILURE;
      } else
        exit(code);
    } else
      exit(code);
  } else if (!strncmp(cmd[0], "pwd", 3)) { // pwd
    char *cwd;
    char buffer[PATH_MAX];

    cwd = getcwd(buffer, sizeof(buffer));
    if (cwd != NULL)
      printf("%s\n", cwd);
    else
      perror("pwd: getcwd failed");
  } else if (!strncmp(cmd[0], "export", 6)) { // export
    char *name, *value;
    split(cmd[1], &name, &value, '=');

    if (setenv(name, value, 1))
      perror("export");
    printf("%s\n", getenv(name));
  } else if (!strncmp(cmd[0], "unset", 5)) { // unset
    if (unsetenv(cmd[1]))
      perror("unset");
  } else if (!strncmp(cmd[0], "history", 7)) { // history
    HIST_ENTRY **list = history_list();
    if (list)
      for (int i = 0; list[i]; i++)
        printf("%d: %s\n", i + history_base, list[i]->line);
  } else // no built-in
    ret_code = execute_command_child(cmd);
  return ret_code;
}

int main() {
  // Set initial previous directory
  getcwd(prev_dir, sizeof(prev_dir));

  // Persistent history
  char *history_path = get_history_path();
  using_history();
  read_history(history_path);

  // Start shell
  while (1) {
    char *buf = read_input(history_path);
    if (!buf)
      continue;

    char **cmd = parse_command(buf);
    execute_command(cmd);

    // Cleanup
    free(cmd);
    free(buf);
  }

  free(history_path);
  return 0;
}
