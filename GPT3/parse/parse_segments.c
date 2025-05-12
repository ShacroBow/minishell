#include "../minishell.h"

t_segment *parse_segments(t_token *tok, int *idx, int n, int in_sub)
{
    t_segment *head = NULL;
    t_segment *tail = NULL;
    t_command *cur  = NULL;
    int        need = 1;
    while (*idx < n)
    {
        t_tokentype ty = tok[*idx].type;
        if (ty == TOK_RPAREN)
        {
            if (!in_sub)
                return NULL;
            (*idx)++;
            break;
        }
        if ((ty == TOK_PIPE || ty == TOK_AND || ty == TOK_OR) && !need)
        {
            push_pipeline(&head, &tail, cur, ty);
            cur  = NULL;
            need = 1;
            (*idx)++;
            continue;
        }
        if (need)
            cur = new_command(), need = 0;
        if (ty == TOK_LPAREN)
            handle_subshell(cur, tok, idx, n);
        else if (ty == TOK_WORD)
            handle_word(cur, tok, idx);
        else if ((ty >= TOK_REDIR_IN && ty <= TOK_APPEND) && handle_redir(cur, tok, idx, n) < 0)
            return NULL;
        else
            (*idx)++;
    }
    if (cur)
        push_pipeline(&head, &tail, cur, 0);
    return head;
}
