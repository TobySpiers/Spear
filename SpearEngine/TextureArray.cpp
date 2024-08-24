#include "TextureArray.h"
#include "SDL_Image.h"
#include "Colour.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <queue>

std::string GetManifestPath(const char* dir) { return std::string(dir) + "/manifest.dat"; };

namespace Spear
{
	TextureArray::TextureArray()
	{}
	
	TextureArray::~TextureArray()
	{
		FreeTexture();
	}

	bool TextureArray::InitialiseFromDirectory(const char* dir, int* out_totalSlots)
	{
		// Make sure any previously set memory is released
		FreeTexture();

		// Load manifest containing prior known .png files (allows texture indexes to always match values saved inside level.dats after file removals/additions)
		std::vector<std::string> manifest;
		std::queue<int> manifestFreeSlots;
		{
			std::ifstream file(GetManifestPath(dir));
			if (file.is_open())
			{
				std::string sFileCount;
				std::getline(file, sFileCount);
				int fileCount{ std::stoi(sFileCount) };

				for (int i = 0; i < fileCount; i++)
				{
					manifest.push_back(std::string());
					std::getline(file, manifest.back());

					// If file no longer exists, mark this as a free slot we can replace with any newly added files
					if (!std::filesystem::exists(manifest.back()))
					{
						manifestFreeSlots.push(i);
					}
				}
			}
		}

		// Iterate directory for new files; use these to replace any missing files, otherwise append to back of list
		for (const std::filesystem::directory_entry& filepath : std::filesystem::directory_iterator(dir))
		{
			if (filepath.path().extension() == ".png")
			{
				const std::string sPath = filepath.path().string();
				bool bIsNew{ true };
				for (std::string& knownPath : manifest)
				{
					if (knownPath == sPath)
					{
						bIsNew = false;
						break;
					}
				}

				if (bIsNew)
				{
					if (manifestFreeSlots.size())
					{
						manifest[manifestFreeSlots.front()] = sPath;
						manifestFreeSlots.pop();
					}
					else
					{
						manifest.push_back(sPath);
					}
				}
			}
		}
		if (!manifest.size() || manifest.size() == manifestFreeSlots.size())
		{
			return false;
		}

		// Save updated manifest
		{
			std::ofstream file(GetManifestPath(dir));
			file << manifest.size() << std::endl;
			for (const std::string& entry : manifest)
			{
				file << entry << std::endl;
			}
		}

		// Load first existing image as a 'template' we can use to configure width/height (assuming all images in a directory are always same resolution)
		for (int i = 0; i < manifest.size(); i++)
		{
			if (manifestFreeSlots.size() && manifest[i] == manifest[manifestFreeSlots.front()])
			{
				manifestFreeSlots.pop();
				continue;
			}

			SDL_Surface* paramsTemplate = IMG_Load(manifest[i].c_str());
			Allocate(paramsTemplate->w, paramsTemplate->h, manifest.size());
			SDL_FreeSurface(paramsTemplate);
			break;
		}

		// Initialise each slot from manifest (any missing files will be generated a flat purple texture internally within SetDataFromFile)
		{
			int i{ 0 };
			for (std::string filepath : manifest)
			{
				SetDataFromFile(i++, filepath.c_str());
			}
		}

		// Return info
		if (out_totalSlots)
		{
			*out_totalSlots = manifest.size();
		}
		return true;
	}

	void TextureArray::Allocate(GLuint width, GLuint height, GLuint slots)
	{
		// Create new texture array
		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureId);
		// nearest pixel filtering
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// repeat wrap
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// initialise format
		GLCheck(glTexStorage3D(
			GL_TEXTURE_2D_ARRAY,
			1,
			GL_RGBA8,
			width,
			height,
			slots
		));

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0); // unbind

		m_textureWidth = width;
		m_textureHeight = height;
		m_textureDepth = slots;
		FreeSDLSurfaces();
		m_pSDLSurfaces.resize(slots);
	}

	bool TextureArray::SetDataFromFile(GLuint slot, const char* filename)
	{
		m_pSDLSurfaces.at(slot) = IMG_Load(filename);
		if (!m_pSDLSurfaces.at(slot))
		{
			LOG(std::string("Texture failed to load: ") + filename);

			m_pSDLSurfaces.at(slot) = SDL_CreateRGBSurface(0, m_textureWidth, m_textureHeight, 32, 0, 0, 0, 0);
			Uint32 color = SDL_MapRGB(m_pSDLSurfaces.at(slot)->format, 255, 0, 255);
			SDL_FillRect(m_pSDLSurfaces.at(slot), NULL, color);
		}
		ASSERT(m_pSDLSurfaces.at(slot)->w == m_textureWidth && m_pSDLSurfaces.at(slot)->h == m_textureHeight);

		if (m_pSDLSurfaces.at(slot)->format->format != SDL_PIXELFORMAT_RGBA32 && m_pSDLSurfaces.at(slot)->format->format != SDL_PIXELFORMAT_BGRA32)
		{
			LOG(std::string("\nWARNING: Converted image from non-suitable texture format: ") + filename);
			SDL_Surface* pConvertedSurface = SDL_ConvertSurfaceFormat(m_pSDLSurfaces.at(slot), SDL_PIXELFORMAT_RGBA32, 0);
			if (!pConvertedSurface)
			{
				LOG("\tABORT: Image conversion failed!");
				return false;
			}
			SDL_FreeSurface(m_pSDLSurfaces.at(slot));		// free up the old image
			m_pSDLSurfaces.at(slot) = pConvertedSurface;	// update pointer to converted image
		}

		// bind THIS texture array
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureId);

		GLCheck(glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,	// type we are assigning to
			0,						// no mipmap
			0,
			0,
			slot,					// 'index' into texture array
			m_textureWidth,			
			m_textureHeight,
			1,						// 'depth' of texture (always 1 for a single slice)
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			m_pSDLSurfaces.at(slot)->pixels
		));		

		// unbind texture
		glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);

		return true;
	}

	bool TextureArray::SetDataFromArrayRGBA(GLuint slot, float* pPixels, int width, int height)
	{
		ASSERT(width == m_textureWidth && height == m_textureHeight);

		// bind THIS texture
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureId);

		GLCheck(glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,	
			0,
			0,
			slot,
			m_textureWidth,
			m_textureHeight,
			1,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			pPixels
		));

		// unbind texture
		glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);

		return true;
	}

	void TextureArray::FreeTexture()
	{
		if (m_textureId != 0)
		{
			glDeleteTextures(1, &m_textureId);
			m_textureId = 0;
		}
		m_textureWidth = 0;
		m_textureHeight = 0;
		m_textureDepth = 0;

		FreeSDLSurfaces();
	}

	void TextureArray::FreeSDLSurfaces()
	{
		for (SDL_Surface*& pSurf : m_pSDLSurfaces)
		{
			SDL_FreeSurface(pSurf);
			pSurf = nullptr;
		}
		m_pSDLSurfaces.clear();
	}
}