CC = g++ -g
CFLAG = -o

all:main


main:main.cpp
	$(CC) *.cpp $(CFLAG) main    -I/usr/include/oracle/10.2.0.4/client/  -L/usr/lib/oracle/10.2.0.4/client/lib -m32 -lm -lnsl  -lclntsh -locci /usr/lib/libstdc++.so.5

clean:
	rm main
