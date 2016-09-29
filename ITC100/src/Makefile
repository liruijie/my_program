CC = g++ -g
CFLAG = -o

all:main


main:main.cpp
	$(CC) *.cpp $(CFLAG) main    -I/usr/include/oracle/12.1/client64 -L/usr/lib64 -L/lib64 -L/usr/lib/oracle/12.1/client64/lib -lm -lnsl  -lclntsh -locci /usr/lib64/libstdc++.so.6

clean:
	rm main
