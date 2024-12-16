#pragma once

namespace Spear
{
	class ShaderCompiler
	{
		NO_CONSTRUCT(ShaderCompiler);

	public:
		// Creates a program linking together a Vertex Shader and Fragment Shader
		static GLuint CreateShaderProgram(const std::string& vsFilename, const std::string& fsFilename);
		// Creates a program for a single Compute Shader
		static GLuint CreateShaderProgram(const std::string& csFilename);

	private:
		static std::string LoadShaderAsString(const std::string& filename);
		static GLuint CompileShader(GLuint type, const std::string& source);
	};
}