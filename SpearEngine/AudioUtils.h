#pragma once
#include "Core.h"
#include "al.h"
#include "sndfile.hh"

namespace Spear
{
	class AudioUtils
	{
	public:

		// Warning: certain filetypes (such as .ogg) rely on float32 formatting to avoid clipping.
		// Current code assumes int16 formatting is appropriate for all files.
		// Float formats will still be playable but will contain unpleasant audio clipping.
		static ALenum GetFormatFromSndFile(SNDFILE* sndfile, SF_INFO fileInfo);
	};
}