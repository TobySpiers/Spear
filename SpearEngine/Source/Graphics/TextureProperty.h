#pragma once
#include "Core/Serializer.h"
#include "ScreenRenderer.h"
#include "imgui.h"
#include <fstream>

namespace Spear
{
	// A serializable property representing a texture inside a fixed batch, with visual support for ImGui
	template<int batchId>
	class TextureProperty
	{
	private:
		int m_spriteId{0};
		static constexpr int m_batchId = batchId;

	public:
		int SpriteId() const {return m_spriteId;}
		int BatchId() const {return m_batchId;}

		friend std::ofstream& operator<<(std::ofstream& stream, const TextureProperty& obj)
		{
			stream << obj.m_spriteId << " ";
			return stream;
		}

		friend std::ifstream& operator>>(std::ifstream& stream, TextureProperty& obj)
		{
			stream >> obj.m_spriteId;
			return stream;
		}

		TextureProperty& operator<<(ExposedPropertyData& property)
		{
			bool bValueChanged{false};

			const Spear::TextureBase* pTextures = Spear::Renderer::Get().GetBatchTextures(m_batchId);
			const int cachedSpriteId = m_spriteId;

			ImGui::ImageButton("CurTexture", pTextures->GetTextureViewForLayer(m_spriteId), ImVec2(pTextures->GetWidth(), pTextures->GetHeight()));
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				ImGui::OpenPopup("PickTexture");
			}

			if (ImGui::BeginPopup("PickTexture", ImGuiWindowFlags_NoMove))
			{
				for (int i = 0; i < pTextures->GetDepth(); i++)
				{
					ImGui::Image(pTextures->GetTextureViewForLayer(i), ImVec2(pTextures->GetWidth(), pTextures->GetHeight()));
					if (ImGui::IsItemClicked())
					{
						bValueChanged = m_spriteId != i;
						m_spriteId = i;
						ImGui::CloseCurrentPopup();
					}

					const int popupItemsPerRow = 4;
					if ((i + 1) % popupItemsPerRow != 0)
					{
						ImGui::SameLine();
					}
				}

				ImGui::EndPopup();
			}
			ImGui::SameLine();
			ImGui::Text(property.propertyName.c_str());

			if (bValueChanged)
			{
				property.propertyChain.push_back(0);
				property.modifiedPropertyName = property.propertyName;
				property.SetOldValue(cachedSpriteId);
				property.SetNewValue(m_spriteId);
			}

			return *this;
		}

		TextureProperty& operator<<(PropertyManipulator& inserter)
		{
			Serializer::SetPropertyInternal<int>(inserter.value, m_spriteId, inserter.outPropertyData);
			return *this;
		}

		const TextureProperty& operator>>(PropertyManipulator& deleter) const
		{
			Serializer::DeletePropertyDataInternal<int>(*deleter.allocatedValue);
			return *this;
		}
	};
}