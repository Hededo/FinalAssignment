#ifndef _finalassignment_cpp
#define _finalassignment_cpp

#include <sb7.h>
#include <vmath.h>

#include <object.h>
#include <sb7ktx.h>
#include <shader.h>
#include <assert.h>
#include <cmath>

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "objObject.cpp"
#include "lodepng.h"

#define PI 3.14159265
#define DEPTH_TEXTURE_SIZE      4096
#define FRUSTUM_DEPTH           1000

class final_app : public sb7::application
{
//Forward Declaration of Public functions
#pragma region protected
public:
	final_app()
		: per_fragment_program(0),
		floorProgram(0) ,
		wallProgram(0),
		toonProgram(0),
		flatColorProgram(0),
		skybox_prog(0),
		point_prog(0)
	{
	}
#pragma endregion

//Forward Declaration of Protected functions and variables
#pragma region protected
protected:
	void init()
	{
		static const char title[] = "Final Assignment";

		sb7::application::init();

		memcpy(info.title, title, sizeof(title));

		info.windowWidth = 512;
		info.windowHeight = 512;
	}

	void startup();
	void render(double currentTime);

	//Listener
	void onKey(int key, int action);
	void onMouseMove(int x, int y);
	void onMouseButton(int button, int action);
	//_________________________________________

	vmath::vec3 getArcballVector(int x, int y);

	void load_shaders();

	int FindUnusedParticle();
	void SortParticles();

	GLuint          per_fragment_program;
	GLuint          floorProgram;
	GLuint          wallProgram;
	GLuint          toonProgram;
	GLuint          flatColorProgram;
	GLuint          skybox_prog;
	GLuint          point_prog;

	GLuint          tex_floor;
	GLuint          tex_floor_normal;
	GLuint          tex_skybox;
	GLuint          tex_particle;

	GLuint          depthBuffer;
	GLuint          depthTexture;

	GLuint          quad_vao;

	//Where uniforms are defined
	struct uniforms_block
	{
		vmath::mat4     mv_matrix;
		vmath::mat4     model_matrix;
		vmath::mat4     view_matrix;
		vmath::mat4     proj_matrix;
		vmath::vec4     uni_color;
		vmath::vec4     lightPos;
		vmath::vec4	    useUniformColor;
		vmath::vec4	    invertNormals;
	};

	GLuint          uniforms_buffer;

	// Variables for mouse interaction
	bool bPerVertex;
	bool bShiftPressed = false;
	bool bZoom = false;
	bool bRotate = false;
	bool bPan = false;

	int iWidth = info.windowWidth;
	int iHeight = info.windowHeight;

	// Rotation and Translation matricies for moving the camera by mouse interaction.
	vmath::mat4 rotationMatrix = vmath::mat4::identity();
	vmath::mat4 translationMatrix = vmath::mat4::identity();

#pragma region Colors
	const GLfloat skyBlue[4] = { 0.529f, 0.808f, 0.922f, 1.0f };
	const GLfloat ones[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const vmath::vec4 white = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	const vmath::vec4 orange = vmath::vec4(1.0f, 0.5f, 0.0f, 1.0f);
	const vmath::vec4 purple = vmath::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	const vmath::vec4 gray = vmath::vec4(0.8f, 0.8f, 0.8f, 1.0f);

#pragma endregion

#pragma region Boolean Vectors
const vmath::vec4 falseVec = vmath::vec4(0.0f, 0.0f, 0.0f, 0.0f);
const vmath::vec4 trueVec = vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f);
#pragma endregion

#pragma region Geometery
ObjObject * cube;
ObjObject * sphere;
ObjObject * teapot;
ObjObject * quad;
ObjObject * humvee;

#pragma endregion

#pragma region Particle Vars
int LastUsedParticle = 0;
int ParticlesCount = 0;

// CPU representation of a particle
struct Particle {
	vmath::vec3 pos, speed;
	unsigned char r, g, b, a; // Color
	float size, angle, weight;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

int MaxParticles = 100000;
Particle * ParticlesContainer;

// The VBO containing the 4 vertices of the particles.
// Thanks to instancing, they will be shared by all particles.
const GLfloat g_vertex_buffer_data[12] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	-0.5f, 0.5f, 0.0f,
	0.5f, 0.5f, 0.0f,
};

GLuint billboard_vertex_buffer;
GLuint particles_position_buffer;
GLuint particles_color_buffer;

GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
GLubyte* g_particule_color_data = new GLubyte[MaxParticles * 4];

#pragma endregion

#pragma endregion

#pragma region private
private:
	// Variables for mouse position to solve the arcball vectors
	int iPrevMouseX = 0;
	int iPrevMouseY = 0;
	int iCurMouseX = 0;
	int iCurMouseY = 0;

	// Scale of the objects in the scene
	float fScale = 7.0f;

	// Initial position of the camera
	float fXpos = 0.0f;
	float fYpos = 0.0f;
	float fZpos = 75.0f;

	// Initial light pos
	vmath::vec4 initalLightPos = vmath::vec4(10.0f, 20.0f, -3.0f, 1.0f);
	vmath::vec3 lightPosOffset = vmath::vec3(0, 0, 0);

#pragma endregion
};


void final_app::startup()
{
	load_shaders();
	cube = new ObjObject("bin\\media\\objects\\cube.obj");
	sphere = new ObjObject("bin\\media\\objects\\sphere.obj");
	teapot = new ObjObject("bin\\media\\objects\\wt_teapot.obj");
	quad = new ObjObject("bin\\media\\objects\\quad.obj");
	humvee = new ObjObject("bin\\media\\objects\\humvee.obj");

#pragma region Buffer For Uniform Block
	glGenBuffers(1, &uniforms_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniforms_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms_block), NULL, GL_DYNAMIC_DRAW);
#pragma endregion

#pragma region Load Textures
	unsigned * square_tex_width = new unsigned;
	unsigned * square_tex_height = new unsigned;

	std::vector<unsigned char> texture_data = loadImageFromFile("bin\\media\\textures\\GraniteWall-ColorMap.png", square_tex_width, square_tex_height);
	// Enable the texture for OpenGL.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); //GL_NEAREST = no smoothing
	// Generate a name for the texture
	glGenTextures(1, &tex_floor); //GLuint tex_floor
	// Now bind it to the context using the GL_TEXTURE_2D binding point
	glBindTexture(GL_TEXTURE_2D, tex_floor); 
	// Specify the amount of storage we want to use for the texture
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, *square_tex_width, *square_tex_height);
	// Assume the texture is already bound to the GL_TEXTURE_2D target
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, *square_tex_width, *square_tex_height, GL_RGBA, GL_UNSIGNED_BYTE, &texture_data[0]);
	//_______________________________________________________________________________________________________________
	texture_data = loadImageFromFile("bin\\media\\textures\\GraniteWall-NormalMap.png", square_tex_width, square_tex_height);
	// Generate a name for the texture
	glGenTextures(1, &tex_floor_normal); //GLuint tex_floor_normal
								  // Now bind it to the context using the GL_TEXTURE_2D binding point
	glBindTexture(GL_TEXTURE_2D, tex_floor_normal);
	// Specify the amount of storage we want to use for the texture
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, *square_tex_width, *square_tex_height);
	// Assume the texture is already bound to the GL_TEXTURE_2D target
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, *square_tex_width, *square_tex_height, GL_RGBA, GL_UNSIGNED_BYTE, &texture_data[0]);
	//_______________________________________________________________________________________________________________
	tex_skybox = sb7::ktx::file::load("bin\\media\\textures\\mountaincube.ktx");
	//_______________________________________________________________________________________________________________
	//texture_data = loadImageFromFile("bin\\media\\textures\\particle.png", square_tex_width, square_tex_height);
	texture_data = loadImageFromFile("bin\\media\\textures\\pikachu.png", square_tex_width, square_tex_height);
	// Enable the texture for OpenGL.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); //GL_NEAREST = no smoothing
																					  // Generate a name for the texture
	glGenTextures(1, &tex_particle); //GLuint tex_floor
								  // Now bind it to the context using the GL_TEXTURE_2D binding point
	glBindTexture(GL_TEXTURE_2D, tex_particle);
	// Specify the amount of storage we want to use for the texture
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, *square_tex_width, *square_tex_height);
	// Assume the texture is already bound to the GL_TEXTURE_2D target
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, *square_tex_width, *square_tex_height, GL_RGBA, GL_UNSIGNED_BYTE, &texture_data[0]);

#pragma endregion

#pragma region OPENGL Settings
    glFrontFace(GL_CW); //glFrontFace(GLenum mode) In a scene composed entirely of opaque closed surfaces, back-facing polygons are never visible.
	glEnable(GL_DEPTH_TEST); //glEnable(GLenum cap) glEnable and glDisable enable and disable various capabilities.
	glDepthFunc(GL_LEQUAL);	//glDepthFunc(GLenum func) specifies the function used to compare each incoming pixel depth value with the depth value present in the depth buffer. 
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#pragma endregion

#pragma region Particle Init

	ParticlesContainer = new Particle[MaxParticles];

	for (int i = 0; i<MaxParticles; i++) {
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

#pragma endregion
}

void final_app::render(double currentTime)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	const float deltaTime = (float)currentTime * 0.1f;

#pragma region Calculations for mouse interaction camera rotation and translation matrix
	float fAngle = 0.0f;
	vmath::vec3 axis_in_camera_coord = (0.0f, 1.0f, 0.0f);
	if (iCurMouseX != iPrevMouseX || iCurMouseY != iPrevMouseY) {
		// Arcball Rotation
		if (bRotate) {
			vmath::vec3 va = getArcballVector(iPrevMouseX, iPrevMouseY);
			vmath::vec3 vb = getArcballVector(iCurMouseX, iCurMouseY);
			fAngle = acos(fmin(1.0f, vmath::dot(va, vb)));
			axis_in_camera_coord = vmath::cross(va, vb);
			axis_in_camera_coord = vmath::normalize(axis_in_camera_coord);
			iPrevMouseX = iCurMouseX;
			iPrevMouseY = iCurMouseY;
			rotationMatrix *= vmath::rotate(vmath::degrees(fAngle), axis_in_camera_coord);
		}
		// Zoom in and out
		if (bZoom) {
			fZpos += (iCurMouseY - iPrevMouseY);
			if (fZpos > 500)
			{
				fZpos = 500;
			}
			else if (fZpos < 10)
			{
				fZpos = 10;
			}
			iPrevMouseY = iCurMouseY;
			iPrevMouseX = iCurMouseX;
		}
		// Pan camera left, right, up, and down
		if (bPan) {
			fXpos += (iCurMouseX - iPrevMouseX);
			fYpos += (iCurMouseY - iPrevMouseY);
			iPrevMouseY = iCurMouseY;
			iPrevMouseX = iCurMouseX;
			translationMatrix = vmath::translate(fXpos / (info.windowWidth / fZpos), -fYpos / (info.windowWidth / fZpos), 0.0f);
		}
	}
#pragma endregion

	glViewport(0, 0, info.windowWidth, info.windowHeight);

	// Create sky blue background
	glClearBufferfv(GL_COLOR, 0, skyBlue);
	glClearBufferfv(GL_DEPTH, 0, ones);

	glBindTexture(GL_TEXTURE_2D, depthTexture);

	// Set up view and perspective matrix
	vmath::vec3 view_position = vmath::vec3(0.0f, 0.0f, fZpos);
	vmath::mat4 view_matrix = vmath::lookat(view_position,
		vmath::vec3(0.0f, 0.0f, 0.0f),
		vmath::vec3(0.0f, 1.0f, 0.0f));
	view_matrix *= translationMatrix;
	view_matrix *= rotationMatrix;

	vmath::mat4 perspective_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);

	glUnmapBuffer(GL_UNIFORM_BUFFER); //release the mapping of a buffer object's data store into the client's address space
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
	uniforms_block * block = (uniforms_block *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(uniforms_block), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

#pragma region Uniforms that remain constant for all geometery
	block->proj_matrix = perspective_matrix;
	vmath::vec4 lightPos = vmath::vec4(initalLightPos[0], initalLightPos[1], initalLightPos[2], 1.0f);
	block->lightPos = vmath::vec4(initalLightPos[0] + lightPosOffset[0], initalLightPos[1] + lightPosOffset[1], initalLightPos[2] + lightPosOffset[2], 1.0f);

#pragma endregion

#pragma region Draw Room
	cube->BindBuffers();

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
	block = (uniforms_block *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(uniforms_block), GL_MAP_WRITE_BIT);

	GLint skybox_location = glGetUniformLocation(skybox_prog, "tex_cubemap");

	glUseProgram(skybox_prog);
	glUniform1i(skybox_location, 0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_skybox);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    vmath::mat4 model_matrix = vmath::scale(250.0f);
	block->model_matrix = model_matrix;
	block->mv_matrix = view_matrix * model_matrix;
	block->view_matrix = view_matrix;
	block->uni_color = gray;
	block->useUniformColor = trueVec;
	block->invertNormals = trueVec;

	glCullFace(GL_BACK);
	cube->Draw();
#pragma endregion

#pragma region Fountain Base Stand
	cube->BindBuffers();

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
	block = (uniforms_block *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(uniforms_block), GL_MAP_WRITE_BIT);

	glUseProgram(per_fragment_program);

	model_matrix =
		vmath::translate(0.0f, -18.0f, -5.0f) *
		vmath::scale(12.0f);
	block->model_matrix = model_matrix;
	block->mv_matrix = view_matrix * model_matrix;
	block->view_matrix = view_matrix;
	block->uni_color = orange;
	block->useUniformColor = trueVec;
	block->invertNormals = falseVec;

	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	cube->Draw();
#pragma endregion

#pragma region Fountain Middle Stand

	sphere->BindBuffers();

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
	block = (uniforms_block *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(uniforms_block), GL_MAP_WRITE_BIT);
	glUseProgram(per_fragment_program);

	model_matrix =
		vmath::translate(0.0f, -4.0f, -5.0f) *
		vmath::scale(11.0f);
	block->model_matrix = model_matrix;
	block->mv_matrix = view_matrix * model_matrix;
	block->view_matrix = view_matrix;
	block->uni_color = purple;

	sphere->Draw();

#pragma endregion

#pragma region Fountain Top

	cube->BindBuffers();
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
	block = (uniforms_block *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(uniforms_block), GL_MAP_WRITE_BIT);

	glUseProgram(per_fragment_program);

	model_matrix =
		vmath::translate(0.0f, 7.0f, -5.0f) *
		vmath::scale(5.0f);
	block->model_matrix = model_matrix;
	block->mv_matrix = view_matrix * model_matrix;
	block->view_matrix = view_matrix;
	block->uni_color = orange;
	block->useUniformColor = trueVec;
	block->invertNormals = falseVec;

	cube->Draw();
#pragma endregion



#pragma region Draw Light Source
	sphere->BindBuffers();

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
	block = (uniforms_block *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(uniforms_block), GL_MAP_WRITE_BIT);

	glUseProgram(flatColorProgram);

	model_matrix =
		vmath::translate(block->lightPos[0], block->lightPos[1], block->lightPos[2]);
	block->model_matrix = model_matrix;
	block->mv_matrix = view_matrix * model_matrix;
	block->view_matrix = view_matrix;
	block->uni_color = white;
	block->useUniformColor = trueVec;
	block->invertNormals = falseVec;

	sphere->Draw();
#pragma endregion

#pragma region Draw Floor
	cube->BindBuffers();

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
	block = (uniforms_block *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(uniforms_block), GL_MAP_WRITE_BIT);

	GLint tex_floor_location = glGetUniformLocation(floorProgram, "colorTexture");
	GLint bump_floor_location = glGetUniformLocation(floorProgram, "normalTexture");

	glUseProgram(floorProgram);
	glUniform1i(tex_floor_location, 1);
	glUniform1i(bump_floor_location, 0);

	glBindTexture(GL_TEXTURE_2D, tex_floor);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, tex_floor_normal);
	glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1

	model_matrix =
		vmath::translate(0.0f, -30.0f, 0.0f) *
		vmath::scale(40.0f, 1.0f, 40.0f);
	block->model_matrix = model_matrix;
	block->mv_matrix = view_matrix * model_matrix;
	block->view_matrix = view_matrix;
	block->uni_color = purple;
	block->useUniformColor = trueVec;
	block->invertNormals = falseVec;

	cube->Draw();
#pragma endregion


#pragma region Point Sprite
	glBindTexture(GL_TEXTURE_2D, tex_particle);
	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, tex_particle);
	glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1

	glUnmapBuffer(GL_UNIFORM_BUFFER); //release the mapping of a buffer object's data store into the client's address space
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
	block = (uniforms_block *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(uniforms_block), GL_MAP_WRITE_BIT);

	model_matrix = vmath::translate(25.0f, 10.0f, -40.0f);
	block->model_matrix = model_matrix;
	block->mv_matrix = view_matrix * model_matrix;
	block->view_matrix = view_matrix;

	glUseProgram(point_prog);

	glPointSize(40.0f);
	glCullFace(GL_FRONT);
    glDrawArrays(GL_POINTS, 0, 1);

#pragma endregion

#pragma region Particles
	// Generate 10 new particule each millisecond,
	// but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
	// newparticles will be huge and the next frame even longer.
	int newparticles = (int)(deltaTime*10000.0);
	if (newparticles > (int)(0.016f*10000.0))
		newparticles = (int)(0.016f*10000.0);

	// Simulate all particles
	int ParticlesCount = 0;
	for (int i = 0; i<MaxParticles; i++) {

		Particle& p = ParticlesContainer[i]; // shortcut

		if (p.life > 0.0f) {

			// Decrease life
			p.life -= deltaTime;
			if (p.life > 0.0f) {

				// Simulate simple physics : gravity only, no collisions
				p.speed += vmath::vec3(0.0f, -9.81f, 0.0f) * (float)deltaTime * 0.5f;
				p.pos += p.speed * (float)deltaTime;
				//vmath::vec3 CameraPosition = vmath::vec3(0.0f, 0.0f, 0.0f);
				//p.cameradistance = vmath::length2(p.pos - CameraPosition);

				p.cameradistance = vmath::length(vmath::vec2(p.pos[0], p.pos[1])); //assumption that camera is at origin

				//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)deltaTime;

				// Fill the GPU buffer
				g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos[0];
				g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos[1];
				g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos[2];

				g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

				g_particule_color_data[4 * ParticlesCount + 0] = p.r;
				g_particule_color_data[4 * ParticlesCount + 1] = p.g;
				g_particule_color_data[4 * ParticlesCount + 2] = p.b;
				g_particule_color_data[4 * ParticlesCount + 3] = p.a;

			}
			else {
				// Particles that just died will be put at the end of the buffer in SortParticles();
				p.cameradistance = -1.0f;
			}

			ParticlesCount++;

		}
	}

	// Update the buffers that OpenGL uses for rendering.
	// There are much more sophisticated means to stream data from the CPU to the GPU,
	// but this is outside the scope of this tutorial.
	// http://www.opengl.org/wiki/Buffer_Object_Streaming

	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glVertexAttribPointer(
		0, // attribute. No particular reason for 0, but must match the layout in the shader.
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
		);

	// 2nd attribute buffer : positions of particles' centers
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glVertexAttribPointer(
		1, // attribute. No particular reason for 1, but must match the layout in the shader.
		4, // size : x + y + z + size => 4
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
		);

	// 3rd attribute buffer : particles' colors
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glVertexAttribPointer(
		2, // attribute. No particular reason for 1, but must match the layout in the shader.
		4, // size : r + g + b + a => 4
		GL_UNSIGNED_BYTE, // type
		GL_TRUE, // normalized? *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
		0, // stride
		(void*)0 // array buffer offset
		);

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad -> 1

								 // Draw the particules !
								 // This draws many times a small triangle_strip (which looks like a quad).
								 // This is equivalent to :
								 // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
								 // but faster.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

#pragma endregion
}

void final_app::load_shaders()
{
	GLuint vs;
	GLuint fs;
	GLuint gs;

	vs = sb7::shader::load("phong_perfragment.vs.txt", GL_VERTEX_SHADER);
	printShaderInfoLog(vs);
	fs = sb7::shader::load("phong_perfragment.fs.txt", GL_FRAGMENT_SHADER);
	printShaderInfoLog(fs);

	if (per_fragment_program)
	{
		glDeleteProgram(per_fragment_program);
	}

	per_fragment_program = glCreateProgram();
	glAttachShader(per_fragment_program, vs);
	glAttachShader(per_fragment_program, fs);
	glLinkProgram(per_fragment_program);

	vs = sb7::shader::load("floor.vs.txt", GL_VERTEX_SHADER);
	fs = sb7::shader::load("floor.fs.txt", GL_FRAGMENT_SHADER);

	if (floorProgram)
	{
		glDeleteProgram(floorProgram);
	}

	floorProgram = glCreateProgram();
	glAttachShader(floorProgram, vs);
	glAttachShader(floorProgram, fs);
	glLinkProgram(floorProgram);

	vs = sb7::shader::load("wall.vs.txt", GL_VERTEX_SHADER);
	fs = sb7::shader::load("wall.fs.txt", GL_FRAGMENT_SHADER);

	if (wallProgram)
	{
		glDeleteProgram(wallProgram);
	}

	wallProgram = glCreateProgram();
	glAttachShader(wallProgram, vs);
	glAttachShader(wallProgram, fs);
	glLinkProgram(wallProgram);

	vs = sb7::shader::load("toon.vs.txt", GL_VERTEX_SHADER);
	printShaderInfoLog(vs);
	fs = sb7::shader::load("toon.fs.txt", GL_FRAGMENT_SHADER);
	printShaderInfoLog(fs);

	if (toonProgram)
	{
		glDeleteProgram(toonProgram);
	}

	toonProgram = glCreateProgram();
	glAttachShader(toonProgram, vs);
	glAttachShader(toonProgram, fs);
	glLinkProgram(toonProgram);

	vs = sb7::shader::load("flatColor.vs.txt", GL_VERTEX_SHADER);
	fs = sb7::shader::load("flatColor.fs.txt", GL_FRAGMENT_SHADER);

	if (flatColorProgram)
	{
		glDeleteProgram(flatColorProgram);
	}

	flatColorProgram = glCreateProgram();
	glAttachShader(flatColorProgram, vs);
	glAttachShader(flatColorProgram, fs);
	glLinkProgram(flatColorProgram);

	vs = sb7::shader::load("skybox.vs.txt", GL_VERTEX_SHADER);
	fs = sb7::shader::load("skybox.fs.txt", GL_FRAGMENT_SHADER);

	if (skybox_prog)
	{
		glDeleteProgram(skybox_prog);
	}

	skybox_prog = glCreateProgram();
	glAttachShader(skybox_prog, vs);
	glAttachShader(skybox_prog, fs);
	glLinkProgram(skybox_prog);

	vs = sb7::shader::load("point.vs.txt", GL_VERTEX_SHADER);
	gs = sb7::shader::load("point.gs.txt", GL_GEOMETRY_SHADER);
	fs = sb7::shader::load("point.fs.txt", GL_FRAGMENT_SHADER);

	point_prog = glCreateProgram();
	glAttachShader(point_prog, vs);
	glAttachShader(point_prog, gs);
	glAttachShader(point_prog, fs);
	glLinkProgram(point_prog);

	glDeleteShader(vs);
	glDeleteShader(gs);
	glDeleteShader(fs);
}

#pragma region Event Handlers
void final_app::onKey(int key, int action)
{
	// Check to see if shift was pressed
	if (action == GLFW_PRESS && (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT))
	{
		bShiftPressed = true;
	}
	if (action)
	{
		switch (key)
		{

		case 'R':
			load_shaders();
			rotationMatrix = vmath::mat4::identity();
			translationMatrix = vmath::mat4::identity();
			fXpos = 0.0f;
			fYpos = 0.0f;
			fZpos = 75.0f;
			lightPosOffset = vmath::vec3(0, 0, 0);
			break;
		case '1':
			lightPosOffset[0] += 1;
			break;
		case '2':
			lightPosOffset[0] -= 1;
			break;
		case 'Q':
			lightPosOffset[1] += 1;
			break;
		case 'W':
			lightPosOffset[1] -= 1;
			break;
		case 'A':
			lightPosOffset[2] += 1;
			break;
		case 'S':
			lightPosOffset[2] -= 1;
			break;
		}
	}
	// Check to see if shift was released
	if (action == GLFW_RELEASE) {
		bShiftPressed = false;
	}
}

void final_app::onMouseButton(int button, int action)
{
	int x, y;

	getMousePosition(x, y);
	// Check to see if left mouse button was pressed for rotation
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		bRotate = true;
		iPrevMouseX = iCurMouseX = x;
		iPrevMouseY = iCurMouseY = y;
	}
	// Check to see if right mouse button was pressed for zoom and pan
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		bZoom = false;
		bPan = false;
		if (bShiftPressed == true)
		{
			bZoom = true;
		}
		else if (bShiftPressed == false)
		{
			bPan = true;
		}
		iPrevMouseX = iCurMouseX = x;
		iPrevMouseY = iCurMouseY = y;
	}
	else {
		bRotate = false;
		bZoom = false;
		bPan = false;
	}

}

void final_app::onMouseMove(int x, int y)
{
	// If rotating, zooming, or panning save mouse x and y
	if (bRotate || bZoom || bPan)
	{
		iCurMouseX = x;
		iCurMouseY = y;
	}
}

// Modified from tutorial at the following website:
// http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Arcball

vmath::vec3 final_app::getArcballVector(int x, int y) {
	// find vector from origin to point on sphere
	vmath::vec3 vecP = vmath::vec3(1.0f*x / info.windowWidth * 2 - 1.0f, 1.0f*y / info.windowHeight * 2 - 1.0f, 0.0f);
	// inverse y due to difference in origin location on the screen
	vecP[1] = -vecP[1];
	float vecPsquared = vecP[0] * vecP[0] + vecP[1] * vecP[1];
	// solve for vector z component
	if (vecPsquared <= 1)
		vecP[2] = sqrt(1 - vecPsquared);
	else
		vecP = vmath::normalize(vecP);
	return vecP;
}
#pragma endregion

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int final_app::FindUnusedParticle() {

	for (int i = LastUsedParticle; i<MaxParticles; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i<LastUsedParticle; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

void final_app::SortParticles() {
	//std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

DECLARE_MAIN(final_app)

#endif
