
# **My UF Computer Graphics Course Projects**

**FOR VIDEO DEMOS AND SPECIFIC PROJECT FILES PLEASE VISIT** - https://drive.google.com/drive/folders/12YUqrEy09uDdxTwPpJuukBXuqxKynt5D?usp=sharing


## Project 1 Description - Curves Subdvision
Source file - misc05_picking/p1_source.cpp

**Part 1**
### Task 1: Create Scene
Display 8 points on the screen each of a different color and arranged uniformly on a circle.

### Task 2: Picking
Upon mouse click, change the color of the selected point to highlight it. Restore the original color upon release.
Use orthogonal projection.

### Task 3: Dragging
Implement the ability to move the points with the mouse
Make all points suitably large.

**Part 2**
### Task 1: (B-spline) Subdivision

Initialize P0i=Pi (white points).
Use these formulas to create a refined set of control points (cyan)

Pk2i:=4Pk−1i−1+4Pk−1i8

Pk2i+1:=Pk−1i−1+6Pk−1i+Pk−1i+18

where k is the level of subdivison and
i is the index of points is in range 0…(N×2k−1) .

The figure illustrates one step of subdivision.
Your implementation should allow repeated refinement (at least 5 times).
Upon pressing key 1, one additional refinement should be triggered.
Initially when (k=0), the control polygon should be drawn without subdivision.
Whenever key 1 is pressed the subdivided control polygon should be redrawn.
Every sixth refinement resets to level k=0.

### 2: C2 Bézier curves

Let P={P1,…,PN} be the the set of input points.
You will construct N Bézier curves of degree 3: one curve segment for each input point.
The coefficients of the ith curve are ci={ci,0,ci,1,ci,2,ci,3}.
The interior Bézier points (yellow) are:

ci,1:=2Pi+Pi+13

ci,2:=Pi+2Pi+13

Determine ci,0 and ci,3=ci+1,0 so that the polynomial pieces join C1.
Write down the formulas for ci,0 and ci,3 and place them into your ReadMe.txt file.

This method should be activated when key 2 is pressed on the keyboard

### Task 3: C1 Catmull-Rom curves
Let P={P1,…,PN} be the the set of input points.
Construct a Catmull-Rom curve that interpolates the N points Pi as follows.
There are N Bézier curve segments of degree 3.
The coefficients of each segment i are ci={ci,0,ci,1,ci,2,ci,3} where ci,0=Pi and ci,3=Pi+1.
The tangent at ci,0 is a multiple of Pi+1−Pi−1.

Once all of the Bézier points (red) are determined use deCasteljau's Algorithm to evaluate the curve at 17 points per segment.
Connecting the points yields the Catmull-Rom curve (green).

This method should be activated when key 3 is pressed on the keyboard

**Part 3**
### Task 1: Picking and Dragging

Enable picking and dragging from Project 1a for the N=10 control points Pi of Project 1b (Tasks 1, 2, and 3) and display the corresponding curve. The curve should change as the Pi are moved.
When the keybord shift key is pressed, instead of the movement in the x-y plane, vertical movement of the mouse moves the point along the Z axis (note: since we look from the top this is not yet visible).
NOTE: Picking should work in Single View, but it is not required in Double View.

### Task 2: Double View

In the top half of the window draw the default view perpendicular to the x-y plane. In the bottom half of the window draw the side view perpendicular to y-z plane.

The double-view should be toggled when 4 is pressed.

**Bonus**
Create a yellow triangle when key  5  is pressed.  
It should loop along the curve indefinitely and have an RGB coordinate frame attached where  
R = tangent, G = main normal, B = bi-normal direction.

## Project 2 Description - Robot Arm Simulation
Source file - misc05_picking/misc05_picking_slow_easy.cpp
### Task 1: Grid

Draw an integer grid on the Y=0-plane for the rectangle (-5,0,-5) to (+5,0,+5).  
Draw the positive X axis in red, the Y axis in green and the Z axis in blue. Only draw the positive portion of each axis, of length 5.

### Task 2: Camera Rotations

-   Use  **Perspective**  projection
-   Place the camera so you can see the whole scene: use glm::LookAt to generate the View matrix.
-   Key  C  selects the camera.
-   Keys  **←**  and  **→**  move the camera along the blue circle parallel to the equator.
-   Keys  **↑**  and  **↓**  rotate the camera along the red circle orthogonal to the equator.
-   Choose the "up" direction so the camera always points to the origin.
    
   ### Task 3: Draw the robot arm

   The robot arm consists of the following parts:
    
   -   Base: truncated tetrahedron placed on the x-z-plane
    -   Top: icosahedron placed on top of and slightly penetrating the Base.
    -   Arm1: rectangular box emmanating from Top and hinged at the center of Top.
    -   Joint: dodecahedron of appropriate radius at the other end of Arm1.
    -   Arm2: cylinder connected to the center of Joint.
    -   Pen: truncated octahedron connected to the other end of Arm2.
    -   Button: a small box on Pen.
    

    
   ### Task 4: Keyboard interaction
      
   Write the code to move the robot arm, rotate the top, rotate the arms and the pen, and twist the pen using the keyboard, as explained below:
   
   -   **Pen**: Select the pen using key  p. The pen should  _rotate_  when the arrow keys are pressed.  **←**,  **→**,  **↑**  and  **↓**  are longitude (J4) and latitude (J5) rotations. (Note that one end is always connected to  _arm2_).  shift+**←**  and  shift+**→**  should twist the pen around its axis (J6) (including buttons).
   -   **Base**: Select the base using key  b. The whole model should  _slide_  on the XZ plane according to the arrow keys.
   -   **Top** : Select the top using key  t. The top, arms and pen should  _rotate_  about the Y direction when pressing the left or right arrow keys (J1).
   -   **Arm1**: Select arm1 using key  1. The arm (and the other connected arm and pen) should  _rotate_  up and down when using the arrow keys (one end is fixed at the center of the  _top_  green cylinder) (J2).
   -   **Arm2**: Select  _Arm2_  using key  2. The arm (and pen) should  _rotate_  up and down when using the arrow keys (one end is fixed at the center of the  _joint_) (J3).
   
   Indicate the selected part by drawing it in a brighter color.
   
   ### Task 5: Light up the scene   
-   Add two lights to the scene.  
   For each light, supply position, diffuse color, ambient color and specular color.  
   Position the lights near the camera so that one light comes from the left and another one from the right.  
   You are free to choose any light colors and positions as long as the scene looks good.
-   Set diffuse and ambient material of the objects to the color of the object. Set specular as a multiple (eg one tenth) of the diffuse color.
   
   ### Task 6: Teleporting
     
-   When  s  is pressed have a Platonic solid exit the tip of the stylus, with tangent equal to the stylus axis and derivative in length equal to the stylus length
-   The solid follows an arc according to Newton's law under gravity until it hits the grid. (Hint: use the BB-form of degree 2)
-   Animate the projectile and, on impact, move the robot arm to the impact location.

## Project 3 - Surface Subdivision and Tessellation

source file - misc05_picking/misc05_picking_custom
### Set up

-   Draw a 600x600 window and set the title to "Yourname".
-   Use  **Perspective**  projection, set the field of view angle to be 45 degree, near plane to be 0.1 and far plane to be 100.
-   **Camera movements from Project 2**: Use  **←**  and  **→**  keys move the camera along the blue circle parallel to the equator.  **↑**  and  **↓**  keys rotate the camera along the red circle orthogonal to the equator. Point the camera always to the origin. Choose a good "up" direction.
-   The  r  key resets the program to its startup state (displays x-y plane, clear rotations, etc.).

### Task 1: Display face geometry as a(n irregular) quad mesh



Points: 12+3

-   Create or find a low poly human head consisting of 3- and 4-sided facets and import it into your openGL program.
-   The  f  key toggles show/hide of the  _wireframe_ of the model (show no facets yet!)
    
### Task 2: Add a texture    
   - Take a photo of your face
    -   Map the photo onto the quad facets of the mesh
    -   The  F  key toggles show/hide of the facetted (Frankenstein) head with texture
    
### Task 3: Render a smooth surface
   -   Apply  [PN triangles](https://www.cise.ufl.edu/research/SurfLab/papers/00ati.pdf) and  [PN quads](https://www.cise.ufl.edu/research/SurfLab/papers/1008PNquad.pdf) to the mesh
    -   The  P  key toggles show/hide of the smoothly rendered head ( = with sufficiently high sample level)
    -   uv-map your face texture onto the front of the curved surface PN quad head model.
    -   The  u  key toggles show/hide of the texture.
    
   ### _BONUS:_
-   Use the tessellation engine for Task 3.

