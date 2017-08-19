#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <pwd.h>


#define MAX_LINE		80 /* 80 chars per line, per command, should be enough. */
#define MAX_COMMANDS	5 /* size of history */

/**
 * The setup function below will read in the next command line;
 * separate it into distinct arguments (using blanks as delimiters),
 * and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings.
 */

int setup(char inputBuffer[], char *args[], int *background)
{
    int length,		/* # of characters in the command line */
	i,				/* loop index for accessing inputBuffer array */
	start,			/* index where beginning of next command parameter is */
	ct;				/* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
	do {
        printf("comp322>");
        fflush(stdout);
		length = read(STDIN_FILENO,inputBuffer,MAX_LINE);
	}
	while (inputBuffer[0] == '\n'); /* swallow newline characters */

    /**
	 *  0 is the system predefined file descriptor for stdin (standard input),
     *  which is the user's screen in this case. inputBuffer by itself is the
     *  same as &inputBuffer[0], i.e. the starting address of where to store
     *  the command that is read, and length holds the number of characters
     *  read in. inputBuffer is not a null terminated C-string.
	 */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

	/**
	 * the <control><d> signal interrupted the read system call
	 * if the process is in the read() system call, read returns -1
  	 * However, if this occurs, errno is set to EINTR. We can check this  value
  	 * and disregard the -1 value
	 */
    if ( (length < 0) && (errno != EINTR) ) {
		perror("error reading the command");
		exit(-1);           /* terminate with error code of -1 */
    }

	/**
	 * Parse the contents of inputBuffer
	 */

    for (i=0;i<length;i++)
    {
		/* examine every character in the inputBuffer, find special characters in the inputBuffer */
		//space and tab are treated the same
		if(inputBuffer[i] == ' ' || inputBuffer[i] == '\t')
        {
            if(start != -1)
            {
                //We must get the actual value
                args[ct] = &inputBuffer[start];
                ct = ct + 1;
            }
            //Adding a null char to make a c string.
            inputBuffer[i] = '\0';
            start = -1;
        }
        //new line character
        else if(inputBuffer[i] == '\n')
        {
            if(start != -1)
            {
                args[ct] = &inputBuffer[start];
                ct = ct + 1;
            }
            inputBuffer[i] = '\0';
            args[ct] = NULL;
        }
        //A character that we will put in our array
        else
        {
            if(start == -1)
            {
                start = i;
            }
            if(inputBuffer[i] == '&')
            {
                *background = 1;
                inputBuffer[i - 1] = '\0';
            }
        }
	}    /* end of for */
    if(*background)
    {
        args[--ct] = NULL;
    }
	args[ct] = NULL; /* just in case the input line was > 80 */

	return 1;

} /* end of setup routine */


int main(void)
{
	char inputBuffer[MAX_LINE]; 	/* buffer to hold the command entered */
	char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
	pid_t child;            		/* process id of the child process */
	int status;           		/* result from execvp system call*/
	int shouldrun = 1;
    int background;
	int i, upper;

    while (shouldrun)
    {            		/* Program terminates normally inside setup */
        background = 0;
		shouldrun = setup(inputBuffer,args,&background);       /* get next command */

		/* Deal with user's command from the new shell*/
        if (strncmp(inputBuffer, "exit", 4) == 0)
        {
            return 0;
        }
        /**
         * Deal with user's command from the new shell:
         * (1) fork a child process
         * (2) read user input (partially done in setup())
         * (3) the child process will invoke execvp()
         */
         //fork the child process
        child = fork();
         /* Do not forget to print out your information*/
         if(child < 0)
         {
             printf("Fork failed.\n");
             exit(0);
         }
         else if(child == 0)
         {
             status = execvp(args[0],args);
             printf("in child process.");
             if(status != 0)
             {
                 printf("Error in execvp.");
                 exit(-1);
             }
         }
         else
         {
             if(background == 0)
             {
                 while(child != wait(NULL));
             }
         }
      register struct passwd *pw;
      register uid_t uid;
      uid = geteuid ();
      pw = getpwuid (uid);

      time_t mytime;
      mytime = time(NULL);

      printf("Program run by: %s at %s", pw->pw_name, ctime(&mytime));
    }
	return 0;
}
