#include "Core/Core.h"
#include <fstream>
#include "ShaderCompiler.h"

namespace Spear
{
	// return handle to created shader program on GPU
	GLuint ShaderCompiler::CreateShaderProgram(const std::string& vsFilename, const std::string& fsFilename)
	{
		GLuint shaderProgram = glCreateProgram();

		std::string vertexShaderSource = ShaderCompiler::LoadShaderAsString(vsFilename);
		std::string fragmentShaderSource = ShaderCompiler::LoadShaderAsString(fsFilename);

		GLuint vShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
		GLuint fShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

		if (!vShader)
		{
			LOG(std::string("File: ") + vsFilename);
		}
		if (!fShader)
		{
			LOG(std::string("File: ") + fsFilename);
		}

		glAttachShader(shaderProgram, vShader);
		glAttachShader(shaderProgram, fShader);
		glLinkProgram(shaderProgram);

		// validate
		glValidateProgram(shaderProgram);

		return shaderProgram;
	}

	GLuint ShaderCompiler::CreateShaderProgram(const std::string& csFilename)
	{
		GLuint computeProgram = glCreateProgram();

		std::string computeShaderSource = ShaderCompiler::LoadShaderAsString(csFilename);

		GLuint computeShader = CompileShader(GL_COMPUTE_SHADER, computeShaderSource);

		if (!computeShader)
		{
			LOG(std::string("File: ") + csFilename);
		}

		glAttachShader(computeProgram, computeShader);
		glLinkProgram(computeProgram);

		// validate
		glValidateProgram(computeProgram);

		return computeProgram;
	}



	std::string ShaderCompiler::LoadShaderAsString(const std::string& filename)
	{
		std::string result = "";

		std::string line = "";
		std::ifstream myFile(filename.c_str());

		if (myFile.is_open())
		{
			while (std::getline(myFile, line))
			{
				result += line + '\n';
			}
			myFile.close();
		}

		return result;
	}

	// compile shader and return handle if succesful
	GLuint ShaderCompiler::CompileShader(GLuint type, const std::string& source)
	{
		GLuint shaderObject{0};

		switch (type)
		{
		case GL_VERTEX_SHADER:
			shaderObject = glCreateShader(GL_VERTEX_SHADER);
			break;
		case GL_FRAGMENT_SHADER:
			shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
			break;
		case GL_COMPUTE_SHADER:
			shaderObject = glCreateShader(GL_COMPUTE_SHADER);
			break;
		default:
			ASSERT(false);
		}

		const char* src = source.c_str();
		glShaderSource(shaderObject, 1, &src, nullptr);
		glCompileShader(shaderObject);

		int result;
		glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

		if (result == GL_FALSE)
		{
			int length;
			glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
			char* errorMessages = new char[length];
			glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

			switch (type)
			{
			case GL_VERTEX_SHADER:
				LOG("\n----------------------------------------------\nERROR: GL_VERTEX_SHADER compilation failed...\n----------------------------------------------");
				break;
			case GL_FRAGMENT_SHADER:
				LOG("\n----------------------------------------------\nERROR: GL_FRAGMENT_SHADER compilation failed...\n----------------------------------------------");
				break;
			case GL_COMPUTE_SHADER:
				LOG("\n----------------------------------------------\nERROR: GL_COMPUTE_SHADER compilation failed...\n----------------------------------------------");
				break;
			default:
				LOG("\n----------------------------------------------\nERROR: SHADER compilation failed...\n----------------------------------------------");
				break;
			}
			LOG(errorMessages);
			delete[] errorMessages;
			glDeleteShader(shaderObject);
			return 0;
		}

		return shaderObject;
	}
}