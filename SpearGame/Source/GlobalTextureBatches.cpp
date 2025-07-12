#include "Graphics/TextureArray.h"
#include "GlobalTextureBatches.h"
#include "Graphics/ScreenRenderer.h"

namespace GlobalTextureBatches
{
	void InitialiseBatches(Spear::TextureArray* textureArrayStorage)
	{
		struct TextureBatchData
		{
			const char* batchFilepath{ nullptr };
			int batchSize{ 500 };
		};

		TextureBatchData batchSetupData[BATCH_TOTALS];
		batchSetupData[BATCH_TILESET_1] = {"../Assets/TILESETS/64", 1000};
		batchSetupData[BATCH_SPRITESET_1] = {"../Assets/SPRITES/SpriteSet1", 1000};

		for (int i = 0; i < BATCH_TOTALS; i++)
		{
			textureArrayStorage[i].InitialiseFromDirectory(batchSetupData[i].batchFilepath);
			Spear::Renderer::Get().CreateSpriteBatch(textureArrayStorage[i], batchSetupData[i].batchSize);
		}
	}
}