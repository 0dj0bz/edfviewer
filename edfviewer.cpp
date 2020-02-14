#include <iostream>
#include <string>
#include <cmath>
#include <string.h>
#define GL_GLEXT_PROTOTYPES

// OpenGL Graphics includes
//#include <GL/gl.h>
//#include <GL/glext.h>
#include "glew.h"
#include <GL/freeglut.h>

#include "edftest.h"

#define REFRESH_DELAY     10 //ms

struct Vertex
{
  GLfloat position[3];
  GLfloat normal[3];
  GLubyte color[4];
};

struct vec4
{
	float x;
	float y;
	float z;
	float w;
};

// GLOBAL VARIABLES
long g_errorCount = 0;
// constants
const unsigned int window_width  = 512;
const unsigned int window_height = 512;

const unsigned int mesh_width    = 256;
const unsigned int mesh_height   = 256;

const char * appString = "EDF Viewer v0.1";



// Auto-Verification Code
unsigned fpsCount = 0;        // FPS count for averaging
int fpsLimit = 1;        // FPS limit for sampling
int g_Index = 0;
float avgFPS = 0.0f;
unsigned int frameCount = 0;
unsigned int g_TotalErrors = 0;
bool g_bQAReadback = false;

// vbo variables
GLuint vbo;
GLuint params;

// vao variables
GLuint vao;
GLuint buffer;
GLuint colorBuffer;

static Vertex vertices[mesh_width][mesh_height];

GLuint vs_program;

// mouse controls
int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;
float translate_z = -3.0;

float g_fAnim = 0.0;

EEGStudy *rstudy;

static const GLchar * vertex_shader_source [] = 
{

	"#version 450 core                                            \n"
	"                                                             \n"
	"layout (location = 0) in vec4 in_Position;                   \n"
	"layout (location = 1) in vec4 in_Color;                      \n"
	"uniform float time;                                          \n"
	"out vec4 ex_fs_color;                                        \n"
	"                                                             \n"
	"void main(void)                                              \n"
	"{                                                            \n"
	"	                                                          \n"
	"   gl_Position = in_Position;                                \n"
	// "   gl_Position.x = sin(in_Position.x+time);                  \n"
	// "   gl_Position.y = cos(in_Position.y+time);                  \n"
	// "   gl_Position.z = cos(in_Position.z+time);                  \n"	
//	"   ex_fs_color = vec4(abs(cos(in_Position.x+time)), abs(sin(in_Position.y+time)), abs(sin(in_Position.z+time)), 1.0-abs(sin(in_Position.z+time)) );                   \n"
	"	ex_fs_color = vec4(1.0, 0.0, 0.0, 1.0);                   \n"
	"}                                                            \n"
};

static const GLchar * geometry_shader_source [] = 
{
	"#version 450 core                                            \n"
	"                                                             \n"
	"layout (lines, invocations = 10) in;                         \n"
	"layout (line_strip) out;                                     \n"
	"layout (max_vertices = 8) out;                               \n"
	"in vec4 ex_gs_color[];                                       \n"
	"out vec4 ex_fs_color;                                        \n"
    "void main(void)                                              \n"
    "{                                                            \n"
    "	for (int i=0; i<gl_in.length(); i++)                      \n"
    "	{                                                         \n"
	"       gl_Position = gl_in[i].gl_Position;                   \n"
	"       ex_fs_color = ex_gs_color[i];                         \n"
	"       gl_ViewportIndex = gl_InvocationID;                   \n"
	"       EmitVertex();                                         \n"
	"   }                                                         \n"
	"   EndPrimitive();                                           \n"
    "}                                                            \n"
};

static const GLchar * fragment_shader_source [] = 
{
	"#version 450 core                                            \n"
	"                                                             \n"
	"in vec4 ex_fs_color;                                         \n"
	"out vec4 out_color;                                          \n"
	"void main(void)                                              \n"
	"{                                                            \n"
	"   out_color = ex_fs_color;                                  \n"
	"}                                                            \n"
};


////////////////////////////////////////////////////////////////////////////////
//! Display callback
////////////////////////////////////////////////////////////////////////////////
void display()
{
	++frameCount;


	glUniform1f(glGetUniformLocation(vs_program, "time"), g_fAnim);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// glViewport(0, 0, window_width*0.5f, window_height*0.5f);
	glDrawArrays(GL_LINE_STRIP, 0, 4);

	// glViewport(window_width*0.5f, window_height*0.5f, (window_width*0.5f), (window_height*0.5f));
	// glDrawArrays(GL_LINE_STRIP, 0, 4);

	glutSwapBuffers();
	glutPostRedisplay();

 	g_fAnim += 0.0001f;
}

float ** makeVertices(short arry[], int numSignals, int numElems)
{

	float ** vertArry = (float **) malloc(numElems*sizeof(float **));
	for (int i=0;i<numElems;i++)
		vertArry[i] = (float *) malloc(4*sizeof(float*));

	std::cout << "malloc successful!" << std::endl;

	for (int j=0;j<numElems;j++)
	{
		// std::cout << "j: " << j << std::endl;
		vertArry[j][0] = (float)j/(float)numElems;
		vertArry[j][1] = (float) arry[j]/(float)numElems;
		vertArry[j][2] = 0.0f;
		vertArry[j][3] = 1.0f;
		// std::cout << "j: " << j << " vertArry (x,y): (" << vertArry[j][0] << ", " << vertArry[j][1] << ")" << std::endl;
	}

	return vertArry;

}

void createVBO(void)
{
	GLfloat vertices[] = {
		-0.2f, -0.2f, 0.0f, 1.0f,
		 0.0f,  0.2f, 0.0f, 1.0f,
		 0.2f, -0.2f, 0.0f, 1.0f,
		-0.2f, -0.2f, 0.0f, 1.0f
	};

	GLfloat colors[] = {
		1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
	};

	GLenum errorCheckValue = glGetError();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	float **d = makeVertices(&rstudy->signalData->data[0][0], 30, 256*8);

	GLfloat *dpoints = (GLfloat*)malloc(256*8*4*sizeof(GLfloat));

	for (int i=0;i<256*8;i++)
		for (int j=0;j<4;j++)
		{
			dpoints[(i*4)+j] = d[i][j];
			// std::cout << "dpoints[%d*4+%d]: " << dpoints[(i*4)+j] << std::endl;
		}

	glBufferData(GL_ARRAY_BUFFER, 8*256*sizeof(GLfloat), dpoints, GL_STATIC_DRAW);

	errorCheckValue = glGetError();

	if (errorCheckValue != GL_NO_ERROR)
	{
		fprintf(stderr,
			"ERROR: glBufferData failed: %s \n",
			gluErrorString(errorCheckValue)
		);
		exit(-1);
	}
	else
		std::cout << "glBufferData successful." << std::endl;

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	errorCheckValue = glGetError();

	if (errorCheckValue != GL_NO_ERROR)
	{
		fprintf(stderr,
			"ERROR: glVertexAttribPointer failed: %s \n",
			gluErrorString(errorCheckValue)
		);
		exit(-1);
	}
	else
		std::cout << "glVertexAttribPointer successful." << std::endl;


	glEnableVertexAttribArray(0);

	// glGenBuffers(1, &colorBuffer);
	// glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	// glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	// glEnableVertexAttribArray(1);

	errorCheckValue = glGetError();

	if (errorCheckValue != GL_NO_ERROR)
	{
		fprintf(stderr,
			"ERROR: glEnableVertexAttribArray: %s \n",
			gluErrorString(errorCheckValue)
		);
		exit(-1);
	}
	else
		std::cout << "glEnableVertexAttribArray successful." << std::endl;

}

void destroyVBO(void)
{
	GLenum errorCheckValue;

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &colorBuffer);
	glDeleteBuffers(1, &vbo);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);

	errorCheckValue = glGetError();

	if (errorCheckValue != GL_NO_ERROR)
	{
		fprintf(stderr,
			"ERROR: Could not destroy the VBO: %s \n",
			gluErrorString(errorCheckValue)
		);
		exit(-1);
	}	
}

void createShaders()
{
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vs, 1, vertex_shader_source, NULL);
	glCompileShader(vs);
	GLint log_length;
	GLchar *compile_log;
	GLint compile_status;

	glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_status);

	if (compile_status == GL_TRUE)
		std::cout << "Vertex Shader compilation successful." <<std::endl;
	else
	{
		std::cout << "Vertex Shader compilation failed." <<std::endl;

		compile_log = (GLchar *)std::malloc(log_length*sizeof(GLchar));


		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &log_length);

		compile_log = (GLchar *)std::malloc(log_length*sizeof(GLchar));

		glGetShaderInfoLog(vs, log_length, NULL, compile_log);

		std::cout << "shader compile log: " << std::endl;
		std::cout << compile_log << std::endl;

		std::free(compile_log);

	}
/////////////////////////

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fs, 1, fragment_shader_source, NULL);
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_status);

	if (compile_status == GL_TRUE)
		std::cout << "Fragment Shader compilation successful." <<std::endl;
	else
	{
		std::cout << "Fragment Shader compilation failed." <<std::endl;

		compile_log = (GLchar *)std::malloc(log_length*sizeof(GLchar));


		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &log_length);

		compile_log = (GLchar *)std::malloc(log_length*sizeof(GLchar));

		glGetShaderInfoLog(fs, log_length, NULL, compile_log);

		std::cout << "shader compile log: " << std::endl;
		std::cout << compile_log << std::endl;

		std::free(compile_log);

	}

////////////////////////	

	GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);

	glShaderSource(gs, 1, geometry_shader_source, NULL);
	glCompileShader(gs);

	glGetShaderiv(gs, GL_COMPILE_STATUS, &compile_status);

	if (compile_status == GL_TRUE)
		std::cout << "Geometry Shader compilation successful." <<std::endl;
	else
	{
		std::cout << "Geometry Shader compilation failed." <<std::endl;

		compile_log = (GLchar *)std::malloc(log_length*sizeof(GLchar));


		glGetShaderiv(gs, GL_INFO_LOG_LENGTH, &log_length);

		compile_log = (GLchar *)std::malloc(log_length*sizeof(GLchar));

		glGetShaderInfoLog(gs, log_length, NULL, compile_log);

		std::cout << "shader compile log: " << std::endl;
		std::cout << compile_log << std::endl;

		std::free(compile_log);

	}

////////////////////////
	vs_program = glCreateProgram();
	glAttachShader(vs_program, vs);
//	glAttachShader(vs_program, gs);
	glAttachShader(vs_program, fs);

//	glProgramParameteri(vs_program, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glLinkProgram(vs_program);
	glUseProgram(vs_program);

	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteShader(gs);

	glGetProgramiv(vs_program, GL_LINK_STATUS, &compile_status);

	if (compile_status == GL_TRUE)
		std::cout << "Shader link successful." <<std::endl;
	else
	{
		std::cout << "Shader link failed." <<std::endl;

		glGetProgramiv(vs_program, GL_INFO_LOG_LENGTH, &log_length);

		compile_log = (GLchar *)std::malloc(log_length*sizeof(GLchar));

		glGetProgramInfoLog(vs_program, log_length, NULL, compile_log);

		std::cout << "shader link log: " << std::endl;
		std::cout << compile_log << std::endl;

		std::free(compile_log);

	}

	GLint outputs;

	glGetProgramInterfaceiv(vs_program, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &outputs);

	static const GLenum props[] = { GL_TYPE, GL_LOCATION, GL_REFERENCED_BY_VERTEX_SHADER};

	GLint i;
	GLint params[2];
	GLchar name[64];

	const char * type_name;

	for (i=0;i<outputs; i++)
	{
		glGetProgramResourceName(vs_program, GL_PROGRAM_INPUT, i, sizeof(name), NULL, name);

		glGetProgramResourceiv(vs_program, GL_PROGRAM_INPUT, i, 2, props, 2, NULL, params);

		
		printf("Index %d %s @ location %d \n", i, name, params[1]);

	}
}

void destroyShaders(void)
{
	GLenum errorCheckValue = glGetError();

	glDeleteProgram(vs_program);

	if (errorCheckValue != GL_NO_ERROR)
	{
		fprintf(stderr,
			"ERROR: Could not destroy shaders: %s \n",
			gluErrorString(errorCheckValue)
		);
		exit(-1);
	}	
}

void idleFunction(void)
{
	glutPostRedisplay();
}

void timerFunction(int value)
{

	if (0 != value)
	{
		char *tempStr = (char *) malloc(512 + strlen(appString));

		sprintf(tempStr, "%s: %d FPS @ %d x %d",
			appString,
			frameCount * 4,
			window_width,
			window_height
		);

		glutSetWindowTitle(tempStr);
		free(tempStr);
	}
	
	frameCount = 0;
	glutTimerFunc(250, timerFunction, 1);	

}


////////////////////////////////////////////////////////////////////////////////
//! Keyboard events handler
////////////////////////////////////////////////////////////////////////////////
void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    switch (key)
    {
        case (27) :
            #if defined(__APPLE__) || defined(MACOSX)
                exit(EXIT_SUCCESS);
            #else
                glutDestroyWindow(glutGetWindow());
                return;
            #endif
    }
}

////////////////////////////////////////////////////////////////////////////////
//! Mouse event handlers
////////////////////////////////////////////////////////////////////////////////
void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        mouse_buttons |= 1<<button;
    }
    else if (state == GLUT_UP)
    {
        mouse_buttons = 0;
    }

    mouse_old_x = x;
    mouse_old_y = y;
}

void motion(int x, int y)
{
    float dx, dy;
    dx = (float)(x - mouse_old_x);
    dy = (float)(y - mouse_old_y);

    if (mouse_buttons & 1)
    {
        rotate_x += dy * 0.2f;
        rotate_y += dx * 0.2f;
    }
    else if (mouse_buttons & 4)
    {
        translate_z += dy * 0.01f;
    }

    mouse_old_x = x;
    mouse_old_y = y;
}

void cleanup(void)
{
	destroyShaders();
	destroyVBO();
}
////////////////////////////////////////////////////////////////////////////////
//! Initialize GL
////////////////////////////////////////////////////////////////////////////////
bool initGL(int *argc, char **argv)
{
    glutInit(argc, argv);
	glutInitContextVersion(4, 6);

    // This option returns control to main() when the window is closed
    // Enables graceful shutdown
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(window_width, window_height);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow(argv[0]);

    if (glewInit())
    {
    	std::cout << "Unable to initialize GLEW..." << std::endl;
    	exit(EXIT_FAILURE);
    }

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

	glutIdleFunc(idleFunction);
	glutTimerFunc(0, timerFunction, 0);
	glutCloseFunc(cleanup);

    // default initialization
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    createShaders();
    createVBO();



    return true;
}



int main(int argc, char **argv)
{

	rstudy = loadEDFfile("00000000_s001_t000.edf", false);

	std::cout << "Inside main..." << std::endl;
	
	// for (int j=0;j<stoi(rstudy->header->numSignals);j++)
	//     for (int i=0;i<stoi(rstudy->header->numDataRecs);i++)
	//     	cout << "Signal[" << j << "] Data Record[" << i << "]\t - " << rstudy->signalData->data[j][i] << endl;

    printf("%s starting...\n", appString);

    printf("\n");


    // First initialize OpenGL context, so we can properly set the GL for CUDA.
    // This is necessary in order to achieve optimal performance with OpenGL/CUDA interop.
    if (false == initGL(&argc, argv))
    {
        return -1;
    }

    // start rendering mainloop
    glutMainLoop();

    printf("%s completed, returned %s\n", appString, (g_errorCount == 0) ? "OK" : "ERROR!");

    exit(g_errorCount == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}