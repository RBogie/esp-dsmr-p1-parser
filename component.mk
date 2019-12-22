COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_PRIV_INCLUDEDIRS := include
COMPONENT_OBJS := crc16.o p1-lib.o p1-parser.o
COMPONENT_EXTRA_CLEAN := p1-parser.rl parser-tools.rl p1-parser.c

p1-parser.o: p1-parser.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(COMPONENT_PATH)/include -c $< -o $@

p1-parser.c:
	cp $(COMPONENT_PATH)/p1-parser.rl .
	cp $(COMPONENT_PATH)/parser-tools.rl .
	ragel -s p1-parser.rl