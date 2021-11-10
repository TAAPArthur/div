LDLIBS += -lImlib2 -lxcb -lxcb-keysyms -lxcb-icccm -lX11-xcb -lX11
CFLAGS = -g

OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

all: div.a div



div.a: $(OBJS)
	ar rcs $@ $^

div: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm *.o
