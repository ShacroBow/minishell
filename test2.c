#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

int main()
{
	char *input;

	// Clear any existing history
	rl_clear_history();

	while (1) {
		// Read user input
		input = readline("Enter command (type 'exit' to quit): ");
		
		// If input is NULL, break the loop (EOF)
		if (input == NULL) {
			break;
		}

		// Add command to history
		add_history(input);

		// Check for exit command
		if (strcmp(input, "exit") == 0) {
			free(input);
			printf("Exiting the program.\n");
			break;
		}

		// Print the entire command history
		printf("Current command history:\n");
		HIST_ENTRY **hist_list = history_list();
		if (hist_list) {
			for (int i = 0; hist_list[i] != NULL; i++) {
				printf("%d: %s\n", i + 1, hist_list[i]->line);
				if (i == 4)
				{
					rl_clear_history();
					break;
				}
			}
		}

		// Free the input buffer
		free(input);
	}

	// Clear history
	rl_clear_history();

	return 0;
}