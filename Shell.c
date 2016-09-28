//*****************************************************************************
// purpose: CSE 4733/6733 Programming Assignment – Phase I
// created: 09-20-2016
// author: Charles Patrick James
// sources: https://brennan.io/2015/01/16/write-a-shell-in-c/
//*****************************************************************************

//*****************************************************************************
// Header Files
//*****************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//*****************************************************************************
// Function Delcarations - This program can be divided into three main parts:
//*****************************************************************************
char *lsh_read_line(void); // Program will read the entire input that the user enters and store it into an array.
char **lsh_split_line(char *line); // Program will then take the array and tokenize it into an array of arguments.
int lsh_execute(char **args); // Program will finally process and execute the arguments based on thier semantics.

//*****************************************************************************
// Define Macros - These macros are used in the functions lsh_read_line() and lsh_split_line()
//*****************************************************************************
#define LSH_RL_BUFSIZE 1024 // Initial input size of buffer for lsh_read_line()
#define LSH_TOK_BUFSIZE 64 // Initial input size of buffer for lsh_split_line()
#define LSH_TOK_DELIM " \t\r\n\a" // The characters that lsh_split_line() tokenizes the arguments on (spaces, tabs, return carriers, and newlines)

//*****************************************************************************
// Builtin Shell Commands - These subfunctions are used by the function lsh_execute() to determine the arguments' semantics.
//*****************************************************************************

/*
  Function Delcarations - There are only two built in commands for this assignment
 */
int lsh_cd(char **args); // Change Directory
int lsh_exit(char **args); // Exit Shell

/*
  An array of builtin commands, followed by an array of their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "e"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_exit
};

/*
  Number of built in commands, used by lsh_execute() later on.
 */
int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

//*****************************************************************************
// Main Function
//*****************************************************************************

int main(int argc, char **argv){
  
  /*
	//This function simply encapsulates the main three functions mentioned above:
	// lsh_read_line()
	// lsh_split_line()
	// lsh_execute()
  */
  lsh_loop();
  
  // End of main()
  return EXIT_SUCCESS;
}

//*****************************************************************************
// Start of function lsh_loop() - The heart of the program.
//*****************************************************************************
void lsh_loop(void){
  // Program declares three variables, one for each subfunction below.
  char *line; // Character buffer for lsh_read_line() to read user input.
  char **args; // 2D character buffer for lsh_split_line() to tokenize arguments.
  int status; // Integer for execution status, primarily for exiting program.
  
  /*
	The Unix Shell begins by entering a seemingly infinite while loop (although not really).
  */
  do {
    printf("%% "); // Prompt the user with '%'.
    line = lsh_read_line(); // After user input, this function will return a character buffer containing that input.
    args = lsh_split_line(line); // After tokenizing the character buffer, this function will return a 2D character buffer, with each element representing a single argument.
    status = lsh_execute(args); // After receiving the tokenized arguments, this function will reutrn a status of 0 (for exiting) or 1 (for continuing).
	
	// These two lines of code simply deallocate the memory allocated to the character buffers, 
	// allowing for more user input to be read and processed.
    free(line);
    free(args);
  } while (status); // While-loop checks the status each time a command has been executed.
}

//*****************************************************************************
// Start of function lsh_read_line() - Part I of the program. Returns character buffer.
//*****************************************************************************
char *lsh_read_line(void){
  int bufsize = LSH_RL_BUFSIZE; // Assign buffer size to integer.
  int position = 0; // Assign initial position of buffer to integer.
  char *buffer = malloc(sizeof(char) * bufsize); // Allocate memory for the buffer using buffer size.
  int c; // Keeps track of each character the user inputs (assigned as integer because EOF is actually an integer).
  
  // Simply checking for any allocation erros. Will exit program on error.
  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  // Function enters an infinite while loop, reading each character the user inputs until a newline or EOF is detected.
  while (1) {
    // Read a character from user input.
    c = getchar();
    // If user enters a newline, then that character will be replaced by a null character and return the buffer. Null 
	// characters will be used to mark the end of a user command.
    if (c == '\n') {
      buffer[position] = '\0';
      return buffer; // Return to lsh_loop()
    }
	// If we hit EOF, replace it with an abitrary character such as 'e', followed by a null character, and return.
	// Because Control-D actually sends an EOF integer to the input stream, this function must drop a flag (i.e. 'e') 
	// and let the program know that it is time to exit.
    else if (c == EOF){
      buffer[position] = 'e';
      position++;
      buffer[position] = '\0';
      return buffer; // Return to lsh_loop()
    }
	// If a newline or EOF character isn't found, then the buffer will store the character.
    else {
      buffer[position] = c;
    }
	// The function increments the position of the buffer by one.
    position++;

    // If we have exceeded the buffer size, then we must reallocate more memory to the buffer.
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

//*****************************************************************************
// Start of function lsh_split_line() - Part II of the program. Returns 2D character buffer.
//*****************************************************************************
char **lsh_split_line(char *line){
  int bufsize = LSH_TOK_BUFSIZE, position = 0; // Assign buffer size and initial position of buffer to integer variables.
  char **tokens = malloc(bufsize * sizeof(char*)); // Allocate memory for 2D character buffer.
  char *token; // Declaring 1-dimensional character buffer for each token/argument. No allocation needed, as input will be fixed each time it is called.
  
  // Simply checking for any allocation erros. Will exit program on error.
  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  
  // Assign the 1D character buffer to the first user's argument, tokenized from input line.
  token = strtok(line, LSH_TOK_DELIM);
  
  // Function enters a while loop for adding each token to the 2D character buffer. Will only break when
  // the token hits the Null character at the end of the user's input, which was assigned by the last 
  // function after each newline or EOF character was read from the input stream. 
  while (token != NULL) {
    // Assign each token to a position in the 2D character buffer
	tokens[position] = token;
    position++;
	
	// If we have exceeded the buffer size, then we must reallocate more memory to the buffer.
    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
	// Get the next token/argument
    token = strtok(NULL, LSH_TOK_DELIM);
  }
  
  tokens[position] = NULL; // Last postion in the 2D character buffer is assigned Null, indicating the program has reached the end of the user's input.
  return tokens; // Return to lsh_loop()
}

//*****************************************************************************
// Start of function lsh_execute() - Part III of the program. Returns status integer.
//*****************************************************************************
int lsh_execute(char **args){
  int i; // Declare integer for simple for-loop iteration below.

  // Check for an empty command. Will re-prompt the user with '%'.
  if (args[0] == NULL) {
    return 1; // Return to lsh_loop()
  }
  
  // The function must check for built in commands first before moving on to system commands.
  // This is done by matching the first argument of the user input against a predefined array of
  // strings (called builtin_str[], as seen at the beginning). If the input matches one of these
  // strings, the function will call on an array of function pointers (builtin_func[], also seen 
  // at the beginning), which will trigger the correct function with the arguments passed to it. 
  // This is done instead of a switch statement for the reason of Inversion of Control; it makes 
  // the program very easy to maintain when a programmer would like to add more functionality to 
  // the shell, such as a built in 'ls' funciton. The implementations of these function can be seen 
  // below (there is only two for this assignment).
  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args); // Return to lsh_loop()
    }
  }
  
  /*
	// After checking for the built in commands, the subfunction below runs the user's input as
	// system commands.
  */
  return lsh_launch(args); // Return to lsh_loop()
}

//*****************************************************************************
// Start of functions lsh_cd() and lsh_exit - Subfunctions that implement built in commands. Return directly to lsh_execute()
//*****************************************************************************

/*
	// Change Directory - Function Implementation:
*/
int lsh_cd(char **args){
  // If the argument that follows cd is Null, change the current directory to the home directory.
  if (args[1] == NULL) {
    chdir(getenv("HOME")); // Home directory obtained from environment variable.
  } 
  // Otherwise, change directory based on input filepath.
  else {
    if (chdir(args[1]) != 0) {
      fprintf(stderr, "No such file or directory was found\n");; // If error occurs, program throws an error.
    }
  }
  return 1; // Return to lsh_execute()
}

/*
	// Exit (Control-D) - Function Implementation:
*/
int lsh_exit(char **args){
   printf("^D\n"); // On exit, show that the user pressed 'control-D'.
   return 0; // Return to lsh_execute()
}

//*****************************************************************************
// Start of function lsh_launch() - Subfunction that returns system commands. Returns directly to lsh_execute().
//*****************************************************************************
int lsh_launch(char **args){
  pid_t pid, wpid; // Declaring process id's for parent process. 
  int status; // Declaring status integer for parent process.
  int i = 0; // Declaring integer for simple while-loop iteration below.
  FILE *stream; // Declaring file object for redirecting output, as seen below.

  pid = fork(); // Fork the parent process.
  
  // If fork() returns a 0, then the current process is a child.
  if (pid == 0) {
	  // This while loop checks for the redirection of stdout.
	  // If a '>' exists in the list of arguments (2D character
	  // buffer), then the function will open a file with the
	  // filename that appears after the '>' character. Once
	  // the file stream is open, the function uses 'dup2' 
	  // system call in order to bind stdout to the file.
	  // The file then closes, and both arguments in the list
	  // are assigned a null character, ensuring that execvp()
	  // doesn't fail. Finally, the while-loop breaks.
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
    // Child process is executed using execvp().
    // If a problem occurs, the program throws an error.
    if (execvp(args[0], args) == -1) {
      fprintf(stderr, "Command not found.\n");
	  exit(EXIT_FAILURE); // Terminate process with failure flag.
    }
   
    exit(EXIT_SUCCESS); // Terminate process with success flag.
  }
  // Error forking if fork() return integer that is less than 0. Print to stderr.
  else if (pid < 0) {
    fprintf(stderr, "Could not fork new process.\n");
  }
  // If fork() returns anything else, it is the parent process.
  else {
    // Parent process waits in while loop while the child is processed
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1; // Returns to lsh_execute().
}