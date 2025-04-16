#include "minishell.h"

int g_signal = 0;

static void	ft_handler(int signal)
{
	if (signal == SIGINT )
	{
		ft_putchar_fd('\n', STDOUT_FILENO);
		rl_replace_line("", -1);
		rl_on_new_line();
		rl_redisplay();
		g_signal = signal;
	}
	else if (signal == SIGSEGV && g_signal != SIGSEGV)
	{
		g_signal = signal;
		rl_clear_history();
		rl_on_new_line();
		exit (0);
	}
}

void	ft_signalhandle(void)
{
	struct sigaction s_signals;

	sigemptyset(&(s_signals.sa_mask));
	s_signals.sa_flags = SA_RESTART;
	s_signals.sa_handler = ft_handler;
	sigaction(SIGINT, &s_signals, NULL);
	sigaction(SIGSEGV, &s_signals, NULL);
	signal(SIGQUIT, SIG_IGN);
}