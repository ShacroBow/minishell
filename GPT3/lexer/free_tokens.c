#include "../minishell.h"

void	free_tokens(t_token *tok, int count)
{
    for (int i = 0; i < count; i++)
        free(tok[i].value);
    free(tok);
}
