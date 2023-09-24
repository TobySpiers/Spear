#include "Core.h"
#include "Renderer.h"
#include "ShaderCompiler.h"


namespace Spear
{
	Renderer::Renderer()
	{
		// ============================================
		// RENDER DATA:
		// ============================================
		const std::vector<GLfloat> vertexPosition{
			-0.5f, -0.5f, 0.0f, // vertex 1 BL
			0.5f, -0.5f, 0.0f,  // vertex 2 BR
			-0.5f, 0.5f, 0.0f,  // vertex 3 TL
			0.5f, 0.5f, 0.0f	// vertex 4 TR
		};

		const std::vector<GLuint> indexBufferData{
			0, 1, 2,	// triangle 1
			1, 3, 2		// triangle 2
		};

		// color values range from 0 to 1
		const std::vector<GLfloat> vertexColour{
			1.f, 0.f, 0.f,		// color 1
			0.f, 1.f, 0.f,		// color 2
			0.f, 0.f, 1.f,		// color 3
			1.f, 0.f, 0.f		// color 4
		};

		const std::vector<GLfloat> vertexTexcoord{
			0.f, 1.f,
			1.f, 1.f,
			0.f, 0.f,
			1.f, 0.f
		};

		// create 1 vertex array
		glGenVertexArrays(1, &m_vertexArray);
		glBindVertexArray(m_vertexArray); // bind the vertex array so calls to EnableVertexAttribArray (.etc) affect THIS

		// ============================================
		// SETUP BUFFER for POSITION:
		// ============================================

		// create and bind buffer, then fill with data
		glGenBuffers(1, &m_vertexBufferPos);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferPos);
		glBufferData(	
			GL_ARRAY_BUFFER,
			vertexPosition.size() * sizeof(GLfloat),
			vertexPosition.data(),
			GL_STATIC_DRAW
		);

		// fill out an attrib array
		glEnableVertexAttribArray(0); // enable attrib array 0 for this object
		glVertexAttribPointer(
			0,
			3,			// three values per 'data' (x, y, z per vertex)
			GL_FLOAT,		// expect values to be of type float
			GL_FALSE,		// not normalized
			0,			// size of stride is zero (each value is directly after previous)
			(GLvoid*)0		// offset to first entry in buffer (currently zero)
		);


		// ============================================
		// SETUP BUFFER for COLOUR:
		// ============================================

		// create and bind buffer, then fill with data
		glGenBuffers(1, &m_vertexBufferCol);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferCol);
		glBufferData(
			GL_ARRAY_BUFFER,
			vertexColour.size() * sizeof(GLfloat),
			vertexColour.data(),
			GL_STATIC_DRAW
		);

		// fill out another attrib array
		glEnableVertexAttribArray(1); // enable attrib array 1 for this object
		glVertexAttribPointer(
			1,
			3,			//r,g,b
			GL_FLOAT,
			GL_FALSE,	// no stride
			0,
			(GLvoid*)0
		);

		// ============================================
		// SETUP BUFFER for TEX COORD:
		// ============================================

		// buffer
		glGenBuffers(1, &m_vertexBufferTexcoord);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferTexcoord);
		glBufferData(
			GL_ARRAY_BUFFER,
			vertexTexcoord.size() * sizeof(GLfloat),
			vertexTexcoord.data(),
			GL_STATIC_DRAW
		);

		// attrib
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,			// slot 2
			2,			// 2 values per coord
			GL_FLOAT,
			GL_FALSE,	// no stride
			0,
			(GLvoid*)0
		);

		// ============================================
		// SETUP INDEX BUFFER:
		// ============================================

		glGenBuffers(1, &m_indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			indexBufferData.size() * sizeof(GLuint),
			indexBufferData.data(),
			GL_STATIC_DRAW
		);

		// ============================================
		// FINISH CONFIGURING VERTEX ARRAY OBJECT (VAO):
		// ============================================
		glBindVertexArray(0); // unbind the vertex array (by binding 0) as we are finished modifying it

		// ============================================
		// CREATE SHADER PROGRAM:
		// ============================================
		m_shaderProgram = ShaderCompiler::CreateShaderProgram("../Shaders/TriangleVS.glsl", "../Shaders/TriangleFS.glsl");

		// ============================================
		// TEXTURE LOADING:
		// ============================================

		m_texture.SetDataFromFile("../Assets/imageBig.png");

		glUseProgram(m_shaderProgram);
		GLint textureLocation = glGetUniformLocation(m_shaderProgram, "textureSampler");
		glUniform1i(textureLocation, 0);

		glUseProgram(0);
	}

	void Renderer::Render()
	{
		/*glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);*/

		glClearColor(0.5f, 0.5f, 0.5f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glUseProgram(m_shaderProgram);

		glBindVertexArray(m_vertexArray); // vertex array we created holds info on both buffers

		glBindTexture(GL_TEXTURE_2D, m_texture.GetTextureId()); // bind the texture we created from the bmp

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glUseProgram(0); // stop using the render pipeline program we bound
	}
}