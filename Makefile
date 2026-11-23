# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/12/27 13:14:37 by elel-yak          #+#    #+#              #
#    Updated: 2023/12/27 16:12:53 by elel-yak         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = rt

CC = cc
CFLAGS = -Wall -Wextra -Werror -std=gnu11 -DCL_TARGET_OPENCL_VERSION=120
CFLAGS += -Ofast
# CFLAGS += -fsanitize=address -fsanitize=leak -g3 
LDFLAGS = -lmlx -lm -lXext -lX11 -lz -pthread -lOpenCL
ifeq ($(RT_NIX_OPENCL),1)
CFLAGS += -I/usr/local/include
LDFLAGS += -L/usr/local/lib -lbsd
else
LDFLAGS += -L./third_party/OpenCL/lib -Wl,-rpath,./third_party/OpenCL/lib
CFLAGS += -I./third_party/OpenCL
endif

SRCS = 	./parsing/miniRT.c\
	  	./parsing/check_elements.c\
	  	./parsing/rt_utils.c\
	  	./parsing/rt_utils1.c\
	  	./parsing/get_next_line/get_next_line.c\
	  	./parsing/get_next_line/get_next_line_utils.c\
		./parsing/check_objects.c\
		./parsing/check_cone.c\
		./parsing/myatoi.c\
		vector.c \
		vector1.c \
		vector2.c \
		camera.c \
		Image/image.c \
		Image/image_utils.c \
		Transformation/init_matrix.c \
		Transformation/transfor_matrix.c\
		Matrix/init.c \
		Matrix/matrix.c \
		Matrix/inverse.c \
		Matrix/minor.c \
		Matrix/Multiplication.c \
		Objects/Sphere.c \
		Objects/Plane.c \
		raytrace_utils.c \
		raytrace.c \
		Material/material.c \
		Material/Diffuse.c \
		Material/specular.c \
		Objects/Cylinder.c \
 		Objects/Cylinder_utils.c \
		Objects/Cone.c \
		Objects/Cone_utils.c \
		Parallel/threads.c \
		gpu/cl_init.c \
		gpu/cl_setup.c \
		gpu/cl_scene.c \
		gpu/cl_render.c \
		cleanup.c \
		main.c

OBJS = $(SRCS:.c=.o)

all : $(NAME)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME) : $(OBJS)
	make -C ./libft -j4
	$(CC) $(CFLAGS) $(OBJS) ./libft/libft.a $(LDFLAGS) -o $(NAME)

test_cone:
	$(CC) $(CFLAGS) -o tests/test_cone_quad tests/test_cone_quad.c -lm
	./tests/test_cone_quad

clean :
	make clean -C ./libft
	rm -rf $(OBJS)
	rm -f tests/test_cone_quad

fclean : clean
	make fclean -C ./libft
	rm -rf $(NAME)

re : fclean all

.PHONY : all clean fclean re test_cone
