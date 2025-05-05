#include "minishell.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>   // perror
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

extern volatile sig_atomic_t    g_exit_status;

static char **env_list;  /* current environment for expansions */

/* ───────── forward declarations ───────── */
static t_token   *tokenize(const char *input, int *out_count);
static t_segment *parse_segments(t_token *tok, int *idx, int n, int in_sub);

/* free_tokens.c – release the token array */

void    free_tokens(t_token *tok, int count)
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
static void buf_append(char **buf, int *cap, int *len, char c) {
	if (*len + 1 >= *cap) {
		*cap *= 2;
		*buf = realloc(*buf, *cap);
		if (!*buf) { perror("minishell"); exit(EXIT_FAILURE); }
	}
	(*buf)[(*len)++] = c;
	(*buf)[*len] = '\0';
}

/* ───────── wildcard pattern matching (recursive) ───────── */
static int match_pattern(const char *pat, const char *text) {
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
static char **expand_wildcard(const char *pattern) {
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

/* ───────── tokenize input string into tokens ───────── */
static t_token *tokenize(const char *input, int *out_count) {
	int cap = 128, len = 0, count = 0;
	char *buf = malloc(cap);
	if (!buf) { perror("minishell"); exit(EXIT_FAILURE); }
	buf[0] = 0;
	t_token *tok = malloc(sizeof(t_token) * (strlen(input) + 1));
	if (!tok) { perror("minishell"); exit(EXIT_FAILURE); }
	int in_s = 0, in_d = 0, quoted = 0;
	for (int i = 0; input[i]; ) {
		char c = input[i];
		if (c == '\'' && !in_d) { in_s ^= 1; quoted = 1; i++; continue; }
		if (c == '\"' && !in_s) { in_d ^= 1; quoted = 1; i++; continue; }
		if (!in_s && !in_d) {
			/* whitespace separates tokens */
			if (c == ' ' || c == '\t') {
				if (len) {
					tok[count].type = TOK_WORD;
					tok[count].value = buf;
					tok[count].quoted = quoted;
					count++;
					cap = 128; len = 0;
					buf = malloc(cap);
					if (!buf) { perror("minishell"); exit(EXIT_FAILURE); }
					buf[0] = 0; quoted = 0;
				}
				i++;
				continue;
			}
			/* two-character operators: &&, ||, <<, >> */
			if ((c == '&' && input[i+1] == '&') ||
				(c == '|' && input[i+1] == '|') ||
				(c == '<' && input[i+1] == '<') ||
				(c == '>' && input[i+1] == '>')) {
				if (len) {
					tok[count].type = TOK_WORD;
					tok[count].value = buf;
					tok[count].quoted = quoted;
					count++;
					cap = 128; len = 0;
					buf = malloc(cap);
					if (!buf) { perror("minishell"); exit(EXIT_FAILURE); }
					buf[0] = 0; quoted = 0;
				}
				t_tokentype tt = TOK_AND;
				if (c == '|' && input[i+1] == '|') tt = TOK_OR;
				else if (c == '<' && input[i+1] == '<') tt = TOK_HEREDOC;
				else if (c == '>' && input[i+1] == '>') tt = TOK_APPEND;
				tok[count].type = tt;
				tok[count].value = ft_strndup(input + i, 2);
				tok[count].quoted = 0;
				count++;
				i += 2;
				continue;
			}
			/* single-character operators: |, <, >, (, ) */
			if (c == '|' || c == '<' || c == '>' || c == '(' || c == ')') {
				if (len) {
					tok[count].type = TOK_WORD;
					tok[count].value = buf;
					tok[count].quoted = quoted;
					count++;
					cap = 128; len = 0;
					buf = malloc(cap);
					if (!buf) { perror("minishell"); exit(EXIT_FAILURE); }
					buf[0] = 0; quoted = 0;
				}
				t_tokentype tt = TOK_PIPE;
				if (c == '<') tt = TOK_REDIR_IN;
				else if (c == '>') tt = TOK_REDIR_OUT;
				else if (c == '(') tt = TOK_LPAREN;
				else if (c == ')') tt = TOK_RPAREN;
				tok[count].type = tt;
				tok[count].value = ft_strndup(input + i, 1);
				tok[count].quoted = 0;
				count++;
				i++;
				continue;
			}
		}
		/* environment variable expansion (skip in single quotes) */
		if (c == '$' && !in_s) {
			if (input[i+1] == '?') {
				char tmp[16];
				int l = sprintf(tmp, "%d", g_exit_status);
				for (int k = 0; k < l; ++k)
					buf_append(&buf, &cap, &len, tmp[k]);
				i += 2;
				continue;
			}
			int j = i + 1;
			if (!ft_isalnum(input[j]) && input[j] != '_') {
				buf_append(&buf, &cap, &len, '$');
				i++;
				continue;
			}
			while (ft_isalnum(input[j]) || input[j] == '_')
				j++;
			char *name = ft_strndup(input + i + 1, j - (i + 1));
			size_t namelen = ft_strlen(name);
			char *val = NULL;
			for (int k = 0; env_list && env_list[k]; ++k) {
				if (!ft_strncmp(env_list[k], name, namelen) &&
					env_list[k][namelen] == '=') {
					val = env_list[k] + namelen + 1;
					break;
				}
			}
			if (!val)
				val = getenv(name);
			if (val) {
				for (int k = 0; val[k]; ++k)
					buf_append(&buf, &cap, &len, val[k]);
			}
			free(name);
			i = j;
			continue;
		}
		/* regular character (or within quotes) */
		buf_append(&buf, &cap, &len, c);
		if (in_s || in_d)
			quoted = 1;
		i++;
	}
	if (len) {
		tok[count].type = TOK_WORD;
		tok[count].value = buf;
		tok[count].quoted = quoted;
		count++;
	} else {
		free(buf);
	}
	*out_count = count;
	return tok;
}

/* ───────── error message for syntax errors ───────── */
static int redir_error(void) {
	write(2, "minishell: syntax error near unexpected token\n", 47);
	g_exit_status = 258;
	return -1;
}

/* ───────── add an argument to a command's argv ───────── */
static void add_arg(t_command *cmd, const char *val) {
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
static void new_command_if_needed(t_command **cmd_head, t_command **cmd_tail, int *need_cmd) {
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
static int handle_parenthesis(t_token *tok, int *idx, int n, t_command *cur) {
	(*idx)++;  /* skip '(' */
	cur->subshell = 1;
	cur->subshell_segments = parse_segments(tok, idx, n, 1);
	cur->argv = malloc(sizeof(char*) * 2);
	if (!cur->argv) { perror("minishell"); exit(EXIT_FAILURE); }
	cur->argv[0] = ft_strdup("SUBSHELL");
	cur->argv[1] = NULL;
	return 0;
}

/* ───────── handle a word token (arguments and wildcards) ───────── */
static int handle_word(t_token *tok, int *idx, t_command *cur) {
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
static int handle_redirection(t_token *tok, int *idx, int n, t_command *cur) {
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
			setup_heredoc_signals();
			/* read heredoc lines until limiter */
			while (1) {
				char *line = readline("> ");
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
							for (int k = 0; env_list && env_list[k]; ++k) {
								if (!ft_strncmp(env_list[k], name, namelen) &&
									env_list[k][namelen] == '=') {
									val = env_list[k] + namelen + 1;
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
			setup_signals();
			if (g_exit_status == 130) {
				/* aborted by Ctrl-C */
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
static void push_pipeline_to_segments(t_segment **h, t_segment **t,
									t_command *pipe, t_tokentype op) {
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
static int handle_bool_or_pipe(t_token *tok, int *idx,
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
static t_segment *parse_segments(t_token *tok, int *idx, int n, int in_sub) {
	t_segment *seg_head = NULL, *seg_tail = NULL;
	t_command *cmd_head = NULL, *cmd_tail = NULL;
	int need_cmd = 1;
	while (*idx < n) {
		t_tokentype typ = tok[*idx].type;
		if (typ == TOK_RPAREN) {  /* end of subshell */
			if (!in_sub)
				return (redir_error(), free_segments(seg_head), NULL);
			(*idx)++;
			break;
		}
		if (typ == TOK_PIPE || typ == TOK_AND || typ == TOK_OR) {
			if (handle_bool_or_pipe(tok, idx, &seg_head, &seg_tail,
									&cmd_head, &cmd_tail, &need_cmd) < 0)
				return (free_segments(seg_head), NULL);
			continue;
		}
		new_command_if_needed(&cmd_head, &cmd_tail, &need_cmd);
		t_command *cur = cmd_tail;
		/* dispatch token types */
		if (typ == TOK_LPAREN) {
			if (handle_parenthesis(tok, idx, n, cur) < 0)
				return (free_segments(seg_head), NULL);
		} else if (typ == TOK_WORD) {
			if (handle_word(tok, idx, cur) < 0)
				return (free_segments(seg_head), NULL);
		} else if (typ == TOK_REDIR_IN || typ == TOK_HEREDOC ||
				typ == TOK_REDIR_OUT || typ == TOK_APPEND) {
			if (handle_redirection(tok, idx, n, cur) < 0)
				return (free_segments(seg_head), NULL);
		} else {
			(*idx)++;
		}
	}
	if (cmd_head)
		push_pipeline_to_segments(&seg_head, &seg_tail, cmd_head, 0);
	return seg_head;
}

/* ───────── top-level parse function ───────── */
t_segment *parse_input(const char *input, char ***envp)
{
	int			quotes_s;
	int			quotes_d;
	int			tcount;
	int			idx;
	int			i;
	t_token		*tok;
	t_segment	*ast;

	env_list = *envp;  /* set current env for expansions */
	
	quotes_s = 0;
	quotes_d = 0;
	i = 0;
	tcount = 0;
	idx = 0;
	// lexer quote
	while (input[i])
	{
		if (input[i] == '\'')
			quotes_s ^= 1;
		else if (input[i] == '\"')
			quotes_d ^= 1;
		i++;
	}
	if (quotes_s || quotes_d)
	{
		ft_putstr_fd("minishell: syntax error unclosed quote\n", STDERR_FILENO);
		g_exit_status = 258;
		return NULL;
	}
	// lexer quote
	tok = tokenize(input, &tcount);
	if (!tok)
		return NULL;
	ast = parse_segments(tok, &idx, tcount, 0);
	if (idx < tcount && ast)
	{
		ft_putstr_fd("minishell: syntax error near unexpected token\n", STDERR_FILENO);
		g_exit_status = 258;
		free_segments(ast);
		ast = NULL;
	}
	free_tokens(tok, tcount);
	if (ast)
		ast->envp = envp;
	return (ast);
}
