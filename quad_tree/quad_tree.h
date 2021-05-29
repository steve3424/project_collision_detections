#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include "small_list.h"
#include "free_list.h"
#include "../line.h"

// - used for describing which child nodes
//   an element belongs to
typedef struct BranchFlags {
  unsigned int tl : 1;
  unsigned int bl : 1;
  unsigned int br : 1;
  unsigned int tr : 1;
} BranchFlags;

// Tree is built out of these
// .first_child: points to first node in QuadNodes if this is branch
//      	 points to first element in QuadElements if this is leaf
// .count: count of elements if this is leaf
//         -1 if this is branch
// child branches are ordered tl, bl, br, tr
typedef struct QuadNode {
  int first_child;
  int count;
} QuadNode;

// One of these is stored in each leaf an element belongs in
// .next:    points to next element in this leaf (linked list structure)
//           indexes into QuadTree.element_nodes
// .element: points to actual element data
//           indexes into QuadTree.Lines**
typedef struct QuadElement {
 int next;
 int element_id;
} QuadElement;

// defines a mid point, half width, and half height
// we only store the root
// used in QuadNodeData as temporary rectangle
typedef struct QuadRect {
  int mid_x;
  int mid_y;
  int size_x;
  int size_y;
} QuadRect;

// Stores some info for the node that gets passed around during insertions, etc.
// .rect  == dimensions of this node rectangle
// .index == index into QuadTree.quad_nodes
// .depth == how deep into the tree this is, used for checking against max depth during split
typedef struct QuadNodeData {
  QuadRect rect;
  int index;
  int depth;
} QuadNodeData;

typedef struct QuadTree {
  // QuadTree does not own this memory !!!
  Line** lines;

  // Stores each branch/leaf in tree. 4 sub rects are 4 in a row.
  SmallList quad_nodes;   // <QuadNode>

  // Stores an entry for each element
  FreeList quad_elements; // <QuadElementNode>
  
  // Root rectangle. All sub rectangles are computed on the fly using bit shifts of this
  QuadRect root_rect;
 
  // Max depth to avoid infinite recursion in edge cases
  int max_depth;

  // Max elements in leaf before split
  int max_elements;
} QuadTree;

void QuadTree_Init(QuadTree* qt, Line** lines, const int width, const int height, 
		   const int max_depth, const int max_elements);
void QuadTree_Free(QuadTree* qt);
void QuadTree_Clear(QuadTree* qt);
void QuadTree_Insert(QuadTree* qt, const unsigned int line_id, const double time_step);
SmallList QuadTree_QueryLines(const QuadTree* qt, const unsigned int line_id, const double time_step);
SmallList QuadTree_GetRectLineSegments(const QuadTree* qt);
//void QuadTree_PrintInfo(const QuadTree* qt);
//void QuadTree_PrintEntireTree(const QuadTree* qt);

#endif
