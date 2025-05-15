NAME = minishell
SRC =	main.c  $(SRC_BUILTIN) $(SRC_EXECUTE) $(SRC_LEXER) $(SRC_SIGNAL)
SRC_BUILTIN =	builtins/ft_builtins.c builtins/ft_exit.c builtins/ft_unset.c \
				builtins/ft_export.c builtins/ft_cd.c builtins/ft_echo.c
SRC_EXECUTE =	execute/ft_execute.c execute/ft_find_binary.c execute/ft_run_pipeline.c \
				execute/ft_spawn_children.c execute/ft_make_pipes.c 
SRC_LEXER   =	lexer/ft_lexer.c lexer/ft_lexer_utils.c lexer/ft_lexer_quotes.c \
				lexer/ft_lexer_operators.c lexer/ft_lexer_expand_var.c \
				lexer/ft_lexer_free_error.c
SRC_PARSER  =	parse/parse_input.c
SRC_SIGNAL  =	signals/ft_signal_setup.c signals/heredoc_signals.c

OBJ_DIR = ./dir/
OBJ = $(SRC:%.c=$(OBJ_DIR)%.o)
LIBFT_PATH = ./libft/
LIBFT = libft.a
CC = cc
CFLAGS = -Wall -Wextra -Werror -g #-fsanitize=address,undefined
LIBFLAGS = -L. $(LIBFT_PATH)$(LIBFT) -lreadline


all: $(NAME)

#bonus: all

$(NAME): $(OBJ_DIR) $(OBJ) $(LIBFT_PATH)$(LIBFT)
	$(CC) $(OBJ) $(CFLAGS) -o $(NAME) $(LIBFLAGS)

$(OBJ_DIR)%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

$(LIBFT_PATH)$(LIBFT):
	make -C $(LIBFT_PATH) all -j8

clean:
	rm -rf $(OBJ)
	make -C $(LIBFT_PATH) clean

fclean: clean
	rm -rf $(NAME) a.out $(OBJ_DIR) files.supp
	make -C $(LIBFT_PATH) fclean

re: fclean all

supp:
	@printf '%s\n' '{' \
	    '    ignore_libreadline_errors' \
	    '    Memcheck:Leak' \
	    '    ...' \
	    '    obj:*/libreadline.so.*' \
	    '}' > files.supp
	@echo "jobs done"
	
.PHONY: all clean fclean re compile supp #bonus
