CC  = gcc
SRC = main.c cache.c
OBJ = main.o cache.o
TARGET = cs311cache

default: cs311cache

.c.o: $(CC) -c -std=c99 $(SRC)
 
$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)
 
clean:
	rm -rf *~ cs311cache

test: cs311cache test_simple test_milc test_gcc

test_simple:
	@echo "Testing simple"; \
        ./cs311cache -c 1024:8:8 -x sample_input/simple | diff -Naur sample_output/simple - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi


test_milc:
	@echo "Testing milc"; \
        ./cs311cache -c 1024:8:8 -x sample_input/milc | diff -Naur sample_output/milc - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
	

test_gcc:
	@echo "Testing gcc"; \
        ./cs311cache -c 1024:8:8 -x sample_input/gcc | diff -Naur sample_output/gcc - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
	

