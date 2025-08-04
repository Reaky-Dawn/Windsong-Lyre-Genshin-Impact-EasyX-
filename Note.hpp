//
// Created by Reaky on 2025/5/11.
//

#pragma once

#include <sndfile.h>
#include <rubberband/RubberBandStretcher.h>

using namespace std;

// 采用 pcm16 加载 wave 文件
vector<short> LoadWave(const char *infile, SF_INFO &sfinfo);

// 采用 32 位浮点采样加载 wave 文件
vector<float> LoadWave_float(const char *infile, SF_INFO &sfinfo);

// 将交叉格式转化为平面格式
vector<vector<float> > ConvInterleavedToFlat(const vector<float> &interleaved, const SF_INFO &sfinfo);

// 将平面格式转化为交叉格式
vector<float> ConvFlatToInterleaved(const vector<vector<float> > &channels);

// 将 32 位浮点采样转化为 pcm16 采样
vector<short> ConvFloatToPCM(const vector<float> &interleaved);

// 调整音高，shift_semitones 表示调整的半音数。
vector<vector<float> > PitchShift(const vector<vector<float> > &inChannels, double shift_semitones, const SF_INFO &sfinfo);