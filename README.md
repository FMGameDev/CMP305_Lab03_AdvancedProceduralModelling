# CMP305 - Procedural Methods
## Lab 1: First Procedural Terrain
### Gaz Robinson
### School of Design and Informatics

# Introduction
This is our first experiment with procedural generation techniques. In this tutorial you are provided an application with which you can experiment and eventually perhaps build your coursework onto.
The framework is the same as you’ll be using in your CMP301 Graphics Programming module, so it should become familiar, but for now you’ll be sticking to the TerrainMesh class.

What you’ve been provided has enough basic content to show some meaningful generation and manipulation of a mesh and should serve to provide a base sandbox for terrain generation.

## What is a Terrain?
A terrain, in this case, is a flat piece of geometry in the X/Z plane, represented by an array of vertices. In a uniformly distributed terrain, the X and Z coordinates of each vertex will match up with the vertex’s position in the array. 
For example, in a 10x10 terrain, the vertex at position[16] would likely be at position ( 7.0, 0.0, 1.0 ) with [0] -> [9] filling out the first Z-row.

Below, you can see a very simple flat 10x10 plane.

 ![image](https://user-images.githubusercontent.com/5627386/134008312-fef8a1f2-12ab-4ee1-a3be-7ddac62e1f87.png)

 
To create an interesting terrain, we’ll be modifying the Y position of each point on the plane to build a height map, and eventually a “realistic” looking landscape!

In the image below, the plane has been colored green and the Y-position of a few vertices have been modified to create a “realistic” terrain.

 ![image](https://user-images.githubusercontent.com/5627386/134008329-fc0e1786-b8f3-42d8-8927-ed98e8334f84.png)


## CPU vs GPU
In this week’s example we are manually altering the vertex information before it’s sent along to the graphics pipeline; this is therefore work being carried out on the CPU.
Considering the power and functionality of modern vertex and pixel shaders, there are other ways of altering the geometry which will become more apparent as the term goes on (and you get more experience with graphics programming!).

If you’re curious, have a look at shadertoy.com for examples of incredible things made entirely through shaders on the GPU. 
(Here’s a particularly wild example https://www.shadertoy.com/view/MdBGzG.)


# The code
You’ll be working mostly in the TerrainMesh class today, and within that you should be focusing on the “BuildHeightMap” function, which runs through an array of float values and assigns a height by passing the current X position into a sin function call.
This array of height values, or “Heightmap” is then accessed in the “Regenerate” function, which handles the actual building of the vertex and index buffers, recalculating vertex normals, and passing the information to the rendering pipeline.

## ~Waves~
If you look at the initially generated “terrain”, and have a peek at the BuildHeightMap function, you’ll see that the landscape is actually just a combination of a sin and cosine wave across the X and Z axes.
This usage of waves to approximate natural landscape features is core to procedural landscape generation techniques! 
# Tasks
## 1.	Modify the existing generation algorithm.
- There’s a couple of magic numbers in the BuildHeightMap function, how does changing them affect the terrain?
- What do these magic numbers represent?
- Try to change the direction of the sine wave from the x-axis to the z-axis, or change it to a cosine wave.

## 2.	Frequency and amplitude.
Create variables to replace the magic numbers and allow you to change the amplitude (height or vertical scale) and the frequency (distance between peaks) of the wave. 
You might want to hook them up to a slider in the GUI function of the App1 file; this will allow you to regenerate the terrain with new values at runtime.

## 3.	Compound waves.
Something that will massively increase the visual fidelity of your terrain is to compound waves together to create finer details in your terrain. Try to create a second sine wave with a smaller amplitude but higher frequency and add the result to the existing one to create a more detailed waveform. See below for an illustrated example.
Fig 1. An example of sine wave composition giving a more detailed output and a further wave addition to create a “mountainous” terrain.

## 4.	Random height field. 
Add an algorithm to generate a random height field, assigning a random value to each height entry in the heightmap. It should end up looking spiky and… not very good.

## 5.	Terrain smoothing. 
The random height map looks pretty awful. 
You can ‘fix’ rough edges on a terrain by ‘smoothing’ the terrain. 
Think about, plan and try to implement a method of smoothing the terrain by pressing or holding a key down.
 
## 6.	Experiment and research! You should now have a grasp of manipulating vertex height values on the CPU.
Try using the techniques so far to do something new and combine them in interesting ways, or devise your own using any mathematical techniques you think might apply. 

Some possibilities:
— When the “Regenerate” button is pressed, blend smoothly between the old and new terrain values
— Regenerate the terrain in real-time, so modifications to amplitude or frequency are seen instantly
— “Move” through the sine or cosine wave over time
— Have a sin/cos wave along the X-axis added to a sin/cos wave on the Z-axis to create a grid of “hills”

Or spend some time at shadertoy.com to see some of the wild possibilities of procedural generation.

If you have any problems, please ask in the lab, or contact me (g.robinson@abertay.ac.uk). 

