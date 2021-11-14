LDLIBS += -lxcb -lxcb-keysyms -lxcb-icccm -lxcb-image -limgloader
BIN=div

OBJS = defaults.o  div.o  functions.o  image_view.o  x.o

all: $(BIN).a $(BIN) $(BIN).sh

$(BIN).a: $(OBJS)
	ar rcs $@ $^

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ samples/no_op.c $(LDLIBS)

$(BIN).sh: $(BIN).sh.template
	sed "s/__$(BIN)_libs__/$(LDLIBS)/g" $^ > $@

install: $(BIN) $(BIN).a $(BIN).sh
	install -Dt $(DESTDIR)/usr/libexec $(BIN)
	install -Dt $(DESTDIR)/usr/lib $(BIN).a
	install -D div.sh $(DESTDIR)/usr/bin/$(BIN)

uninstall:
	rm -f $(DESTDIR)/usr/libexec/$(BIN) $(DESTDIR)/usr/lib/$(BIN).a $(DESTDIR)/usr/bin/$(BIN)

clean:
	rm -f *.o $(BIN).a $(BIN) $(BIN).sh libimgloader/imgloader.o
