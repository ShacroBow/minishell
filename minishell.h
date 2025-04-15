#ifndef H_MINISHELL
#define H_MINISHELL
//normals
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
//errors
# include <signal.h>
# include <errno.h>
//idk
# include <sys/types.h>
# include <sys/wait.h>
//readline
# include <readline/readline.h>
# include <readline/history.h>
//selfwritten
# include "libft/libft.h"

void	ft_signalhandle(void);

#endif