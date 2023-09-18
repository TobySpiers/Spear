#include "Core.h"
#include "GfxManager.h"

const std::string gVertexShaderSource =
"#version 410 core\n"
"in vec4 position;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(position.x, position.y, position.z, position.w);\n"
"}\n";

const std::string gFragmentShaderSource =
"#version 410 core\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"	color = vec4(1.0f, 0.5f, 0.0f, 1.0f);\n"
"}\n";

// return handle to compiled shader
GLuint CompileShader(GLuint type, const std::string& source)
{
	GLuint shaderObject;
	
	if (type == GL_VERTEX_SHADER)
	{
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}
	else if (type == GL_FRAGMENT_SHADER)
	{
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();
	glShaderSource(shaderObject, 1, &src, nullptr);
	glCompileShader(shaderObject);

	return shaderObject;
}

// return handle to created shader program on GPU
GLuint CreateShaderProgram(const std::string& vs, const std::string& fs)
{
	GLuint shaderProgram = glCreateProgram();

	GLuint vShader = CompileShader(GL_VERTEX_SHADER, vs);
	GLuint fShader = CompileShader(GL_FRAGMENT_SHADER, fs);

	glAttachShader(shaderProgram, vShader);
	glAttachShader(shaderProgram, fShader);
	glLinkProgram(shaderProgram);

	// validate
	glValidateProgram(shaderProgram);

	// glDetachShader
	// glDeleteShader

	return shaderProgram;
}

namespace Spear
{
	GfxManager::GfxManager()
	{
		
	}

	void GfxManager::CreateWindow(const WindowParams& params)
	{
		// SDL setup
		if (SDL_Init(SDL_INIT_EVERYTHING) == 0) // 0 = no error returned
		{
			LOG("SDL Initialised");

			// Specify our OpenGL version: version 4.1, profile mask = core profile (no backward compat)
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

			// enable double-buffer to avoid screentearing, set depth buffer to 24 bits (common balance for precision/memory use)
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

			// Create window with OpenGL surface
			m_window = SDL_CreateWindow(params.title, params.xpos, params.ypos, params.width, params.height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | (params.fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
			ASSERT(m_window);

			// Context for working with OpenGL
			m_context = SDL_GL_CreateContext(m_window);

			// Setup OpenGL function pointers
			if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
			{
				LOG("ERROR: glad load failed");
			}

			LOG("scale not yet configured for glViewport");
			glViewport(0, 0, params.width, params.height);

			LOG("Render Info:")
			LOG(glGetString(GL_VENDOR));
			LOG(glGetString(GL_RENDERER));
			LOG(glGetString(GL_VERSION));
			LOG(glGetString(GL_SHADING_LANGUAGE_VERSION));


			// ============================================
			// SETUP VERTEX DATA:
			// ============================================
			const std::vector<GLfloat> vertexPosition{
				-0.8f, -0.8f, 0.0f, // vertex 1
				0.8f, -0.8f, 0.0f,  // vertex 2
				0.0f, 0.8f, 0.0f,   // vertex 3
			};

			// create 1 vertex array and bind it (select it)
			glGenVertexArrays(1, &m_vertexArray);
			glBindVertexArray(m_vertexArray);

			// create 1 vertex buffer and bind it (select it)
			glGenBuffers(1, &m_vertexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

			// assign our vertex data to the vertex buffer
			glBufferData(GL_ARRAY_BUFFER,
						 vertexPosition.size() * sizeof(GLfloat), 
						 vertexPosition.data(), 
						 GL_STATIC_DRAW
			);

			// fill out our attrib array
			glEnableVertexAttribArray(0); // activate to fill out
			glVertexAttribPointer(0,
								  3,			// three values per 'data' (x, y, z per vertex)
								  GL_FLOAT,		// expect values to be of type float
								  GL_FALSE,		// not normalized
								  0,			// size of stride is zero (each value is directly after previous)
								  (void*)0		// offset to first entry in buffer (currently zero)
			);
			glBindVertexArray(0); // unbind the vertex array (by binding 0) as we are finished
			glDisableVertexAttribArray(0); // disable when finished

			
			// ============================================
			// SETUP RENDERING PIPELINE:
			// ============================================
			m_renderPipeline = CreateShaderProgram(gVertexShaderSource, gFragmentShaderSource);
		}
	}

	GfxManager::~GfxManager()
	{
		// Shutdown SDL
		SDL_DestroyWindow(m_window);
		SDL_Quit();
		LOG("SDL shutdown");
	}

	SDL_Window& GfxManager::GetWindow()
	{
		ASSERT(m_window);
		return *m_window;
	}

	SDL_GLContext& GfxManager::GetContext()
	{
		ASSERT(m_context);
		return m_context;
	}
}