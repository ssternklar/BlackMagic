#include "Tool.h"

#include "dear imgui\imgui.h"
#include "dear imgui\imgui_impl_dx11.h"

#include "Input.h"

bool comboPaths(void* data, int idx, const char** out_text)
{
	std::vector<std::string>* vec = reinterpret_cast<std::vector<std::string>*>(data);
	if (idx < 0 || idx >= (int)vec->size())
		return false;
	*out_text = vec->at(idx).c_str();
	return true;
}

void Tool::invokeGUI()
{
	if (Input::WasControlPressed("Quit"))
		Quit();

	ImGui::ShowTestWindow(NULL);

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
				meshImporter = true;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// mesh import
	if (meshImporter)
	{
		ImGui::OpenPopup("Import a new Mesh file");
		meshImporter = false;
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
			ImGui::Combo("Mesh", &meshIndex, comboPaths, reinterpret_cast<void*>(&MeshData::Instance().filePaths), MeshData::Instance().filePaths.size());
			selectedEntity->mesh = MeshData::Instance().Get(MeshData::Instance().filePaths[meshIndex]);
		}
	}
	ImGui::End();
}
