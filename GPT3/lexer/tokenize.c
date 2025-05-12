#include "../minishell.h"

extern volatile sig_atomic_t g_exit_status;

int is_op_char(char c)
{
    return (c=='|' || c=='&' || c=='<' ||
            c=='>' || c=='(' || c==')');
}

char *dup_range(const char *s, int st, int ed)
{
    char *res = malloc(ed - st + 1);
    if (!res)
        exit(EXIT_FAILURE);
    ft_memcpy(res, s + st, ed - st);
    res[ed - st] = '\0';
    return res;
}

int count_tokens(const char *s)
{
    int i = 0, cnt = 0;
    while (s[i])
    {
        while (ft_isspace(s[i]))
            i++;
        if (!s[i])
            break;
        cnt++;
        if (is_op_char(s[i]) && s[i+1] == s[i])
            i += 2;
        else if (is_op_char(s[i]))
            i++;
        else
            while (s[i] && !isspace(s[i]) && !is_op_char(s[i]))
                i++;
    }
    return cnt;
}

void add_tok(t_token *arr, int *idx, t_tokentype type, char *val)
{
    arr[*idx].type   = type;
    arr[*idx].value  = val;
    arr[*idx].quoted = 0;
    (*idx)++;
}

void process_word(const char *s, int *p, t_token *arr, int *idx)
{
    int start = *p;
    while (s[*p] && !isspace(s[*p]) && !is_op_char(s[*p]))
        (*p)++;
    add_tok(arr, idx, TOK_WORD,
            dup_range(s, start, *p));
}

void process_op(const char *s, int *p, t_token *arr, int *idx)
{
    int len;
    t_tokentype ty;

    if ((s[*p]=='&' && s[*p+1]=='&') || (s[*p]=='|' && s[*p+1]=='|') ||
        (s[*p]=='<' && s[*p+1]=='<') || (s[*p]=='>' && s[*p+1]=='>'))
        len = 2;
    else
        len = 1;
    if (len == 2)
    {
        if (s[*p] == '&')
            ty = TOK_AND;
        else if (s[*p] == '|')
            ty = TOK_OR;
        else if (s[*p] == '<')
            ty = TOK_HEREDOC;
        else
            ty = TOK_APPEND;
    }
    else
    {
        if (s[*p] == '|')
            ty = TOK_PIPE;
        else if (s[*p] == '<')
            ty = TOK_REDIR_IN;
        else
            ty = TOK_REDIR_OUT;
    }
    add_tok(arr, idx, ty,
        dup_range(s, *p, *p + len));
    *p += len;
}

t_token *tokenize(const char *input, int *out_count)
{
    int      total = count_tokens(input);
    t_token *arr   = malloc(sizeof(*arr) * (total + 1));
    int      idx   = 0;
    int      pos   = 0;

    if (!arr)
        return NULL;
    arr[total].type  = TOK_END;
    arr[total].value = NULL;
    while (input[pos])
    {
        if (isspace(input[pos]))
            pos++;
        else if (is_op_char(input[pos]))
            process_op(input, &pos, arr, &idx);
        else
            process_word(input, &pos, arr, &idx);
    }
    *out_count = idx;
    return arr;
}
