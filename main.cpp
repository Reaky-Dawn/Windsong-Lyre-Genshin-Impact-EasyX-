#include <conio.h>
#include <graphics.h>
#include <vector>
#include <thread>
#include <random>

#include "Piano.hpp"
#include "button.hpp"

#include "Scene1.hpp"

#define QWORD unsigned long long

using namespace std;

Piano piano;
thread pianoLoadShift;


void InitPiano(Piano &p)
{
	vector<string> files;
	files.emplace_back("audio\\1-.mp3.wav");
	files.emplace_back("audio\\2-.mp3.wav");
	files.emplace_back("audio\\3-.mp3.wav");
	files.emplace_back("audio\\4-.mp3.wav");
	files.emplace_back("audio\\5-.mp3.wav");
	files.emplace_back("audio\\6-.mp3.wav");
	files.emplace_back("audio\\7-.mp3.wav");
	files.emplace_back("audio\\1.mp3.wav");
	files.emplace_back("audio\\2.mp3.wav");
	files.emplace_back("audio\\3.mp3.wav");
	files.emplace_back("audio\\4.mp3.wav");
	files.emplace_back("audio\\5.mp3.wav");
	files.emplace_back("audio\\6.mp3.wav");
	files.emplace_back("audio\\7.mp3.wav");
	files.emplace_back("audio\\1+.mp3.wav");
	files.emplace_back("audio\\2+.mp3.wav");
	files.emplace_back("audio\\3+.mp3.wav");
	files.emplace_back("audio\\4+.mp3.wav");
	files.emplace_back("audio\\5+.mp3.wav");
	files.emplace_back("audio\\6+.mp3.wav");
	files.emplace_back("audio\\7+.mp3.wav");

	p.LoadNotes(files);

	thread th1(Piano::LoadShift, &piano, files);
	pianoLoadShift.swap(th1);

}

void PlayAnimat()
{
	struct Point
	{
		int x, y;
		int vx, vy;
		COLORREF cor;
	};

	IMAGE temp, img;
	int move[4][2] = {{-1, 0}, {1, 0}, {0, 1}, {0, -1}};

	setbkcolor(WHITE);
	cleardevice();

	// 生成随机数表
	mt19937_64 mt_rand(0x796B616552);// Reaky!!

	int *rand_tab = new int [50 * 960 * 540];
	for (int i = 0; i < 50 * 960 * 540; i++)
		rand_tab[i] = static_cast<int>(mt_rand() % 4);

	// 初始化图片
	temp.Resize(960, 540);
	loadimage(&img, _T("IMAGE"), _T("Reaky.png"));

	SetWorkingImage(&temp);

	setbkcolor(WHITE);
	cleardevice();
	putimage(330, 171, &img);

	// 计算初始帧
	auto points = new Point[960 * 540];
	DWORD *buf = GetImageBuffer(&temp);
	for (int i = 0, p = 0; i < 960; i++)
		for (int j = 0; j < 540; j++, p++)
			points[p].x = i, points[p].y = j, points[p].cor = buf[j * 960 + i], points[p].vx = points[p].vy = 0;

	for (int i = 0, p = 25 * 960 * 540 - 1; i < 25; i++)
		for (int j = 960 * 540 - 1; j >= 0; j--, p--)
		{
			int mov = rand_tab[p];
			points[j].x += points[j].vx;
			points[j].y += points[j].vy;
			points[j].vx += move[mov][0] * 2;
			points[j].vy += move[mov][1];
		}

	SetWorkingImage();
	buf = GetImageBuffer();

	// 绘制
	BeginBatchDraw();
	ExMessage msg{};

	for (int i = 0, p = 0; i < 25; i++)
	{
		for (int j = 0; j < 960 * 540; j++, p++)
		{
			int mov =  rand_tab[p];
			points[j].vx -= move[mov][0] * 2;
			points[j].vy -= move[mov][1];
			points[j].x -= points[j].vx;
			points[j].y -= points[j].vy;

			if (points[j].x >= 0 && points[j].x < 960 && points[j].y >= 0 && points[j].y < 540)
				buf[points[j].x + points[j].y * 960] = points[j].cor;

			if (p % 5000 == 0 && peekmessage(&msg))
				if (msg.message == WM_KEYDOWN || msg.message == WM_LBUTTONDOWN)
				{
					delete[] points;
					delete[] rand_tab;
					return;
				}
		}
		Sleep(1);
		FlushBatchDraw();
	}

	QWORD alarm = GetTickCount64() + 1500;
	while (GetTickCount64() < alarm)
	{
		Sleep(5);
		if (peekmessage(&msg))
			if (msg.message == WM_KEYDOWN || msg.message == WM_LBUTTONDOWN)
			{
				delete[] points;
				delete[] rand_tab;
				return;
			}
	}

	for (int i = 0, p = 0; i < 25; i++)
	{
		for (int j = 0; j < 960 * 540; j++, p++)
		{
			int mov =  rand_tab[p];
			points[j].vx -= move[mov][0] * 2;
			points[j].vy -= move[mov][1];
			points[j].x -= points[j].vx;
			points[j].y -= points[j].vy;

			if (points[j].x >= 0 && points[j].x < 960 && points[j].y >= 0 && points[j].y < 540)
				buf[points[j].x + points[j].y * 960] = points[j].cor;
			if (p % 5000 == 0 && peekmessage(&msg))
				if (msg.message == WM_KEYDOWN || msg.message == WM_LBUTTONDOWN)
				{
					delete[] points;
					delete[] rand_tab;
					return;
				}
		}
		Sleep(1);
		FlushBatchDraw();
	}
	delete[] points;
	delete[] rand_tab;
}

int main()
{
	InitPiano(piano);

	HWND hwnd = initgraph(960, 540);

// 设置鼠标样式
	auto hcur = static_cast<HCURSOR>(LoadImage(GetModuleHandle(nullptr), "Cursor.cur", IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE));
	SetClassLongPtr(hwnd, GCLP_HCURSOR, reinterpret_cast<long long>(hcur));

// 播放开场动画
	PlayAnimat();

// 进入场景1
	Scene1();
}
