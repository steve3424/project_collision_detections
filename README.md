- project_collision_detections from MIT OCW 6.712 https://ocw.mit.edu/courses/electrical-engineering-and-computer-science/6-172-performance-engineering-of-software-systems-fall-2018/ 

- I added a quad tree so far which was almost a translation of this: https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det 

- There are links in the comments to code in C++. I translated it to C and made some changes to the structure.

- Elements are stored in the lowest node that they are completely contained in. This means the elements can be stored in any node and not just leaves. Queries are slightly different too. I did this because determining if an element is in multiple child nodes is very fast. Determining exactly which child nodes it is in was slower in some edge cases. It was probably fast enough, but I didn't test it.

- The keycodes to toggle the quad_tree, visualization, and to pause the sim are hard coded. I am not sure how to do this dynamically in xlib.
