
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include "SDL.h"
#include "SDL_ttf.h"
using namespace std;

#include "logic.h"


#define INITIAL_SPEED 100
#define FPS 50


int main(int argc, char *argv[]) {
  if (argc != 2 || argv[1][0] == '-') {
    fprintf(stderr, "usage: %s <zaznamovy-adresar>\n", argv[0]);
    return 0;
  }

  nacitajAdresar(argv[1]);

  srand(time(NULL) * getpid());

  SDL_Init(SDL_INIT_VIDEO);
  atexit(SDL_Quit);

  TTF_Init();
  atexit(TTF_Quit);

  nacitajMedia();

  int w, h;
  zistiVelkostObrazovky(&w, &h);
  const char *fs = getenv("FULLSCREEN");
  SDL_Surface *screen = SDL_SetVideoMode(w, h, 32, SDL_SWSURFACE | (fs && *fs ? SDL_FULLSCREEN : 0));

  string title = string("Observer - ") + argv[1];
  SDL_WM_SetCaption(title.c_str(), title.c_str());

  int programBegin = SDL_GetTicks();
  bool paused = false;
  double pause = 0;
  double speed = INITIAL_SPEED;

  while (true) {
    int frameBegin = SDL_GetTicks();

    double now = (paused ? pause : (frameBegin - programBegin) / speed);
    if (now < 0) {
      pause = now = 0;
      paused = true;
      if (speed < 0) speed = -speed;
    }
    vykresluj(screen, now);
    SDL_Flip(screen);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) return 1;
      if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
        int sym = event.key.keysym.sym;
        int mod = (event.key.keysym.mod & (KMOD_NUM - 1));
	//if(event.type == SDL_KEYDOWN && sym== SDLK_d) speed+=5;
	//if(event.type == SDL_KEYDOWN&& sym== SDLK_f) speed-=5;
        if (event.type == SDL_KEYUP && sym == SDLK_s && (mod == KMOD_LALT || mod == KMOD_RALT)) return 0;
        if (event.type == SDL_KEYUP && sym == SDLK_q && (mod == KMOD_LALT || mod == KMOD_RALT)) return 1;
        if (event.type == SDL_KEYDOWN && (sym == SDLK_p || sym == SDLK_SPACE) && mod == 0) {
          paused = !paused;
          if (paused) {
            pause = now;
          } else {
            programBegin = frameBegin - (int)(pause * speed);
          }
        }
        if (event.type == SDL_KEYDOWN && sym == SDLK_PERIOD && mod == 0) {
          if (paused) {
            pause = 1 + (int)pause;
          } else {
            pause = now;
            paused = true;
          }
        }
        double speedmod = 0;
        switch (sym) {
          case SDLK_MINUS: speedmod = 1.25; break;
          case SDLK_EQUALS: speedmod = 0.8; break;
          case SDLK_r: speedmod = -1; break;
          case SDLK_s: speedmod = 1.5; break;   
          case SDLK_f: speedmod = 0.75; break; 
      //speed *= 0.1;
        }
        if (event.type == SDL_KEYUP && speedmod) {
          speed *= speedmod;
          if (!paused) programBegin = frameBegin - (int)(now * speed);
        }
      }

    }

    Uint32 duration = SDL_GetTicks() - frameBegin;
    if (duration < 1000/FPS) SDL_Delay(1000/FPS - duration);
  }
}
