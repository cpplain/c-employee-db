TARGET = bin/dbcli
SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(SOURCES))

run: clean default
	./$(TARGET)
	./$(TARGET) -n -f employee.db
	./$(TARGET) -f employee.db
	./$(TARGET) -a -f employee.db

clean:
	rm -f bin/*
	rm -f obj/*.o
	rm -f *.db

default: $(TARGET)

$(TARGET): $(OBJECTS)
	gcc -o $@ $?

obj/%.o : src/%.c
	gcc -c -Iinclude -o $@ $< 
