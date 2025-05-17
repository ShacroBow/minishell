/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_exit_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:03:33 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:21:56 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell_bonus.h"

static int	ft_parse_numeric(const char *str, long *num)
{
	const char	*ptr;
	int			digits;
	int			i;

	ptr = str;
	i = 0;
	while (ft_isspace((unsigned char)ptr[i]))
		i++;
	*num = ft_atol(ptr, &digits);
	if (digits == 0)
		return (1);
	if (ptr[i] == '+' || ptr[i] == '-')
		i++;
	while (ptr[i] == '0')
		i++;
	i += digits;
	while (ft_isspace((unsigned char)ptr[i]))
		i++;
	return (ptr[i] != '\0');
}

int	ft_exit(char **argv)
{
	int		argc;
	long	code;

	argc = 0;
	code = 0;
	while (argv[argc])
		argc++;
	if (argc > 2)
		return (ft_putstr_fd("minishell: exit: too many arguments\n", 2), 258);
	if (argc == 2)
	{
		if (ft_parse_numeric(argv[1], &code))
		{
			ft_putstr_fd("minishell: exit: numeric argument required\n", 2);
			ft_putstr_fd("exit\n", 1);
			return (258);
		}
		code &= 0xFF;
	}
	else
		code = g_exit_status & 0xFF;
	ft_putstr_fd("exit\n", 1);
	return ((int)code);
}
