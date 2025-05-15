#include "../minishell.h"

extern volatile sig_atomic_t g_exit_status;


/* free_tokens.c – release the token array */

void	free_tokens(t_token *tok, int count)
{
	if (!tok)
		return;
	for (int i = 0; i < count; ++i)
		free(tok[i].value);
	free(tok);
}


/* ───────── free command and segment lists ───────── */
void free_commands(t_command *cmd) {
	while (cmd) {
		t_command *nx = cmd->next;
		if (cmd->argv) {
			for (int i = 0; cmd->argv[i]; ++i)
				free(cmd->argv[i]);
			free(cmd->argv);
		}
		if (cmd->infile) {
			if (cmd->heredoc)
				unlink(cmd->infile);
			free(cmd->infile);
		}
		if (cmd->outfile)
			free(cmd->outfile);
		if (cmd->subshell && cmd->subshell_segments)
			free_segments(cmd->subshell_segments);
		free(cmd);
		cmd = nx;
	}
}

void free_segments(t_segment *seg)
{
	while (seg) {
		t_segment *nx = seg->next;
		if (seg->pipeline)
			free_commands(seg->pipeline);
		free(seg);
		seg = nx;
	}
}

/* ───────── dynamic buffer append for tokenization ───────── */
static void buf_append(char **buf, int *cap, int *len, char c)
{
	if (*len + 1 >= *cap) {
		*cap *= 2;
		*buf = realloc(*buf, *cap);
		if (!*buf)
		{
			perror("minishell");
			exit(EXIT_FAILURE);
		}
	}
	(*buf)[(*len)++] = c;
	(*buf)[*len] = '\0';
}

/* ───────── wildcard pattern matching (recursive) ───────── */
static int match_pattern(const char *pat, const char *text)
{
	if (!*pat) return !*text;
	if (*pat == '*') {
		while (*(pat+1) == '*') pat++;
		if (match_pattern(pat+1, text))
			return 1;
		return (*text && match_pattern(pat, text+1));
	}
	if (*pat == *text && *text)
		return match_pattern(pat+1, text+1);
	return 0;
}

/* ───────── expand wildcard pattern to list of matches ───────── */
static char **expand_wildcard(const char *pattern)
{
	DIR *d = opendir(".");
	if (!d) return NULL;
	struct dirent *ent;
	char **matches = NULL;
	int count = 0;
	int pat_has_dot = (pattern[0] == '.');
	while ((ent = readdir(d))) {
		char *name = ent->d_name;
		if (!pat_has_dot && name[0] == '.')
			continue;
		if (match_pattern(pattern, name)) {
			char **tmp = realloc(matches, sizeof(char*) * (count + 2));
			if (!tmp) {
				/* allocation failed: free and return */
				for (int k = 0; k < count; ++k) free(matches[k]);
				free(matches);
				closedir(d);
				return NULL;
			}
			matches = tmp;
			matches[count++] = ft_strdup(name);
			matches[count] = NULL;
		}
	}
	closedir(d);
	if (!matches)
		return NULL;
	/* sort matches lexicographically */
	for (int i = 0; i < count - 1; ++i) {
		for (int j = i + 1; j < count; ++j) {
			if (ft_strcmp(matches[i], matches[j]) > 0) {
				char *swap = matches[i];
				matches[i] = matches[j];
				matches[j] = swap;
			}
		}
	}
	return matches;
}

/* ───────── error message for syntax errors ───────── */
int redir_error(void)
{
	write(2, "minishell: syntax error near unexpected token\n", 47);
	g_exit_status = 258;
	return (-1);
}

/* ───────── add an argument to a command's argv ───────── */
static void add_arg(t_command *cmd, const char *val)
{
	int argc = 0;
	while (cmd->argv && cmd->argv[argc])
		argc++;
	char **newargv = malloc((argc + 2) * sizeof(char *));
	if (!newargv) { perror("minishell"); exit(EXIT_FAILURE); }
	for (int i = 0; i < argc; ++i)
		newargv[i] = cmd->argv[i];
	newargv[argc] = ft_strdup(val);
	if (!newargv[argc]) { perror("minishell"); exit(EXIT_FAILURE); }
	newargv[argc + 1] = NULL;
	free(cmd->argv);
	cmd->argv = newargv;
}

/* ───────── allocate a new command in the pipeline if needed ───────── */
void new_command_if_needed(t_command **cmd_head, t_command **cmd_tail, int *need_cmd)
{
	if (*need_cmd) {
		t_command *cm = ft_calloc(1, sizeof(*cm));
		if (!cm) { perror("minishell"); exit(EXIT_FAILURE); }
		if (!*cmd_head)
			*cmd_head = cm;
		else
			(*cmd_tail)->next = cm;
		*cmd_tail = cm;
		*need_cmd = 0;
	}
}

/* ───────── handle subshell start '(' ───────── */
int handle_parenthesis(t_token *tok, int *idx, int n, t_command *cur)
{
	t_parse_segments	ps;

	(*idx)++;  /* skip '(' */
	cur->subshell = 1;
	ps.idx = *idx;
	ps.n = n;
	ps.in_sub = 1;
	cur->subshell_segments = parse_segments(tok, &ps, cur->envp);
	cur->argv = malloc(sizeof(char*) * 2);
	if (!cur->argv) { perror("minishell"); exit(EXIT_FAILURE); }
	cur->argv[0] = ft_strdup("SUBSHELL");
	cur->argv[1] = NULL;
	return 0;
}

/* ───────── handle a word token (arguments and wildcards) ───────── */
int handle_word(t_token *tok, int *idx, t_command *cur)
{
	char *val = tok[*idx].value;
	if (!tok[*idx].quoted && ft_strchr(val, '*')) {
		char **matches = expand_wildcard(val);
		if (matches) {
			for (int k = 0; matches[k]; ++k)
				add_arg(cur, matches[k]);
			for (int k = 0; matches[k]; ++k)
				free(matches[k]);
			free(matches);
		} else {
			add_arg(cur, val);
		}
	} else {
		add_arg(cur, val);
	}
	(*idx)++;
	return 0;
}

/* ───────── handle redirection tokens (<, <<, >, >>) ───────── */
int handle_redirection(t_token *tok, int *idx, int n, t_command *cur)
{
	char **env;

	env = cur->envp;
	t_tokentype typ = tok[*idx].type;
	(*idx)++;
	if (*idx >= n || tok[*idx].type != TOK_WORD)
		return redir_error();
	char *file = tok[*idx].value;
	if (typ == TOK_REDIR_IN || typ == TOK_HEREDOC) {
		if (cur->infile && cur->heredoc)
			unlink(cur->infile);
		free(cur->infile);
		cur->heredoc = 0;
		if (typ == TOK_HEREDOC) {
			/* open a temp file for heredoc content */
			char tmp_name[PATH_MAX];
			static int hd_count = 0;
			sprintf(tmp_name, "/tmp/minishell_hd_%d_%d", getpid(), hd_count++);
			int fd = open(tmp_name, O_WRONLY | O_CREAT | O_TRUNC, 0600);
			if (fd < 0) {
				perror("minishell");
				return -1;
			}
			char *lim = tok[*idx].value;
			int no_expand = tok[*idx].quoted;
			ft_signals_heredoc_setup();
			/* read heredoc lines until limiter */
			while (1) {
				char *line = readline("> ");
				if (g_exit_status == 130)
					break;
				if (!line) {
					write(2, "minishell: warning: here-document delimited by end-of-file (wanted `", 65);
					write(2, lim, strlen(lim));
					write(2, "`)\n", 3);
					break;
				}
				if (strcmp(line, lim) == 0) {
					free(line);
					break;
				}
				if (!no_expand) {
					/* expand variables in heredoc content */
					char *expanded = NULL;
					int cap2 = 128, len2 = 0;
					expanded = malloc(cap2);
					if (!expanded) { perror("minishell"); exit(EXIT_FAILURE); }
					expanded[0] = 0;
					for (int x = 0; line[x]; ) {
						if (line[x] == '$') {
							if (line[x+1] == '?') {
								char tmp[16];
								int l = sprintf(tmp, "%d", g_exit_status);
								for (int k = 0; k < l; ++k)
									buf_append(&expanded, &cap2, &len2, tmp[k]);
								x += 2;
								continue;
							}
							int j = x + 1;
							if (!ft_isalnum(line[j]) && line[j] != '_') {
								buf_append(&expanded, &cap2, &len2, '$');
								x++;
								continue;
							}
							while (ft_isalnum(line[j]) || line[j] == '_')
								j++;
							char *name = ft_strndup(line + x + 1, j - (x + 1));
							size_t namelen = ft_strlen(name);
							char *val = NULL;
							for (int k = 0; env && env[k]; ++k) {
								if (!ft_strncmp(env[k], name, namelen) &&
									env[k][namelen] == '=') {
									val = env[k] + namelen + 1;
									break;
								}
							}
							if (!val)
								val = getenv(name);
							if (val) {
								for (int k = 0; val[k]; ++k)
									buf_append(&expanded, &cap2, &len2, val[k]);
							}
							free(name);
							x = j;
							continue;
						}
						buf_append(&expanded, &cap2, &len2, line[x]);
						x++;
					}
					write(fd, expanded, len2);
					write(fd, "\n", 1);
					free(expanded);
				} else {
					write(fd, line, strlen(line));
					write(fd, "\n", 1);
				}
				free(line);
			}
			close(fd);
			ft_signal_setup();
			if (g_exit_status == 130)
			{
				/* aborted by Ctrl-C */
				free_commands(cur);
				unlink(tmp_name);
				return -1;
			}
			cur->infile = ft_strdup(tmp_name);
			cur->heredoc = 1;
		} else {
			cur->infile = ft_strdup(file);
			if (!cur->infile) { perror("minishell"); exit(EXIT_FAILURE); }
		}
	} else {  /* output redirection */
		free(cur->outfile);
		cur->outfile = ft_strdup(file);
		if (!cur->outfile) { perror("minishell"); exit(EXIT_FAILURE); }
		cur->append = (typ == TOK_APPEND);
	}
	(*idx)++;
	return 0;
}

/* ───────── push a completed pipeline into segment list ───────── */
void push_pipeline_to_segments(t_segment **h, t_segment **t, t_command *pipe, t_tokentype op)
{
	t_segment *seg = malloc(sizeof(*seg));
	if (!seg) { perror("minishell"); exit(EXIT_FAILURE); }
	seg->pipeline = pipe;
	seg->op = op;
	seg->next = NULL;
	if (!*h)
		*h = seg;
	else
		(*t)->next = seg;
	*t = seg;
}

/* ───────── handle &&, ||, | operators ───────── */
int handle_bool_or_pipe(t_token *tok, int *idx,
		t_segment **seg_h, t_segment **seg_t, t_command **pipe_h, t_command **pipe_t, int *need_cmd)
{
	t_tokentype typ = tok[*idx].type;
	if (*need_cmd)
		return redir_error();
	if (typ == TOK_PIPE) {
		(*idx)++;
		*need_cmd = 1;
		return 0;
	}
	/* if && or ||, end the current pipeline */
	push_pipeline_to_segments(seg_h, seg_t, *pipe_h, typ);
	*pipe_h = *pipe_t = NULL;
	*need_cmd = 1;
	(*idx)++;
	return 0;
}

/* ───────── recursive descent parser for command sequence ───────── */


static t_segment	*handle_parse_segments(t_token *tok, int tcount, char **env)
{
	t_segment			*ast;
	t_parse_segments	ps;

	ps.idx = 0;
	ps.in_sub = 0;
	ps.n = tcount;
	ast = parse_segments(tok, &ps , env);
	if (ps.idx < tcount && ast)
	{
		ft_putstr_fd("minishell: syntax error near unexpected token\n", STDERR_FILENO);
		g_exit_status = 258;
		free_segments(ast);
		ast = NULL;
	}
	return (ast);
}

/* ───────── top-level parse function ───────── */
static int  detect_unclosed_quotes(const char *input)
{
	int quotes_s;
	int quotes_d;
	int i;

	quotes_s = 0;
	quotes_d = 0;
	i = 0;
	while (input[i] != '\0')
	{
		if (input[i] == '\'' && !quotes_d)
			quotes_s ^= 1;
		else if (input[i] == '"' && !quotes_s)
			quotes_d ^= 1;
		i++;
	}
	return (quotes_s || quotes_d);
}

/* ───────── top-level parse function ───────── */
t_segment *parse_input(const char *input, char ***envp)
{
	int					tcount;
	t_token				*tok;
	t_segment			*ast;

	tcount = 0;
	if (detect_unclosed_quotes(input))
	{
		ft_putstr_fd("minishell: syntax error unclosed quote\n", STDERR_FILENO);
		return (g_exit_status = 258, NULL);
	}
	tok = ft_tokenize(input, &tcount, *envp);
	if (!tok)
		return (NULL);
	ast = handle_parse_segments(tok, tcount, *envp);
	free_tokens(tok, tcount);
	if (ast)
		ast->envp = envp;
	return (ast);
}
