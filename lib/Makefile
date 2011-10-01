CC	:= g++
FLAGS	:= -fPIC -DWITH_OPENSSL -DWITH_COOKIES

target	:= libSoapVim.so
objects	:= soapC.o soapClient.o stdsoap2.o

all : $(target)

$(target) : $(objects)
	$(CC) -o $@ $(objects) $(FLAGS) -shared

%.o : %.cpp
	$(CC) -o $@ -c $< $(FLAGS)

.PHONY : clean

clean :
	rm $(target) $(objects)