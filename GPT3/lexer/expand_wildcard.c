#include "../minishell.h"

int	match_pattern(const char *pat, const char *txt)
{
    if (!*pat)
        return (!*txt);
    if (*pat == '*')
    {
        while (*(pat + 1) == '*')
            pat++;
        if (match_pattern(pat + 1, txt))
            return 1;
        return (*txt && match_pattern(pat, txt + 1));
    }
    if (*txt && *pat == *txt)
        return match_pattern(pat + 1, txt + 1);
    return 0;
}

char	**expand_wildcard(const char *pattern)
{
    DIR           *d;
    struct dirent *ent;
    char         **matches = NULL;
    int            count   = 0;

    d = opendir(".");
    if (!d)
        return NULL;
    while ((ent = readdir(d)))
    {
        if (match_pattern(pattern, ent->d_name))
        {
            char **tmp = realloc(matches, sizeof(*matches)*(count+2));
            if (!tmp)
            {
                while (count--)
                    free(matches[count]);
                free(matches);
                closedir(d);
                return NULL;
            }
            matches = tmp;
            matches[count] = strdup(ent->d_name);
            matches[++count] = NULL;
        }
    }
    closedir(d);
    return matches;
}
