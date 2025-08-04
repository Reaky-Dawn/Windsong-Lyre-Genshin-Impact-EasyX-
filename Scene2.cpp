//
// Created by Reaky on 2025/5/31.
//
/*
 * 场景 2：用于调整音高的简单操作界面
 */

#include <conio.h>
#include <graphics.h>
#include <thread>

#include "button.hpp"
#include "graphics.hpp"
#include "scene.hpp"
#include "Scene2.hpp"

using namespace std;

extern thread pianoLoadShift;
extern Piano piano;

int pitchshift = 0;

void InitScene2(IMAGE &background, IMAGE exit[2], IMAGE pitchshift_icon[3])
{
// 初始化背景图片
	IMAGE scene_bg;

	loadimage(&background, "IMAGE", "Background.jpg");
	loadimage(&scene_bg, "IMAGE", "Scene2.png");

	AdjustBrightness(&background, -25);

	SetWorkingImage(&background);
	putimage(183, 85, &scene_bg);
	SetWorkingImage();

// 初始化其他图标
	loadimage(&exit[0], "IMAGE", "Exit.png");
	loadimage(&pitchshift_icon[0], "IMAGE", "Shift.png");

// 初始化 exit
	DWORD *buf = GetImageBuffer(&exit[0]);
	for (int i = 0; i < exit[0].getwidth() * exit[0].getheight(); i++)
		buf[i] &= WHITE;

	exit[1] = exit[0];
	buf = GetImageBuffer(&exit[1]);
	for (int i = 0; i < exit[1].getwidth() * exit[1].getheight(); i++)
		if (buf[i] != BGR(RGB(62, 69, 86)))
			buf[i] &= WHITE;
		else
			buf[i] |= 0xff000000;
	AdjustBrightness(&exit[1], 57);

// 初始化 pitchshift 增减图标
	buf = GetImageBuffer(&pitchshift_icon[0]);
	for (int i = 0; i < pitchshift_icon[0].getwidth() * pitchshift_icon[0].getheight(); i++)
		buf[i] &= WHITE;
	pitchshift_icon[1] = pitchshift_icon[0];
	AdjustBrightness(&pitchshift_icon[1], -25);
	pitchshift_icon[2] = pitchshift_icon[1];

	DWORD *dst = GetImageBuffer(&pitchshift_icon[1]);
	buf = GetImageBuffer(&pitchshift_icon[0]);
	memcpy(
		dst + pitchshift_icon[0].getwidth() * pitchshift_icon[0].getheight() / 2,
		buf + pitchshift_icon[0].getwidth() * pitchshift_icon[0].getheight() / 2,
		pitchshift_icon[0].getwidth() * pitchshift_icon[0].getheight() / 2 * sizeof(DWORD)
	);

	dst = GetImageBuffer(&pitchshift_icon[2]);
	memcpy(dst, buf, pitchshift_icon[0].getwidth() * pitchshift_icon[0].getheight() / 2 * sizeof(DWORD));
}

void Scene2_PitchShift()
{
	int lastShift = pitchshift;
	IMAGE lastScene;
	Scene scene2;

	IMAGE background, exit[2], pitchshift_icon[3];
	InitScene2(background, exit, pitchshift_icon);
	getimage(&lastScene, 0, 0, 960, 540);

	ShiftButton shift_button;
	ExitButton exit_button;
	NumberButton number_button;

	for (int i = 0; i < 3; i++)
		shift_button.imgs[i] = pitchshift_icon[i];
	shift_button.set_pos(368, 153);
	shift_button.number = &pitchshift;
	shift_button.maxv = 12;
	shift_button.minv = -12;

	exit_button.set_ImgResting(exit[0]);
	exit_button.set_ImgActivated(exit[1]);
	exit_button.set_pos(714, 104);

	number_button.font = "Inter";
	number_button.number = &pitchshift;
	number_button.text_cor = BLACK;
	number_button.tHeight = 20;
	number_button.tWidth = 10;
	number_button.tWeight = FW_HEAVY;
	number_button.set_pos(291, 152);

	scene2.SetBackground(background);
	scene2.SetDstImg(nullptr);
	scene2.AddButton(&shift_button);
	scene2.AddButton(&exit_button);
	scene2.AddButton(&number_button);

// 过渡到场景2

	IMAGE newScene;
	scene2.Draw();
	getimage(&newScene, 0, 0, 960, 540);
	gradient(&lastScene, &newScene);

	while (scene2.Detect())
	{
		scene2.Draw();
		FlushBatchDraw();
		Sleep(1000 / 24);
	}

	piano.set_shift(pitchshift);

	if (pianoLoadShift.joinable() && pitchshift != lastShift)
	{
		IMAGE message;
		loadimage(&message, "IMAGE", "Message.png");

		AdjustBrightness(nullptr, -25);
		getimage(&newScene, 0, 0, 960, 540);

		putimage(960 / 2 - 297 / 2, 540 / 2 - 185 / 2, &message);
		getimage(&message, 0, 0, 960, 540);

		gradient(&newScene, &message);

		pianoLoadShift.join();
	}

	// 过渡到场景1

	getimage(&newScene, 0, 0, 960, 540);
	gradient(&newScene, &lastScene);
}
