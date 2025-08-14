# Windsong-Lyre-Genshin-Impact-EasyX-
【原神】原神风物之诗琴仿真（EasyX）

## 程序简介

此程序仿制了原神的风物之诗琴。没有使用除 EasyX 以外的任何图形库或是游戏引擎。
编译环境：CLion2025.1 + EasyX for TDM-GCC 2025-01-17
此程序一定程度上仿照了 bilibili 上的网页版模拟 WindsongLyre-Sim 同时弥补了变调等功能需在部署本地服务器的前提下才能使用的缺陷。


## 游戏操作


按下对应按键以弹奏音符，键位和原神键位相同。按下非字母键，停止所有音符。画面右上角的两个按钮，分别是变调和自动播放功能。以下是自动播放乐谱的格式：
#xxx 表示将音符间停顿改为 xxx 毫秒
[xxx] 表示这几个音符依次按下，但平分原有停顿时间
(xxx) 表示这几个音符同时按下
-- 表示休止，停顿但不按下音符

当然，也可以结合风物之诗琴的 midi 播放工具：[【原神】自动演奏风物之诗琴的程序](https://github.com/luern0313/WindSong-Lyre-Genshin-Impact)

## 需注意的

1. Bilibili 上的部分原琴谱不符合此格式，若直接播放可能出现节奏问题。
2. 当出现弹奏时漏音的情况，是因为部分键盘无法识别多个键同时按下而非程序问题。可以换一个键盘或是使用自动播放功能。

## 如何编译

1. 将源代码下载到没有中文的路径下，防止 CLion 无法准确链接库文件
2. 按照 [CodeBus](https://codebus.cn/bestans/easyx-for-mingw) 上的教程将 EasyX 库安装到 CLion（如已安装请跳过）
3. 打开 CLion，使用默认配置的配置（Release 或 Debug 都可）
4. 编译后将源码路径下的 audio 文件夹复制，粘贴在与编译出的 WindsongLyre.exe 同一目录下
5. 双击 WindsongLyre.exe 即可运行

