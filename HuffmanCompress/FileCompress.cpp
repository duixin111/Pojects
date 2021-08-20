#define  _CRT_SECURE_NO_WARNINGS

#include <cstdio>

#include <iostream>

#include "FileCompress.h"
#include "HuffmanTree.hpp"
using namespace std;

FileCompress::FileCompress()
{
	// 1. ͳ���ַ�����Ƶ��
	for (int i = 0; i < 256; ++i)
	{
		fileByteInfo[i].ch = i;
	}
}

bool FileCompress::CompressFile(const string& filePath)     // ѹ��
{
	// 1. ͳ���ļ��и��ֽڳ��ִ���
	FILE* pf = fopen(filePath.c_str(), "rb");
	if (pf == nullptr)
	{
		cout << "���ļ�ʧ�ܣ�����" << endl;
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

	// 2. ����Huffman��
	HuffmanTree<ByteInfo> ht;
	ByteInfo invalid;
	ht.CreatHuffmanTree(fileByteInfo, 256, invalid);

	// 3. ��ȡ����
	GenerateHuffmanCode(ht.GetRoot());

	FILE* fOut = fopen("222.hzp", "wb");

	// �����ļ���ָ��λ��
	fseek(pf, 0, SEEK_SET);

	// 4. д��ѹ��ʱ��Ҫ��������Ϣ
	WriteHead(fOut, filePath);	

	// 5. ��Huffman������ļ�����ѹ��
	if (fOut == nullptr)
	{
		perror("�ļ�����ʧ�ܣ�����");
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

			// ת��
			for (size_t j = 0; j < strCode.size(); ++j)
			{
				ch <<= 1;    //��λ��������λ��0
				if (strCode[j] == '1')
					ch |= 1;
				// ��ch��8λд���,�Ѹ��ֽ���Ϣд��ѹ���ļ���
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

bool FileCompress::UNCompressFile(const string& filePath)   // ��ѹ��
{
	// 1. ��ѹ���ļ��ж�ȡ��ѹ��ʱ��Ҫ����Ϣ
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (fIn == nullptr)
	{
		perror("��ѹ���ļ�ʧ�ܣ�����");
		return false;
	}

	// ��ȡԴ�ļ���׺
	string postFix;
	GetLine(fIn, postFix);

	// Ƶ����Ϣ������
	string strContent;
	GetLine(fIn, strContent);
	int lineCount = atoi(strContent.c_str());

	// ѭ����ȡlineCount�У���ȡ�ֽ�Ƶ����Ϣ
	strContent = "";
	for (int i = 0; i < lineCount; ++i)
	{
		GetLine(fIn, strContent);
		if (strContent == "")
		{
			// ��ȡ������"\n"
			strContent += "\n";
			GetLine(fIn, strContent);
		}
		// fileByteInfo[strContent[0]].ch = strContent[0];
		fileByteInfo[strContent[0]].appearCount = atoi(strContent.c_str() + 2);
		strContent = "";
	}

	// 2. �ָ�Huffman��
	HuffmanTree<ByteInfo> ht;
	ByteInfo invalid;
	ht.CreatHuffmanTree(fileByteInfo, 256, invalid);

	// 3.��ȡѹ�����ݣ����Huffman�����н�ѹ��
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
					
					// ����ɹ���ѹ���ֽڸ�����Դ�ļ���С��ͬ���ѹ������
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
	// 1. ��ȡԴ�ļ���׺
	string postFix = filePath.substr(filePath.rfind('.'));
	postFix += "\n";
	fwrite(postFix.c_str(), 1, postFix.size(), fOut);

	// 2. ͳ��
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

	// 3. д��������Ƶ����Ϣ
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

