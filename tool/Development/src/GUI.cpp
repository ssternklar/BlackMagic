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
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Scene"))
		{
			if (ImGui::MenuItem("New Scene"))
				gui.sceneCreate = true;

			ImGui::Combo("Scene", &gui.sceneIndex, ComboAssetNames<SceneData>, NULL, AssetManager::Instance().GetAssetCount<SceneData>());
			SceneData::Instance().SwapScene(gui.sceneIndex);

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	PromptImport();

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
				ImGui::DragFloat3("Position", &scene->selectedEntity->transform->pos.x, 0.005f);
				ImGui::DragFloat("Scale", &scene->selectedEntity->transform->scale, 0.005f);
				ImGui::Combo("Mesh", &gui.entityData.meshIndex, ComboAssetNames<MeshData>, NULL, AssetManager::Instance().GetAssetCount<MeshData>());
				scene->selectedEntity->mesh = AssetManager::Instance().GetAsset<MeshData>(gui.entityData.meshIndex).handle;
			}
		}

		ImGui::End();
	}

	PromptExport();
	ExitToolGUI();
}

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

		float itemHeight = ImGui::GetTextLineHeightWithSpacing();
		int displayStart = 0, displayEnd = gui.exportData.sceneCount;
		int hoverIndex = -1;
		vector<SceneData::Handle>& scenes = SceneData::Instance().sceneExportConfig;

		ImGui::Text("Drag and drop to change the scene order.\n");
		ImGui::Separator();

		ImGui::CalcListClipping(gui.exportData.sceneCount, itemHeight, &displayStart, &displayEnd);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (displayStart * itemHeight));

		int sceneID = -1;
		for (int i = displayStart; i < displayEnd; ++i)
		{
			char buffer[132];
			if (scenes[i]->willExport)
				snprintf(buffer, sizeof(buffer), "%d) %s", ++sceneID, AssetManager::Instance().GetAsset<SceneData>(scenes[i]).name.c_str());
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

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ((gui.exportData.sceneCount - displayEnd) * itemHeight));
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
