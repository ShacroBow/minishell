NAME = minishell
SRC =	main.c $(SRC_BUILTIN) $(SRC_EXECUTE) $(SRC_TOKEN) $(SRC_PARSER) $(SRC_SIGNAL) $(SRC_UTILITY)
SRC_BUILTIN =	builtins/ft_builtins.c builtins/ft_exit.c builtins/ft_unset.c \
				builtins/ft_export.c builtins/ft_cd.c builtins/ft_echo.c
SRC_EXECUTE =	execute/ft_execute.c execute/ft_find_binary.c execute/ft_run_pipeline.c \
				execute/ft_spawn_children.c execute/ft_make_pipes.c
SRC_TOKEN   =	token/ft_tokenize.c token/utils.c token/exit_utils.c \
				token/ft_handle_filler.c token/ft_handle_expansion.c \
				token/ft_handle_operator.c
SRC_PARSER  =	parse/parse_input.c parse/ft_parse_segments.c  parse/handle_redir.c \
				parse/write_expanded_line.c parse/handle_heredoc.c parse/ft_expandwildcard.c \
				parse/utils.c
SRC_SIGNAL  =	signals/ft_signal_setup.c signals/heredoc_signals.c
SRC_UTILITY =	utility/ft_dupenvp.c utility/ft_free_segments.c


NAME_BONUS = minishell_bonus
SRC_BONUS =	bonus/main_bonus.c																\
			bonus/builtins/ft_builtins_bonus.c		bonus/builtins/ft_exit_bonus.c			\
			bonus/builtins/ft_unset_bonus.c			bonus/builtins/ft_export_bonus.c		\
			bonus/builtins/ft_cd_bonus.c			bonus/builtins/ft_echo_bonus.c			\
			bonus/execute/ft_execute_bonus.c		bonus/execute/ft_find_binary_bonus.c	\
			bonus/execute/ft_run_pipeline_bonus.c	bonus/execute/ft_spawn_children_bonus.c	\
			bonus/execute/ft_make_pipes_bonus.c												\
			bonus/token/ft_tokenize_bonus.c			bonus/token/utils_bonus.c				\
			bonus/token/exit_utils_bonus.c			bonus/token/ft_handle_filler_bonus.c	\
			bonus/token/ft_handle_expansion_bonus.c	bonus/token/ft_handle_operator_bonus.c	\
			bonus/parse/parse_input_bonus.c			bonus/parse/ft_parse_segments_bonus.c	\
			bonus/parse/handle_redir_bonus.c		bonus/parse/write_expanded_line_bonus.c	\
			bonus/parse/handle_heredoc_bonus.c		bonus/parse/ft_expandwildcard_bonus.c	\
			bonus/parse/utils_bonus.c														\
			bonus/signals/ft_signal_setup_bonus.c	bonus/signals/heredoc_signals_bonus.c	\
			bonus/utility/ft_dupenvp_bonus.c		bonus/utility/ft_free_segments_bonus.c


OBJ_DIR = ./dir/
OBJ = $(SRC:%.c=$(OBJ_DIR)%.o)
OBJ_BONUS = $(SRC_BONUS:%.c=$(OBJ_DIR)%.o)

LIBFT_PATH = ./libft/
LIBFT = libft.a
CC = cc
CFLAGS = -Wall -Wextra -Werror -g #-fsanitize=address,undefined
LIBFLAGS = -L. $(LIBFT_PATH)$(LIBFT) -lreadline


all: $(NAME)

bonus: $(NAME_BONUS)

$(NAME): $(OBJ_DIR) $(OBJ) $(LIBFT_PATH)$(LIBFT)
	$(CC) $(OBJ) $(CFLAGS) -o $(NAME) $(LIBFLAGS)

$(NAME_BONUS): $(OBJ_DIR) $(OBJ_BONUS) $(LIBFT_PATH)$(LIBFT)
	$(CC) $(OBJ_BONUS) $(CFLAGS) -o $@ $(LIBFLAGS)

$(OBJ_DIR)%.o : %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

$(LIBFT_PATH)$(LIBFT):
	make -C $(LIBFT_PATH) all -j8

clean:
	rm -rf $(OBJ)
	rm -rf $(OBJ_BONUS)
	make -C $(LIBFT_PATH) clean

fclean: clean
	rm -rf $(NAME) $(NAME_BONUS) a.out $(OBJ_DIR) files.supp
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
