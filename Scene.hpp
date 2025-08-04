//
// Created by Reaky on 2025/5/24.
//

#pragma once
#include "button.hpp"

using namespace std;

/*
 * 每个 Scene 的实例指向一个场景，场景在初始化完毕后可直接调用简单接口以实现互动。
 * 当 Scene::Draw 返回 false 时，表示应当退出当前场景。而 Scene::Draw 的返回值来自于
 * Button::CheckClick，这意味着允许设定一个按钮，在触发时推出场景。
 */
class Scene
{
protected:
	IMAGE background, *dstimg;
	int x, y;
	vector<Button *> buttons;

public:
	Scene()
	{
		this->x = this->y = 0;
		this->dstimg = nullptr;
	}
	Scene(const Scene &s)
	{
		this->buttons = s.GetButtons();
		this->background = s.GetBackground();
		this->dstimg = s.GetDstImg();
		this->x = s.GetX();
		this->y = s.GetY();
	}
	virtual ~Scene() = default;

	void Draw() const
	{
		putimage(this->x, this->y, &this->background);
		for (Button *button : this->buttons)
			button->Draw(dstimg);
	}

	bool Detect() const
	{

		bool ret = true;
		ExMessage msg;
		while (peekmessage(&msg))
			for (Button *button : this->buttons)
				ret = button->CheckClick(msg) && ret;
		return ret;
	}

	void SetBackground(const IMAGE &background)
	{
		this->background = background;
	}
	void AddButton(Button *button)
	{
		buttons.emplace_back(button);
	}
	void SetDstImg(IMAGE *dstimg)
	{
		this->dstimg = dstimg;
	}
	void SetPos(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	vector<Button *> GetButtons() const
	{
		return this->buttons;
	}
	IMAGE GetBackground() const
	{
		return this->background;
	}
	IMAGE *GetDstImg() const
	{
		return this->dstimg;
	}
	int GetX() const
	{
		return this->x;
	}
	int GetY() const
	{
		return this->y;
	}

	Scene &operator=(const Scene &s)
	{
		this->buttons = s.GetButtons();
		this->background = s.GetBackground();
		this->dstimg = s.GetDstImg();
		this->x = s.GetX();
		this->y = s.GetY();
		return *this;
	}
};
