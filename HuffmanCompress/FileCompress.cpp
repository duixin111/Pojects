#define  _CRT_SECURE_NO_WARNINGS

#include <cstdio>

#include <iostream>

#include "FileCompress.h"
#include "HuffmanTree.hpp"
using namespace std;

FileCompress::FileCompress()
{
	// 1. 统计字符出现频率
	for (int i = 0; i < 256; ++i)
	{
		fileByteInfo[i].ch = i;
	}
}

bool FileCompress::CompressFile(const string& filePath)     // 压缩
{
	// 1. 统计文件中各字节出现次数
	FILE* pf = fopen(filePath.c_str(), "rb");
	if (pf == nullptr)
	{
		cout << "打开文件失败！！！" << endl;
		return false;
	}

	un_ch readBuff[1024];
	while (true)
	{
		size_t rdSize = fread(readBuff, 1, 1024, pf);
		if (rdSize == 0)
			break;
		for (size_t i = 0; i < rdSize; ++i)
		{
			fileByteInfo[readBuff[i]].appearCount++;
		}

	}

	// 2. 构建Huffman树
	HuffmanTree<ByteInfo> ht;
	ByteInfo invalid;
	ht.CreatHuffmanTree(fileByteInfo, 256, invalid);

	// 3. 获取编码
	GenerateHuffmanCode(ht.GetRoot());

	FILE* fOut = fopen("222.hzp", "wb");

	// 设置文件流指针位置
	fseek(pf, 0, SEEK_SET);

	// 4. 写解压缩时所要包含的信息
	WriteHead(fOut, filePath);	

	// 5. 用Huffman编码对文件进行压缩
	if (fOut == nullptr)
	{
		perror("文件创建失败！！！");
		return false;
	}

	un_ch ch = 0;
	size_t bitCount = 0;
	while(true)
	{
		size_t rdSize = fread(readBuff, 1, 1024, pf);
		if (rdSize == 0)
			break;
		for (size_t i = 0; i < rdSize; ++i)
		{
			// 
			string& strCode = fileByteInfo[readBuff[i]].strCode;

			// 转换
			for (size_t j = 0; j < strCode.size(); ++j)
			{
				ch <<= 1;    //高位丢弃，低位补0
				if (strCode[j] == '1')
					ch |= 1;
				// 当ch的8位写完后,把该字节信息写入压缩文件中
				++bitCount;
				if (bitCount == 8)
				{
					fputc(ch, fOut);
					bitCount = 0;
				}
			}
		}
	}
	if (bitCount > 0 && bitCount < 8)
	{
		ch <<= (8 - bitCount);
		fputc(ch, fOut);
	}
	 

	fclose(pf);
	fclose(fOut);
	return true;
}

bool FileCompress::UNCompressFile(const string& filePath)   // 解压缩
{
	// 1. 从压缩文件中读取解压缩时需要的信息
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (fIn == nullptr)
	{
		perror("打开压缩文件失败！！！");
		return false;
	}

	// 获取源文件后缀
	string postFix;
	GetLine(fIn, postFix);

	// 频次信息总行数
	string strContent;
	GetLine(fIn, strContent);
	int lineCount = atoi(strContent.c_str());

	// 循环读取lineCount行，获取字节频次信息
	strContent = "";
	for (int i = 0; i < lineCount; ++i)
	{
		GetLine(fIn, strContent);
		if (strContent == "")
		{
			// 读取到的是"\n"
			strContent += "\n";
			GetLine(fIn, strContent);
		}
		// fileByteInfo[strContent[0]].ch = strContent[0];
		fileByteInfo[strContent[0]].appearCount = atoi(strContent.c_str() + 2);
		strContent = "";
	}

	// 2. 恢复Huffman树
	HuffmanTree<ByteInfo> ht;
	ByteInfo invalid;
	ht.CreatHuffmanTree(fileByteInfo, 256, invalid);

	// 3.读取压缩数据，结合Huffman树进行解压缩
	string filename("333");
	filename += postFix;
	FILE* fOut = fopen(filename.c_str(), "wb");

	un_ch readBuffer[1024];
	un_ch bitCount = 0;
	HuffmanTreeNode<ByteInfo>* cur = ht.GetRoot();

	const size_t fileSize = cur->weight.appearCount;
	size_t compressSize = 0;

	while (true)
	{
		size_t rdSize = fread(readBuffer, 1, 1024, fIn);
		if (rdSize == 0)
			break;

		for (size_t i = 0; i < rdSize; ++i)
		{
			un_ch ch = readBuffer[i];
			bitCount = 0;
			while (bitCount < 8)
			{
				if (ch & 0x80)
					cur = cur->right;
				else
					cur = cur->left;

				if (cur->left == nullptr && cur->right == nullptr)
				{
					fputc(cur->weight.ch, fOut);
					cur = ht.GetRoot();
					++compressSize;
					
					// 如果成功解压缩字节个数与源文件大小相同则解压缩结束
					if (compressSize == fileSize)
						break;
				}

				++bitCount;
				ch <<= 1;
			}
		}
	}


	fclose(fIn);
	fclose(fOut);

	return true;
}

void FileCompress::GetLine(FILE* fIn, string& strContent)
{
	un_ch ch;
	while (!feof(fIn))
	{
		ch = fgetc(fIn);
		if (ch == '\n')
			break;

		strContent += ch;
	}
}

void FileCompress::WriteHead(FILE* fOut, const string& filePath)
{
	// 1. 获取源文件后缀
	string postFix = filePath.substr(filePath.rfind('.'));
	postFix += "\n";
	fwrite(postFix.c_str(), 1, postFix.size(), fOut);

	// 2. 统计
	string chAppearCount;
	int lineCount = 0;
	for (int i = 0; i < 256; ++i)
	{
		if (fileByteInfo[i].appearCount > 0)
		{
			chAppearCount += fileByteInfo[i].ch;
			chAppearCount += ':';
			chAppearCount += to_string(fileByteInfo[i].appearCount);
			chAppearCount += "\n";
			++lineCount;
		}
	}

	// 3. 写总行数及频次信息
	string totalLine = to_string(lineCount);
	totalLine += "\n";
	fwrite(totalLine.c_str(), 1, totalLine.size(), fOut);
	fwrite(chAppearCount.c_str(), 1, chAppearCount.size(), fOut);
}

void FileCompress::GenerateHuffmanCode(HuffmanTreeNode<ByteInfo>* root)
{
	if (root == nullptr)
		return;
	if (root->left == nullptr && root->right == nullptr)
	{
		HuffmanTreeNode<ByteInfo>* cur = root;
		HuffmanTreeNode<ByteInfo>* parent = cur->parent;
		string& strCode = fileByteInfo[cur->weight.ch].strCode;
		while (parent)
		{
			if (parent->left == cur)
				strCode += '0';
			else
				strCode += '1';
			cur = parent;
			parent = cur->parent;
		}
		reverse(strCode.begin(), strCode.end());
	}

	GenerateHuffmanCode(root->left);
	GenerateHuffmanCode(root->right);
}

