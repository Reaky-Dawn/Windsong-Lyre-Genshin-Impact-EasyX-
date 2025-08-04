//
// Created by Reaky on 2025/5/25.
//

#include <graphics.h>
#include <algorithm>

#include "graphics.hpp"

using namespace std;

/**
 * @brief 半透明贴图函数，在原作者的基础上，对阿尔法混合计算的优化：使用位运算代替除法，使用 RGB 等宏优化代码可读性
 *
 * @param dstimg 目标 IMAGE（NULL 表示默认窗体）
 * @param x	目标贴图位置 x
 * @param y	目标贴图位置 y
 * @param srcimg 源 IMAGE 对象指针
 * @author CodeBus 灵音(improve-transparent-putimage)
 */

void TransparentImage(IMAGE *dstimg, int x, int y, IMAGE *srcimg)
{
	// 变量初始化
	DWORD *dst = GetImageBuffer(dstimg);
	DWORD *src = GetImageBuffer(srcimg);
	int src_width = srcimg->getwidth();
	int src_height = srcimg->getheight();
	int dst_width = (dstimg == nullptr ? getwidth() : dstimg->getwidth());
	int dst_height = (dstimg == nullptr ? getheight() : dstimg->getheight());

	// 计算贴图的实际长宽
	int iwidth = (x + src_width > dst_width) ? dst_width - x : src_width;		// 处理超出右边界
	int iheight = (y + src_height > dst_height) ? dst_height - y : src_height;	// 处理超出下边界
	if (x < 0) { src += -x;				iwidth -= -x;	x = 0; }				// 处理超出左边界
	if (y < 0) { src += src_width * -y;	iheight -= -y;	y = 0; }				// 处理超出上边界

	// 修正贴图起始位置
	dst += dst_width * y + x;

	// 实现透明贴图
	for (int iy = 0; iy < iheight; ++iy)
	{
		for (int i = 0; i < iwidth; ++i)
		{
			DWORD sa = 255 - ((src[i] & 0xff000000) >> 24);//获取阿尔法值
			if (sa != 0)//假如是完全透明就不处理
				if (sa == 255)//假如完全不透明则直接拷贝
					dst[i] = src[i];
				else//真正需要阿尔法混合计算的图像边界才进行混合
					dst[i] = RGB(
						GetRValue(src[i]) * sa + GetRValue(dst[i]) * (256 - sa) >> 8,
						GetGValue(src[i]) * sa + GetGValue(dst[i]) * (256 - sa) >> 8,
						GetBValue(src[i]) * sa + GetBValue(dst[i]) * (256 - sa) >> 8
					);
		}
		dst += dst_width;
		src += src_width;
	}
}

void TransparentImage(IMAGE *dstimg, int x, int y, IMAGE *srcimg, int sa)
{
	// 变量初始化
	sa = 255 - sa;
	DWORD *dst = GetImageBuffer(dstimg);
	DWORD *src = GetImageBuffer(srcimg);
	int src_width = srcimg->getwidth();
	int src_height = srcimg->getheight();
	int dst_width = (dstimg == nullptr ? getwidth() : dstimg->getwidth());
	int dst_height = (dstimg == nullptr ? getheight() : dstimg->getheight());

	// 计算贴图的实际长宽
	int iwidth = (x + src_width > dst_width) ? dst_width - x : src_width;		// 处理超出右边界
	int iheight = (y + src_height > dst_height) ? dst_height - y : src_height;	// 处理超出下边界
	if (x < 0) { src += -x;				iwidth -= -x;	x = 0; }				// 处理超出左边界
	if (y < 0) { src += src_width * -y;	iheight -= -y;	y = 0; }				// 处理超出上边界

	// 修正贴图起始位置
	dst += dst_width * y + x;

	// 实现透明贴图
	for (int iy = 0; iy < iheight; ++iy)
	{
		for (int i = 0; i < iwidth; ++i)
		{
			if (sa != 0)//假如是完全透明就不处理
				if (sa == 255)//假如完全不透明则直接拷贝
					dst[i] = src[i];
				else//真正需要阿尔法混合计算的图像边界才进行混合
					dst[i] = RGB(
						GetRValue(src[i]) * sa + GetRValue(dst[i]) * (256 - sa) >> 8,
						GetGValue(src[i]) * sa + GetGValue(dst[i]) * (256 - sa) >> 8,
						GetBValue(src[i]) * sa + GetBValue(dst[i]) * (256 - sa) >> 8
					);
		}
		dst += dst_width;
		src += src_width;
	}
}

/**
 * @brief 调整图像亮度
 * @param img 需要调整的 IMAGE 指针
 * @param delta 亮度变化值（正数变亮，负数变暗）
 */
void AdjustBrightness(IMAGE *img, int delta)
{
    DWORD *buffer = GetImageBuffer(img);
    int size = img != nullptr
        ? img->getwidth() * img->getheight()
        : getwidth() * getheight();
    for (int i = 0; i < size; i++)
    {
        DWORD YCbCr = RGBToYCbCr(BGR(buffer[i]));
        const int Y = clamp(
            static_cast<int>((YCbCr >> 16) & 0xff) + delta,
            0, 255
        );
        YCbCr = (static_cast<DWORD>(Y) << 16) | (YCbCr & 0xff00ffff);
        buffer[i] = YCbCrToRGB(YCbCr);
    }
}

/**
 * @brief 场景渐变，从 lastScene 渐变到 newScene
 * @param lastScene 上一个场景的 IMAGE 指针
 * @param newScene 新场景的 IMAGE 指针
 */
void gradient(const IMAGE *lastScene, IMAGE *newScene)
{
    for (int i = 0; i < 4; i++)
    {
        putimage(0, 0, lastScene);
        TransparentImage(
            nullptr, 0, 0, newScene,
            255 * (4 - i) / 4
        );
        FlushBatchDraw();
        Sleep(1000 / 24);
    }
}

/**
 * @brief RGB格式转换为YCbCr格式
 * @param rgb RGB格式的DWORD (0xAARRGGBB)
 * @return YCbCr格式的DWORD (0xAAYYCbCr)
 */
DWORD RGBToYCbCr(DWORD rgb)
{
    BYTE A = (rgb >> 24) & 0xFF;
    BYTE R = (rgb >> 16) & 0xFF;
    BYTE G = (rgb >> 8) & 0xFF;
    BYTE B = rgb & 0xFF;

    double Y  = 0.299 * R + 0.587 * G + 0.114 * B;
    double Cb = -0.168736 * R - 0.331264 * G + 0.5 * B + 128;
    double Cr = 0.5 * R - 0.418688 * G - 0.081312 * B + 128;

    return  (A << 24)
        | (static_cast<BYTE>(min(255.0, max(0.0, Y + 0.5))) << 16)
        | (static_cast<BYTE>(min(255.0, max(0.0, Cb + 0.5))) << 8)
        | static_cast<BYTE>(min(255.0, max(0.0, Cr + 0.5)));
}

/**
 * @brief YCbCr格式转换为RGB格式
 * @param ycbcr YCbCr格式的DWORD (0xAAYYCbCr)
 * @return RGB格式的DWORD (0xAARRGGBB)
 */
DWORD YCbCrToRGB(DWORD ycbcr)
{
    BYTE A  = (ycbcr >> 24) & 0xFF;
    BYTE Y  = (ycbcr >> 16) & 0xFF;
    BYTE Cb = (ycbcr >> 8) & 0xFF;
    BYTE Cr = ycbcr & 0xFF;

    double R = Y + 1.402 * (Cr - 128);
    double G = Y - 0.344136 * (Cb - 128) - 0.714136 * (Cr - 128);
    double B = Y + 1.772 * (Cb - 128);

    return (A << 24)
        | RGB(
            static_cast<BYTE>(min(255.0, max(0.0, R + 0.5))),
            static_cast<BYTE>(min(255.0, max(0.0, G + 0.5))),
            static_cast<BYTE>(min(255.0, max(0.0, B + 0.5)))
        );
}