#include "../minishell.h"

t_command *new_command(void)
{
    t_command *c = ft_calloc(1, sizeof(*c));
    if (!c)
    {
        perror("minishell");
        exit(EXIT_FAILURE);
    }
    return c;
}

void add_arg(t_command *c, const char *val)
{
    int     cnt = 0;
    char  **na;

    while (c->argv && c->argv[cnt])
        cnt++;
    na = ft_calloc(cnt + 2, sizeof(*na));
    if (!na)
    {
        perror("minishell");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < cnt; i++)
        na[i] = c->argv[i];
    na[cnt] = ft_strdup(val);
    na[cnt + 1] = NULL;
    free(c->argv);
    c->argv = na;
}

int handle_subshell(t_command *cur, t_token *tok, int *i, int n)
{
    cur->subshell = 1;
    (*i)++;
    cur->subshell_segments =
        parse_segments(tok, i, n, 1);
    return 0;
}

int handle_word(t_command *cur, t_token *tok, int *i)
{
    add_arg(cur, tok[*i].value);
    (*i)++;
    return 0;
}

int handle_redir(t_command *cur, t_token *tok, int *i, int n)
{
    t_tokentype ty = tok[*i].type;
    (*i)++;
    if (*i >= n || tok[*i].type != TOK_WORD)
        return -1;
    if (ty == TOK_REDIR_IN || ty == TOK_HEREDOC)
        cur->infile = ft_strdup(tok[*i].value);
    else
    {
        cur->outfile = ft_strdup(tok[*i].value);
        cur->append  = (ty == TOK_APPEND);
    }
    (*i)++;
    return 0;
}

int push_pipeline(t_segment **h, t_segment **t, t_command *pipe, t_tokentype op)
{
    t_segment *seg = ft_calloc(1, sizeof(*seg));
    if (!seg)
    {
        perror("minishell");
        exit(EXIT_FAILURE);
    }
    seg->pipeline = pipe;
    seg->op       = op;
    if (!*h)
        *h = seg;
    else
        (*t)->next = seg;
    *t = seg;
    return 0;
}
