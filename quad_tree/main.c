#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "quad_tree.h"
#include "small_list.h"
#include "free_list.h"
#include "../line.h"

//void PrintLine(const void* element) {
//	assert(element);
//
//	Line* line = (Line*)element;
//
//	printf("id: %d\n"
//	       "p1: (%lf, %lf)\n"
//	       "p2: (%lf, %lf)\n"
//	       "vc: (%lf, %lf)\n", 
//		line->id,
//		line->p1.x, line->p1.y,
//		line->p2.x, line->p2.y,
//		line->velocity.x, line->velocity.y);
//}

void PrintLine(const void* element) {
 assert(element);

 const unsigned int* line_id = (unsigned int*)element;
 printf("%d", *line_id);
}

const char* LineDemo_input_file_path = "../input/koch.in";

int main() {
  Line** lines;
  unsigned int lineId = 0;
  unsigned int numOfLines;
  window_dimension px1;
  window_dimension py1;
  window_dimension px2;
  window_dimension py2;
  window_dimension vx;
  window_dimension vy;
  int isGray;
  FILE *fin;
  fin = fopen(LineDemo_input_file_path, "r");
  if (fin == NULL) {
    fprintf(stderr, "Input file not found (%s)\n", LineDemo_input_file_path);
    exit(1);
  }

  fscanf(fin, "%d\n", &numOfLines);
  lines = malloc(sizeof(Line*) * numOfLines);
  if(lines == NULL) {
  	printf("Couldn't malloc for lines...\n");
	exit(1);
  }

  while (EOF
      != fscanf(fin, "(%lf, %lf), (%lf, %lf), %lf, %lf, %d\n", &px1, &py1, &px2,
                &py2, &vx, &vy, &isGray)) {
    Line *line = malloc(sizeof(Line));

    // convert window coordinates to box coordinates
    windowToBox(&line->p1.x, &line->p1.y, px1, py1);
    windowToBox(&line->p2.x, &line->p2.y, px2, py2);

    // convert window velocity to box velocity
    velocityWindowToBox(&line->velocity.x, &line->velocity.y, vx, vy);

    // store color
    line->color = (Color) isGray;

    // store line ID
    line->id = lineId;
    lines[lineId] = line;
    lineId++;
  }
  fclose(fin);


  const double time_step = 0.5;
  const int width = 1180;
  const int height = 800;
  const int max_depth = 4;
  const int max_elems = 10;
	
  QuadTree qt;
  QuadTree_Init(&qt, lines, width, height, max_depth, max_elems);
  printf("num_lines: %d\n", numOfLines);
  printf("\ninserting lines...\n");
  for(int i=0; i < 20 ; ++i) {
	QuadTree_Insert(&qt, lines[i]->id, time_step);
  }
  QuadTree_PrintInfo(&qt);
  QuadTree_PrintEntireTree(&qt);

  const unsigned int id = 10;
  printf("Query for line_id: %d\n", id);
  SmallList line_ids = QuadTree_QueryLines(&qt, lines[id], time_step);
  SmallList_PrintData(&line_ids, PrintLine);

  return 0;
}
