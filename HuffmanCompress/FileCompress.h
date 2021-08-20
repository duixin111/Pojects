#pragma once
#include <iostream>
#include <string>

#include "HuffmanTree.hpp"
using namespace std;

typedef unsigned char un_ch;

struct ByteInfo
{
	un_ch ch;
	size_t appearCount;   // ch 字符出现次数
	string strCode;    // ch对应编码

	ByteInfo(size_t count = 0)
		: appearCount(count)
	{}



	ByteInfo operator+(const ByteInfo& b)const
	{
		ByteInfo tmp;
		tmp.appearCount = appearCount + b.appearCount;
		return tmp;
	}
	bool operator>(const ByteInfo& b)const
	{
		return appearCount > b.appearCount;
	}

	bool operator==(const ByteInfo& b)const
	{
		return appearCount == b.appearCount;
	}
	bool operator!=(const ByteInfo& b)const
	{
		return appearCount != b.appearCount;
	}
};
 
class FileCompress
{
public:
	FileCompress();     // 构造
	bool CompressFile(const string& filePath);     // 压缩
	bool UNCompressFile(const string& filePath);   // 解压缩

private:
	void GenerateHuffmanCode(HuffmanTreeNode<ByteInfo>* root);
	void WriteHead(FILE* fOut, const string& filePath);
	void GetLine(FILE* fIn, string& strContent);

	ByteInfo fileByteInfo[256];
};