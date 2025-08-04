//
// Created by Reaky on 2025/5/10.
//

#pragma once

#include <windows.h>

using namespace std;

/**
 * @class Channel
 * @brief 每个实例对应一个wave设备，负责音频通道的资源管理。
 *
 * Channel具备自我回收资源的能力，但建议通过Sound类进行统一管理。
 */
class Channel
{
public:
    /**
     * @brief 默认构造函数，初始化成员变量。
     */
    Channel()
    {
        buf = nullptr;
        hwo = nullptr;
        wavehdr = {nullptr};
        format = {0};

        memset(&wavehdr, 0, sizeof(wavehdr));
        memset(&format, 0, sizeof(format));
    }

    /**
     * @brief 拷贝构造函数，深拷贝音频缓冲区和相关参数。
     * @param e 另一个Channel实例
     */
    Channel(const Channel &e)
    {
        hwo = e.hwo;
        wavehdr = e.wavehdr;
        format = e.format;
        if (e.buf != nullptr)
        {
            buf = new short[wavehdr.dwBufferLength];
            memcpy(
                buf, e.buf,
                wavehdr.dwBufferLength * sizeof(short)
            );
        }
        else
            buf = nullptr;
    }

    /**
     * @brief 析构函数，释放wave设备和缓冲区资源。
     */
    ~Channel()
    {
        if (hwo == nullptr)
            return;
        // 资源释放前暂停播放
        if (!(wavehdr.dwFlags & WHDR_DONE))
            waveOutPause(hwo);
        waveOutUnprepareHeader(hwo, &wavehdr, sizeof(wavehdr));
        waveOutClose(hwo);
        delete[] buf;
    }

    /**
     * @brief 赋值运算符重载，深拷贝音频缓冲区和相关参数。
     * @param e 另一个Channel实例
     * @return 当前实例的引用
     */
    Channel &operator = (const Channel &e)
    {
        hwo = e.hwo;
        wavehdr = e.wavehdr;
        format = e.format;
        delete[] buf;
        if (e.buf != nullptr)
        {
            buf = new short[wavehdr.dwBufferLength];
            memcpy(
                buf, e.buf,
                wavehdr.dwBufferLength * sizeof(short)
            );
        }
        else
            buf = nullptr;
        return *this;
    }

    /**
     * @brief wave输出设备句柄
     */
    HWAVEOUT hwo;

    /**
     * @brief wave头信息结构体
     */
    WAVEHDR wavehdr;

    /**
     * @brief wave格式结构体
     */
    WAVEFORMATEX format;

    /**
     * @brief 音频数据缓冲区
     */
    short *buf;
};