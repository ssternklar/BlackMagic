#include "Tool.h"

#include "dear imgui\imgui.h"
#include "dear imgui\imgui_impl_dx11.h"

#include "Input.h"
#include "Assets.h"
#include "FileUtil.h"

// TODO make this generic, somehow
bool MeshNames(void* data, int idx, const char** out_text)
{
	if (idx < 0 || idx >= (int)AssetManager::Instance().GetAssetCount<MeshData>())
		return false;
	*out_text = AssetManager::Instance().GetAsset<MeshData>(idx).name.c_str();
	return true;
}

void Tool::helloGUI()
{
	ImGuiIO& io = ImGui::GetIO();
	
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
			if (ImGui::Button("OK", ImVec2(120, 0)))
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
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Import"))
		{
			if (ImGui::MenuItem("Mesh"))
				gui.meshImporter = true;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	PromptImport();

	// status info window
	ImGui::SetNextWindowPos(ImVec2((float)graphics->GetWidth() - 309, 19));
	if (ImGui::Begin("Stats Bar", NULL, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs))
	{
		ImGui::Text("Res: %.0fx%.0f\tFPS: %.0f Delta: %.5f", io.DisplaySize.x, io.DisplaySize.y, io.Framerate, io.DeltaTime * 1000);
	}
	ImGui::End();

	// entity editor
	ImGui::SetNextWindowPos(ImVec2((float)graphics->GetWidth() - 309, 51));
	ImGui::SetNextWindowSize(ImVec2(309, (float)graphics->GetHeight() - 51));
	if (ImGui::Begin("EntityEditor", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		if (ImGui::Button("Spawn"))
			SelectEntity(EntityData::Instance().Get());

		if (selectedEntity.ptr())
		{
			ImGui::DragFloat3("Position", &selectedEntity->transform->pos.x, 0.005f);
			ImGui::Combo("Mesh", &gui.meshIndex, MeshNames, NULL, AssetManager::Instance().GetAssetCount<MeshData>());
			selectedEntity->mesh = AssetManager::Instance().GetAsset<MeshData>(gui.meshIndex).handle;
		}
	}
	ImGui::End();
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

		if (ImGui::InputText("", path, 128, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) || ImGui::Button("Load", ImVec2(120, 0)))
		{
			MeshData::Handle h = MeshData::Instance().Get(path);
			if (!h.ptr())
				ImGui::OpenPopup("Bad mesh path");
			else
				ImGui::CloseCurrentPopup();
		}

		if (ImGui::BeginPopupModal("Bad mesh path"))
		{
			ImGui::Text("The path '%s%s' is invalid.", MeshData::Instance().root.c_str(), path);
			if (ImGui::Button("OK", ImVec2(120, 0)))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}
