
SERVERDIR = ../server
CXXFLAGS = -O2 -MMD -MP -Wall -I$(SERVERDIR)

SDL_CONFIG = sdl-config
CXXFLAGS += $(shell $(SDL_CONFIG) --cflags)
LDLIBS += $(shell $(SDL_CONFIG) --libs) -lSDL2_image -lX11

CLEAN = *.d *.o observer

all: observer

observer: main.o logic.o server_marshal.o server_util.o
	$(LINK.cpp) $^ $(LDLIBS) -o $@

server_%.o: $(SERVERDIR)/%.cpp
	$(COMPILE.cpp) -o $@ $<

clean:
	-rm -f $(CLEAN)

-include *.d
