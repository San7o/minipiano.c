//////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
//
// minipiano.c
// ===========
//
// Play the piano using signwaves and your keyboard!
//
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// License: MIT
//
// Keys
// ----
//
// Notes follow a piano-like layout on your keyboard:
//
//         w e  t y u
//       a s d f g h j k
//
// Additional keys:
//
// z: raise starting frequency by one half tone
// x: decrease starting frequency by one half tone
// q: quit
//

#include <stdio.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>

#include <stdio.h>
#include <math.h>

#include "miniaudio.h"

#ifndef MA_PI
#define MA_PI      3.14159265358979323846264f
#endif

#define WINDOW_NAME   "minipiano"
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 500
#define WINDOW_FLAGS  0

static double c_frequency = 440.0;
static double phase = 0.0;
double frequency;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
  (void) pDevice;
  (void) pInput;
  // In playback mode copy data to pOutput. In capture mode read data from pInput. In full-duplex mode, both
  // pOutput and pInput will be valid and you can move data from pInput into pOutput. Never process more than
  // frameCount frames.

  double sample_rate = pDevice->sampleRate;
  float* output = (float*)pOutput;
  for (unsigned int i = 0; i < frameCount; ++i)
  {
    output[i] = 0.2f * sin(phase * 2 * MA_PI);
    phase += frequency / sample_rate;
    if (phase >= 1.0) phase -= 1.0;
  }
}

int main(void)
{
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;
  
  bool ret = SDL_CreateWindowAndRenderer(WINDOW_NAME, WINDOW_WIDTH,
                                         WINDOW_HEIGHT, WINDOW_FLAGS,
                                         &window, &renderer);
  if (!ret)
  {
    fprintf(stderr, "Error Creating SDL Window: %s\n", SDL_GetError());
    return 1;
  }

  ma_device_config config = ma_device_config_init(ma_device_type_playback);
  config.playback.format   = ma_format_f32;   // [-1, 1]. Set to ma_format_unknown to use the device's native format.
  config.playback.channels = 1;               // Set to 0 to use the device's native channel count.
  config.sampleRate        = 44100;           // Set to 0 to use the device's native sample rate.
  config.dataCallback      = data_callback;   // This function will be called when miniaudio needs more data.
  config.pUserData         = NULL;   // Can be accessed from the device object (device.pUserData).

  ma_device device;
  if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
    return -1;  // Failed to initialize the device.
  }

  frequency = c_frequency;

  ma_device_start(&device);     // The device is sleeping by default so you'll need to start it manually.

  while(1)
  {  
    if (SDL_PollEvent(&event))
    {
      if (SDL_EVENT_KEY_DOWN == event.type)
      {
        switch(event.key.key)
        {
        case 'q':
          goto cleanup;
        case 'z':
          c_frequency *= pow(2, 1 / 12.0);
          frequency = c_frequency;
          break;
        case 'x':
          c_frequency /= pow(2, 1 / 12.0);
          frequency = c_frequency;
          break;
          // Piano keys
        case 'a': // C
          frequency = c_frequency;
          break;
        case 'w': // C#
          frequency = c_frequency * pow(2, 1.0 / 12);
          break;
        case 's': // D
          frequency = c_frequency * pow(2, 2.0 / 12);
          break;
        case 'e': // D#
          frequency = c_frequency * pow(2, 3.0 / 12);
          break; 
        case 'd': // E
          frequency = c_frequency * pow(2, 4.0 / 12);
          break;
        case 'f': // F
          frequency = c_frequency * pow(2, 5.0 / 12);
          break;
        case 't': // F#
          frequency = c_frequency * pow(2, 6.0 / 12);
          break;
        case 'g': // G
          frequency = c_frequency * pow(2, 7.0 / 12);
          break;
        case 'y': // G#
          frequency = c_frequency * pow(2, 8.0 / 12);
          break;
        case 'h': // A
          frequency = c_frequency * pow(2, 9.0 / 12);
          break;
        case 'u': // A#
          frequency = c_frequency * pow(2, 10.0 / 12);
          break;
        case 'j': // B
          frequency = c_frequency * pow(2, 11.0 / 12);
          break;
        case 'k': // C
          frequency = c_frequency * pow(2, 12.0 / 12);
          break;
        default:
          break;
        }

        printf("Frequency: %f\n", frequency);
      }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    if (!SDL_RenderClear(renderer))
    {
      fprintf(stderr, "Error Clearing SDL Window: %s\n", SDL_GetError());    
      goto cleanup;
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Render frame...
    char frequency_str[100] = {0};
    sprintf(frequency_str, "%f Hz", frequency);
    
    SDL_SetRenderScale(renderer, 4.0f, 4.0f);
    SDL_RenderDebugText(renderer, 55, 55, frequency_str);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    
    if (!SDL_RenderPresent(renderer))
    {
      fprintf(stderr, "Error Rendering SDL Window: %s\n", SDL_GetError());    
      goto cleanup;
    }
  }

 cleanup:
  ma_device_uninit(&device);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
