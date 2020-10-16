DISTTYPE = Release

ARCH = 64

CXXFLAGS =	-std=c++11 -ggdb -Wall -fmessage-length=0 -mms-bitfields -I/usr/include -isystem /usr/include/boost-1_55 

SRCDIR  =   automationd

OBJDIR = 	obj

SRC =		$(SRCDIR)/automationd.cpp $(SRCDIR)/tcpsession.cpp $(SRCDIR)/serialsession.cpp $(SRCDIR)/insteon.cpp $(SRCDIR)/insteonhub.cpp \
			$(SRCDIR)/insteonplm.cpp $(SRCDIR)/tcpconnection.cpp $(SRCDIR)/engine.cpp $(SRCDIR)/hue.cpp $(SRCDIR)/sun.cpp

OBJS =		$(subst $(SRCDIR)/,$(OBJDIR)/,$(subst .cpp,.o,$(SRC)))

LIBS =		-L/usr/lib -lcurl -lpthread

TARGET =	bin/automationd

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) -c ${CXXFLAGS} $< -o $@ $(LIBS)

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(DISTTYPE)/$(ARCH)/$(OBJS) $(TARGET)

