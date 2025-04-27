TARGET = proj2


$(TARGET): proj2.o
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -g -o $@ $< -lpthread

proj2.o: proj2.c
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -g -c -o $@ $< 	

clean:
	rm -f $(TARGET) *.o
