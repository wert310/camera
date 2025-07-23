#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Camera *camera = NULL;
static SDL_Texture *texture = NULL;
static int height = 0, width = 0;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  goto start;
 usage:
  fprintf(stderr,
          "Usage: webcam [-l] [-c CAMERA] [-g GEOMETRY] [-h]              \n"
          "                                                               \n"
          "OPTIONS:                                                       \n"
          "  -l           List available cameras and exit                 \n"
          "  -c CAMERA    Use camera id CAMERA (default: first camera)    \n"
          "  -g GEOMETRY  Set window geometry (default: camera feed size) \n"
          "  -h           Display this help message                       \n"
          );
  return SDL_APP_FAILURE;
 start:
  bool list_cameras = false;
  int  camera_id = -1;

  char c;
  while ((c = getopt(argc, argv, "lc:g:h")) != -1) {
    switch (c) {
    case 'l': list_cameras = true; break;
    case 'c': camera_id = atoi(optarg); break;
    case 'g': sscanf(optarg, "%dx%d", &width, &height); break;
    case 'h':
    default: goto usage;
    }
  }
  argc -= optind;
  argv += optind;
  if (argc) goto usage;

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  int devcount = 0;
  SDL_CameraID *devices = SDL_GetCameras(&devcount);
  if (devices == NULL) {
    SDL_Log("Couldn't enumerate camera devices: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  } else if (devcount == 0) {
    SDL_Log("Couldn't find any camera devices! Please connect a camera and try again.");
    return SDL_APP_FAILURE;
  }
  
  if (list_cameras) {
    for (int i=0; i<devcount; ++i) {
      SDL_CameraID cid = devices[i];
      printf("%d. %s\n", cid, SDL_GetCameraName(cid));
    }

    return SDL_APP_SUCCESS;
  }

  int camera_index = 0;
  if (camera_id >=0) {
    camera_index = -1;
    for (int i=0; i < devcount; ++i) {
      if (devices[i] == camera_id) {
        camera_index = i;
        break;
      }
    }
    if (camera_index == -1) {
      SDL_Log("Invalid camera id: use -l to list available cameras");
      return SDL_APP_FAILURE;
    }
  }
  if (!SDL_CreateWindowAndRenderer("camera", 640, 480, 0, &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  camera = SDL_OpenCamera(devices[camera_index], NULL);
  SDL_free(devices);
  if (camera == NULL) {
    SDL_Log("Couldn't open camera: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  } else if (event->type == SDL_EVENT_CAMERA_DEVICE_DENIED) {
    return SDL_APP_FAILURE;
  }
  if (event->type == SDL_EVENT_KEY_DOWN) {
    if (event->key.key == SDLK_F) {
      SDL_SetWindowFullscreen(window, !(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN));
    }
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  Uint64 timestampNS = 0;
  SDL_Surface *frame = SDL_AcquireCameraFrame(camera, &timestampNS);
  if (frame != NULL) {
    if (!texture) {
      if (!width || !height) {
        width = frame->w;
        height = frame->h;
      }
      SDL_SetWindowSize(window, width, height);
      texture = SDL_CreateTexture(renderer,
                                  frame->format, SDL_TEXTUREACCESS_STREAMING,
                                  frame->w, frame->h);
    }
    if (texture) SDL_UpdateTexture(texture, NULL, frame->pixels, frame->pitch); 
    SDL_ReleaseCameraFrame(camera, frame);
  }
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
  if (texture) SDL_RenderTexture(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  SDL_CloseCamera(camera);
  SDL_DestroyTexture(texture);
}

