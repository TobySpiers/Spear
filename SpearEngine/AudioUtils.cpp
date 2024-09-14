#include "AudioUtils.h"
#include "alext.h"
#include "inttypes.h"

namespace Spear
{
    ALenum Spear::AudioUtils::GetFormatFromSndFile(SNDFILE* sndfile, SF_INFO fileInfo)
    {
        ALenum format = AL_NONE;
        if (fileInfo.channels == 1)
        {
            format = AL_FORMAT_MONO16;
        }
        else if (fileInfo.channels == 2)
        {
            format = AL_FORMAT_STEREO16;
        }
        else if (fileInfo.channels == 3)
        {
            if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            {
                format = AL_FORMAT_BFORMAT2D_16;
            }
        }
        else if (fileInfo.channels == 4)
        {
            if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            {
                format = AL_FORMAT_BFORMAT3D_16;
            }
        }

        return format;
    }
}