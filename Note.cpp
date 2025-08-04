//
// Created by Reaky on 2025/5/25.
//

#include <cmath>

#include <sndfile.h>
#include <rubberband/RubberBandStretcher.h>

using namespace std;

/**
 * @brief 使用pcm16格式加载WAV文件
 * @param infile 输入文件路径
 * @param sfinfo 音频文件信息结构体
 * @return 加载的音频数据（short类型向量）
 */
vector<short> LoadWave(const char *infile, SF_INFO &sfinfo)
{
    SNDFILE *sndfile = sf_open(infile, SFM_READ, &sfinfo);

    if (!sndfile)
        return {};

    if ((sfinfo.format & (SF_FORMAT_WAV | SF_FORMAT_PCM_16)) !=
        (SF_FORMAT_WAV | SF_FORMAT_PCM_16) ||
        sfinfo.samplerate <= 0)
    {
        sf_close(sndfile);
        return {};
    }

    sf_count_t num_samples = sfinfo.frames * sfinfo.channels;
    vector<short> buf(num_samples);

    sf_read_short(sndfile, buf.data(), num_samples);
    sf_close(sndfile);

    return buf;
}

/**
 * @brief 使用32位浮点采样加载WAV文件
 * @param infile 输入文件路径
 * @param sfinfo 音频文件信息结构体
 * @return 加载的音频数据（float类型向量）
 */
vector<float> LoadWave_float(const char *infile, SF_INFO &sfinfo)
{
    sfinfo = { 0 };
    SNDFILE *sndfile = sf_open(infile, SFM_READ, &sfinfo);

    if (!sndfile)
        return {};

    if ((sfinfo.format & (SF_FORMAT_WAV)) != SF_FORMAT_WAV ||
        sfinfo.samplerate <= 0)
    {
        sf_close(sndfile);
        return {};
    }

    vector<float> buf(sfinfo.frames * sfinfo.channels);

    sf_readf_float(sndfile, buf.data(), sfinfo.frames);
    sf_close(sndfile);

    return buf;
}

/**
 * @brief 将交叉格式音频数据转换为平面格式
 * @param interleaved 交叉格式音频数据
 * @param sfinfo 音频文件信息结构体
 * @return 平面格式音频数据（每个通道一个向量）
 */
vector<vector<float> > ConvInterleavedToFlat(
    const vector<float> &interleaved,
    const SF_INFO &sfinfo
)
{
    vector<vector<float> > channels(sfinfo.channels);
    for (int ch = 0; ch < sfinfo.channels; ch++)
    {
        channels[ch].resize(sfinfo.frames);
        for (size_t i = 0; i < sfinfo.frames; i++)
            channels[ch][i] = interleaved[i * sfinfo.channels + ch];
    }
    return channels;
}

/**
 * @brief 将平面格式音频数据转换为交叉格式
 * @param channels 平面格式音频数据
 * @return 交叉格式音频数据
 */
vector<float> ConvFlatToInterleaved(
    const vector<vector<float> > &channels
)
{
    vector<float> interleaved(
        channels.size() * channels[0].size()
    );
    for (int ch = 0; ch < channels.size(); ch++)
        for (size_t i = 0; i < channels[ch].size(); i++)
            interleaved[i * channels.size() + ch] = channels[ch][i];
    return interleaved;
}

/**
 * @brief 将32位浮点采样转换为pcm16采样
 * @param interleaved 浮点采样数据
 * @return pcm16采样数据
 */
vector<short> ConvFloatToPCM(const vector<float> &interleaved)
{
    vector<short> pcm(interleaved.size());
    for (size_t i = 0; i < interleaved.size(); i++)
        pcm[i] = static_cast<short>(
            32767.0f * max(-1.0f, min(1.0f, interleaved[i]))
        );
    return pcm;
}

/**
 * @brief 调整音高
 * @param inChannels 输入的平面格式音频数据
 * @param shift_semitones 调整的半音数
 * @param sfinfo 音频文件信息结构体
 * @return 调整后音高的平面格式音频数据
 */
vector<vector<float> > PitchShift(
    const vector<vector<float> > &inChannels,
    double shift_semitones,
    const SF_INFO &sfinfo
)
{
    // 配置参数
    double scale = pow(2.0, shift_semitones / 12.0);
    constexpr int option =
        RubberBand::RubberBandStretcher::OptionProcessOffline |
        RubberBand::RubberBandStretcher::OptionPitchHighQuality;

    RubberBand::RubberBandStretcher stretcher(
        sfinfo.samplerate,
        sfinfo.channels,
        option
    );
    // 设置最大处理帧数（每通道样本数）
    stretcher.setMaxProcessSize(inChannels[0].size());
    stretcher.setPitchScale(scale);
    stretcher.setTimeRatio(1.0);

    // 处理数据
    vector<const float*> inputChannelPtr(sfinfo.channels);
    for (int c = 0; c < sfinfo.channels; c++)
        inputChannelPtr[c] = inChannels[c].data();

    // 使用实际帧数（每通道样本数）
    stretcher.process(
        inputChannelPtr.data(),
        inChannels[0].size(),  // 使用实际帧数而非sfinfo.frames
        true
    );

    // 收集处理完的数据
    vector<vector<float> > outChannels(sfinfo.channels);
    int available;

    // 获取可用帧数
    while ((available = stretcher.available()) > 0)
    {
        vector<float *> outputChannelPtr(sfinfo.channels);
        vector<vector<float> > outtemp(sfinfo.channels);

        for (int c = 0; c < sfinfo.channels; c++) {
            outtemp[c].resize(available);
            outputChannelPtr[c] = outtemp[c].data();
        }

        // 获取处理后的数据
        int retrieved = stretcher.retrieve(outputChannelPtr.data(), available);

        // 只添加实际获取的样本
        for (int c = 0; c < sfinfo.channels; c++) {
            outChannels[c].insert(
                outChannels[c].end(),
                outtemp[c].begin(),
                outtemp[c].begin() + retrieved
            );
        }
    }
    return outChannels;
}