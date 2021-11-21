LDLIBS += -lxcb -lxcb-keysyms -lxcb-icccm -lxcb-image -lm
BIN=div
LIB=lib$(BIN).a

CUSTOM_IMAGE_LOADER ?= 0
DISABLE_DEFAULT_IMAGE_LOADER_1 = -DCUSTOM_IMAGE_LOADER
DEFAULT_IMAGE_LOADER_LIB_0 = -limgloader

CFLAGS += $(DISABLE_DEFAULT_IMAGE_LOADER_$(CUSTOM_IMAGE_LOADER))
LDLIBS += $(DEFAULT_IMAGE_LOADER_LIB_$(CUSTOM_IMAGE_LOADER))

OBJS = arg_parse.o defaults.o  div.o  functions.o  image_view.o  x.o
CFLAGS ?= -std=c99 -Wall -pedantic

all: $(LIB) $(BIN) $(BIN).sh

$(LIB): $(OBJS)
	ar rcs $@ $^

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ samples/no_op.c $(LDLIBS)

$(BIN).sh: $(BIN).sh.template
	sed "s/__$(BIN)_libs__/$(LDLIBS)/g" $^ > $@

install: $(BIN) $(LIB) $(BIN).sh
	install -Dt $(DESTDIR)/usr/libexec $(BIN)
	install -Dt $(DESTDIR)/usr/lib $(LIB)
	install -Dt $(DESTDIR)/usr/include/$(BIN) *.h
	install -D div.sh $(DESTDIR)/usr/bin/$(BIN)

uninstall:
	rm -f $(DESTDIR)/usr/libexec/$(BIN) $(DESTDIR)/usr/lib/$(LIB) $(DESTDIR)/usr/bin/$(BIN)

clean:
	rm -f *.o $(LIB) $(BIN) $(BIN).sh *_gen.h

arg_parse.c: arg_parse_gen.h

arg_parse_gen.h: div.h
	printf '// Generated file do not modify\n' > $@
	printf '#include <strings.h>\n\n#include "div.h"\n\n' >> $@
	printf "int getFromEnumValue(const char* str) {\n" >> $@
	sed -nE 's/^\s*((ALIGN|SCALE)_(([A-Z]*)?_?([A-Z]*))).*/    if(!strcasecmp("\3", str) || !strcasecmp(str, "\4") || !strcasecmp(str, "\5") || "\4"[0] == str[0] || "\5"[0] == str[0])return \1;/p' $^ >> $@
	printf "\treturn 0;\n}" >> $@
