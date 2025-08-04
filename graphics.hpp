//
// Created by Reaky on 2025/5/25.
//

#pragma once

#include <graphics.h>

using namespace std;

void TransparentImage(IMAGE *dstimg, int x, int y, IMAGE *srcimg);
void TransparentImage(IMAGE *dstimg, int x, int y, IMAGE *srcimg, int sa);
void AdjustBrightness(IMAGE *img, int offset);
void gradient(const IMAGE *lastScene, IMAGE *newScene);

DWORD RGBToYCbCr(DWORD rgb);
DWORD YCbCrToRGB(DWORD ycbcr);