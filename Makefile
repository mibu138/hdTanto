CC = g++
#USDLIB = $(HOME)/dev/USD/build/lib
#USDINC = $(HOME)/dev/USD/build/include
USDLIB = $(HFS)/dsolib 
USDINC = $(HFS)/toolkit/include
PYTHONLIB = $(HFS)/python/lib
CFLAGS  = -Wall -fPIC -g -std=c++14 -D_GLIBCXX_USE_CXX11_ABI=0
LDFLAGS = -L$(USDLIB) -L$(PYTHONLIB) -L$(HOME)/lib -Ltantoren
INFLAGS = -I$(USDINC) -I/usr/include/python3.7m -I$(HOME)/dev
USDLIBS  = -lusd -lhd -lsdf -lpxOsd -lvt -ltrace -lgf -ltf -lpython3.7m -lboost_python37 -lhdx -lhf
HUSDLIBS = -lpxr_tf -lpxr_usd -lpxr_hd -lpxr_sdf -lpxr_pxOsd -lpxr_vt -lpxr_trace -lpxr_gf -lpxr_hdx -lpxr_hf -lpython2.7 -lhboost_python27 
LIBS = -ltanto -ltantoren $(HUSDLIBS) -lvulkan -lfreetype -lxcb -lxcb-keysyms

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
	$(CC) $(LDFLAGS) -shared -Wl,--no-undefined -o $(NAME).so $(OBJS) $(LIBS) 

test: testenv/testMyDelegate.cpp delegate
	$(CC) $(CFLAGS) $(INFLAGS) $(LDFLAGS) testenv/testMyDelegate.cpp -o testenv/test $(LIBS)

build/%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) $(INFLAGS) -c $< -o $@

clean:
	rm -f build/* ; rm hdTanto.so
