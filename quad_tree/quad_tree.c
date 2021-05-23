#include <stdbool.h>
#include "logging.h"
#include "small_list.h"
#include "free_list.h"
#include "quad_tree.h"
#include "../line.h"
#include "../intersection_detection.h"


// PRIVATE DECLARATIONS
static void         QuadTree_QuadElementInsert(QuadTree* qt, const QuadNodeData node_data, 
                                                const unsigned int line_id, double time_step);
static QuadNodeData QuadTree_FindNode(const QuadTree* qt, const QuadNodeData node_data, 
                                      const unsigned int line_id, double time_step);
static void         QuadTree_InsertIntoNode(QuadTree* qt, const QuadNodeData node_data, 
		                            const unsigned int line_id, double time_step);
static void QuadTree_PrintQuadNodeData(const QuadNodeData* element);
static void QuadTree_PrintQuadRect(const QuadRect* rect);
static void QuadTree_PrintElements(const QuadTree* qt, const unsigned int first_child_index, const int depth);
static void QuadTree_PrintBranches(const QuadTree* qt, const QuadRect parent_rect,
                                   const unsigned int first_branch_index, const int depth);


// INLINES
static inline QuadNodeData QuadTree_GetRootNodeData(const QuadTree* qt) {
  assert(qt);

  QuadNodeData rnd = {0};
  rnd.rect  = qt->root_rect;
  rnd.index = 0;
  rnd.depth = 0;
  return rnd;
}

// Checks if line is ENTIRELY inside rectangle
static inline bool QuadTree_LineInRect(const Line* line, const QuadRect* rect) {
  const double left_x  = (double)(rect->mid_x - rect->size_x);
  const double right_x = (double)(rect->mid_x + rect->size_x);
  const double top_y   = (double)(rect->mid_y + rect->size_y);
  const double bot_y   = (double)(rect->mid_y - rect->size_y);

  bool p1_in_rect = left_x <= line->p1.x  &&
                    line->p1.x <= right_x &&
                    bot_y <= line->p1.y   &&
                    line->p1.y <= top_y;
  bool p2_in_rect = left_x <= line->p2.x  &&
                    line->p2.x <= right_x &&
                    bot_y <= line->p2.y   &&
                    line->p2.y <= top_y;

  return p1_in_rect && p2_in_rect;
}


// PUBLIC
void QuadTree_Init(QuadTree* qt, const Line** lines, const int width, const int height, const int max_depth, const int max_elements) {
  assert(qt);
  assert(lines);
  assert(0 < width);
  assert(0 < height);
  assert(0 < max_depth);
   
  qt->lines = lines;
  SmallList_Init(&qt->quad_nodes, sizeof(QuadNode));
  FreeList_Init(&qt->quad_elements, sizeof(QuadElement));

  QuadNode root_node;
  root_node.first_element = -1;
  root_node.count         =  0;
  root_node.first_node    = -1;
  SmallList_PushBack(&qt->quad_nodes, &root_node);

  QuadRect root_rect;
  root_rect.mid_x = width >> 1;
  root_rect.mid_y = height >> 1;
  root_rect.size_x = root_rect.mid_x;
  root_rect.size_y = root_rect.mid_y;
  qt->root_rect = root_rect;
  qt->max_depth = max_depth;
  qt->max_elements = max_elements;
}

void QuadTree_Free(QuadTree* qt) {
  assert(qt);

  //LOG("Quad tree freeing...\n");

  qt->lines = NULL;
  SmallList_Free(&qt->quad_nodes);
  FreeList_Free(&qt->quad_elements);
}

void QuadTree_Clear(QuadTree* qt) {
  assert(qt);

  SmallList_Clear(&qt->quad_nodes);
  FreeList_Clear(&qt->quad_elements);

  QuadNode root_node;
  root_node.first_element = -1;
  root_node.count         =  0;
  root_node.first_node    = -1;
  SmallList_PushBack(&qt->quad_nodes, &root_node);
}

// Inserts a brand new element into the quad tree
void QuadTree_Insert(QuadTree* qt, const unsigned int line_id, const double time_step) {
  assert(qt);

  QuadNodeData root_node_data = QuadTree_GetRootNodeData(qt);
  QuadTree_QuadElementInsert(qt, root_node_data, line_id, time_step);
}

SmallList QuadTree_QueryLines(const QuadTree* qt, const Line* line,
			      const double time_step) {
  assert(qt);

  // get a copy of current_line in box coordinates
  // calculate future line in box coordinates
  Line current_line = *line;
  Line future_line;
  future_line.p1.x = current_line.p1.x + (time_step * current_line.velocity.x);
  future_line.p1.y = current_line.p1.y + (time_step * current_line.velocity.y);
  future_line.p2.x = current_line.p2.x + (time_step * current_line.velocity.x);
  future_line.p2.y = current_line.p2.y + (time_step * current_line.velocity.y);

  // convert both lines to window coordinates for placement in tree
  boxToWindow(&current_line.p1.x, &current_line.p1.y, current_line.p1.x, current_line.p1.y);
  boxToWindow(&current_line.p2.x, &current_line.p2.y, current_line.p2.x, current_line.p2.y);
  boxToWindow(&future_line.p1.x, &future_line.p1.y, future_line.p1.x, future_line.p1.y);
  boxToWindow(&future_line.p2.x, &future_line.p2.y, future_line.p2.x, future_line.p2.y);

  SmallList line_ids_to_check;
  SmallList_Init(&line_ids_to_check, sizeof(unsigned int));
  QuadNodeData root_node_data = QuadTree_GetRootNodeData(qt);
  SmallList to_process_qnd = {0};
  SmallList_Init(&to_process_qnd, sizeof(QuadNodeData));
  SmallList_PushBack(&to_process_qnd, &root_node_data);
  QuadNodeData current_node_data;
  bool line_found = false;
  while(0 < to_process_qnd.num_elements) {
    SmallList_PopBackCopy(&to_process_qnd, &current_node_data);
    const QuadNode* current_node = SmallList_GetAtIndexRef(&qt->quad_nodes, 
                                                            current_node_data.index);
    // grab all elements from current_node
    unsigned int element_i = current_node->first_element;
    QuadElement* element;
    while(element_i != -1) {
      element = FreeList_GetAtIndexRef(&qt->quad_elements, element_i);
      if(element->element_id == current_line.id) {
     	line_found = true; 
      }
      else {
        SmallList_PushBack(&line_ids_to_check, &element->element_id);
      }

      element_i = element->next;
    }

    // stop if no children
    if(current_node->first_node == -1) {
      continue;
    }
    else {
      // calculate all 4 child rects of node
      // tl, bl, br, tr
      const int child_size_x = current_node_data.rect.size_x >> 1;
      const int child_size_y = current_node_data.rect.size_y >> 1;
      QuadRect child_rects[4];
      child_rects[0].mid_x   = current_node_data.rect.mid_x - child_size_x;
      child_rects[0].mid_y   = current_node_data.rect.mid_y + child_size_y;
      child_rects[0].size_x  = child_size_x;
      child_rects[0].size_y  = child_size_y;

      child_rects[1].mid_x   = current_node_data.rect.mid_x - child_size_x;
      child_rects[1].mid_y   = current_node_data.rect.mid_y - child_size_y;
      child_rects[1].size_x  = child_size_x;
      child_rects[1].size_y  = child_size_y;

      child_rects[2].mid_x   = current_node_data.rect.mid_x + child_size_x;
      child_rects[2].mid_y   = current_node_data.rect.mid_y - child_size_y;
      child_rects[2].size_x  = child_size_x;
      child_rects[2].size_y  = child_size_y;

      child_rects[3].mid_x   = current_node_data.rect.mid_x + child_size_x;
      child_rects[3].mid_y   = current_node_data.rect.mid_y + child_size_y;
      child_rects[3].size_x  = child_size_x;
      child_rects[3].size_y  = child_size_y;

      if(line_found) {
        for(int i = 0; i < 4; ++i) {
	  QuadNodeData child_node_data;
          child_node_data.rect  = child_rects[i];
	  child_node_data.index = current_node->first_node + i;
	  child_node_data.depth = current_node_data.depth + 1;
          SmallList_PushBack(&to_process_qnd, &child_node_data);
        }
      }
      else {
        for(int i = 0; i < 4; ++i) {
          bool current_line_in_rect = QuadTree_LineInRect(&current_line, &child_rects[i]);
          bool future_line_in_rect  = QuadTree_LineInRect(&future_line, &child_rects[i]);
          if(current_line_in_rect && future_line_in_rect) {
            QuadNodeData child_node_data;
            child_node_data.rect  = child_rects[i];
            child_node_data.index = current_node->first_node + i;
            child_node_data.depth = current_node_data.depth + 1;
            SmallList_PushBack(&to_process_qnd, &child_node_data);
          }
        }
      }
    }
  }

  return line_ids_to_check;
}

SmallList QuadTree_GetRectLineSegments(const QuadTree* qt) {
  SmallList rect_line_segments;
  SmallList_Init(&rect_line_segments, sizeof(Line));

  QuadNodeData root_node_data = QuadTree_GetRootNodeData(qt);
  SmallList to_process_qnd = {0};
  SmallList_Init(&to_process_qnd, sizeof(QuadNodeData));
  SmallList_PushBack(&to_process_qnd, &root_node_data);
  while(0 < to_process_qnd.num_elements) {
    QuadNodeData* current_node_data = SmallList_PopBackRef(&to_process_qnd);
    QuadNode* current_node = SmallList_GetAtIndexRef(&qt->quad_nodes, 
                                                      current_node_data->index);

    QuadRect* rect = &current_node_data->rect;
    Line lbrt[4];
    lbrt[0].p1.x = rect->mid_x - rect->size_x;
    lbrt[0].p1.y = rect->mid_y - rect->size_y;
    lbrt[0].p2.x = lbrt[0].p1.x;
    lbrt[0].p2.y = rect->mid_y + rect->size_y;

    lbrt[1].p1.x = rect->mid_x - rect->size_x;
    lbrt[1].p1.y = rect->mid_y + rect->size_y;
    lbrt[1].p2.x = rect->mid_x + rect->size_x;
    lbrt[1].p2.y = lbrt[1].p1.y;

    lbrt[2].p1.x = rect->mid_x + rect->size_x;
    lbrt[2].p1.y = rect->mid_y - rect->size_y;
    lbrt[2].p2.x = lbrt[2].p1.x;
    lbrt[2].p2.y = rect->mid_y + rect->size_y;

    lbrt[3].p1.x = rect->mid_x - rect->size_x;
    lbrt[3].p1.y = rect->mid_y - rect->size_y;
    lbrt[3].p2.x = rect->mid_x + rect->size_x;
    lbrt[3].p2.y = lbrt[3].p1.y;

    for(int i = 0; i < 4; ++i) {
      SmallList_PushBack(&rect_line_segments, &lbrt[i]);
    }

    if(current_node->first_node != -1) {
      // calculate all 4 child rects of node
      // tl, bl, br, tr
      const int child_size_x = rect->size_x >> 1;
      const int child_size_y = rect->size_y >> 1;
      QuadRect child_rects[4];
      child_rects[0].mid_x   = rect->mid_x - child_size_x;
      child_rects[0].mid_y   = rect->mid_y + child_size_y;
      child_rects[0].size_x  = child_size_x;
      child_rects[0].size_y  = child_size_y;

      child_rects[1].mid_x   = rect->mid_x - child_size_x;
      child_rects[1].mid_y   = rect->mid_y - child_size_y;
      child_rects[1].size_x  = child_size_x;
      child_rects[1].size_y  = child_size_y;

      child_rects[2].mid_x   = rect->mid_x + child_size_x;
      child_rects[2].mid_y   = rect->mid_y - child_size_y;
      child_rects[2].size_x  = child_size_x;
      child_rects[2].size_y  = child_size_y;

      child_rects[3].mid_x   = rect->mid_x + child_size_x;
      child_rects[3].mid_y   = rect->mid_y + child_size_y;
      child_rects[3].size_x  = child_size_x;
      child_rects[3].size_y  = child_size_y;

      for(int i = 0; i < 4; ++i) {
	QuadNodeData child_node_data;
        child_node_data.rect  = child_rects[i];
	child_node_data.index = current_node->first_node + i;
	child_node_data.depth = current_node_data->depth + 1;
        SmallList_PushBack(&to_process_qnd, &child_node_data);
      }
    }
  }

  return rect_line_segments;
}

// PRIVATE
static void QuadTree_QuadElementInsert(QuadTree* qt, const QuadNodeData node_data, 
                                           const unsigned int line_id, 
					   const double time_step) {
  assert(qt);

  QuadNodeData node_to_insert = QuadTree_FindNode(qt, node_data, line_id, time_step);
  QuadTree_InsertIntoNode(qt, node_to_insert, line_id, time_step);
}

static QuadNodeData QuadTree_FindNode(const QuadTree* qt, const QuadNodeData node_data, 
                                      const unsigned int line_id, const double time_step) {
  assert(qt);

  // get a copy of current_line in box coordinates
  // calculate future line in box coordinates
  Line current_line = *(qt->lines[line_id]);
  Line future_line;
  future_line.p1.x = current_line.p1.x + (time_step * current_line.velocity.x);
  future_line.p1.y = current_line.p1.y + (time_step * current_line.velocity.y);
  future_line.p2.x = current_line.p2.x + (time_step * current_line.velocity.x);
  future_line.p2.y = current_line.p2.y + (time_step * current_line.velocity.y);
	
  // convert both lines to window coordinates for placement in tree
  boxToWindow(&current_line.p1.x, &current_line.p1.y, current_line.p1.x, current_line.p1.y);
  boxToWindow(&current_line.p2.x, &current_line.p2.y, current_line.p2.x, current_line.p2.y);
  boxToWindow(&future_line.p1.x, &future_line.p1.y, future_line.p1.x, future_line.p1.y);
  boxToWindow(&future_line.p2.x, &future_line.p2.y, future_line.p2.x, future_line.p2.y);

  SmallList to_process_qnd = {0};
  SmallList_Init(&to_process_qnd, sizeof(QuadNodeData));
  SmallList_PushBack(&to_process_qnd, &node_data);
  QuadNodeData current_node_data;
  while(0 < to_process_qnd.num_elements) {
    SmallList_PopBackCopy(&to_process_qnd, &current_node_data);
    const QuadNode* current_node = SmallList_GetAtIndexRef(&qt->quad_nodes, 
                                                            current_node_data.index);
    if(current_node->first_node == -1) {
      break;
    }
    else {
      // calculate all 4 child rects of node
      // tl, bl, br, tr
      const int child_size_x = current_node_data.rect.size_x >> 1;
      const int child_size_y = current_node_data.rect.size_y >> 1;
      QuadRect child_rects[4];
      child_rects[0].mid_x   = current_node_data.rect.mid_x - child_size_x;
      child_rects[0].mid_y   = current_node_data.rect.mid_y + child_size_y;
      child_rects[0].size_x  = child_size_x;
      child_rects[0].size_y  = child_size_y;

      child_rects[1].mid_x   = current_node_data.rect.mid_x - child_size_x;
      child_rects[1].mid_y   = current_node_data.rect.mid_y - child_size_y;
      child_rects[1].size_x  = child_size_x;
      child_rects[1].size_y  = child_size_y;

      child_rects[2].mid_x   = current_node_data.rect.mid_x + child_size_x;
      child_rects[2].mid_y   = current_node_data.rect.mid_y - child_size_y;
      child_rects[2].size_x  = child_size_x;
      child_rects[2].size_y  = child_size_y;

      child_rects[3].mid_x   = current_node_data.rect.mid_x + child_size_x;
      child_rects[3].mid_y   = current_node_data.rect.mid_y + child_size_y;
      child_rects[3].size_x  = child_size_x;
      child_rects[3].size_y  = child_size_y;

      bool found_child = false;
      for(int i = 0; i < 4; ++i) {
        bool current_line_in_rect = QuadTree_LineInRect(&current_line, &child_rects[i]);
        bool future_line_in_rect  = QuadTree_LineInRect(&future_line, &child_rects[i]);
        if(current_line_in_rect && future_line_in_rect) {
	  QuadNodeData child_node_data;
          child_node_data.rect  = child_rects[i];
	  child_node_data.index = current_node->first_node + i;
	  child_node_data.depth = current_node_data.depth + 1;
          SmallList_PushBack(&to_process_qnd, &child_node_data);
	  found_child = true;
	  break;
        }
      }
      
      if(!found_child) {
        break;
      }
    }
  }

  SmallList_Free(&to_process_qnd);

  return current_node_data;
}

static void QuadTree_InsertIntoNode(QuadTree* qt, const QuadNodeData node_data, 
		                    const unsigned int line_id, double time_step) {
  assert(qt);

  // grab ref to QuadNode leaf where we are inserting
  // create new element node
  // insert into element_nodes list
  // attach as head to linked list structure of QuadTree.quad_nodes
  QuadNode* quad_node = SmallList_GetAtIndexRef(&qt->quad_nodes, node_data.index);
  QuadElement new_quad_element;
  new_quad_element.next       = quad_node->first_element;
  new_quad_element.element_id = line_id;
  quad_node->first_element    = FreeList_Insert(&qt->quad_elements, &new_quad_element);
  quad_node->count++;
  
  // split if necessary
  if((quad_node->count > qt->max_elements) && 
     (node_data.depth < qt->max_depth)    && 
     (quad_node->first_node == -1)) {
    // Pop all element_nodes off of this node
    SmallList quad_elements_temp;
    SmallList_Init(&quad_elements_temp, sizeof(QuadElement));
    QuadElement* element;
    int index;
    while(quad_node->first_element != -1) {
      index = quad_node->first_element;
      element = FreeList_GetAtIndexRef(&qt->quad_elements, index);

      quad_node->first_element = element->next;
      SmallList_PushBack(&quad_elements_temp, element);
      FreeList_EraseAtIndex(&qt->quad_elements, index);
    }
    quad_node->count = 0;
    
    // this quad_node is now a branch
    // add 4 children and initialize them
    // quad_nodes are appended to end of list AND kept contiguous so this can point to the first one
    quad_node->first_node = qt->quad_nodes.num_elements;
    SmallList_Resize(&qt->quad_nodes, qt->quad_nodes.num_elements + 4);
    for(int i = 0; i < 4; ++i) {
      QuadNode child_node;
      child_node.first_element = -1;
      child_node.count         =  0;
      child_node.first_node    = -1;
      SmallList_PushBack(&qt->quad_nodes, &child_node);
    }

    // insert all elements back into tree
    // done backwards to reinsert in same order they were before
    QuadElement* element_to_reinsert;
    for(int i = quad_elements_temp.num_elements - 1; 0 <= i; --i) {
      element_to_reinsert = SmallList_GetAtIndexRef(&quad_elements_temp, i);
      QuadTree_QuadElementInsert(qt, node_data, element_to_reinsert->element_id, time_step);
    }

    SmallList_Free(&quad_elements_temp);
  }
}

// PRINTING
void QuadTree_PrintInfo(const QuadTree* qt) {
  printf("\n*** QUAD TREE INFO ***\n");
  printf("Number of nodes:         %d\n", qt->quad_nodes.num_elements);
  printf("Number of element_nodes: %d\n", FreeList_GetNumElements(&qt->quad_elements));
}

void QuadTree_PrintEntireTree(const QuadTree* qt) {
  assert(qt);
  
  printf("ROOT -> ");
  QuadTree_PrintQuadRect(&qt->root_rect);
  printf("\n");

  const QuadNode* root_node = SmallList_GetAtIndexRef(&qt->quad_nodes, 0);

  if(root_node->first_element != -1) {
    QuadTree_PrintElements(qt, root_node->first_element, 0);
    printf("\n");
  }

  if(root_node->first_node != -1) {
    QuadTree_PrintBranches(qt, qt->root_rect, root_node->first_node, 1);
  }

  printf("\n");
}

static void QuadTree_PrintBranches(const QuadTree* qt, const QuadRect parent_rect,
                                   const unsigned int first_branch_index, const int depth) {
  QuadRect child_rect;
  child_rect.size_x = parent_rect.size_x >> 1;
  child_rect.size_y = parent_rect.size_y >> 1;
  int mid_x_options[] = { parent_rect.mid_x - child_rect.size_x, // TL
                          parent_rect.mid_x - child_rect.size_x, // BL
                          parent_rect.mid_x + child_rect.size_x, // BR
                          parent_rect.mid_x + child_rect.size_x  // TR
                        };
  int mid_y_options[] = { parent_rect.mid_y + child_rect.size_y, // TL
                          parent_rect.mid_y - child_rect.size_y, // BL
                          parent_rect.mid_y - child_rect.size_y, // BR
                          parent_rect.mid_y + child_rect.size_y  // TR
                        };
  char labels[4][3] = { "TL", "BL", "BR", "TR"};
  
  QuadNode* node;
  for(int i = 0; i < 4; ++i) {
    node = SmallList_GetAtIndexRef(&qt->quad_nodes, first_branch_index + i);
    child_rect.mid_x = mid_x_options[i];
    child_rect.mid_y = mid_y_options[i];

    printf("\n");

    for(int i = 0; i < depth; ++i) {
      printf("\t");
    }

    printf("%s -> ", labels[i]);
    QuadTree_PrintQuadRect(&child_rect);
    printf("\n");

    if(node->first_element != -1) {
      QuadTree_PrintElements(qt, node->first_element, depth);
      printf("\n");
    }

    if(node->first_node != -1) {
      QuadTree_PrintBranches(qt, child_rect, node->first_node, depth + 1);
    }
  }

}

static void QuadTree_PrintQuadNodeData(const QuadNodeData* element) {
  const QuadNodeData* qnd = element;
  printf("(index: %d depth: %d mid_xy: [%d, %d] size_xy: [%d, %d])", qnd->index,
                                                                   qnd->depth,
                                                                   qnd->rect.mid_x,
                                                                   qnd->rect.mid_y,
                                                                   qnd->rect.size_x,
                                                                   qnd->rect.size_y);
}

static void QuadTree_PrintQuadRect(const QuadRect* rect) {
  assert(rect);

  const int x_start = rect->mid_x - rect->size_x;
  const int x_end   = rect->mid_x + rect->size_x;
  const int y_start = rect->mid_y - rect->size_y;
  const int y_end   = rect->mid_y + rect->size_y;
  printf("mid: [%d,%d], x: [%d,%d], y: [%d,%d]", rect->mid_x, rect->mid_y,
                                                 x_start, x_end,
                                                 y_start, y_end);
}

static void QuadTree_PrintElements(const QuadTree* qt, const unsigned int first_element_index, const int depth) {
    for(int i = 0; i < depth; ++i) {
      printf("\t");
    }

    unsigned int element_index = first_element_index;
    QuadElement* element;
    printf("--> element_ids: ");
    printf("[");
    while(element_index != -1) {
      element = FreeList_GetAtIndexRef(&qt->quad_elements, element_index);
    
      printf("%d,", element->element_id);

      element_index = element->next;
    }
    printf("]");
}

