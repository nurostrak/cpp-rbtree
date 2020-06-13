#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <new>

typedef unsigned char uint8;
typedef unsigned int uint32;

template<typename K, typename I>
class RedBlackTree
{
public:
	const static uint8 Red = 0;
	const static uint8 Black = 1;

	struct Node
	{
		uint8 Color;
		Node* Parent;
		Node* Left;
		Node* Right;
		K Key;
		I Item;
	};

	Node* Root;
	uint32 NodeCount;

	explicit RedBlackTree()
	{
		Root = nullptr;
		NodeCount = 0;
	}

	~RedBlackTree()
	{
		DestructInternal(Root);
	}

	Node* FindNode(const K& pKey, Node*& pLastNode)
	{
		if (Root == nullptr)
		{
			pLastNode = nullptr;
			return nullptr;
		}

		pLastNode = Root;

		while (true)
		{
			if (pKey < pLastNode->Key)
			{
				if (pLastNode->Left == nullptr)
				{
					return nullptr;
				}

				pLastNode = pLastNode->Left;
			}
			else if (pKey > pLastNode->Key)
			{
				if (pLastNode->Right == nullptr)
				{
					return nullptr;
				}

				pLastNode = pLastNode->Right;
			}
			else
			{
				assert(pKey == pLastNode->Key);
				return pLastNode;
			}
		}
	}

	Node* FindNode(const K& pKey)
	{
		Node* parentNode;
		return FindNode(pKey, parentNode);
	}

	I* Find(const K& pKey)
	{
		Node* foundNode;
		foundNode = FindNode(pKey);
		if (foundNode == nullptr)
		{
			return nullptr;
		}

		return &foundNode->I;
	}

	I* PtrInsert(const K& pKey)
	{
		if (Root == nullptr)
		{
			assert(NodeCount == 0);

			Node* newNode = static_cast<Node*>(malloc(sizeof(Node)));
			newNode->Color = Black;
			newNode->Parent = nullptr;
			newNode->Left = nullptr;
			newNode->Right = nullptr;
			newNode->Key = pKey;

			Root = newNode;
			NodeCount = 1;
			return &newNode->Item;
		}

		// Find parent
		Node* parentNode;
		if (FindNode(pKey, parentNode) != nullptr)
		{
			return nullptr; // Item already exists
		}

		// Insert node
		Node* newNode = static_cast<Node*>(malloc(sizeof(Node)));
		newNode->Color = Red;
		newNode->Parent = parentNode;
		newNode->Left = nullptr;
		newNode->Right = nullptr;
		newNode->Key = pKey;
		// newNode->Item is un-initialized, waiting to be initialized by in-place new.
		if (pKey > parentNode->Key)
		{
			assert(parentNode->Right == nullptr);
			parentNode->Right = newNode;
		}
		else
		{
			assert(parentNode->Key != pKey);
			assert(parentNode->Left == nullptr);
			parentNode->Left = newNode;
		}

		// Fix tree
		Node* curNode = newNode;
		while (curNode != Root && curNode->Parent->Color == Red)
		{
			if (curNode->Parent == curNode->Parent->Parent->Left)
			{
				Node* temp = curNode->Parent->Parent->Right;
				if (temp != nullptr && temp->Color == Red)
				{
					curNode->Parent->Color = Black;
					temp->Color = Black;
					curNode->Parent->Parent->Color = Red;
					curNode = curNode->Parent->Parent;
				}
				else
				{
					if (curNode == curNode->Parent->Right)
					{
						curNode = curNode->Parent;
						LeftRotate(curNode);
					}
					curNode->Parent->Color = Black;
					curNode->Parent->Parent->Color = Red;
					RightRotate(curNode->Parent->Parent);
				}
			}
			else
			{
				Node* temp = curNode->Parent->Parent->Left;
				if (temp != nullptr && temp->Color == Red)
				{
					curNode->Parent->Color = Black;
					temp->Color = Black;
					curNode->Parent->Parent->Color = Red;
					curNode = curNode->Parent->Parent;
				}
				else
				{
					if (curNode == curNode->Parent->Left)
					{
						curNode = curNode->Parent;
						RightRotate(curNode);
					}
					curNode->Parent->Color = Black;
					curNode->Parent->Parent->Color = Red;
					LeftRotate(curNode->Parent->Parent);
				}
			}
		}

		Root->Color = Black;
		NodeCount++;
		return &newNode->Item;
	}

	bool Insert(const K& pKey, const I& pItem)
	{
		I* newItem = PtrInsert(pKey);
		if (newItem == nullptr)
		{
			return false;
		}

		new(newItem) I(pItem);
		return true;
	}

	bool Delete(const K& pKey)
	{
		Node* target = FindNode(pKey);
		if (target == nullptr)
		{
			return false;
		}

		if (target->Left != nullptr && target->Right != nullptr)
		{
			Node* pred = target->Left;
			while (pred->Right != nullptr)
			{
				pred = pred->Right;
			}

			SwapNodes(target, pred); // Turns target into an end node
		}

		assert(target->Left == nullptr || target->Right == nullptr);

		Node* child = target->Right != nullptr ? target->Right : target->Left;
		if (target->Color == Black)
		{
			target->Color = child != nullptr ? child->Color : Black;
			DeleteCase1(target);
		}

		// Replacing target with child
		if (target->Parent == nullptr)
		{
			Root = child;
			if (child != nullptr)
			{
				child->Parent = nullptr;
				Root->Color = Black;
			}
		}
		else
		{
			if (target == target->Parent->Left)
			{
				target->Parent->Left = child;
			}
			else
			{
				assert(target == target->Parent->Right);
				target->Parent->Right = child;
			}
			if (child != nullptr)
			{
				child->Parent = target->Parent;
			}
		}

		free(target);
		NodeCount--;
		return true;
	}

	void Validate() const
	{
		uint32 nodeCount = 0;

		if (Root != nullptr)
		{
			assert(Root->Color == Black);

			uint32 pathLength = 0;
			ValidateInternal(Root, 0, pathLength, nodeCount);
		}

		assert(NodeCount == nodeCount);
	}

	void Print()
	{
		PrintInternal(0, Root);
	}

	uint32 Size() const
	{
		return NodeCount;
	}

	void TransferTo(RedBlackTree& pTree)
	{
		assert(pTree.Root == nullptr);
		assert(pTree.NodeCount == 0);

		pTree.Root = Root;
		pTree.NodeCount = NodeCount;
		Root = nullptr;
		NodeCount = 0;
	}

private:
	void DeleteCase1(Node* pNode)
	{
		if (pNode->Parent == nullptr)
		{
			return; // No fix needs to be made if node was root
		}
		else
		{
			DeleteCase2(pNode);
		}
	}

	void DeleteCase2(Node* pNode)
	{
		Node* sibling = GetSibling(pNode);
		assert(pNode->Parent != nullptr);

		if (sibling != nullptr && sibling->Color == Red)
		{
			pNode->Parent->Color = Red;
			sibling->Color = Black;
			if (pNode == pNode->Parent->Left)
			{
				LeftRotate(pNode->Parent);
			}
			else
			{
				RightRotate(pNode->Parent);
			}
		}
		DeleteCase3(pNode);
	}

	void DeleteCase3(Node* pNode)
	{
		Node* sibling = GetSibling(pNode);
		assert(sibling != nullptr);
		assert(pNode->Parent != nullptr);

		if (pNode->Parent->Color == Black && sibling->Color == Black
			&& (sibling->Left == nullptr || sibling->Left->Color == Black)
			&& (sibling->Right == nullptr || sibling->Right->Color == Black))
		{
			sibling->Color = Red;
			DeleteCase1(pNode->Parent);
		}
		else
		{
			DeleteCase4(pNode);
		}
	}

	void DeleteCase4(Node* pNode)
	{
		Node* sibling = GetSibling(pNode);
		assert(sibling != nullptr);
		assert(pNode->Parent != nullptr);

		if (pNode->Parent->Color == Red && sibling->Color == Black
			&& (sibling->Left == nullptr || sibling->Left->Color == Black)
			&& (sibling->Right == nullptr || sibling->Right->Color == Black))
		{
			sibling->Color = Red;
			pNode->Parent->Color = Black;
		}
		else
		{
			DeleteCase5(pNode);
		}
	}

	void DeleteCase5(Node* pNode)
	{
		Node* sibling = GetSibling(pNode);
		assert(sibling != nullptr);
		assert(pNode->Parent != nullptr);

		if (pNode == pNode->Parent->Left && sibling->Color == Black
			&& (sibling->Left != nullptr && sibling->Left->Color == Red)
			&& (sibling->Right == nullptr || sibling->Right->Color == Black))
		{
			sibling->Color = Red;
			sibling->Left->Color = Black;
			RightRotate(sibling);
		}
		else if (pNode == pNode->Parent->Right && sibling->Color == Black
			&& (sibling->Right != nullptr && sibling->Right->Color == Red)
			&& (sibling->Left == nullptr || sibling->Left->Color == Black))
		{
			sibling->Color = Red;
			sibling->Right->Color = Black;
			LeftRotate(sibling);
		}
		DeleteCase6(pNode);
	}

	void DeleteCase6(Node* pNode)
	{
		Node* sibling = GetSibling(pNode);
		assert(sibling != nullptr);
		assert(pNode->Parent != nullptr);

		sibling->Color = pNode->Parent->Color;
		pNode->Parent->Color = Black;
		if (pNode == pNode->Parent->Left)
		{
			assert(sibling->Right->Color == Red);
			sibling->Right->Color = Black;
			LeftRotate(pNode->Parent);
		}
		else
		{
			assert(sibling->Left->Color == Red);
			sibling->Left->Color = Black;
			RightRotate(pNode->Parent);
		}
	}

	void DestructInternal(Node* pNode)
	{
		if (pNode == nullptr)
		{
			return;
		}

		DestructInternal(pNode->Left);
		DestructInternal(pNode->Right);
		free(pNode);
	}

	Node* GetSibling(Node* pNode)
	{
		assert(pNode->Parent != nullptr);
		if (pNode == pNode->Parent->Left)
		{
			return pNode->Parent->Right;
		}
		else
		{
			assert(pNode == pNode->Parent->Right);
			return pNode->Parent->Left;
		}
	}

	void LeftRotate(Node* pNode)
	{
		Node* rotateTarget = pNode->Right;
		pNode->Right = rotateTarget->Left;
		if (rotateTarget->Left != nullptr)
		{
			rotateTarget->Left->Parent = pNode;
		}

		rotateTarget->Parent = pNode->Parent;
		if (pNode->Parent == nullptr)
		{
			assert(Root == pNode);
			Root = rotateTarget;
		}
		else
		{
			if (pNode == pNode->Parent->Left)
			{
				pNode->Parent->Left = rotateTarget;
			}
			else
			{
				assert(pNode == pNode->Parent->Right);
				pNode->Parent->Right = rotateTarget;
			}
		}
		rotateTarget->Left = pNode;
		pNode->Parent = rotateTarget;
	}

	void PrintInternal(uint32 pIndent, Node* pCurrentNode)
	{
		if (pCurrentNode != nullptr)
		{
			PrintInternal(pIndent + 1, pCurrentNode->Left);

			for (uint32 i = 0; i < pIndent; i++)
			{
				printf(" ");
			}

			printf("%p %s %u\n", pCurrentNode, pCurrentNode->Color == Red ? "Red" : "Black", (uint32)pCurrentNode->Key);

			PrintInternal(pIndent + 1, pCurrentNode->Right);
		}
	}

	void RightRotate(Node* pNode)
	{
		Node* rotateTarget = pNode->Left;
		pNode->Left = rotateTarget->Right;
		if (rotateTarget->Right != nullptr)
		{
			rotateTarget->Right->Parent = pNode;
		}

		rotateTarget->Parent = pNode->Parent;
		if (pNode->Parent == nullptr)
		{
			assert(Root == pNode);
			Root = rotateTarget;
		}
		else
		{
			if (pNode == pNode->Parent->Left)
			{
				pNode->Parent->Left = rotateTarget;
			}
			else
			{
				assert(pNode == pNode->Parent->Right);
				pNode->Parent->Right = rotateTarget;
			}
		}
		rotateTarget->Right = pNode;
		pNode->Parent = rotateTarget;
	}

	void SwapNodes(Node* pNode1, Node* pNode2)
	{
		assert(pNode1 != nullptr);
		assert(pNode2 != nullptr);

		Node* temp;
		uint8 tempColor;

		// Swap parent
		temp = pNode1->Parent;
		pNode1->Parent = pNode2->Parent;
		pNode2->Parent = temp;
		if (pNode2->Parent != nullptr)
		{
			if (pNode2->Parent->Left == pNode1)
			{
				pNode2->Parent->Left = pNode2;
			}
			else
			{
				assert(pNode2->Parent->Right == pNode1);
				pNode2->Parent->Right = pNode2;
			}
		}
		else
		{
			assert(Root == pNode1);
			Root = pNode2;
		}

		if (pNode1->Parent != nullptr)
		{
			if (pNode1->Parent->Left == pNode2)
			{
				pNode1->Parent->Left = pNode1;
			}
			else
			{
				assert(pNode1->Parent->Right == pNode2);
				pNode1->Parent->Right = pNode1;
			}
		}
		else
		{
			assert(Root == pNode2);
			Root = pNode1;
		}

		// Swap left
		temp = pNode1->Left;
		pNode1->Left = pNode2->Left;
		pNode2->Left = temp;
		if (pNode1->Left != nullptr)
		{
			assert(pNode1->Left->Parent == pNode2);
			pNode1->Left->Parent = pNode1;
		}
		if (pNode2->Left != nullptr)
		{
			assert(pNode2->Left->Parent == pNode1);
			pNode2->Left->Parent = pNode2;
		}

		// Swap right
		temp = pNode1->Right;
		pNode1->Right = pNode2->Right;
		pNode2->Right = temp;
		if (pNode1->Right != nullptr)
		{
			assert(pNode1->Right->Parent == pNode2);
			pNode1->Right->Parent = pNode1;
		}
		if (pNode2->Right != nullptr)
		{
			assert(pNode2->Right->Parent == pNode1);
			pNode2->Right->Parent = pNode2;
		}

		// Swap color
		tempColor = pNode1->Color;
		pNode1->Color = pNode2->Color;
		pNode2->Color = tempColor;
	}

	void ValidateInternal(Node* pCurrentNode, uint32 pBlackHeight, uint32& pPathLength, uint32& pNodeCount) const
	{
		pNodeCount++;

		if (pCurrentNode->Color == Black)
		{
			pBlackHeight += 1;
		}
		else
		{
			assert(pCurrentNode->Color == Red && "Invalid color!");
		}

		if (pCurrentNode->Left == nullptr)
		{
			if (pPathLength == 0)
			{
				pPathLength = pBlackHeight;
			}
			else
			{
				assert(pPathLength == pBlackHeight);
			}
		}
		else
		{
			assert(pCurrentNode->Left->Parent == pCurrentNode);
			assert(pCurrentNode->Left->Key < pCurrentNode->Key);

			if (pCurrentNode->Color == Red)
			{
				assert(pCurrentNode->Left->Color == Black && "Two consecutive red nodes!");
			}

			ValidateInternal(pCurrentNode->Left, pBlackHeight, pPathLength, pNodeCount);
		}

		if (pCurrentNode->Right == nullptr)
		{
			if (pPathLength == 0)
			{
				pPathLength = pBlackHeight;
			}
			else
			{
				assert(pPathLength == pBlackHeight);
			}
		}
		else
		{
			assert(pCurrentNode->Right->Parent == pCurrentNode);
			assert(pCurrentNode->Right->Key > pCurrentNode->Key);

			if (pCurrentNode->Color == Red)
			{
				assert(pCurrentNode->Right->Color == Black && "Two consecutive red nodes!");
			}

			ValidateInternal(pCurrentNode->Right, pBlackHeight, pPathLength, pNodeCount);
		}
	}

public:
	class Iterator
	{
	private:
		const RedBlackTree& myTree;
		Node* myCurrentNode;
		bool myIsConst = false;

	public:
		explicit Iterator(RedBlackTree& pTree) :
			myTree(pTree)
		{
			myIsConst = false;

			if (myTree.Root == nullptr)
			{
				myCurrentNode = nullptr;
			}
			else
			{
				myCurrentNode = myTree.Root;
				while (myCurrentNode->Left != nullptr)
				{
					myCurrentNode = myCurrentNode->Left;
				}
			}
		}

		explicit Iterator(const RedBlackTree& pTree) :
			myTree(pTree)
		{
			myIsConst = true;

			if (myTree.Root == nullptr)
			{
				myCurrentNode = nullptr;
			}
			else
			{
				myCurrentNode = myTree.Root;
				while (myCurrentNode->Left != nullptr)
				{
					myCurrentNode = myCurrentNode->Left;
				}
			}
		}

		Node* Next()
		{
			assert(myIsConst == false);

			return NextInternal();
		}

		const Node* NextConst()
		{
			assert(myIsConst == true); // Not strictly necessary

			return NextInternal();
		}

	private:
		Node* NextInternal()
		{
			if (myCurrentNode == nullptr)
			{
				return nullptr;
			}

			Node* node = myCurrentNode;

			if (node->Right == nullptr)
			{
				while (myCurrentNode->Parent != nullptr && myCurrentNode->Parent->Right == myCurrentNode)
				{
					myCurrentNode = myCurrentNode->Parent;
				}
				myCurrentNode = myCurrentNode->Parent; // Can be nullptr, doesn't matter

				return node;
			}
			else
			{
				myCurrentNode = node->Right;
				while (myCurrentNode->Left != nullptr)
				{
					myCurrentNode = myCurrentNode->Left;
				}

				return node;
			}
		}
	};
};
