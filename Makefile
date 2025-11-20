TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) -f ./data/employee.db -n
	./$(TARGET) -f ./data/employee.db -a "Michael,Home,42"

default: $(TARGET)

.PHONY: clean
clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db
	rm -f data/*.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

obj/%.o: src/%.c
	gcc -c $< -o $@ -Iinclude