#ifndef INTEROP_H
#define INTEROP_H

// Lots of poor man's enums in this :()
#ifndef __cplusplus
#define constexpr const
#endif

///////////////////////////////////
// Constant IDs, poor man's enum //
///////////////////////////////////
constexpr int CID_VERTEXCOLOR = 0;
constexpr int LAST_SHARED_CID = 1000;

constexpr int SB_MATERIAL_CONSTANTS = 0;
constexpr int LAST_SHARED_BINDING = SB_MATERIAL_CONSTANTS;

constexpr int CID_TEXTURE_MODE_START = 2000;

constexpr int TEXTURE_MODE_INVALID = -1;
constexpr int TEXTURE_MODE_1D = 0;
constexpr int TEXTURE_MODE_2D = TEXTURE_MODE_1D + 1;
constexpr int TEXTURE_MODE_3D = TEXTURE_MODE_2D + 1;

#endif