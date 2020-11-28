CC = g++
USD = $(HOME)/dev/USD/build
CFLAGS  = -Wall -fPIC -g
LDFLAGS = -L$(USD)/lib -L$(HOME)/lib -Ltantoren
INFLAGS = -I$(USD)/include -I/usr/include/python3.7m -I$(HOME)/dev
LIBS    = -lusd -lhd -lsdf -lpxOsd -lvt -ltrace -lgf -ltf -lpython3.7m -lboost_python37 -lhdx -ltanto -ltantoren -lvulkan -lfreetype -lxcb -lxcb-keysyms

NAME = hdTanto

DEPS = \
	rendererPlugin.h \
	renderDelegate.h \
	mesh.h \
	renderBuffer.h \
	renderPass.h  \
	renderer.h

OBJS = \
	build/rendererPlugin.o \
	build/renderDelegate.o \
	build/mesh.o \
	build/renderPass.o \
	build/renderBuffer.o  \
	build/renderer.o

all: delegate 

renderer:
	cd tanto ; make ; cd ../tantoren ; make ; cd ..

delegate: renderer $(OBJS) 
	$(CC) $(LDFLAGS) -shared -Wl,--no-undefined -o $(NAME).so $(OBJS) $(LIBS) -lhf

test: testenv/testMyDelegate.cpp delegate
	$(CC) $(CFLAGS) $(INFLAGS) $(LDFLAGS) testenv/testMyDelegate.cpp -o testenv/test $(LIBS)

build/%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) $(INFLAGS) -c $< -o $@

clean:
	rm -f build/* ; rm hdTanto.so
