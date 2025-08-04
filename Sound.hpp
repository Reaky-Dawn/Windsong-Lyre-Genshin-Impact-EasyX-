//
// Created by Reaky on 2025/5/5.
//

#pragma once

#include <vector>
#include <windows.h>
#include <memory>
#include "Channel.hpp"

using namespace std;

/**
 * @class Sound
 * @brief 管理音频通道的类，支持播放、暂停、分配和初始化通道等操作。
 *
 * 采用 shared_ptr 管理 Channel 实例的生命周期，确保拷贝构造和赋值时通道资源共享。
 * 单个项目中的所有 Sound 实例应由同一个 Sound 拷贝构造或赋值产生。
 */
class Sound
{
private:
    /**
     * @brief 音频通道列表
     */
    vector<shared_ptr<Channel> > channels;

public:
    /**
     * @brief 默认构造函数
     */
    Sound() = default;

    /**
     * @brief 拷贝构造函数
     * @param e 另一个 Sound 实例
     */
    Sound(const Sound &e)
    {
        channels = e.get_channels();
    }

    /**
     * @brief 析构函数
     */
    ~Sound() = default;

    /**
     * @brief 播放音频数据
     * @param buffer 音频数据缓冲区
     * @param buf_size 缓冲区大小（采样数）
     * @param format 音频格式
     * @return 使用的通道索引
     */
    int playsound(
        const short *buffer,
        size_t buf_size,
        const WAVEFORMATEX &format
    )
    {
        for (int i = 0; i < channels.size(); i++)
        {
            shared_ptr<Channel> &ch = channels[i];
            if ((ch->wavehdr.dwFlags & WHDR_DONE) &&
                memcmp(&format, &ch->format, sizeof(WAVEFORMATEX)) == 0)
            {
                waveOutUnprepareHeader(
                    ch->hwo, &ch->wavehdr, sizeof(WAVEHDR)
                );
                delete[] ch->buf;
                ch->buf = new short[buf_size];
                memcpy(ch->buf, buffer, buf_size);

                ch->wavehdr = {nullptr};
                ch->wavehdr.dwBufferLength = buf_size;
                ch->wavehdr.lpData = reinterpret_cast<LPSTR>(ch->buf);

                waveOutPrepareHeader(
                    ch->hwo, &ch->wavehdr, sizeof(WAVEHDR)
                );
                waveOutWrite(
                    ch->hwo, &ch->wavehdr, sizeof(WAVEHDR)
                );

                return i;
            }
        }
        int i = this->AllocateChannel(buffer, buf_size, format);
        this->playsound(i);
        return i;
    }

    /**
     * @brief 播放指定通道的音频
     * @param i 通道索引
     * @return 成功返回通道索引，失败返回-1
     */
    int playsound(int i)
    {
        if (i >= 0 && i < channels.size() &&
            ((channels[i]->wavehdr.dwFlags & WHDR_DONE) ||
             (channels[i]->wavehdr.dwFlags & WHDR_PREPARED)))
        {
            shared_ptr<Channel> &ch = channels[i];
            waveOutWrite(ch->hwo, &ch->wavehdr, sizeof(WAVEHDR));
            return i;
        }
        return -1;
    }

    /**
     * @brief 分配一个新的音频通道并初始化
     * @param buffer 音频数据缓冲区
     * @param buf_size 缓冲区大小（采样数）
     * @param format 音频格式
     * @return 新通道的索引
     */
    int AllocateChannel(
        const short *buffer,
        size_t buf_size,
        const WAVEFORMATEX &format
    )
    {
        channels.emplace_back(make_shared<Channel>());
        shared_ptr<Channel> &ch = channels[channels.size() - 1];
        ch->buf = new short[buf_size];
        memcpy(ch->buf, buffer, buf_size * sizeof(short));

        waveOutOpen(
            &ch->hwo, WAVE_MAPPER, &format, 0, 0, CALLBACK_NULL
        );

        ch->wavehdr = {nullptr};
        ch->wavehdr.dwBufferLength = buf_size * sizeof(short);
        ch->wavehdr.lpData = reinterpret_cast<LPSTR>(ch->buf);
        ch->wavehdr.dwFlags = WHDR_DONE;

        waveOutPrepareHeader(
            ch->hwo, &ch->wavehdr, sizeof(WAVEHDR)
        );
        return static_cast<int>(channels.size()) - 1;
    }

    /**
     * @brief 分配一个空的音频通道
     * @return 新通道的索引
     */
    int AllocateChannel()
    {
        channels.emplace_back(make_shared<Channel>());
        return static_cast<int>(channels.size()) - 1;
    }

    /**
     * @brief 初始化指定通道
     * @param channel 通道索引
     * @param buffer 音频数据缓冲区
     * @param buf_size 缓冲区大小（采样数）
     * @param format 音频格式
     */
    void InitChannel(
        int channel,
        const short *buffer,
        size_t buf_size,
        const WAVEFORMATEX &format
    )
    {
        channels.emplace_back(make_shared<Channel>());
        shared_ptr<Channel> &ch = channels[channel];
        ch->buf = new short[buf_size];
        memcpy(ch->buf, buffer, buf_size * sizeof(short));

        waveOutOpen(
            &ch->hwo, WAVE_MAPPER, &format, 0, 0, CALLBACK_NULL
        );

        ch->wavehdr = {nullptr};
        ch->wavehdr.dwBufferLength = buf_size * sizeof(short);
        ch->wavehdr.lpData = reinterpret_cast<LPSTR>(ch->buf);
        ch->wavehdr.dwFlags = WHDR_DONE;

        waveOutPrepareHeader(
            ch->hwo, &ch->wavehdr, sizeof(WAVEHDR)
        );
    }

    /**
     * @brief 暂停指定通道的播放
     * @param i 通道索引
     */
    void pause(int i) const
    {
        if (i >= 0 && i < channels.size())
            waveOutReset(channels[i]->hwo);
    }

    /**
     * @brief 判断指定通道是否正在播放
     * @param i 通道索引
     * @return 正在播放返回true，否则返回false
     */
    bool is_playing(int i) const
    {
        return i >= 0 &&
            i < channels.size() &&
            !(channels[i]->wavehdr.dwFlags & WHDR_DONE);
    }

    /**
     * @brief 获取所有通道
     * @return 通道列表
     */
    vector<shared_ptr<Channel> > get_channels() const
    {
        return channels;
    }

    /**
     * @brief 赋值运算符重载
     * @param e 另一个 Sound 实例
     * @return 当前实例的引用
     */
    Sound &operator = (const Sound &e)
    {
        channels = e.get_channels();
        return *this;
    }
};