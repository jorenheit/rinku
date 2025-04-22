
#ifndef MUGEN_IMAGES_H
#define MUGEN_IMAGES_H

#ifdef __cplusplus
#include <cstddef>
#define IMAGE_VAR images
namespace Mugen {

  constexpr size_t IMAGE_SIZE = 2048;
  constexpr size_t N_IMAGES = 3;
#else
#define IMAGE_VAR mugen_images
#define IMAGE_SIZE 2048
#define N_IMAGES 3
#endif

  extern unsigned char const IMAGE_VAR[N_IMAGES][IMAGE_SIZE];

#ifdef __cplusplus
}
#endif
#endif // MUGEN_IMAGES_H
