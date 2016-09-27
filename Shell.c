#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/*
  Function Delcarations for main loop of shell.
*/
char *lsh_read_line(void);
char **lsh_split_line(char *line);
int lsh_execute(char **args);

/*
  Function Declarations for builtin shell commands:
*/
int lsh_cd(char **args);
int lsh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "e"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
 Main function initialized
*/

int main(int argc, char **argv){
  // Load config files, if any.
  
  // Run command loop.
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}

void lsh_loop(void){
  char *line;
  char **args;
  int status;

  do {
    printf("%% ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

char *lsh_read_line(void){
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();
    // If we hit EOF, replace it with a null character and return.
    if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } 
    else if (c == EOF){
      buffer[position] = 'e';
      position++;
      buffer[position] = '\0';
      return buffer;
    }
    else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **lsh_split_line(char *line){
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int lsh_execute(char **args){
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }
  
  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

/*
  Builtin function implementations.
*/
int lsh_cd(char **args){
  if (args[1] == NULL) {
    chdir(getenv("HOME"));
  } 
  else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_exit(char **args){
   printf("^D\n");
   return 0;
}

int lsh_launch(char **args){
  pid_t pid, wpid;
  int status;
  int i = 0;
  FILE *stream;

  pid = fork();
  if (pid == 0) {
    while(args[i] != NULL){
	if(strcmp(args[i], ">") == 0){
	    args[i] = '\0';
	    stream  = fopen(args[++i], "w");
	    dup2(fileno(stream), STDOUT_FILENO);
	    fclose(stream);
	    args[i] = '\0';
	    break;
	}
	i++;
    }
    // Child process is executed using exevp().
    // If a problem occurs, the program throws an error.
    if (execvp(args[0], args) == -1) {
      fprintf(stderr, "Command not found.\n");
    }
   
    exit(EXIT_FAILURE);
  }
  else if (pid < 0) {
    // Error forking
    fprintf(stderr, "Could not fork new process.\n");
  }
  else {
    // Parent process waits in while loop while the child is processed
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}
