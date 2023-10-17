#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/InputManager.h"
#include "SpearEngine/ScreenRenderer.h"
#include "SpearEngine/UiButton.h"
#include "SpearEngine/ScreenRenderer.h"

#include "eFlowstate.h"
#include "FlowstateEditor.h"

enum eTextures
{
	TEX_WALL_STONE,
	TEX_WALL_WOOD,
};

void GridNode::Reset()
{
	texIdRoof = 0;
	texIdWall = 0;
	texIdFloor = 0;

	extendUp = 0;
	extendDown = 0;
}

void EditorMapData::ClearData()
{
	for (int i = 0; i < MAP_WIDTH_MAX_SUPPORTED * MAP_HEIGHT_MAX_SUPPORTED; i++)
	{
		gridNodes[i].Reset();
	}
}

void EditorMapData::Draw()
{
	Spear::ScreenRenderer& rend = Spear::ServiceLocator::GetScreenRenderer();
	for (int x = 0; x < gridWidth; x++)
	{
		for (int y = 0; y < gridHeight; y++)
		{
			Spear::ScreenRenderer::LinePolyData square;
			square.segments = 4;
			square.colour = Colour4f::White();
			square.pos = Vector2f(x, y) + Vector2f(0.5, 0.5f);
			square.radius = 0.65f;
			square.rotation = TO_RADIANS(45.f);

			const int visualScale{75};
			square.pos *= visualScale;
			square.radius *= visualScale;

			rend.AddLinePoly(square);
		}
	}
}

void FlowstateEditor::StateEnter()
{
	m_map.ClearData();
	
	// Configure Inputs
	int config[INPUT_COUNT];
	config[INPUT_SELECT] = SDL_BUTTON_LEFT;
	config[INPUT_SELECT_ALT] = SDL_BUTTON_RIGHT;
	config[INPUT_QUIT] = SDL_SCANCODE_ESCAPE;
	Spear::ServiceLocator::GetInputManager().ConfigureInputs(config, INPUT_COUNT);

	// Set background colour
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);

	// Load menu textures
	m_menuTextures.Allocate(64, 64, 2); // 64x64 textures (2 slots)
	m_menuTextures.SetDataFromFile(0, "../Assets/SPRITES/mode_Editor.png");
	m_menuTextures.SetDataFromFile(1, "../Assets/SPRITES/mode_Play.png");
	Spear::ServiceLocator::GetScreenRenderer().CreateSpriteBatch(m_menuTextures, 100);
	//Spear::ServiceLocator::GetScreenRenderer().SetTextureArrayData(m_menuTextures);
}

int FlowstateEditor::StateUpdate(float deltaTime)
{
	Spear::InputManager& input = Spear::ServiceLocator::GetInputManager();
	if (input.InputRelease(INPUT_QUIT))
	{
		return static_cast<int>(eFlowstate::STATE_MENU);
	}

	return static_cast<int>(eFlowstate::STATE_THIS);
}

void FlowstateEditor::StateRender()
{
	m_map.Draw();

	Spear::ServiceLocator::GetScreenRenderer().Render();
}

void FlowstateEditor::StateExit()
{
	Spear::ServiceLocator::GetScreenRenderer().ClearSpriteBatches();
	Spear::ServiceLocator::GetScreenRenderer().ClearLineBatches();
}