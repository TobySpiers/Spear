#pragma once

namespace Spear
{
	class TextureArray;
}

namespace GlobalTextureBatches
{
	enum eTextureBatch
	{
		BATCH_TILESET_1,
		BATCH_SPRITESET_1,

		BATCH_TOTALS
	};

	void InitialiseBatches(Spear::TextureArray* textureArrayStorage);
}