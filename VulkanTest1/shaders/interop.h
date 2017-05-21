#ifndef INTEROP_H
#define INTEROP_H

// Lots of poor man's enums in this :()
#ifndef __cplusplus
#define constexpr const
#endif

constexpr int TEXTURE_MODE_INVALID = -1;
constexpr int TEXTURE_MODE_1D = 0;
constexpr int TEXTURE_MODE_2D = TEXTURE_MODE_1D + 1;
constexpr int TEXTURE_MODE_3D = TEXTURE_MODE_2D + 1;

#endif