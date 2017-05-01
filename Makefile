#
# Makefile to make a cgi applications on the 89 device server
#
OUTPATH=./

CC=gcc
#CFLAGS=-Wall -g -D TEST -D BOOST_NO_CXX11_SCOPED_ENUMS
CFLAGS=-Wall -g -D TEST
LIBS := -lpthread -lboost_system -lboost_thread -lboost_date_time -lboost_regex -lboost_serialization -lboost_filesystem

TRACEFLAGS	:= -std=gnu++11

GNOSTIC_SERIAL_DRIVER    = $(OUTPATH)gnostic_serial_driver
TRACE_OBJS   = $(OUTPATH)gnostic_serial_driver.o $(OUTPATH)Trace.o $(OUTPATH)GetOpt.o

HEADERS: Trace.hpp

SOURCES: Trace.cpp \
		 gnostic_serial_driver.cpp

all: $(GNOSTIC_SERIAL_DRIVER)

$(GNOSTIC_SERIAL_DRIVER): $(TRACE_OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(TRACE_OBJS) $(LDLIBS) $(LIBS)

.cpp.o:
	$(CXX) $(CFLAGS) $(TRACEFLAGS) $(CXXFLAGS) -c -o $@ $<

dirs:
	mkdir -p $(OUTPATH)
clean:
	rm -f $(OUTPATH)*.o $(TRACE_APP)

#install: all
#	install -m 0755 -d $(CGI_BIN)
#	install -m 0755 -d $(MCP_HOME)
#	install -m 0755 -D *.cgi $(CGI_BIN)
#	install -m 0755 -D *.sh $(CGI_BIN)

