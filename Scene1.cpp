//
// Created by Reaky on 2025/5/25.
//
/*
 * Scene1：弹奏场景，包括 21 个可被鼠标和键盘触发的音符和两个仅可被鼠标触发的升降调、自动播放按钮。
 * 当升降调或自动播放按钮被触发时，将进入 Scene2 和 Scene3.
 */

#include <conio.h>
#include <graphics.h>

#include "Piano.hpp"
#include "button.hpp"
#include "Scene.hpp"
#include "Scene2.hpp"
#include "Scene3.hpp"

using namespace std;

extern Piano piano;

/**
 * @brief 初始化音符图标
 * @param notes 存储音符图标的 IMAGE 数组
 */
void InitNote(IMAGE *notes)
{
    IMAGE note_icon;
    loadimage(&note_icon, _T("IMAGE"), _T("Notes.png"));
    DWORD *note_iconBuffer = GetImageBuffer(&note_icon);
    for (int i = 0; i < note_icon.getwidth() * note_icon.getheight(); ++i)
        note_iconBuffer[i] &= WHITE;

    for (int i = 0; i < 7; i++)
    {
        const char *str[7] = {"do", "re", "mi", "fa", "so", "la", "ti"};
        // 绘制音符背景
        notes[i].Resize(60, 60);
        SetWorkingImage(notes + i);

        DWORD *buffer = GetImageBuffer(notes + i);
        fill_n(buffer, 60 * 60, 0xff000000);

        setlinestyle(PS_SOLID, 1);
        setlinecolor(RGB(211, 211, 196));
        setfillcolor(RGB(211, 211, 196));
        fillcircle(30, 30, 30);

        setlinecolor(RGB(224, 223, 209));
        setfillcolor(RGB(224, 223, 209));
        fillcircle(30, 30, 25);

        // 绘制音符图标
        IMAGE tmp;
        tmp.Resize(27, 24);
        SetWorkingImage(&tmp);

        putimage(-i * 27, 0, &note_icon);

        SetWorkingImage(notes + i);
        putimage(17, 18, &tmp);

        settextstyle(12, 6, "宋体");
        settextcolor(RGB(150, 168, 175));
        setbkmode(TRANSPARENT);

        outtextxy(25, 43, str[i]);
        settextcolor(RGB(150, 148, 120));
        outtextxy(24, 42, str[i]);
    }
    SetWorkingImage();
}

/**
 * @brief 初始化场景1的所有图片资源
 * @param background 背景图片
 * @param notes_resting 静息状态音符图片数组
 * @param notes_activated 激活状态音符图片数组
 * @param pitchShift 升降调按钮图片数组
 * @param autoPlay 自动播放按钮图片数组
 */
void InitScene1(
    IMAGE &background,
    IMAGE notes_resting[7],
    IMAGE notes_activated[7],
    IMAGE pitchShift[2],
    IMAGE autoPlay[2]
)
{
    // 初始化背景
    IMAGE sign, sign_single, lines;
    loadimage(&background, "IMAGE", "Background.jpg");
    loadimage(&sign, "IMAGE", "Sign.png");

    DWORD *buffer = GetImageBuffer(&sign);
    for (int i = 0; i < sign.getheight() * sign.getwidth(); ++i)
        if ((buffer[i] & WHITE) == BLACK)
            buffer[i] = 0xffffffff;
        else
            buffer[i] &= WHITE;

    sign_single.Resize(29, 50);
    for (int i = 0; i < 3; ++i)
    {
        DWORD *sign_buf = GetImageBuffer(&sign_single);
        for (int y = 0, di = 0; y < 50; y++)
            for (int x = 0; x < 29; x++, di++)
                sign_buf[di] = buffer[y * 87 + x + i * 29];
        TransparentImage(&background, 161, 272 + i * 90, &sign_single);
    }

    lines.Resize(background.getwidth(), background.getheight());
    buffer = GetImageBuffer(&lines);
    memset(buffer, 0xff, background.getwidth() * background.getheight() * sizeof(DWORD));
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 5; j++)
        {
            for (int k = 205; k < 825; k++)
                buffer[k + 960 * (i * 86 + j * 8 + 275)] = WHITE | (0xaf << 24);
            for (int k = 205; k < 825; k++)
                buffer[k + 960 * (i * 86 + j * 8 + 275) + 960] = WHITE | (0xcf << 24);
        }
    TransparentImage(&background, 0, 0, &lines);

    // 初始化音符图标
    InitNote(notes_resting);

    for (int i = 0; i < 7; i++)
    {
        notes_activated[i].Resize(60, 60);
        DWORD *src = GetImageBuffer(&notes_resting[i]);
        DWORD *dst = GetImageBuffer(&notes_activated[i]);
        for (int j = 0; j < 60 * 60; j++)
            if ((src[j] & 0xff000000) == 0xff000000)
                dst[j] = src[j];
            else if (src[j] == BGR(RGB(224, 223, 209)))
                dst[j] = BGR(RGB(144, 249, 227));
            else if (src[j] == BGR(RGB(211, 211, 196)))
                dst[j] = BGR(RGB(144 * 0.8, 249 * 0.8, 227 * 0.8));
            else
                dst[j] = BGR(RGB(224, 223, 209));
    }

    // 绘制升降调图标和自动播放图标的基底
    pitchShift[0].Resize(40, 40);
    SetWorkingImage(&pitchShift[0]);
    buffer = GetImageBuffer(&pitchShift[0]);
    fill_n(buffer, 40 * 40, 0xff000000);

    setlinestyle(PS_SOLID, 1);
    setlinecolor(RGB(211, 211, 196));
    setfillcolor(RGB(211, 211, 196));
    fillcircle(20, 20, 20);

    setlinecolor(RGB(224, 223, 209));
    setfillcolor(RGB(224, 223, 209));
    fillcircle(20, 20, 12);

    setlinecolor(RGB(200, 200, 200));
    circle(20, 20, 12);

    autoPlay[0] = pitchShift[0];

    // 绘制升降调符号
    SetWorkingImage(&pitchShift[0]);
    setlinestyle(PS_SOLID, 2);
    setlinecolor(RGB(88, 153, 144));
    line(16, 12, 16, 28);
    line(15, 12, 12, 15);
    line(16, 12, 19, 15);
    line(24, 12, 24, 28);
    line(23, 28, 20, 25);
    line(24, 28, 27, 25);

    // 绘制自动播放图标
    SetWorkingImage(&autoPlay[0]);
    setlinestyle(PS_SOLID, 2);
    setlinecolor(RGB(88, 153, 144));
    line(14, 12, 28, 20);
    line(28, 20, 14, 28);
    line(14, 12, 14, 28);

    SetWorkingImage();

    // 绘制升降调、自动播放图标的激活状态
    pitchShift[1] = pitchShift[0];
    autoPlay[1] = autoPlay[0];
    IMAGE *p[2] = {&pitchShift[1], &autoPlay[1]};

    for (IMAGE *&k : p)
    {
        buffer = GetImageBuffer(k);
        for (int i = 0; i < 40 * 40; i++)
        {
            if ((buffer[i] & 0xff000000) != 0xff000000)
            {
                float H, S, L;
                RGBtoHSL(BGR(buffer[i]), &H, &S, &L);
                if (buffer[i] == BGR(RGB(88, 153, 144)))
                    L += 0.2;
                else
                    L -= 0.1;
                buffer[i] = BGR(HSLtoRGB(H, S, L));
                buffer[i] |= 16 << 24;
            }
        }
    }
}

/**
 * @brief 场景1主循环，处理弹奏、升降调、自动播放等交互
 * @note 该函数不会返回
 */
[[noreturn]] void Scene1()
{
    constexpr char note_chtab[22] = {"QWERTYUASDFGHJZXCVBNM"};
    IMAGE background, notes_resting[7], notes_activated[7];
    IMAGE pitchShift[2], autoPlay[2];

    NoteButton notes_button[21];
    OptionButton pitchShift_button, autoPlay_button;
    Scene scene1;

    // 初始化图片、按钮
    InitScene1(
        background,
        notes_resting,
        notes_activated,
        pitchShift,
        autoPlay
    );

    scene1.SetBackground(background);
    scene1.SetDstImg(nullptr);

    for (int i = 0; i < 21; i++)
    {
        notes_button[i].set_ImgResting(notes_resting[i % 7]);
        notes_button[i].set_ImgActivated(notes_activated[i % 7]);
        notes_button[i].set_pos(225 + 87 * (i % 7), 260 + 88 * (i / 7));
        notes_button[i].piano = &piano;
        notes_button[i].note = note_chtab[i];
        scene1.AddButton(notes_button + i);
    }

    pitchShift_button.set_pos(830, 30);
    pitchShift_button.set_ImgActivated(pitchShift[1]);
    pitchShift_button.set_ImgResting(pitchShift[0]);
    pitchShift_button.Option = Scene2_PitchShift;

    autoPlay_button.set_pos(890, 30);
    autoPlay_button.set_ImgActivated(autoPlay[1]);
    autoPlay_button.set_ImgResting(autoPlay[0]);
    autoPlay_button.Option = Scene3_AutoPlay;

    scene1.AddButton(&pitchShift_button);
    scene1.AddButton(&autoPlay_button);

    BeginBatchDraw();
    while (true)
    {
        scene1.Detect();
        scene1.Draw();
        FlushBatchDraw();
        Sleep(1000 / 25);
    }
}