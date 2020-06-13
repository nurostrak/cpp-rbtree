#include "RedBlackTree.h"

int main()
{
	RedBlackTree<int, int> tree;

	tree.Validate();
	tree.Insert(0, 5);
	tree.Validate();
	
	RedBlackTree<int, int>::Iterator iter(tree);
	assert(iter.Next()->Item == 5);
	assert(iter.Next() == nullptr);

	tree.Validate();
	tree.Delete(0);
	tree.Validate();

	return 0;
}