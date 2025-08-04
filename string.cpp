//
// Created by Reaky on 2025/6/14.
//

#include <iostream>
#include <windows.h>

using namespace std;

/**
 * @brief 将UTF8编码字符串转换为ANSI编码
 * @param strUTF8 UTF8编码的字符串
 * @return ANSI编码的字符串
 * @author CSDN博主「元小旦」
 */
string UTF8toAnsi(string strUTF8)
{
	UINT nLen = MultiByteToWideChar(
		CP_UTF8, 0,
		strUTF8.c_str(), -1,
		nullptr, 0
	);
	WCHAR *wszBuffer = new WCHAR[nLen + 1];
	CHAR *szBuffer = new CHAR[nLen + 1];

	nLen = MultiByteToWideChar(
		CP_UTF8, 0,
		strUTF8.c_str(), -1,
		wszBuffer, nLen
	);
	wszBuffer[nLen] = 0;

	nLen = WideCharToMultiByte(
		936, 0,
		wszBuffer, -1,
		nullptr, 0,
		nullptr, nullptr
	);

	nLen = WideCharToMultiByte(
		936, 0,
		wszBuffer, -1,
		szBuffer, nLen,
		nullptr, nullptr
	);
	szBuffer[nLen] = 0;
	strUTF8 = szBuffer;

	delete[] szBuffer;
	delete[] wszBuffer;
	return strUTF8;
}

/**
 * @brief 读取文本文件内容
 * @param f 文件路径
 * @return 文件内容字符串
 */
string LoadTXT(const char *f)
{
	string ret;
	FILE *rd = fopen(f, "r");
	if (!rd)
		return string();
	char ch;
	while (fread(&ch, 1, 1, rd))
		ret.push_back(ch);
	return ret;
}