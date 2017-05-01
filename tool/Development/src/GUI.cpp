#include "Tool.h"

#include "dear imgui\imgui.h"
#include "dear imgui\imgui_impl_dx11.h"

#include "Input.h"
#include "Assets.h"
#include "FileUtil.h"

void Tool::HelloGUI()
{
	ImGuiIO& io = ImGui::GetIO();

	//ImGui::ShowTestWindow(NULL);
	ImGui::OpenPopup("Create or Load a Project");

	if (ImGui::BeginPopupModal("Create or Load a Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static string folder;
		static bool newButton;
		if (ImGui::Button("New Project", ImVec2(120, 0)))
		{
			folder = FileUtil::BrowseFolder();
			if (folder.length() != 0)
				if (AssetManager::Instance().CreateProject(folder))
				{
					ImGui::CloseCurrentPopup();
				}
				else
				{
					newButton = true;
					ImGui::OpenPopup("Invalid folder");
				}
		}

		ImGui::SameLine();
		if (ImGui::Button("Load Project", ImVec2(120, 0)))
		{
			folder = FileUtil::BrowseFolder();
			if (folder.length() != 0)
				if (AssetManager::Instance().LoadProject(folder))
				{
					ImGui::CloseCurrentPopup();
				}
				else
				{
					newButton = false;
					ImGui::OpenPopup("Invalid folder");
				}
		}

		if (ImGui::BeginPopupModal("Invalid folder"))
		{
			ImGui::Text("The folder '%s' %s.", folder.c_str(), newButton ? "is not empty" : "does not contain a project");
			if (ImGui::Button("OK", ImVec2(60, 0)))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		ImGui::EndPopup();
	}
}

void Tool::InvokeGUI()
{
	ImGuiIO& io = ImGui::GetIO();

	// menu bar
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
				AssetManager::Instance().SaveProject();

			if (SceneData::Instance().Size())
				if (ImGui::MenuItem("Export"))
					gui.exportData.prompt = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Import"))
		{
			if (ImGui::MenuItem("Mesh"))
				gui.meshImporter = true;

			if (ImGui::MenuItem("Texture"))
				gui.textureImporter = true;

			if (ImGui::MenuItem("Shader"))
				gui.shaderImporter = true;

			if (ImGui::MenuItem("Misc"))
				gui.miscImporter = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Scene"))
		{
			if (ImGui::MenuItem("New Scene"))
				gui.sceneCreate = true;

			if (SceneData::Instance().Size())
			{
				if (SceneData::Instance().CurrentScene().ptr())
					if (ImGui::MenuItem("Misc Passthrough"))
						gui.miscEdit = true;

				if (ImGui::Combo("Scene", &gui.sceneIndex, ComboAssetNames<SceneData>, NULL, AssetManager::Instance().GetAssetCount<SceneData>()))
					SceneData::Instance().SwapScene(gui.sceneIndex);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Material"))
		{
			if (ImGui::MenuItem("New Material"))
				gui.materialData.create = true;

			if (ImGui::Combo("Material", &gui.materialData.index, ComboAssetNames<MaterialData>, NULL, AssetManager::Instance().GetAssetCount<MaterialData>()) || ImGui::Button("Edit"))
			{
				gui.materialData.edit = true;
				MaterialData::Handle mat = AssetManager::Instance().GetAsset<MaterialData>(gui.materialData.index).handle;
				gui.materialData.vertexShaderIndex = AssetManager::Instance().GetIndex<VertexShaderData>(mat->vertexShader);
				gui.materialData.pixelShaderIndex = AssetManager::Instance().GetIndex<PixelShaderData>(mat->pixelShader);

				gui.materialData.textureIndices.clear();
				for (size_t i = 0; i < mat->textures.size(); ++i)
					gui.materialData.textureIndices.push_back(AssetManager::Instance().GetIndex<TextureData>(mat->textures[i]));
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	PromptImport();
	PromptCreate();
	PromptEdit();

	// status info window
	ImGui::SetNextWindowPos(ImVec2((float)Graphics::Instance().GetWidth() - 309, 19));
	if (ImGui::Begin("Stats Bar", NULL, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs))
	{
		ImGui::Text("Res: %.0fx%.0f\tFPS: %.0f Delta: %.5f", io.DisplaySize.x, io.DisplaySize.y, io.Framerate, io.DeltaTime * 1000);
	}
	ImGui::End();

	SceneData::Handle scene = SceneData::Instance().CurrentScene();

	// entity editor
	if (scene.ptr())
	{
		ImGui::SetNextWindowPos(ImVec2((float)Graphics::Instance().GetWidth() - 309, 51));
		ImGui::SetNextWindowSize(ImVec2(309, (float)Graphics::Instance().GetHeight() - 51));
		if (ImGui::Begin("Entity Editor", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
		{
			if (ImGui::Button("Spawn"))
			{
				EntityData::Handle entity = EntityData::Instance().Get();
				scene->entities.push_back(entity);
				SceneData::Instance().SelectEntity(entity);
			}

			if (scene->selectedEntity.ptr())
			{
				EntityData::Handle ent = scene->selectedEntity;

				int type = (int)ent->type;
				ImGui::InputInt("Type", &type);
				ent->type = (uint16_t)type;

				ImGui::DragFloat3("Position", &ent->transform->pos.x, 0.005f);

				DirectX::XMFLOAT3 euler = TransformData::Instance().GetEuler(ent->transform);
				ImGui::DragFloat3("Rot", &euler.x, 0.005f);
				TransformData::Instance().SetEuler(ent->transform, euler);

				ImGui::DragFloat("Scale", &ent->transform->scale, 0.005f);
				if (ImGui::Combo("Mesh", &gui.entityData.meshIndex, ComboAssetNames<MeshData>, NULL, AssetManager::Instance().GetAssetCount<MeshData>()))
					ent->mesh = AssetManager::Instance().GetAsset<MeshData>(gui.entityData.meshIndex).handle;
				if (ImGui::Combo("Material", &gui.entityData.materialIndex, ComboAssetNames<MaterialData>, NULL, AssetManager::Instance().GetAssetCount<MaterialData>()))
					ent->material = AssetManager::Instance().GetAsset<MaterialData>(gui.entityData.materialIndex).handle;
			}
		}

		ImGui::End();
	}

	PromptExport();
	ExitToolGUI();
}

// TODO macro these
void Tool::PromptImport()
{
	// mesh import
	if (gui.meshImporter)
	{
		ImGui::OpenPopup("Import a new Mesh file");
		gui.meshImporter = false;
	}

	if (ImGui::BeginPopupModal("Import a new Mesh file", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Please specify which model you would like to load.\n");
		ImGui::Separator();

		ImGui::Text(MeshData::Instance().root.c_str());
		ImGui::SameLine();
		static char path[128] = "";

		if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);

		if (ImGui::InputText("", path, 128, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) || ImGui::Button("Load", ImVec2(60, 0)))
		{
			if (strnlen_s(path, 128) > 0)
			{
				MeshData::Handle h = MeshData::Instance().Get(path);
				if (!h.ptr())
					ImGui::OpenPopup("Bad mesh path");
				else
					ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::BeginPopupModal("Bad mesh path"))
		{
			ImGui::Text("The path '%s%s' is invalid.", MeshData::Instance().root.c_str(), path);
			if (ImGui::Button("OK", ImVec2(60, 0)))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(60, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	// texture import
	if (gui.textureImporter)
	{
		ImGui::OpenPopup("Import a new Texture file");
		gui.textureImporter = false;
	}

	if (ImGui::BeginPopupModal("Import a new Texture file", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Please specify which texture you would like to load.\n");
		ImGui::Separator();

		ImGui::Text(TextureData::Instance().root.c_str());
		ImGui::SameLine();
		static char path[128] = "";

		if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);

		if (ImGui::InputText("", path, 128, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) || ImGui::Button("Load", ImVec2(60, 0)))
		{
			if (strnlen_s(path, 128) > 0)
			{
				TextureData::Handle h = TextureData::Instance().Get(path);
				if (!h.ptr())
					ImGui::OpenPopup("Bad texture path");
				else
					ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::BeginPopupModal("Bad texture path"))
		{
			ImGui::Text("The path '%s%s' is invalid.", TextureData::Instance().root.c_str(), path);
			if (ImGui::Button("OK", ImVec2(60, 0)))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(60, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	// shader import
	if (gui.shaderImporter)
	{
		ImGui::OpenPopup("Import a new Shader file");
		gui.shaderImporter = false;
	}

	if (ImGui::BeginPopupModal("Import a new Shader file", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Please specify which shader you would like to load.\n");
		ImGui::Separator();

		ImGui::Combo("Shader Type", &gui.shaderType, "Vertex\0Pixel");

		switch (gui.shaderType)
		{
		case 0:
			ImGui::Text(VertexShaderData::Instance().root.c_str());
			break;
		case 1:
			ImGui::Text(PixelShaderData::Instance().root.c_str());
			break;
		}

		ImGui::SameLine();
		static char path[128] = "";

		if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);

		if (ImGui::InputText("", path, 128, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) || ImGui::Button("Load", ImVec2(60, 0)))
		{
			if (strnlen_s(path, 128) > 0)
			{
				VertexShaderData::Handle vertexHandle;
				PixelShaderData::Handle pixelHandle;

				switch (gui.shaderType)
				{
				case 0:
					vertexHandle = VertexShaderData::Instance().Get(path);
					break;
				case 1:
					pixelHandle = PixelShaderData::Instance().Get(path);
					break;
				}

				if (!vertexHandle.ptr() && !pixelHandle.ptr())
					ImGui::OpenPopup("Bad shader path/type");
				else
					ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::BeginPopupModal("Bad shader path/type"))
		{
			switch (gui.shaderType)
			{
			case 0:
				ImGui::Text("The path '%s%s' is invalid\nor it is not a vertex shader.", VertexShaderData::Instance().root.c_str(), path);
				break;
			case 1:
				ImGui::Text("The path '%s%s' is invalid\nor it is not a pixel shader.", PixelShaderData::Instance().root.c_str(), path);
				break;
			}

			if (ImGui::Button("OK", ImVec2(60, 0)))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(60, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	// misc import
	if (gui.miscImporter)
	{
		ImGui::OpenPopup("Import a miscellaneous file");
		gui.miscImporter = false;
	}

	if (ImGui::BeginPopupModal("Import a miscellaneous file", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Please specify which file you would like to add.\n");
		ImGui::Separator();

		ImGui::Text(MiscData::Instance().root.c_str());
		ImGui::SameLine();
		static char path[128] = "";

		if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);

		if (ImGui::InputText("", path, 128, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) || ImGui::Button("Add", ImVec2(60, 0)))
		{
			if (strnlen_s(path, 128) > 0)
			{
				MiscData::Handle h = MiscData::Instance().Get(path);
				if (!h.ptr())
					ImGui::OpenPopup("Bad file path");
				else
					ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::BeginPopupModal("Bad file path"))
		{
			ImGui::Text("The path '%s%s' is invalid.", MiscData::Instance().root.c_str(), path);
			if (ImGui::Button("OK", ImVec2(60, 0)))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(60, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void Tool::PromptCreate()
{
	// scene create
	if (gui.sceneCreate)
	{
		ImGui::OpenPopup("Name your new scene:");
		gui.sceneCreate = false;
	}

	if (ImGui::BeginPopupModal("Name your new scene:", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Omit the file extension.\n");
		ImGui::Separator();

		ImGui::Text(SceneData::Instance().root.c_str());
		ImGui::SameLine();
		static char name[128] = "";

		if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);

		if (ImGui::InputText("", name, 128, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) || ImGui::Button("Create", ImVec2(60, 0)))
		{
			if (strnlen_s(name, 128) > 0)
			{
				SceneData::Handle h = SceneData::Instance().Create(name);
				if (!h.ptr())
					ImGui::OpenPopup("Failed to create the scene...");
				else
				{
					ImGui::CloseCurrentPopup();
					gui.sceneIndex = AssetManager::Instance().GetIndex<SceneData>(h);
				}
			}
		}

		if (ImGui::BeginPopupModal("Failed to create the scene..."))
		{
			if (ImGui::Button("Yell at David.", ImVec2(120, 0)))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(60, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	// material create
	if (gui.materialData.create)
	{
		ImGui::OpenPopup("Name your new material:");
		gui.materialData.create = false;
	}

	if (ImGui::BeginPopupModal("Name your new material:", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Omit the file extension.\n");
		ImGui::Separator();

		ImGui::Text(MaterialData::Instance().root.c_str());
		ImGui::SameLine();
		static char name[128] = "";

		if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);

		if (ImGui::InputText("", name, 128, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) || ImGui::Button("Create", ImVec2(60, 0)))
		{
			if (strnlen_s(name, 128) > 0)
			{
				MaterialData::Handle h = MaterialData::Instance().Create(name);
				if (!h.ptr())
					ImGui::OpenPopup("Failed to create the material...");
				else
				{
					ImGui::CloseCurrentPopup();
					gui.materialData.index = AssetManager::Instance().GetIndex<MaterialData>(h);
				}
			}
		}

		if (ImGui::BeginPopupModal("Failed to create the material..."))
		{
			if (ImGui::Button("Yell at David.", ImVec2(120, 0)))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(60, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void Tool::PromptEdit()
{
	// material editor
	if (gui.materialData.edit)
	{
		if (ImGui::Begin("Material Editor", &gui.materialData.edit, ImGuiWindowFlags_AlwaysAutoResize))
		{
			Asset<MaterialData> mat = AssetManager::Instance().GetAsset<MaterialData>(gui.materialData.index);

			ImGui::Text("Name: %s", mat.name.c_str());
			ImGui::Separator();

			if (ImGui::Combo("Vertex Shader", &gui.materialData.vertexShaderIndex, ComboAssetNames<VertexShaderData>, NULL, AssetManager::Instance().GetAssetCount<VertexShaderData>()))
				mat.handle->vertexShader = AssetManager::Instance().GetAsset<VertexShaderData>(gui.materialData.vertexShaderIndex).handle;

			if (ImGui::Combo("Pixel Shader", &gui.materialData.pixelShaderIndex, ComboAssetNames<PixelShaderData>, NULL, AssetManager::Instance().GetAssetCount<PixelShaderData>()))
				if (MaterialData::Instance().FlushPixelShader(mat.handle, AssetManager::Instance().GetAsset<PixelShaderData>(gui.materialData.pixelShaderIndex).handle))
				{
					gui.materialData.textureIndices.clear();
					for (size_t i = 0; i < mat.handle->textures.size(); ++i)
						gui.materialData.textureIndices.push_back(AssetManager::Instance().GetIndex<TextureData>(mat.handle->textures[i]));
				}

			ImGui::Separator();
			ImGui::Text("Textures:");

			for (size_t i = 0; i < mat.handle->textures.size(); ++i)
			{
				ImGui::PushID(i);

				if (ImGui::Combo(mat.handle->pixelShader->textures[i].c_str(), &gui.materialData.textureIndices[i], ComboAssetNames<TextureData>, NULL, AssetManager::Instance().GetAssetCount<TextureData>()))
					mat.handle->textures[i] = AssetManager::Instance().GetAsset<TextureData>(gui.materialData.textureIndices[i]).handle;

				ImGui::PopID();
			}
		}

		ImGui::End();
	}

	// misc editor
	if (gui.miscEdit)
	{
		if (ImGui::Begin("Misc Passthrough Config", &gui.miscEdit, ImGuiWindowFlags_AlwaysAutoResize))
		{
			size_t itemCount = AssetManager::Instance().GetAssetCount<MiscData>();
			float itemHeight = ImGui::GetItemsLineHeightWithSpacing();
			SceneData::Handle scene = SceneData::Instance().CurrentScene();
			Asset<MiscData> miscAsset;

			ImGui::Text("Select which miscellaneous files\nought to be included with this scene.\n");
			ImGui::Separator();

			ImGui::BeginChild("Scrolling Region", ImVec2(0, itemHeight * (itemCount < 5 ? itemCount : 5)));
			ImGuiListClipper clipper;
			clipper.Begin(itemCount, itemHeight);

			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
			{
				miscAsset = AssetManager::Instance().GetAsset<MiscData>(i);

				char buffer[128];
				snprintf(buffer, sizeof(buffer), "%s", miscAsset.name.c_str());

				ImGui::PushID(i);

				auto miscElement = std::find(scene->misc.begin(), scene->misc.end(), miscAsset.handle);
				bool isIncluded = miscElement != scene->misc.end();
				if (ImGui::Checkbox("", &isIncluded))
				{
					if (isIncluded)
						scene->misc.push_back(miscAsset.handle);
					else
						scene->misc.erase(miscElement);
				}

				ImGui::SameLine();
				ImGui::Text(buffer);

				ImGui::PopID();
			}

			clipper.End();
			ImGui::EndChild();
		}

		ImGui::End();
	}
}

void Tool::ExitToolGUI()
{
	if (gui.exitTool)
	{
		ImGui::OpenPopup("Save changes before quitting?");
		gui.exitTool = false;
	}

	if (ImGui::BeginPopupModal("Save changes before quitting?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("Yes", ImVec2(60, 0)))
		{
			AssetManager::Instance().SaveProject();
			Quit();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(60, 0)))
		{
			Quit();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(60, 0)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

void Tool::PromptExport()
{
	if (gui.exportData.prompt)
	{
		gui.exportData.sceneCount = SceneData::Instance().Size();

		ImGui::OpenPopup("Export your project");
		gui.exportData.prompt = false;
	}

	if (ImGui::BeginPopupModal("Export your project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// draggable list heavily modified/adapted from this code https://gist.github.com/Roflraging/f4af1d688237a7d367f9
		// updated to use clipper https://github.com/ocornut/imgui/issues/150

		float itemHeight = ImGui::GetItemsLineHeightWithSpacing();
		int hoverIndex = -1;
		vector<SceneData::Handle>& scenes = SceneData::Instance().sceneExportConfig;

		ImGui::Text("Drag and drop to change the scene order.\n");
		ImGui::Separator();

		ImGui::BeginChild("Scrolling Region", ImVec2(0, itemHeight * (gui.exportData.sceneCount < 5 ? gui.exportData.sceneCount : 5)));
		ImGuiListClipper clipper;
		clipper.Begin(gui.exportData.sceneCount, itemHeight);

		int sceneID = -1;
		for (int i = 0; i < gui.exportData.sceneCount; ++i)
		{
			if (scenes[i]->willExport)
				++sceneID;

			if (i >= clipper.DisplayStart && i <= clipper.DisplayEnd)
			{
				char buffer[132];
				if (scenes[i]->willExport)
					snprintf(buffer, sizeof(buffer), "%d) %s", sceneID, AssetManager::Instance().GetAsset<SceneData>(scenes[i]).name.c_str());
				else
					snprintf(buffer, sizeof(buffer), "na %s", AssetManager::Instance().GetAsset<SceneData>(scenes[i]).name.c_str());

				ImGui::PushID(i);

				ImGui::Checkbox("", &scenes[i]->willExport);
				ImGui::SameLine();
				ImGui::Selectable(buffer, scenes[i]->willExport, ImGuiSelectableFlags_DontClosePopups);

				if (ImGui::IsItemHoveredRect())
					hoverIndex = i;

				ImGui::PopID();
			}
		}

		clipper.End();
		ImGui::EndChild();

		ImGuiIO& io = ImGui::GetIO();

		if (hoverIndex >= 0)
		{
			if (ImGui::IsMouseDragging() && gui.exportData.dragIndex != -1 && gui.exportData.dragIndex != hoverIndex)
			{
				SceneData::Handle dragging = scenes[gui.exportData.dragIndex];
				
				int numMoving = gui.exportData.dragIndex - hoverIndex;
				SceneData::Handle* src = &scenes[hoverIndex];
				SceneData::Handle* dest = src + 1;

				if (hoverIndex >= gui.exportData.dragIndex)
				{
					numMoving *= -1;
					dest = &scenes[gui.exportData.dragIndex];
					src = dest + 1;
				}
				
				memmove(dest, src, sizeof(SceneData::Handle) * numMoving);
				scenes[hoverIndex] = dragging;
				gui.exportData.dragIndex = hoverIndex;
			}
			else if (io.MouseClicked[0])
				gui.exportData.dragIndex = hoverIndex;
		}

		if (io.MouseReleased[0] && gui.exportData.dragIndex != -1)
			gui.exportData.dragIndex = -1;

		ImGui::Separator();
		if (sceneID != -1)
			ImGui::Text("Name your export please:\n");

		static char name[128] = "";
		static bool exported = false;

		if (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);

		if (sceneID != -1 && (ImGui::InputText("", name, 128, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) || ImGui::Button("Export", ImVec2(60, 0))))
		{
			if (strnlen_s(name, 128) > 0)
			{
				if (AssetManager::Instance().Export(name, false))
					ImGui::CloseCurrentPopup();
				else
					ImGui::OpenPopup("Export already exists");
			}
		}

		if (exported)
		{
			ImGui::CloseCurrentPopup();
			exported = false;
		}

		if (ImGui::BeginPopupModal("Export already exists"))
		{
			if (ImGui::Button("Overwrite", ImVec2(90, 0)))
			{
				exported = AssetManager::Instance().Export(name, true);
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(60, 0)))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(60, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}
