#pragma once

#include <iostream>
#include <vector>
#include <queue>

using namespace std;

template<class W>
struct HuffmanTreeNode
{
	HuffmanTreeNode<W>* left;
	HuffmanTreeNode<W>* right;
	HuffmanTreeNode<W>* parent;
	W weight;

	HuffmanTreeNode(const W& w)
		: left(nullptr)
		, right(nullptr)
		, parent(nullptr)
		, weight(w)
	{}

};

template<class W>
struct Com
{
	typedef HuffmanTreeNode<W> Node;

	bool operator()(const Node* left, const Node* right)
	{
		return left->weight > right->weight;
	}
};

template<class W>
class HuffmanTree
{
	typedef HuffmanTreeNode<W> Node;
public:
	HuffmanTree()
		: root(nullptr)
	{}

	~HuffmanTree()
	{
		Destroy(root);
	}

	void CreatHuffmanTree(const W array[], size_t size, const W& invalid)
	{
		// 小堆
		priority_queue<Node*, vector<Node*>, Com<W>> q;
		// 1. 创建只有根节点的森林
		for (size_t i = 0; i < size; ++i)
		{
			if(array[i] != invalid)
				q.push(new Node(array[i]));
		}

		// 2. 循环进行以下步骤
		while (q.size() > 1)
		{
			// 取权值最小两棵树
			Node* left = q.top();
			q.pop();

			Node* right = q.top();
			q.pop();

			Node* parent = new Node(left->weight + right->weight);
			parent->left = left;
			parent->right = right;
			left->parent = parent;
			right->parent = parent;

			// 将新创建的二叉树插入到二叉树
			q.push(parent);
		}
		root = q.top();
	}

	Node* GetRoot()
	{
		return root;
	}

	void Destroy(Node*& proot)
	{
		if (proot)
		{
			Destroy(proot->left);
			Destroy(proot->right);
			delete proot;
			proot = nullptr;
		}
	}

private:
	Node* root;
};
