#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include "small_list.h"
#include "free_list.h"
#include "../line.h"

// Tree is built out of these
// .first_element: points to first element if it doesn't fit into any child rects
// 		   indexes into QuadTree.quad_element_nodes
// .first_node:    points to first branch/leaf
//                 indexes into QuadTree.quad_nodes
//                 4 children are stored contiguously
// child branches are ordered tl, bl, br, tr
typedef struct QuadNode {
  int first_element;
  int count;
  int first_node;
} QuadNode;

// One of these is stored for each element
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

void QuadTree_Init(QuadTree* qt, const Line** lines, const int width, const int height, const int max_depth, const int max_elements);
void QuadTree_Free(QuadTree* qt);
void QuadTree_Clear(QuadTree* qt);
void QuadTree_Insert(QuadTree* qt, const unsigned int line_id, double time_step);
SmallList QuadTree_QueryLines(const QuadTree* qt, const Line* line, const double time_step);
SmallList QuadTree_GetRectLineSegments(const QuadTree* qt);
void QuadTree_PrintInfo(const QuadTree* qt);
void QuadTree_PrintEntireTree(const QuadTree* qt);

#endif
