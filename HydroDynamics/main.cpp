/*
Title: HydroDynamics
File Name: VertexShader.glsl
Copyright © 2015
Original authors: Brockton Roth
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Description:
This is a program demonstrating hydrodynamics in fluids.
When a fluid is put in a two containers (one wide and one narrow) with a
tube connecting them at the bottom, the height of fluid in both of them 
would be the same. Even when we add more fluid to one of the containers.

|    |      | |
|    |      | |
|wwww|      |w|
|wwww|      |w|						w is water
|wwww+------+w|
|wwwwwwwwwwwww|
+-------------+

The Bigger side has more fluid and exerts more force than the smaller side, but 
the pressure exerted by both the sides are the same.
Pressure can be defined as the forece exerted divided by the surface area. 
Thus the pressure depends on the height of the water level.

P = density * height * gravity. 

The pressure is independant of the cross section of the pipe. This force acts downward,
but the fluid makes it act in an isotropic way.

Use "Space" to increase pressure on the Left (big) side, thereby causing the water level 
to go up on the right side. 
Use "left Shift" to decrease the pressure on Left (big) side, thereby causing a vacuum
causing the water to flow from right to left.
*/

#include "GLIncludes.h"

#define density 1.0f
#define gravity 9.8f

float externalPressure = 0;

//This structure holds the attributes of a single side of the apparatus.
struct container
{
	float height;
	float width;
	// These values are used for rendering. 
	// Since we need to change individual vertices to reflect the changes, we shall use openGL primitives to render quads.
	// This does not require any shaders.
	glm::vec2 bottomLeft, bottomRight;
	glm::vec2 topLeft, topRight;
	float pressure;
}big, small;

void setup()
{
	// Set up the variables and attributes for both sides of the apparatus
	big.height = 0.5f;
	big.width = 0.5f;
	big.bottomLeft = glm::vec2(-0.75f, -0.5f);
	big.bottomRight = glm::vec2(-0.25f, -0.5f);
	big.topLeft = glm::vec2(-0.75f, 0.0f);
	big.topRight = glm::vec2(-0.25f, 0.0f);
	big.pressure = big.height * density *  gravity;

	small.height = 0.5f;
	small.width = 0.25f;
	small.bottomLeft = glm::vec2(0.5f, -0.5f);
	small.bottomRight = glm::vec2(0.75f, -0.5f);
	small.topLeft = glm::vec2(0.5f, 0.0f);
	small.topRight = glm::vec2(0.75f, 0.0f);
	small.pressure = small.height * density * gravity;
}

// Global data members
#pragma region Base_data
// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;

// This is a reference to your uniform MVP matrix in your vertex shader
GLuint uniMVP;

// Reference to the window object being created by GLFW.
GLFWwindow* window;
#pragma endregion Base_data								  

inline void drawPiston()
{
	glm::vec2 pistonTopLeft, pistonTopRight;
	pistonTopLeft = big.topLeft + glm::vec2(0, 0.1f);
	pistonTopRight = big.topRight + glm::vec2(0, 0.1f);
	
	glLineWidth(2.5);
	glColor3f(0.8, 0.2, 0.2);
	glBegin(GL_QUADS);

	glVertex2fv((float*) &big.topLeft);
	glVertex2fv((float*) &big.topRight);
	glVertex2fv((float*)&pistonTopRight);
	glVertex2fv((float*)&pistonTopLeft);
	
	glVertex2f(((big.topLeft.x + big.topRight.x) / 2.0f) - 0.01f, big.topLeft.y);
	glVertex2f(((big.topLeft.x + big.topRight.x) / 2.0f) + 0.01f, big.topLeft.y);
	glVertex2f(((big.topLeft.x + big.topRight.x) / 2.0f) + 0.01f, 1.0f);
	glVertex2f(((big.topLeft.x + big.topRight.x) / 2.0f) - 0.01f, 1.0f);

	glEnd();
}

// Functions called only once every time the program is executed.
#pragma region Helper_functions
std::string readShader(std::string fileName)
{
	std::string shaderCode;
	std::string line;

	// We choose ifstream and std::ios::in because we are opening the file for input into our program.
	// If we were writing to the file, we would use ofstream and std::ios::out.
	std::ifstream file(fileName, std::ios::in);

	// This checks to make sure that we didn't encounter any errors when getting the file.
	if (!file.good())
	{
		std::cout << "Can't read file: " << fileName.data() << std::endl;

		// Return so we don't error out.
		return "";
	}

	// ifstream keeps an internal "get" position determining the location of the element to be read next
	// seekg allows you to modify this location, and tellg allows you to get this location
	// This location is stored as a streampos member type, and the parameters passed in must be of this type as well
	// seekg parameters are (offset, direction) or you can just use an absolute (position).
	// The offset parameter is of the type streamoff, and the direction is of the type seekdir (an enum which can be ios::beg, ios::cur, or ios::end referring to the beginning, 
	// current position, or end of the stream).
	file.seekg(0, std::ios::end);					// Moves the "get" position to the end of the file.
	shaderCode.resize((unsigned int)file.tellg());	// Resizes the shaderCode string to the size of the file being read, given that tellg will give the current "get" which is at the end of the file.
	file.seekg(0, std::ios::beg);					// Moves the "get" position to the start of the file.

	// File streams contain two member functions for reading and writing binary data (read, write). The read function belongs to ifstream, and the write function belongs to ofstream.
	// The parameters are (memoryBlock, size) where memoryBlock is of type char* and represents the address of an array of bytes are to be read from/written to.
	// The size parameter is an integer that determines the number of characters to be read/written from/to the memory block.
	file.read(&shaderCode[0], shaderCode.size());	// Reads from the file (starting at the "get" position which is currently at the start of the file) and writes that data to the beginning
	// of the shaderCode variable, up until the full size of shaderCode. This is done with binary data, which is why we must ensure that the sizes are all correct.

	file.close(); // Now that we're done, close the file and return the shaderCode.

	return shaderCode;
}

// This method will consolidate some of the shader code we've written to return a GLuint to the compiled shader.
// It only requires the shader source code and the shader type.
GLuint createShader(std::string sourceCode, GLenum shaderType)
{
	// glCreateShader, creates a shader given a type (such as GL_VERTEX_SHADER) and returns a GLuint reference to that shader.
	GLuint shader = glCreateShader(shaderType);
	const char *shader_code_ptr = sourceCode.c_str(); // We establish a pointer to our shader code string
	const int shader_code_size = sourceCode.size();   // And we get the size of that string.

	// glShaderSource replaces the source code in a shader object
	// It takes the reference to the shader (a GLuint), a count of the number of elements in the string array (in case you're passing in multiple strings), a pointer to the string array 
	// that contains your source code, and a size variable determining the length of the array.
	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader); // This just compiles the shader, given the source code.

	GLint isCompiled = 0;

	// Check the compile status to see if the shader compiled correctly.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		char infolog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, infolog);

		// Print the compile error.
		std::cout << "The shader failed to compile with the error:" << std::endl << infolog << std::endl;

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.

		// NOTE: I almost always put a break point here, so that instead of the program continuing with a deleted/failed shader, it stops and gives me a chance to look at what may 
		// have gone wrong. You can check the console output to see what the error was, and usually that will point you in the right direction.
	}

	return shader;
}

// Initialization code
void init()
{
	// Initializes the glew library
	glewInit();

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);
}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions
// This runs once every physics timestep.
void update()
{
	// These variables hold the pressure values on the containers.
	float LeftPressure, rightPressure;
	
	// Calculate pressure on each side
	big.pressure = (big.height) * gravity * density;
	small.pressure = (small.height) * gravity * density;

	// Since we are adding pressure to the left (big) container, add the external pressure to the pressure caused by the volume of water.
	LeftPressure = big.pressure + externalPressure;
	rightPressure = small.pressure;
	
	//If the pressure is the same on both sides, then the fluid is already at equilibrium. 
	if (LeftPressure == rightPressure)
	{
		return;
	}

	// Calculate the height on the right container which is needed to maintain equilibrium.
	float h = LeftPressure / (gravity * density);
	
	//Calculate the change in height of the small container.
	float change = small.height;
	change -= h;
	
	// We are only taking the average of the height to cause quilibrium. In reality, the level on the smaller side
	// oscillates along with the water level on the other side and eventually comes to an equilibrium due to 
	// external dampening forces. Since we are simulating an isolated system under no external forces, the water level will continue to osscilate infinitly.
	change /= 2.0f;

	//This is to ensure that if one side of the apparatus is completly drained, then variation in pressure will have no effect unless it is to fill the drained side.
	if (big.height + change < 0.0f || h + change < 0.0f)
	{
		return;
	}

	//Change in height of the bigger side
	big.height += change;
	
	h += change;

	//Set the vertices of the smaller and the bigger side.
	small.topLeft.y = h - 0.5f;
	small.topRight.y = h - 0.5f; 

	big.topLeft.y = big.height - 0.5f;
	big.topRight.y = big.height - 0.5f;

	small.height = small.topLeft.y - small.bottomLeft.y;
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(0);

	// Draw the Gameobjects
	glLineWidth(2.5);
	glColor3f(0.2, 0.2, 0.8);
	glBegin(GL_QUADS);
	
	//Big container
	glVertex2fv((float *) &big.bottomLeft);
	glVertex2fv((float *)&big.bottomRight);
	glVertex2fv((float *)&big.topRight);
	glVertex2fv((float *)&big.topLeft);
	
	//small container
	glVertex2fv((float *)&small.bottomLeft);
	glVertex2fv((float *)&small.bottomRight);
	glVertex2fv((float *)&small.topRight);
	glVertex2fv((float *)&small.topLeft);

	//Tube joining the two containers
	glVertex2f(big.bottomRight.x, big.bottomRight.y);
	glVertex2f(small.bottomLeft.x, small.bottomLeft.y);
	glVertex2f(small.bottomLeft.x, small.bottomLeft.y + 0.02f);
	glVertex2f(big.bottomRight.x, big.bottomRight.y + 0.02f);

	glEnd();

	//The draw piston function draws the piston over the larger side. 
	//This is mainly done for representational purposes i.e. to make the example more obvious.
	drawPiston();
}

// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//This set of controls are used to move one point (point1) of the line.
	if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
		externalPressure += 0.1f;
	if (key == GLFW_KEY_LEFT_SHIFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		externalPressure -= 0.1f;
	//if (key == GLFW_KEY_S && action == GLFW_PRESS)
	//	Line.point1.y -= movrate;
	//if (key == GLFW_KEY_D && action == GLFW_PRESS)
	//	Line.point1.x += movrate;

	////This set of controls are used to move one point (point2) of the line.
	//if (key == GLFW_KEY_I && action == GLFW_PRESS)
	//	Line.point2.y += movrate;
	//if (key == GLFW_KEY_J && action == GLFW_PRESS)
	//	Line.point2.x -= movrate;
	//if (key == GLFW_KEY_K && action == GLFW_PRESS)
	//	Line.point2.y -= movrate;
	//if (key == GLFW_KEY_L && action == GLFW_PRESS)
	//	Line.point2.x += movrate;
}

#pragma endregion util_functions


void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 800, "HydroDynamics", nullptr, nullptr);

	std::cout << "\n This example demostrates the hydrodynamic property of fluid. \n Namely trhe isotropic behaviour of fluid i.e. it maintains the same level across \n different containers despite the difference in shapes and sizes when connected by a tube at the bottom. ";
	std::cout << "\n\n\n\n Use \" Space\" to add pressure on the bigger container using the piston. \n Use \"Left Shift\" to reduce pressure on the bigger side using the piston.";

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	setup();
	// Sets the number of screen updates to wait before swapping the buffers.
	// Setting this to zero will disable VSync, which allows us to actually get a read on our FPS. Otherwise we'd be consistently getting 60FPS or lower, 
	// since it would match our FPS to the screen refresh rate.
	// Set to 1 to enable VSync.
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	// Sends the funtion as a funtion pointer along with the window to which it should be applied to.
	glfwSetKeyCallback(window, key_callback);

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to update() which will update the gameobjects.
		update();

		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		// Remember, you're rendering to the back buffer, then once rendering is complete, you're moving the back buffer to the front so it can be displayed.
		glfwSwapBuffers(window);

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// After the program is over, cleanup your data!
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	// Note: If at any point you stop using a "program" or shaders, you should free the data up then and there.

	// Frees up GLFW memory
	glfwTerminate();
}