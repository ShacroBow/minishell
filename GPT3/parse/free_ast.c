#include "../minishell.h"

void free_commands(t_command *cmd)
{
    while (cmd)
    {
        t_command *next = cmd->next;
        if (cmd->argv)
        {
            for (int i = 0; cmd->argv[i]; i++)
                free(cmd->argv[i]);
            free(cmd->argv);
        }
        free(cmd->infile);
        free(cmd->outfile);
        if (cmd->subshell)
            free_segments(cmd->subshell_segments);
        free(cmd);
        cmd = next;
    }
}

void free_segments(t_segment *seg)
{
    while (seg)
    {
        t_segment *next = seg->next;
        free_commands(seg->pipeline);
        free(seg);
        seg = next;
    }
}
