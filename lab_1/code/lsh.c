/*
 * Main source file for the lsh shell program.
 *
 * You are free to add functions to this file.
 * If you want to add functions in separate files,
 * you will need to modify CMakeLists.txt to compile
 * your additional files.
 *
 * Add appropriate comments to make your code
 * easier for us to grade.
 *
 * Using assert statements is a good way to catch errors early and make debugging easier.
 * Think of them as mini self-checks that ensure your program behaves as expected.
 * By setting up these guardrails, you're creating a more robust and maintainable solution.
 * So go ahead, sprinkle some asserts in your code; they're your friends in disguise!
 *
 * All the best!
 */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <wait.h>

// The <unistd.h> header is your gateway to the OS's process management facilities.
#include <unistd.h>

#include "parse.h"

static void print_cmd(Command *cmd);
static void print_pgm(Pgm *p);
void stripwhite(char *);

int main(void)
{
  signal(SIGINT, SIG_IGN); // Sets the disposition of SIGINT to SIG_IGN which ignores the signal (like ctrl + c)
  for (;;)
  {
    char *line;
    line = readline("> ");

    // Remove leading and trailing whitespace from the line
    stripwhite(line);

    // If the stripped line is not blank
    if (*line)
    {
      add_history(line);

      Command cmd;
      if (parse(line, &cmd) == 1)
      {
        if (strcmp(cmd.pgm->pgmlist[0], "exit") == 0) { // Checks if the command's string is equal to "exit"
          exit(0); // executes the built-in function exit(0) with status 0
        } 
        if (strcmp(cmd.pgm->pgmlist[0], "cd") == 0) { // Checks if the command's string is "cd"
          chdir(cmd.pgm->pgmlist[1]); // executes the built-in function chdir to enter the folder held in the second argument of pgmlist
          continue;
        }

        pid_t pid = fork(); // forks the process
        if (pid < 0) { // checks if fork failed
          perror("Fork failed");
        }
        else if (pid == 0) { // if pid == 0, it is the child process
          signal(SIGINT, SIG_DFL);
          // Child process
          if (cmd.background) { // If the child is not a background process, set the disposition of SIGINT to SIG_DFL which listens to ctrl + c
            setpgid(0,0);
          }
          execvp(cmd.pgm->pgmlist[0], cmd.pgm->pgmlist); // Execute the command using execvp in the child process
          printf("Command failed: %s\n", cmd.pgm->pgmlist[0]); // This is only run if there is an issue with execvp
          exit(1);
        }
        else {
          // Parent process
          if (cmd.background == 0) { // If the child is not a background job, we need to wait for it, otherwise we don't
            wait(NULL);
          }
        }

        // Print the parsed command
        print_cmd(&cmd);
      }
      else
      {
        printf("Parse ERROR\n");
      }
    }

    // Free the input buffer
    free(line);
  }

  return 0;
}

/*
 * Print a Command structure as returned by parse on stdout.
 *
 * Helper function, no need to change. Might be useful to study as inspiration.
 */
static void print_cmd(Command *cmd_list)
{
  printf("------------------------------\n");
  printf("Parse OK\n");
  printf("stdin:      %s\n", cmd_list->rstdin ? cmd_list->rstdin : "<none>");
  printf("stdout:     %s\n", cmd_list->rstdout ? cmd_list->rstdout : "<none>");
  printf("background: %s\n", cmd_list->background ? "true" : "false");
  printf("Pgms:\n");
  print_pgm(cmd_list->pgm);
  printf("------------------------------\n");
}

/* Print a linked list of Pgm structures.
 *
 * Helper function, no need to change. It may be useful to study for inspiration.
 */
static void print_pgm(Pgm *p)
{
  if (p == NULL)
  {
    return;
  }
  else
  {
    char **pl = p->pgmlist;

    /* The list is stored in reverse order, so print
     * it in reverse to restore the original order.
     */
    print_pgm(p->next);
    printf("            * [ ");
    while (*pl)
    {
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}


/* Strip whitespace from the start and end of a string.
 *
 * Helper function, no need to change.
 */
void stripwhite(char *string)
{
  size_t i = 0;

  while (isspace(string[i]))
  {
    i++;
  }

  if (i)
  {
    memmove(string, string + i, strlen(string + i) + 1);
  }

  i = strlen(string) - 1;
  while (i > 0 && isspace(string[i]))
  {
    i--;
  }

  string[++i] = '\0';
}
