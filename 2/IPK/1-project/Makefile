CC = gcc
CFLAGS = -Wall -Wextra -g

PREF_SRC = ./src/
PREF_OBJ = ./obj/

SRC = $(wildcard $(PREF_SRC)*.c)
OBJ = $(patsubst $(PREF_SRC)%.c, $(PREF_OBJ)%.o, $(SRC))
TARGET = ipk24_client

$(TARGET) : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

$(PREF_OBJ)%.o : $(PREF_SRC)%.c  
	$(CC) $(CFLAGS) -c $< -o $@ 


clean:
	rm $(TARGET) $(PREF_OBJ)*.o
