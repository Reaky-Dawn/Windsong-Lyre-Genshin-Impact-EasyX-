//
// Created by Reaky on 2025/5/31.
//
/*
 * 场景 3：用于调整自动播放的简单操作界面
 */

#include <iostream>

#include <conio.h>
#include <graphics.h>
#include <codecvt>
#include <locale>
#include <functional>

#include "button.hpp"
#include "graphics.hpp"
#include "scene.hpp"
#include "Scene3.hpp"
#include "string.hpp"

using namespace std;

extern Piano piano;

constexpr int maxBytePerLine = 40;

vector<string> lineScore;
bool autoplaying = false;
QWORD alarm, MsPerBeat = 200;
int displayBegin, score_ptr = 0;

Scene scene3;

void InitScene3(IMAGE &background, IMAGE buttons[3][2], IMAGE exit[2])
{
// 初始化字体、背景模式
	LOGFONT globalFont;
	gettextstyle(&globalFont);
	globalFont.lfHeight = 18;
	globalFont.lfWeight = FW_HEAVY;
	_tcscpy(globalFont.lfFaceName, _T("宋体"));
	settextstyle(&globalFont);
	setbkmode(TRANSPARENT);

// 初始化背景图片、exit 图标
	IMAGE scene_bg;

	loadimage(&background, "IMAGE", "Background.jpg");
	loadimage(&scene_bg, "IMAGE", "Scene3.png");
	loadimage(&exit[0], "IMAGE", "Exit.png");

	AdjustBrightness(&background, -25);

	SetWorkingImage(&background);
	putimage(183, 85, &scene_bg);
	SetWorkingImage();

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

// 加载三个图标
	IMAGE autoPlay_icon;
	loadimage(&autoPlay_icon, "IMAGE", "Autoplay.png");

	buf = GetImageBuffer(&autoPlay_icon);
	for (int i = 0; i < autoPlay_icon.getwidth() * autoPlay_icon.getheight(); i++)
		buf[i] &= WHITE;

	string str[3] = {UTF8toAnsi("开始"), UTF8toAnsi("停止"), UTF8toAnsi("加载")};

	for (int i = 0; i < 3; i++)
	{
		wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

		buttons[i][0].Resize(160, 33);
		SetWorkingImage(&buttons[i][0]);
		putimage(-i * 160, 0, &autoPlay_icon);

		LOGFONT font;
		gettextstyle(&font);
		font.lfHeight = 18;
		font.lfWeight = FW_HEAVY;
		_tcscpy(font.lfFaceName, _T("宋体"));
		settextstyle(&font);
		settextcolor(RGB(60, 70, 85));
		setbkmode(TRANSPARENT);
		outtextxy(83, 8, str[i].c_str());
	}

	SetWorkingImage();

// 初始化三个图标样式

	COLORREF cors[3] = {BGR(RGB(255, 228, 60)), BGR(RGB(65, 200, 255)), BGR(RGB(255, 176, 150))};
	for (int i = 0; i < 3; i++)
	{
		buttons[i][1] = buttons[i][0];

		buf = GetImageBuffer(&buttons[i][1]);
		for (int j = 0; j < buttons[i][1].getwidth() * buttons[i][1].getheight(); j++)
			if (buf[j] == BGR(RGB(60, 70, 85)))
				buf[j] = cors[i];
	}

}

void FormatScore(const string &autoplayScore)
{
	lineScore.clear();
	for (int i = 0; i < autoplayScore.size(); i++)
	{
		string line;
		switch (autoplayScore[i])
		{
			case '(':		// 数个键同时按下
			{
				for (; autoplayScore[i] != ')' && i < autoplayScore.size(); line += autoplayScore[i++]) {}
				line += autoplayScore[i];
				break;
			}
			case'[':	// 数个键顺次按下，但平分原节拍
			{
				int j = 0;
				line += autoplayScore[i];
				for (; autoplayScore[i + j] != ']'; j++)
				{
					if (autoplayScore[i + j] == '(')
					{
						for (; autoplayScore[i + j] != ')' &&
							   i + j < autoplayScore.size();
							   line += autoplayScore[i + j++])
							{}

						line += autoplayScore[i + j];
					}
					if (isalpha(autoplayScore[i + j]) || autoplayScore[i + j] == '-')
						line += autoplayScore[i + j];
				}
				line += autoplayScore[i = i + j];
				break;
			}
			case '-':		// 休止一个节拍
			{
				line += autoplayScore[i];
				break;
			}
			case '#':		// 调整节奏
			{
				line += autoplayScore[i];
				for (i++; isdigit(autoplayScore[i]) && i < autoplayScore.size(); line += autoplayScore[i++]) {}
				line += ' ';
				break;
			}
			default:
			{
				if (isalpha(autoplayScore[i]))
					line += autoplayScore[i];
			}
		}
		if (!line.empty())
			lineScore.push_back(line);
	}
}

void Option_Load()
{
	autoplaying = false;

	OPENFILENAME ofn;
	char szFile[260];
	HWND hwndOwner = nullptr;

// 初始化 ofn
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwndOwner;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Text(*.txt)\0*.TXT\0All(*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

// 显示加载窗口并加载文件
	if (GetOpenFileName(&ofn) == TRUE)
	{
		string autoplayScore = LoadTXT(szFile);
		score_ptr = 0;
		FormatScore(autoplayScore);
	}
}

void Option_Stop()
{
	autoplaying = false;
}

void Option_Start()
{
	if (!autoplaying)
	{
		autoplaying = !lineScore.empty();
		alarm = GetTickCount();
		displayBegin = 0;
	}
}

/*
 * 自动播放函数
 * 每次处理一组音符，而非单个！
 */
bool AutoPlay()
{
	if (GetTickCount64() < alarm && lineScore[score_ptr][0] != '[' )
		return false;

	string autoplayScore = lineScore[score_ptr++];
	autoplaying = score_ptr < lineScore.size();

	if (!autoplaying)
		score_ptr = 0;

	int i = 0;
	switch (autoplayScore[i])
	{
		case '(':		// 数个键同时按下
		{
			for (; autoplayScore[i] != ')' && i < autoplayScore.size(); i++)
				piano.PlayNote(autoplayScore[i]);
			alarm = GetTickCount64() + MsPerBeat;
			break;
		}
		case '[':	// 数个键顺次按下，但平分原节拍
		{
			int count = 0;
			for (int j = 0; autoplayScore[i + j] != ']' && i + j < autoplayScore.size(); j++)
			{
				if (autoplayScore[i + j] == '(')
				{
					count++;
					for (; autoplayScore[i + j] != ')' && i + j < autoplayScore.size(); j++);
				}
				if (isalpha(autoplayScore[i + j]) || autoplayScore[i + j] == '-')
					count++;
			}

			for (; autoplayScore[i] != ']'; i++)
			{
				alarm = GetTickCount64() + MsPerBeat / count;
				if (isalpha(autoplayScore[i]))
					piano.PlayNote(autoplayScore[i]);
				if (autoplayScore[i] == '(')
				{
					for (; autoplayScore[i] != ')' && i < autoplayScore.size(); i++)
						piano.PlayNote(autoplayScore[i]);
				}
				while (GetTickCount64() < alarm && (autoplayScore[i + 1] != ']' || count > 2))
					Sleep(1);
			}

			alarm = GetTickCount64() + MsPerBeat / count;
			break;
		}
		case '#':		// 调整节奏
		{
			MsPerBeat = 0;
			for (i++; isdigit(autoplayScore[i]) && i < autoplayScore.size(); i++)
				MsPerBeat = MsPerBeat * 10 + (autoplayScore[i] - '0');
			break;
		}
		case '-':		// 休止一个节拍
		{
			alarm += MsPerBeat;
			if (score_ptr + 1 < lineScore.size() && lineScore[score_ptr][0] == '[')
				alarm -= MsPerBeat * 2 / 3;
			break;
		}
		default:
		{
			if (isalpha(autoplayScore[i]))
			{
				piano.PlayNote(autoplayScore[i]);
				alarm = GetTickCount64() + MsPerBeat;
			}
		}
	}
	return true;
}

void DrawScore()
{
	constexpr int textX = 260, textY = 155;
	int p, lineSize = 0;
	int textOffsetX = 0;

// 对于第一行，特殊处理
	while (true)
	{
		scene3.Draw();

		lineSize = textOffsetX = 0;
		p = displayBegin;
		while (lineSize + lineScore[p].size() < maxBytePerLine && p < lineScore.size())
		{
			settextcolor(p < score_ptr ? RGB(255, 165, 145) : RGB(224, 223, 209));
			outtextxy(textX + textOffsetX, textY, lineScore[p].c_str());
			textOffsetX += static_cast<int>(lineScore[p].size() * 10);
			lineSize += static_cast<int>(lineScore[p++].size());
		}
		if (p < score_ptr)
			displayBegin = score_ptr - 1;
		else
			break;
	}
// 统一处理剩余 10 行
	settextcolor(RGB(224, 223, 209));
	for (int i = 1; i < 11 && p < lineScore.size(); i++)
	{
		if (i == 9)
			settextcolor(RGB(224 * 0.8 + 62 * 0.2, 223 * 0.8 + 69 * 0.2, 209 * 0.8 + 86 * 0.2));
		if (i == 10)
			settextcolor(RGB(224 * 0.5 + 62 * 0.5, 223 * 0.5 + 69 * 0.5, 209 * 0.5 + 86 * 0.5));
		lineSize = 0;
		textOffsetX = 0;
		while (lineSize + lineScore[p].size() < maxBytePerLine && p < lineScore.size())
		{
			outtextxy(textX + textOffsetX, textY + i * 20, lineScore[p].c_str());
			textOffsetX += static_cast<int>(lineScore[p].size() * 11);
			lineSize += static_cast<int>(lineScore[p++].size());
		}
	}
}

void Scene3_AutoPlay()
{

	IMAGE background, buttons_icon[3][2], lastScene, exit_icon[2];
	InitScene3(background, buttons_icon, exit_icon);

	getimage(&lastScene, 0, 0, 960, 540);

	putimage(0, 0, &background);
	FlushBatchDraw();

// 初始化场景3
	ExitButton exit_button;
	OptionButton_square option_button[3];
	function<void()> functions[3] = {Option_Start, Option_Stop, Option_Load};

	scene3.SetBackground(background);
	scene3.SetDstImg(nullptr);

	exit_button.set_ImgResting(exit_icon[0]);
	exit_button.set_ImgActivated(exit_icon[1]);
	exit_button.set_pos(714, 104);
	scene3.AddButton(&exit_button);

	for (int i = 0; i < 3; i++)
	{
		option_button[i].set_ImgResting(buttons_icon[i][0]);
		option_button[i].set_ImgActivated(buttons_icon[i][1]);
		option_button[i].set_pos(220 + i * 180, 402);
		option_button[i].Option = functions[i];
		scene3.AddButton(&option_button[i]);
	}

// 过渡到场景 3

	IMAGE newScene;
	scene3.Draw();
	getimage(&newScene, 0, 0, 960, 540);
	gradient(&lastScene, &newScene);

	while (scene3.Detect())
	{
		if (autoplaying)
			AutoPlay();
		if (!lineScore.empty())
			DrawScore();
		else
			scene3.Draw();
		FlushBatchDraw();
		if (!autoplaying)
			Sleep(1000 / 24);
		else
			Sleep(1);
	}

}
