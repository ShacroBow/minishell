#include "../minishell.h"

extern volatile sig_atomic_t	g_exit_status;

static void	ft_handler_heredoc_sigint(int sig)
{
	if (sig == SIGINT)
	{
		g_exit_status = 130;
		ioctl(STDIN_FILENO, TIOCSTI, "\n");
		rl_replace_line("", 0);
		rl_on_new_line();
	}
}

void	ft_signals_heredoc_setup(void)
{
	struct sigaction	sa;

	ft_signals_print_handler(1);
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = ft_handler_heredoc_sigint;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
}
