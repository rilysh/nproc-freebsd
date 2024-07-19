CFLAGS = -Wall -Wextra -O2 -s
PROGRAM = nproc

all:
	$(CC) $(CFLAGS) -o $(PROGRAM) $(PROGRAM).c

clean:
	@rm -f $(PROGRAM)
