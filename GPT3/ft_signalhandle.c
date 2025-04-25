#include "minishell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>

/* Global exit status (updated by signals and commands) */
volatile sig_atomic_t g_exit_status = 0;

/* SIGINT handler for prompt mode (Ctrl-C) */
static void sigint_prompt(int sig) {
	(void)sig;
	write(STDOUT_FILENO, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
	g_exit_status = 130;
}

/* SIGINT handler for heredoc mode (Ctrl-C) */
static void sigint_heredoc(int sig) {
	(void)sig;
	g_exit_status = 130;
	ioctl(STDIN_FILENO, TIOCSTI, "\n");  /* simulate Enter key to break readline */
	rl_replace_line("", 0);
	rl_on_new_line();
}

/* Setup signal handlers for interactive prompt */
void setup_signals(void) {
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = sigint_prompt;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signals_print_handler(0);
}

/* Setup signal handlers for heredoc reading */
void setup_heredoc_signals(void) {
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = sigint_heredoc;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
}

/* Restore default signal actions (for child processes) */
void restore_default_signals(void) {
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}

/* Control visibility of control characters (like ^C) */
void signals_print_handler(int print) {
	struct termios tc;
	tcgetattr(STDIN_FILENO, &tc);
	if (print)
		tc.c_lflag |= ECHOCTL;
	else
		tc.c_lflag &= ~ECHOCTL;
	tcsetattr(STDIN_FILENO, TCSANOW, &tc);
}
