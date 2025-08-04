//
// Created by Reaky on 2025/5/24.
//

#pragma once

#include <cmath>
#include <algorithm>
#include <functional>

#include <easyx.h>

#include "graphics.hpp"
#include "piano.hpp"

#define CALCDISTANCE(x1, y1, x2, y2) (static_cast<int>(sqrt(((x2) - (x1)) * ((x2) - (x1)) + ((y2) - (y1)) * ((y2) - (y1)))))

using namespace std;

class Button
{
protected:
	int x, y, frame;
	bool is_activated;									// 处在 静息 | 点击 状态
	bool is_animating;									// 处在播放动画阶段
	IMAGE resting, activated;							// 按键在 静息 | 点击 | 播放动画 时的图标
public:
	Button()
	{
		this->frame = 0;
		this->x = this->y = 0;
		this->is_animating = false;
		this->is_activated = false;
	}
	virtual ~Button() = default;
	Button(const Button &b)
	{
		this->x = b.get_Posx();
		this->y = b.get_Posy();
		this->resting = b.get_ImgResting();
		this->activated = b.get_Imgactivated();
		this->is_activated = false;
		this->is_animating = false;
		this->frame = 0;
	}

	void set_ImgResting(const IMAGE &img)
	{
		this->resting = img;
	}
	void set_ImgActivated(const IMAGE &img)
	{
		this->activated = img;
	}
	void set_pos(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	virtual void Draw(IMAGE *dst)
	{
		if ((this->is_activated ? &this->activated : &this->resting) != nullptr)
			TransparentImage(dst, this->x, this->y, this->is_activated ? &this->activated : &this->resting);
		if (this->is_animating)
			this->AniEffect(dst);
	}

	virtual bool CheckClick(const ExMessage msg)			// 判断是否被激活，若被激活，执行相关操作并返回 true
	{
		return true;
	}
	virtual int AniEffect(IMAGE *dst)						// 动画效果
	{
		return 0;
	}

	[[nodiscard]] int get_Posx() const
	{
		return this->x;
	}
	[[nodiscard]] int get_Posy() const
	{
		return this->y;
	}
	[[nodiscard]] IMAGE get_ImgResting() const
	{
		return this->resting;
	}
	[[nodiscard]] IMAGE get_Imgactivated() const
	{
		return this->activated;
	}

	Button &operator=(const Button &b)
	{
		this->x = b.get_Posx();
		this->y = b.get_Posy();
		this->resting = b.get_ImgResting();
		this->activated = b.get_Imgactivated();
		this->is_activated = false;
		this->is_animating = false;
		this->frame = 0;
		return *this;
	}
};

// 音符按钮，点击或按下对应按键触发
class NoteButton : public Button
{
private:
	QWORD alarm;
	bool trigger_byKb, tenuto;

public:
	Piano *piano;
	char note;

	NoteButton(): trigger_byKb(false), piano(nullptr), note(0)
	{
		this->alarm = static_cast<QWORD>(-1);
		this->tenuto = true;
	}

	~NoteButton() override = default;

	bool CheckClick(const ExMessage msg) override
	{
		if (msg.message == WM_KEYDOWN &&
			!this->is_activated &&
			msg.vkcode == this->note ||
			msg.message == WM_LBUTTONDOWN &&
			!this->is_activated &&
			CALCDISTANCE(msg.x, msg.y, this->x + this->resting.getwidth() / 2,
				this->y + this->resting.getheight() / 2) < resting.getwidth() / 2)
		{
			this->trigger_byKb = msg.message == WM_KEYDOWN && msg.vkcode == this->note;
			this->is_activated = true;
			this->is_animating = true;
			this->frame = 0;
			piano->PlayNote(this->note);
		}
		if (msg.message == WM_KEYUP &&
			this->is_activated &&
			msg.vkcode == this->note &&
			this->trigger_byKb ||
			msg.message == WM_LBUTTONUP && !this->trigger_byKb)
			this->is_activated = false;

		if (msg.message == WM_KEYDOWN &&
			!this->is_activated&&
			!isalpha(msg.vkcode))
			piano->PauseNote(this->note);

		return true;
	}
	int AniEffect(IMAGE *dst) override
	{
		IMAGE temp(90, 90);

		SetWorkingImage(&temp);

		DWORD *buffer = GetImageBuffer(&temp);
		memset(buffer, 0xff, 90 * 90 * sizeof(DWORD));

		setlinecolor(RGB(144 * 0.8, 249 * 0.8, 227 * 0.8));
		setlinestyle(PS_SOLID, 2);
		setbkmode(TRANSPARENT);

		circle(45, 45, static_cast<int>(this->frame * 2.5 + 30));

		for (int i = 0; i < 90 * 90; i++)
			if (buffer[i] != 0xffffffff)
				buffer[i] = (buffer[i] & WHITE) | (static_cast<DWORD>(0xff * this->frame / 6 & 0xff) << 24);

		SetWorkingImage();
		TransparentImage(dst, this->x - 15, this->y - 15, &temp);

		if (++this->frame > 6)
		{
			this->frame = 0;
			this->is_animating = false;
			return -1;
		}

		return 0;
	}

	NoteButton &operator=(const NoteButton &b)
	{
		if (this == &b)
			return *this;

		this->x = b.get_Posx();
		this->y = b.get_Posy();
		this->resting = b.get_ImgResting();
		this->activated = b.get_Imgactivated();
		this->is_activated = false;
		this->is_animating = false;
		this->trigger_byKb = false;
		this->frame = 0;
		this->piano = b.piano;
		this->note = b.note;
		this->alarm = static_cast<unsigned long long>(-1);
		this->tenuto = true;
		return *this;
	}
};

// 选项按钮，点击触发。用于进入新场景
class OptionButton : public Button
{
public:
	function<void()> Option;

	~OptionButton() override = default;

	bool CheckClick(const ExMessage msg) override
	{
		this->is_activated = (msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN) &&
			CALCDISTANCE(msg.x, msg.y, this->x + this->resting.getwidth() / 2,\
			this->y + this->resting.getheight() / 2) < resting.getwidth() / 2;

		if (msg.message == WM_LBUTTONDOWN && this->is_activated)
		{
			this->is_activated = false;
			this->Option();
		}
		return true;
	}

	OptionButton &operator=(const OptionButton &b)
	{
		this->x = b.get_Posx();
		this->y = b.get_Posy();
		this->resting = b.get_ImgResting();
		this->activated = b.get_Imgactivated();
		this->is_activated = false;
		this->is_animating = false;
		this->frame = 0;
		this->Option = b.Option;
		return *this;
	}
};

class OptionButton_square : public OptionButton
{
public:
	bool CheckClick(const ExMessage msg) override
	{
		this->is_activated = (msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN) &&
			msg.x >= this->x && msg.x < this->x + this->resting.getwidth() &&
			msg.y >= this->y && msg.y < this->y + this->resting.getheight();

		if (msg.message == WM_LBUTTONDOWN && this->is_activated)
		{
			this->is_activated = false;
			this->Option();
		}
		return true;
	}
};

/*
 * 注意：ShiftButton 和 NumberButton 由于使用传统指针指向外部数据，不允许出现跨越场景使用的情况。
 * 同时，要求所指向的数据生命周期大于等于按钮所在场景的生命周期
 */
class ShiftButton : public Button
{
public:
	int *number;
	int maxv, minv, img_p;
	IMAGE imgs[3];
	ShiftButton(): number(nullptr), maxv(0), minv(0)
	{
		this->img_p = 0;
	}

	~ShiftButton() override = default;

	bool CheckClick(const ExMessage msg) override
	{
		// 处理“增加”点击
		if (msg.message == WM_KEYDOWN && msg.vkcode == VK_UP ||
			msg.message == WM_LBUTTONDOWN &&
			msg.x > this->x &&
			msg.y > this->y &&
			msg.x < this->x + this->imgs[this->img_p].getwidth() &&
			msg.y < this->y + this->imgs[this->img_p].getheight() / 2)
		{
			*this->number = min(*this->number + 1, this->maxv);
		}

		// 处理“减少”点击
		if (msg.message == WM_KEYDOWN && msg.vkcode == VK_DOWN ||
			msg.message == WM_LBUTTONDOWN &&
			msg.x > this->x &&
			msg.y > this->y + this->imgs[this->img_p].getheight() / 2 &&
			msg.x < this->x + this->imgs[this->img_p].getwidth() &&
			msg.y < this->y + this->imgs[this->img_p].getheight())
		{
			*this->number = max(*this->number - 1, this->minv);
		}

		// 更新按钮状态
		if ((msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN) &&
			msg.x > this->x &&
			msg.y > this->y &&
			msg.x < this->x + this->imgs[this->img_p].getwidth() &&
			msg.y < this->y + this->imgs[this->img_p].getheight() / 2)
		{
			this->img_p = 1;
		}
		else if ((msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN) &&
				 msg.x > this->x &&
				 msg.y > this->y + this->imgs[this->img_p].getheight() / 2 &&
				 msg.x < this->x + this->imgs[this->img_p].getwidth() &&
				 msg.y < this->y + this->imgs[this->img_p].getheight())
		{
			this->img_p = 2;
		}
		else
		{
			this->img_p = 0;
		}


		return true;
	}

	void Draw(IMAGE *dst) override
	{
		TransparentImage(dst, this->x, this->y, this->imgs + this->img_p);
	}

	ShiftButton &operator=(const ShiftButton &b)
	{
		if (this == &b)
			return *this;
		this->x = b.get_Posx();
		this->y = b.get_Posy();
		this->resting = b.get_ImgResting();
		this->activated = b.get_Imgactivated();
		this->is_activated = false;
		this->is_animating = false;
		this->frame = 0;
		this->number = b.number;
		this->img_p = b.img_p;
		this->maxv = b.maxv;
		this->minv = b.minv;
		for (int i = 0; i < 3; i++)
			this->imgs[i] = b.imgs[i];
		return *this;
	}
};

class NumberButton : public Button
{
public:
	int *number;
	COLORREF text_cor;
	int tWidth, tHeight, tWeight;
	string font;

	~NumberButton() override = default;

	void Draw(IMAGE *dst) override
	{
		IMAGE *working_image = GetWorkingImage();
		SetWorkingImage(dst);

		string txt;
		int tmp = *this->number;
		bool minus = false;
		if (!tmp)
			txt = "0";
		if (tmp < 0)
		{
			tmp = -tmp;
			minus = true;
		}
		while (tmp)
		{
			txt += static_cast<char>(tmp % 10 + '0');
			tmp /= 10;
		}
		if (minus)
			txt += '-';
		reverse(txt.begin(), txt.end());

		settextcolor(this->text_cor);

		LOGFONT f;
		gettextstyle(&f);

		_tcscpy_s(f.lfFaceName, this->font.c_str());
		f.lfQuality = ANTIALIASED_QUALITY;
		f.lfHeight = this->tHeight;
		f.lfWidth = this->tWidth;
		f.lfWeight = this->tWeight;

		settextstyle(&f);
		setbkmode(TRANSPARENT);
		outtextxy(this->x, this->y, txt.c_str());


		SetWorkingImage(working_image);
	}

	NumberButton &operator=(const NumberButton &b)
	{
		if (this == &b)
			return *this;

		this->x = b.get_Posx();
		this->y = b.get_Posy();
		this->resting = b.get_ImgResting();
		this->activated = b.get_Imgactivated();
		this->is_activated = false;
		this->is_animating = false;
		this->frame = 0;
		this->number = b.number;
		this->text_cor = b.text_cor;
		this->tWidth = b.tWidth;
		this->tHeight = b.tHeight;
		this->font = b.font;
		return *this;
	}
};

class ExitButton : public Button
{
public:
	~ExitButton() override = default;
	bool CheckClick(const ExMessage msg) override
	{
		if (msg.message == WM_LBUTTONDOWN &&
			msg.x > this->x &&
			msg.y > this->y &&
			msg.x < this->x + this->resting.getwidth() &&
			msg.y < this->y + this->resting.getheight())
			return false;
		this->is_activated = (
			(msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN) &&
			msg.x > this->x &&
			msg.y > this->y &&
			msg.x < this->x + this->resting.getwidth() &&
			msg.y < this->y + this->resting.getheight()
		);
		return true;
	}
};