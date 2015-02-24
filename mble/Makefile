SOURCES=$(wildcard *.c)

LDFLAGS = -lbluetooth

mble: ${SOURCES}
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
