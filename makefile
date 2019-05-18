CC = gcc
COPS = 
LIBTHREAD = -lpthread
RTLIB = -lrt

all: pCp.c consumer.c producer.c buffer.c extensions.c
	$(CC) $(COPS) -o pCp pCp.c consumer.c producer.c buffer.c extensions.c -lm $(RTLIB) $(LIBTHREAD)

debug: pCp.c consumer.c producer.c buffer.c extensions.c
	$(CC) $(COPS) -o pCp pCp.c consumer.c producer.c buffer.c extensions.c -lm $(RTLIB) $(LIBTHREAD) -DDEBUG

clean:
	rm -f *.o pCp
