# Freeze Detector Makefile

EXECUTABLE = logo-finder
OBJS = \
    main.o \
    logo_finder.o \
    logo_finder_factory.o

HEADER = \
    logo_finder.h \
    logo_finder_factory.h

INCLUDEPATH = . /usr/local/include/libvideoprocessor
LIBPATH = /usr/local/lib
STATICLIBS = libvideoprocessor.a
LIBS = -lavcodec -lavformat -lavutil -lswscale -lswresample -lpthread -lboost_regex -lpng16

CPP = g++
CPPFLAGS = -Wall -std=c++11 -g -I ./ -I /usr/local/include/
# End of configuration options

#What needs to be built to make all files and dependencies
all: $(EXECUTABLE)

#Create the main executable
$(EXECUTABLE): $(OBJS)
	$(CPP) $(CPPFLAGS) -o $(EXECUTABLE) $(OBJS) $(LIBPATH)/$(STATICLIBS) $(LIBS)
	
#$(CPP) $(CPPFLAGS) $(OBJS) -o $(EXECUTABLE) -L $(LIBPATH) $(STATICLIBS) $(LIBS)

#Recursively build object files
%.o: %.cpp %.h
	$(CPP) $(CPPFLAGS) -c -o $@ $<
	#Recursively build object files

clean:
	-rm -f $(EXECUTABLE) *.o

run: $(EXECUTABLE)
	./$(EXECUTABLE)

tarball:
	-rm -f $(EXECUTABLE) *.o
	(cd .. ; tar czf Your_Name_a1.tar.z shell )

# End of Makefile
