CXX = g++
LD = g++
CXXFLAGS = -c -g -std=c++11 -Wall -Wextra -pedantic
LDFLAGS = -std=c++11 -pthreads

LIBFILES = types.h parameters.h
SENDER_OBJFILES = sender_main.o tcp_sender.o send_buffer.o
RECEIVER_OBJFILES = receiver_main.o tcp_receiver.o receive_buffer.o

all: reliable_sender reliable_receiver

reliable_sender: $(SENDER_OBJFILES) $(LIBFILES)
	$(LD) $(SENDER_OBJFILES) $(LDFLAGS) -o reliable_sender

reliable_receiver: $(RECEIVER_OBJFILES) $(LIBFILES)
	$(LD) $(RECEIVER_OBJFILES) $(LDFLAGS) -o reliable_receiver

sender_main.o: sender_main.cpp $(LIBFILES)
	$(CXX) $(CXXFLAGS) sender_main.cpp

receiver_main.o: receiver_main.cpp $(LIBFILES)
	$(CXX) $(CXXFLAGS) receiver_main.cpp

tcp_sender.o: tcp_sender.cpp tcp_sender.h $(LIBFILES)
	$(CXX) $(CXXFLAGS) tcp_sender.cpp

tcp_receiver.o: tcp_receiver.cpp tcp_receiver.h $(LIBFILES)
	$(CXX) $(CXXFLAGS) tcp_receiver.cpp

send_buffer.o: send_buffer.cpp send_buffer.h $(LIBFILES)
	$(CXX) $(CXXFLAGS) send_buffer.cpp

receive_buffer.o: receive_buffer.cpp receive_buffer.h $(LIBFILES)
	$(CXX) $(CXXFLAGS) receive_buffer.cpp

clean:
	rm -f reliable_sender reliable_receiver *.o
