CFLAGS := -std=c99 -lpthread -lrt
OBJ := $(subst .c,.o,$(wildcard src/*.c))
INC_DIR := inc/

app.exe:  main.o $(OBJ)
	gcc $^  $(CFLAGS)
	rm *.o src/*.o

%.o: %.c
	gcc -c -I $(INC_DIR) $^ $(CFLAGS)

src/%.o: src/%.c
	gcc -o $@ -c -I $(INC_DIR) $^ $(CFLAGS)

clean:
	rm -f *.o *.exe src/*.o

run:
	./a.out