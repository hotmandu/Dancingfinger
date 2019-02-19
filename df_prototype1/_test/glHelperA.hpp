#pragma once
#include "../pch.h"

/************************************
 GLShader
  Helps compiling external shader codes
        linking them
		activating them
************************************/
class GLShader
{
public:
	unsigned int ID;
	// constructor generates the shader on the fly
	// ------------------------------------------------------------------------
	GLShader(const char* vertexPath, const char* fragmentPath)
	{
		// 1. retrieve the vertex/fragment source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		// ensure ifstream objects can throw exceptions:
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			// read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char * fShaderCode = fragmentCode.c_str();
		// 2. compile shaders
		unsigned int vertex, fragment;
		// vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");
		// fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");
		// shader Program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");
		// delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
	// activate the shader
	// ------------------------------------------------------------------------
	void use()
	{
		glUseProgram(ID);
	}
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const std::string &name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt(const std::string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setFloat(const std::string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const std::string &name, const glm::vec2 &value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string &name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const std::string &name, const glm::vec3 &value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string &name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const std::string &name, const glm::vec4 &value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string &name, float x, float y, float z, float w) const
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const std::string &name, const glm::mat2 &mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3(const std::string &name, const glm::mat3 &mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4(const std::string &name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(unsigned int shader, std::string type)
	{
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};

/************************************
 GLTexture2DHelper
  Helps binding 2D textures
        setting up
************************************/
class GLTexture2DHelper
{
public:
	const int textureCount;
	std::vector<unsigned int> textureID;

	GLTexture2DHelper(int textureCount = 16) : textureCount(textureCount) {
		// some init code
		textureID.resize(textureCount);
	}
	
	class InvaildSlot {
	public:
		const int maxSlot;
		const int requestedSlot;
		InvaildSlot(int maxSlot, int requestedSlot) : maxSlot(maxSlot), requestedSlot(requestedSlot) {}
	};

	int bindTextureStart(int slot) /*throw (InvaildSlot)*/ {
		if (slot < 0 || slot >= textureCount) {
			throw InvaildSlot(textureCount, slot);
		}

		unsigned int texture;
		glGenTextures(1, &texture);
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, texture);

		textureID[slot] = texture;
		return texture;
	}

	int bindTextureEnd(int slot, const char* imgPath, int mipmapLevel, int internalFormat, unsigned int imgFormat, bool flipVertical = false, int width=-1, int height=-1, int border=0) /*throw (InvaildSlot)*/ {
		if (slot < 0 || slot >= textureCount) {
			throw InvaildSlot(textureCount, slot);
		}

		// load and generate the texture
		int iwidth, iheight, nrChannels;
		stbi_set_flip_vertically_on_load(flipVertical);
		unsigned char *data = stbi_load(imgPath, &iwidth, &iheight, &nrChannels, 0);
		if (data)
		{
			if (width == -1) width = iwidth;
			if (height == -1) height = iheight;
			/*
The first argument specifies the texture target;
     setting this to GL_TEXTURE_2D means this operation will generate a texture on the currently bound texture object at the same target
	 (so any textures bound to targets GL_TEXTURE_1D or GL_TEXTURE_3D will not be affected).
The second argument specifies the mipmap level for which we want to create a texture for if you want to set each mipmap level manually, but we'll leave it at the base level which is 0.
The third argument tells OpenGL in what kind of format we want to store the texture.
     Our image has only RGB values so we'll store the texture with RGB values as well.
The 4th and 5th argument sets the width and height of the resulting texture.
     We stored those earlier when loading the image so we'll use the corresponding variables.
The next argument should always be 0 (some legacy stuff).
The 7th and 8th argument specify the format and datatype of the source image.
     We loaded the image with RGB values and stored them as chars (bytes) so we'll pass in the corresponding values.
The last argument is the actual image data. */
			glTexImage2D(GL_TEXTURE_2D, mipmapLevel, internalFormat, width, height, border, imgFormat, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);

		return textureID[slot];
	}

	void bind() {
		for (int i = 0; i < textureCount; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, textureID[i]);
		}
	}
};