#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	char *pwd;
	char *string;

	string = getenv("PWD");
	printf("string (initial PWD): %s\n", string);
	
	pwd = getcwd(NULL, 0);
	printf("pwd before chdir: %s\n", pwd);
	free(pwd);
	
	if (chdir("../") != 0) {
		perror("chdir failed");
		return 0;
	}
	// this is the equivilent of us just changing a local equivalent.
	setenv("OLDPWD", pwd, 1);
	
	// Update the PWD environment variable after changing directory
	pwd = getcwd(NULL, 0); // Get new current directory
	printf("pwd after chdir: %s\n", pwd);
	free(pwd);
	
	// Clean up
	
	if (chdir("./minishell") != 0) //assuming the file name matches whats written.
	{
		perror("chdir failed");
		return 0;
	}
	setenv("OLDPWD", pwd, 1);
	pwd = getcwd(NULL, 0); // Get new current directory
	printf("pwd after chdir 2: %s\n", pwd);
	// Clean up
	free(pwd);
	return 0;
}