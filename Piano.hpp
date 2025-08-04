//
// Created by Reaky on 2025/5/11.
//

#pragma once

#include <sndfile.h>
#include "Note.hpp"
#include "Sound.hpp"

#define QWORD unsigned long long

#define FORM_WAVEFORMATX(wfx, sfinfo) {\
	(wfx).wFormatTag = WAVE_FORMAT_PCM;\
	(wfx).nChannels = (sfinfo).channels;\
	(wfx).nSamplesPerSec = (sfinfo).samplerate;\
	(wfx).wBitsPerSample = 16;\
	(wfx).nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;\
	(wfx).nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;\
}

class Piano
{
private:
	// mapper 表示 按下某个按键后对应的音符，根据 InvMapper 生成
	Sound sounds;
	int mapper[26];
	int shift;

public:

	// std_InvMapper 表示 从双点低音 do 到 双点高音 ti 对应的字母序号（从 'a' -> 0 开始）
	const int std_InvMapper[60] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		'z' - 'a', -1, 'x' - 'a', -1, 'c' - 'a', 'v' - 'a', -1, 'b' - 'a', -1, 'n' - 'a', -1, 'm' - 'a',
		0, -1, 's' - 'a', -1, 'd' - 'a', 'f' - 'a', -1, 'g' - 'a', -1, 'h' - 'a', -1, 'j' - 'a',
		'q' - 'a', -1, 'w' - 'a', -1, 'e' - 'a', 'r' - 'a', -1, 't' - 'a', -1, 'y' - 'a', -1, 'u' - 'a',
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};

	// 按低音 do 到高音 ti 的顺序排列的 21 个音符
	Piano(const vector<string> &files)
	{
		this->LoadNotes(files);
		shift = 0;
		this->set_InvMapper(Piano::std_InvMapper);
	}
	Piano(const Piano &other)
	{
		this->sounds = other.get_sound();
		this->set_InvMapper(Piano::std_InvMapper);
		shift = other.get_shift();
	}

	Piano()
	{
		this->set_InvMapper(Piano::std_InvMapper);
		shift = 0;
	}

	~Piano() = default;

	void PlayNote(char ch)
	{
		int note_num = (ch >= 'a' && ch <= 'z') ? ch - 'a' : (ch >= 'A' && ch <= 'Z') ? ch - 'A' : -1;
		if (note_num >= 0 && this->mapper[note_num] >= 0)
		{
			if (this->sounds.is_playing(this->mapper[note_num] + shift))
			{
				this->sounds.pause(this->mapper[note_num] + shift);
				//Sleep(5);
			}
			this->sounds.playsound(this->mapper[note_num] + shift);
		}
	}

	void PauseNote(char ch) const
	{
		int note_num = (ch >= 'a' && ch <= 'z') ? ch - 'a' : (ch >= 'A' && ch <= 'Z') ? ch - 'A' : -1;
		if (note_num >= 0 && this->mapper[note_num] >= 0)
			if (this->sounds.is_playing(this->mapper[note_num] + shift))
				this->sounds.pause(this->mapper[note_num] + shift);
	}

	bool LoadOneNote(const string &file)
	{
		SF_INFO sfinfo;
		vector<short> pcmData = LoadWave(file.c_str(), sfinfo);

		if (pcmData.size() == 0)
			return false;
		WAVEFORMATEX wfx = {0};
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = sfinfo.channels;
		wfx.nSamplesPerSec = sfinfo.samplerate;
		wfx.wBitsPerSample = 16;
		wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

		this->sounds.AllocateChannel(pcmData.data(), pcmData.size(), wfx);
		return true;
	}

	// 加载音符
	bool LoadNotes(const vector<string> &files)
	{
		if (files.size() != 21)
			return false;

		for (int i = 0; i < 12; i++)
			this->sounds.AllocateChannel();

		// 加载正常的三组音符
		for (int i = 0, p = 0; i < 3; i++)
		{
			// do & re
			for (int j = 0; j < 2; j++, p++)
			{
				if (!this->LoadOneNote(files[p]))
					return false;

				this->sounds.AllocateChannel();
			}

			// mi:
			this->LoadOneNote(files[p++]);

			// fa ~ la:
			for (int j = 0; j < 3; j++, p++)
			{
				if (!this->LoadOneNote(files[p]))
					return false;

				this->sounds.AllocateChannel();
			}

			// ti:
			this->LoadOneNote(files[p++]);
		}

		return true;
	}

	void LoadShift(vector<string> files)
	{
		// 特殊处理 单点低音 do，以产生双点低音
		SF_INFO sfinfo;
		{
			vector<float> floatData = LoadWave_float(files[0].c_str(), sfinfo);
			for (int i = -12; i < 0; i++)
			{
				vector<short> pcm_Pitch = ConvFloatToPCM(ConvFlatToInterleaved(
					PitchShift(ConvInterleavedToFlat(floatData, sfinfo), i, sfinfo)));
				WAVEFORMATEX wfx = {0};
				FORM_WAVEFORMATX(wfx, sfinfo);
				this->sounds.InitChannel(i + 12, pcm_Pitch.data(), pcm_Pitch.size(), wfx);
			}
		}

		// 处理正常的三组音符

		int channel = 12;

		for (int i = 0, p = 0; i < 3; i++)
		{
			// do & re
			for (int j = 0; j < 2; j++, p++)
			{
				vector<float> floatData = LoadWave_float(files[p].c_str(), sfinfo);
				vector<short> pcm_Pitch = ConvFloatToPCM(ConvFlatToInterleaved( \
					PitchShift(ConvInterleavedToFlat(floatData, sfinfo), 1, sfinfo)));

				WAVEFORMATEX wfx = {0};
				FORM_WAVEFORMATX(wfx, sfinfo);

				channel++;
				this->sounds.InitChannel(channel, pcm_Pitch.data(), pcm_Pitch.size(), wfx);
				channel++;
			}


			channel++;
			p++;

			// fa ~ la:
			for (int j = 0; j < 3; j++, p++)
			{
				vector<float> floatData = LoadWave_float(files[p].c_str(), sfinfo);
				vector<short> pcm_Pitch = ConvFloatToPCM(ConvFlatToInterleaved(
					PitchShift(ConvInterleavedToFlat(floatData, sfinfo), 1, sfinfo)));

				WAVEFORMATEX wfx = {0};
				FORM_WAVEFORMATX(wfx, sfinfo);

				channel++;
				this->sounds.InitChannel(channel, pcm_Pitch.data(), pcm_Pitch.size(), wfx);
				channel++;
			}
			channel++;
			p++;
		}

		// 特殊处理 单点高音 ti，以产生双点高音
		{
			vector<float> floatData = LoadWave_float(files[20].c_str(), sfinfo);
			for (int i = 1; i <= 12; i++)
			{
				vector<short> pcm_Pitch = ConvFloatToPCM(ConvFlatToInterleaved(
					PitchShift(ConvInterleavedToFlat(floatData, sfinfo), i, sfinfo)));
				WAVEFORMATEX wfx = {0};
				FORM_WAVEFORMATX(wfx, sfinfo);
				this->sounds.AllocateChannel(pcm_Pitch.data(), pcm_Pitch.size(), wfx);
			}
		}
	}

	// 设定键位
	void set_InvMapper(const int mp[60])
	{
		memset(this->mapper, 0xff, sizeof(this->mapper));
		for (int i = 0; i < 60; i++)
			if (std_InvMapper[i] != -1)
				this->mapper[std_InvMapper[i]] = i;
	}

	// 设置音高，限定在 [-12, 12] 的整数部分
	void set_shift(int s)
	{
		this->shift = max(-12, min(12, s));
	}

	Piano& operator=(const Piano &other)
	{
		this->sounds = other.get_sound();
		memcpy(this->mapper, other.get_mapper(), 26 * sizeof(int));
		return *this;
	}

	Sound get_sound() const
	{
		return this->sounds;
	}

	const int *get_mapper() const
	{
		return this->mapper;
	}

	int get_shift() const
	{
		return this->shift;
	}
};