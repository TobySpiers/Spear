#pragma once

namespace Spear
{
	class ShaderCompiler
	{
		NO_CONSTRUCT(ShaderCompiler);

	public:
		static GLuint CreateShaderProgram(const std::string& vsFilename, const std::string& fsFilename);

	private:
		static std::string LoadShaderAsString(const std::string& filename);
		static GLuint CompileShader(GLuint type, const std::string& source);
	};
}