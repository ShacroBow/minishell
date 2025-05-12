#include "../minishell.h"

void	buf_append(char **buf, int *cap, int *len, char c)
{
    if (*len + 2 >= *cap)
    {
        //*cap *= 2;
        *buf = ft_realloc(*buf, *cap, *cap * 2, sizeof(int));
        if (!*buf)
        {
            perror("minishell");
            exit(EXIT_FAILURE);
        }
    }
    (*buf)[(*len)++] = c;
    (*buf)[*len] = '\0';
}
