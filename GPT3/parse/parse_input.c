#include "../minishell.h"

extern volatile sig_atomic_t g_exit_status;

static	int check_quotes(const char *s)
{
    int	i = 0;
	int	qs = 0;
	int	qd = 0;

	i = 0;
	qs = 0;
	qd = 0;
    while (s[i])
	{
		if (s[i] == '\'')
			qs ^= 1;
		else if (s[i] == '\"')
			qd ^= 1;
		i++;
	}
	return (!qs && !qd);
}

//static void print_error(const char *msg)
static void print_error(char *msg)
{
    ft_putstr_fd(msg, STDERR_FILENO);
    ft_putstr_fd("\n", STDERR_FILENO);
    g_exit_status = 258;
}

t_segment *parse_input(const char *input, char ***envp)
{
    t_token   *tok;
    t_segment *ast;
    int        cnt;
    int        idx;

    idx = 0;
	cnt = 0;
	if (!check_quotes(input))
    {
        print_error("minishell: unclosed quote");
        return NULL;
    }
	tok = tokenize(input, &cnt);
    if (!tok)
        return NULL;
    ast = parse_segments(tok, &idx, cnt, 0);
    free_tokens(tok, cnt);
    if (ast && idx < cnt)
    {
        print_error("minishell: syntax error near unexpected token");
        free_segments(ast);
        return NULL;
    }
    ast->envp = envp;
    return ast;
}
