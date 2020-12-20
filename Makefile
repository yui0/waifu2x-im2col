# ©2020 YUICHIRO NAKADA

PROGRAM = waifu2x_im2col

CC	= clang
CPP	= clang++
#CFLAGS  = -Ofast -march=native -funroll-loops -mf16c -DDEBUG
CFLAGS  = -Ofast -march=native -funroll-loops -mf16c
CPPFLAGS= $(CFLAGS)
LDFLAGS	= -lm
CSRC	= $(wildcard *.c)
CPPSRC	= $(wildcard *.cpp)
DEPS	= $(wildcard *.h) Makefile
OBJS	= $(patsubst %.c,%.o,$(CSRC)) $(patsubst %.cpp,%.o,$(CPPSRC))

#USE_GLES:= 1
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
#CFLAGS  += `pkg-config --cflags OpenCL`
#LDFLAGS	+= `pkg-config --libs OpenCL`
CFLAGS  += `pkg-config --cflags gl egl gbm`
LDFLAGS	+= `pkg-config --libs gl egl gbm` -lglfw -lm
endif
ifeq ($(UNAME_S),Darwin)
	LDFLAGS	+= -framework opencl
endif

%.o: %.cpp $(DEPS)
	$(CPP) -c -o $@ $< $(CPPFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROGRAM): $(OBJS)
	$(CPP) -o $@ $^ -s $(LDFLAGS)

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS) *.o *.s
