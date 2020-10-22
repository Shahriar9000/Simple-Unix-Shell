// Shell starter file
// You may make any changes to any part of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>


#define COMMAND_LENGTH 1024
#define n_TOKENS (COMMAND_LENGTH / 2 + 1)
#define BUFFER_SIZE 1024
#define HISTORY_DEPTH 10
char history[HISTORY_DEPTH][COMMAND_LENGTH];
int history_index = 0 ;
int history_command_counter = 0;

char Buffer_for_write[BUFFER_SIZE];

//_Bool Control_C = false;


ssize_t write_function(char *message)
{
	int length_of_message = strlen(message);
	return write(STDOUT_FILENO, message, length_of_message);
}


void current_directory()
{
	char* pwd = getcwd(Buffer_for_write, BUFFER_SIZE);
		if (pwd == NULL)
		{
			write_function("Unable to get current directory.");
		}

		write_function(Buffer_for_write);
}


void add_command_to_history (char* command)
{
	memcpy(history[history_index], command, COMMAND_LENGTH);
	history_index = (history_index+1) % HISTORY_DEPTH;
	history_command_counter++;

}

char* get_command_from_history (int _number)
{ 
	if (_number < 1 ) 
	{
		return NULL;
	}

	if (_number > history_command_counter) 
	{
		return NULL;
	}

	if (history_command_counter > HISTORY_DEPTH) 
	{
		return NULL;
	}

	if (_number <= (history_command_counter - HISTORY_DEPTH))
	{
		return NULL;
	}

	int i = (_number- 1) % HISTORY_DEPTH;
	return history[i];
}

void print_command_history()
{
	int n = history_command_counter;

	if (history_command_counter > HISTORY_DEPTH) 
	{
		n = HISTORY_DEPTH;
	}

	int i = n-1;

	while (i >=0)
	{
		int j = (HISTORY_DEPTH + history_index - 1 - i) % HISTORY_DEPTH;

		int c = history_command_counter - i;
		snprintf(Buffer_for_write, BUFFER_SIZE, "%d", c);
	    write_function(Buffer_for_write);

		write_function("\t");
		write_function(history[j]); 
		write_function("\n");
		i--;
	}

}

void handle_SIGINT()
{
	write_function("\n");
	print_command_history();
	//Control_C = true;
	
}

void main_sigint_handler()
{
	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	handler.sa_flags = 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);
}





/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: nber of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int n_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < n_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least n_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */


void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;
	char command[COMMAND_LENGTH];

	// Read input

	int length = read(STDIN_FILENO, command, COMMAND_LENGTH-1);


		// If read failure was not caused by ctrl-c, then exit
	if ((length < 0) && (errno != EINTR) )
	{
			perror("Unable to read command. Terminating.\n");
			exit(-1);
	}
	

	// Null terminate and strip \n.
	command[length] = '\0';
	if (command[strlen(command) - 1] == '\n') 
	{
		command[strlen(command) - 1] = '\0';
	}

	memcpy(buff, command, COMMAND_LENGTH);

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) 
	{
		return;
	}


	if (tokens[0][0] == '!' ) 
	{
		int i = 0;

		if (tokens[0][1] == '!') 
		{
			i = history_command_counter;
 		} 

 		else
 		{	
 			
 			i = atoi(&tokens[0][1]); 			
 			
 		}

		char *command_taken_from_history = get_command_from_history(i);

		if (command_taken_from_history != NULL) 
		{
			memcpy(buff, command_taken_from_history, COMMAND_LENGTH);
			memcpy(command, command_taken_from_history, COMMAND_LENGTH);
			write_function(buff);
			write_function("\n");
			token_count = tokenize_command(buff, tokens);
			
			
		}

		else
		{	
			write_function("SHELL: Unknown history command.\n");
			tokens[0] = 0;
			token_count = 0;			
		}
	}

	// Record command in history
	if (token_count > 0) 
	{
		add_command_to_history(command);
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) 
	{
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}

_Bool If_Internal_Commands(char* tokens[n_TOKENS])
{
	
	
	if (strcmp(tokens[0], "exit") == 0) 
	{
		exit(0);
	}
	
	else if (strcmp(tokens[0], "pwd") == 0) 
	{	
		current_directory();

		write_function("\n");
	}

	else if (strcmp(tokens[0], "cd") == 0)
	{
		int cd = chdir(tokens[1]);
		if (cd != 0) 
		{
			write_function("Invalid directory.\n");
		}


	}

	else if (strcmp(tokens[0], "type") == 0)
	{
		if (tokens[1] != NULL)
		{
			if (strcmp(tokens[1], "exit") == 0)
			{
				write_function("exit is a shell300 builtin");
				write_function("\n");
			}	

			else if (strcmp(tokens[1], "pwd") == 0)
			{
				write_function("pwd is a shell300 builtin");
				write_function("\n");
			}

			else if (strcmp(tokens[1], "cd") == 0)
			{
				write_function("cd is a shell300 builtin");
				write_function("\n");
			}

			else if (strcmp(tokens[1], "type") == 0)
			{
				write_function("type is a shell300 builtin");
				write_function("\n");
			}
			else
			{	
			write_function(tokens[1]);
			write_function(" is external to shell300");
			write_function("\n");
			}

		}	
		else
		{
			return false;
		}
		

		

	}

	else if (strcmp(tokens[0], "history") == 0) 
	{
			print_command_history();
	}
	
	else 
	{
		return false;
	} 

	return true;
}

/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{	

	main_sigint_handler();
	
	char input_buffer[COMMAND_LENGTH];
	char *tokens[n_TOKENS];
	while (true) 
	{
		
		// Cleanup any previously exited background child processes
		while (waitpid(-1, NULL, WNOHANG) > 0); // do nothing

		current_directory(); write_function("$ "); 
		
		// Get command		
		_Bool in_background = false;
		read_command(input_buffer, tokens, &in_background);


		// Handeles Enter key
		if (tokens[0] == NULL)
		{	
			continue;
		}
		
		if (in_background) 
		{
			write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
		}

		if (If_Internal_Commands(tokens) == false)
		{

			pid_t child_pid = fork ();

			if (child_pid < 0)
			{
				perror("Error, aborting. \n");
				exit(-1);
			}
			if (child_pid == 0)
			{
				execvp(tokens[0], tokens);

				write_function(tokens[0]);
				perror(":- Command not recognized.\n");
				exit(-1);
			}

			if (!in_background)
			{
				pid_t pid = waitpid(child_pid, NULL, 0);
				if (pid < 0) 
				{
					perror("Error waiting for child.\n");
					exit(-1);
				}
			}
		}

		
	
	}
	return 0;
}
