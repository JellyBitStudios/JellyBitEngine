#include "ComponentSlider.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleInput.h"
#include "ModuleUI.h"

#include "GameObject.h"
#include "ComponentCanvas.h"
#include "ComponentRectTransform.h"
#include "ComponentButton.h"

#include "ResourceTexture.h"

#include "imgui\imgui.h"

#include "MathGeoLib/include/Geometry/Frustum.h"

ComponentSlider::ComponentSlider(GameObject * parent, bool includeComponents) : Component(parent, ComponentTypes::SliderComponent)
{
	referenceSize = abs(parent->cmp_rectTransform->GetRect()[ComponentRectTransform::Rect::XDIST]);

	if (includeComponents)
	{
		if (parent->cmp_canvasRenderer == nullptr)
			parent->AddComponent(ComponentTypes::CanvasRendererComponent);

		referenceSize = actualSize = parent->cmp_rectTransform->GetRect()[ComponentRectTransform::Rect::XDIST];

		App->glCache->RegisterBufferIndex(&offsetB, &indexB, ComponentTypes::SliderComponent, this);
		App->glCache->RegisterBufferIndex(&offsetF, &indexF, ComponentTypes::SliderComponent, this);
		needed_fillBuffer = true;
	}
}

ComponentSlider::ComponentSlider(const ComponentSlider & ComponentSlider, GameObject * parent, bool includeComponents) : Component(parent, ComponentTypes::SliderComponent)
{
	sType = ComponentSlider.sType;
	actualSize = ComponentSlider.actualSize;
	referenceSize = ComponentSlider.referenceSize;
	percentage = ComponentSlider.percentage;
	backTexture = ComponentSlider.backTexture;
	frontTexture = ComponentSlider.frontTexture;

	if (includeComponents)
	{
		if (parent->cmp_canvasRenderer == nullptr)
			parent->AddComponent(ComponentTypes::CanvasRendererComponent);

		if (backTexture > 0u) App->res->SetAsUsed(backTexture);
		if (frontTexture > 0u) App->res->SetAsUsed(frontTexture);

		App->glCache->RegisterBufferIndex(&offsetB, &indexB, ComponentTypes::SliderComponent, this);
		App->glCache->RegisterBufferIndex(&offsetF, &indexF, ComponentTypes::SliderComponent, this);
		needed_fillBuffer = true;
	}
}


ComponentSlider::~ComponentSlider()
{
	if (backTexture > 0u) App->res->SetAsUnused(backTexture);
	if (frontTexture > 0u) App->res->SetAsUnused(frontTexture);
	if (indexB != -1) App->glCache->UnRegisterBufferIndex(offsetB, ComponentTypes::SliderComponent);
	if (indexF != -1) App->glCache->UnRegisterBufferIndex(offsetF, ComponentTypes::SliderComponent);
}

void ComponentSlider::OnSystemEvent(System_Event event)
{
	switch (event.type)
	{
	case System_Event_Type::ScreenChanged:
	case System_Event_Type::CanvasChanged:
	case System_Event_Type::RectTransformUpdated:
		referenceSize = parent->cmp_rectTransform->GetRect()[ComponentRectTransform::Rect::XDIST];
		CalculateFrontSizeByPercentage();
		needed_fillBuffer = true;
		break;
	}
}

void ComponentSlider::Update()
{
	if (App->GetEngineState() == engine_states::ENGINE_PLAY && parent->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::RECT)
	{
		int* rect = parent->cmp_rectTransform->GetRect();

		if (ComponentButton::MouseInScreen(rect))
		{
			if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN ||
				App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT)
			{
				actualSize = App->input->GetMouseX() - rect[ComponentRectTransform::Rect::X];
				CalculatePercentageByFrontSize();
				needed_fillBuffer = true;
			}
		}
	}

	if (needed_fillBuffer)
	{
		CalculateCorners();
		if(App->glCache->isNvidia())
			FillBuffers();
		needed_fillBuffer = false;
	}
}

void ComponentSlider::CalculateFrontSizeByPercentage()
{
	actualSize = referenceSize * percentage;
}

void ComponentSlider::CalculatePercentageByFrontSize()
{
	percentage = (float)actualSize / (float)referenceSize;
}

void ComponentSlider::CalculateCorners()
{
	math::float3* pCorners = parent->cmp_rectTransform->GetCorners();
	memcpy(corners, pCorners, sizeof(math::float3) * 4);

	if (parent->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::RECT)
	{
		if (App->ui->ScreenOnWorld())
			CalculateCornersFromWorld();
		else
			CalculateCornersFromScreen();
	}
	else
		CalculateCornersFromWorld();
}

float ComponentSlider::GetPercentage() const
{
	return percentage;
}

void ComponentSlider::SetPercentage(float i)
{
	if (i > 1.0f) i = 1.0f;
	if (i < 0.0f) i = 0.0f;
	percentage = i;
	CalculateFrontSizeByPercentage();
	needed_fillBuffer = true;
}

int ComponentSlider::GetBackBufferIndex() const
{
	return indexB;
}

int ComponentSlider::GetFrontBufferIndex() const
{
	return indexF;
}

void ComponentSlider::SetBufferRangeAndFIll(uint offs, int index)
{
	if (indexB == -1)
	{
		indexB = index;
		offsetB = offs;
	}
	else
	{
		indexF = index;
		offsetF = offs;
	}

	FillBuffers();
}

math::float3 * ComponentSlider::GetCorners()
{
	return corners;
}

uint ComponentSlider::GetBackTexture() const
{
	ResourceTexture* texture = (ResourceTexture*)App->res->GetResource(backTexture);
	if (texture)
		return texture->GetId();
	else
		return 0;
}

uint ComponentSlider::GetFrontTexture() const
{
	ResourceTexture* texture = (ResourceTexture*)App->res->GetResource(frontTexture);
	if (texture)
		return texture->GetId();
	else
		return 0;
}

void ComponentSlider::SetFromInvadilateResource(uint uuid)
{
	if (uuid == backTexture)
	{
		if (backTexture > 0)
			App->res->SetAsUnused(backTexture);

		backTexture = uuid;

		if (backTexture > 0)
			App->res->SetAsUsed(backTexture);
	}
	else if(uuid == frontTexture)
	{
		if (frontTexture > 0)
			App->res->SetAsUnused(frontTexture);

		frontTexture = uuid;

		if (frontTexture > 0)
			App->res->SetAsUsed(frontTexture);
	}
}

uint ComponentSlider::GetInternalSerializationBytes()
{
	return sizeof(SliderType) + sizeof(float) + sizeof(uint) * 4;
}

void ComponentSlider::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(SliderType);
	memcpy(cursor, &sType, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &percentage, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(cursor, &actualSize, bytes);
	cursor += bytes;

	memcpy(cursor, &referenceSize, bytes);
	cursor += bytes;

	memcpy(cursor, &backTexture, bytes);
	cursor += bytes;

	memcpy(cursor, &frontTexture, bytes);
	cursor += bytes;
}

void ComponentSlider::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(SliderType);
	memcpy(&sType, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(float);
	memcpy(&percentage, cursor, bytes);
	cursor += bytes;

	bytes = sizeof(uint);
	memcpy(&actualSize, cursor, bytes);
	cursor += bytes;

	memcpy(&referenceSize, cursor, bytes);
	cursor += bytes;

	memcpy(&backTexture, cursor, bytes);
	cursor += bytes;

	memcpy(&frontTexture, cursor, bytes);
	cursor += bytes;

	if(backTexture > 0) App->res->SetAsUsed(backTexture);
	if(frontTexture > 0) App->res->SetAsUsed(frontTexture);
}

void ComponentSlider::OnUniqueEditor()
{
#ifndef GAMEMODE
	if (ImGui::CollapsingHeader("Slider", ImGuiTreeNodeFlags_DefaultOpen))
	{
		float tmpP = percentage * 100;
		float min = 0, max = 100;
		if (ImGui::DragFloat("Percentage", &tmpP, 1.0f, min, max, "%.f"))
		{
			percentage = tmpP / 100;
			CalculateFrontSizeByPercentage();
			needed_fillBuffer = true;
		}

		ResourceTexture* texture = nullptr;
		if (backTexture != 0)
			texture = (ResourceTexture*)App->res->GetResource(backTexture);
		ImGui::Button(texture == nullptr ? "Empty Back Texture" : texture->GetName(), ImVec2(100.0f, 0.0f));

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", backTexture);
			ImGui::EndTooltip();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_INSPECTOR_SELECTOR"))
			{
				if(backTexture > 0u)
					App->res->SetAsUnused(backTexture);
				backTexture = *(uint*)payload->Data;
				App->res->SetAsUsed(backTexture);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		texture = nullptr;
		if (frontTexture != 0)
			texture = (ResourceTexture*)App->res->GetResource(frontTexture);
		ImGui::Button(texture == nullptr ? "Empty Front Texture" : texture->GetName(), ImVec2(100.0f, 0.0f));

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("%u", frontTexture);
			ImGui::EndTooltip();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_INSPECTOR_SELECTOR"))
			{
				if (frontTexture > 0u)
					App->res->SetAsUnused(frontTexture);
				frontTexture = *(uint*)payload->Data;
				App->res->SetAsUsed(frontTexture);
			}
			ImGui::EndDragDropTarget();
		}
	}
#endif
}

void ComponentSlider::FillBuffers()
{
	math::float3* rCorners = corners;
	float one = 1.0f;
	char* cursor = buffer;
	size_t bytes = sizeof(float) * 3;
	if (indexB != -1)
	{
		memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RTOPLEFT], bytes);
		cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
		memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RTOPRIGHT], bytes);
		cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
		memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RBOTTOMLEFT], bytes);
		cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
		memcpy(cursor, &rCorners[ComponentRectTransform::Rect::RBOTTOMRIGHT], bytes);
		cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);

		App->glCache->FillBufferRange(offsetB, UI_BYTES_RECT, buffer);
	}

	if (indexF != -1)
	{
		cursor = buffer;

		memcpy(cursor, &rCorners[4 + ComponentRectTransform::Rect::RTOPLEFT], bytes);
		cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
		memcpy(cursor, &rCorners[4 + ComponentRectTransform::Rect::RTOPRIGHT], bytes);
		cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
		memcpy(cursor, &rCorners[4 + ComponentRectTransform::Rect::RBOTTOMLEFT], bytes);
		cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);
		memcpy(cursor, &rCorners[4 + ComponentRectTransform::Rect::RBOTTOMRIGHT], bytes);
		cursor += bytes; memcpy(cursor, &one, sizeof(float)); cursor += sizeof(float);

		App->glCache->FillBufferRange(offsetF, UI_BYTES_RECT, buffer);
	}
}

void ComponentSlider::CalculateCornersFromScreen()
{
	int* rectTransform = parent->cmp_rectTransform->GetRect();
	int* screen = App->ui->GetScreen();
	uint w_width = screen[ModuleUI::Screen::WIDTH];
	uint w_height = screen[ModuleUI::Screen::HEIGHT];
	corners[4 + ComponentRectTransform::Rect::RTOPLEFT] = { math::Frustum::ScreenToViewportSpace({ (float)rectTransform[ComponentRectTransform::Rect::X], (float)rectTransform[ComponentRectTransform::Rect::Y] }, w_width, w_height), 0.0f };
	corners[4 + ComponentRectTransform::Rect::RTOPRIGHT] = { math::Frustum::ScreenToViewportSpace({ (float)rectTransform[ComponentRectTransform::Rect::X] + (float)actualSize, (float)rectTransform[ComponentRectTransform::Rect::Y] }, w_width, w_height), 0.0f };
	corners[4 + ComponentRectTransform::Rect::RBOTTOMLEFT] = { math::Frustum::ScreenToViewportSpace({ (float)rectTransform[ComponentRectTransform::Rect::X], (float)rectTransform[ComponentRectTransform::Rect::Y] + (float)rectTransform[ComponentRectTransform::Rect::YDIST] }, w_width, w_height), 0.0f };
	corners[4 + ComponentRectTransform::Rect::RBOTTOMRIGHT] = { math::Frustum::ScreenToViewportSpace({ (float)rectTransform[ComponentRectTransform::Rect::X] + (float)actualSize, (float)rectTransform[ComponentRectTransform::Rect::Y] + (float)rectTransform[ComponentRectTransform::Rect::YDIST] }, w_width, w_height), 0.0f };
}

void ComponentSlider::CalculateCornersFromWorld()
{
	int* rectTransform = parent->cmp_rectTransform->GetRect();
	math::float3* parentCorners = nullptr;
	int* rectParent = nullptr;
#ifndef GAMEMODE
	if (parent->cmp_rectTransform->GetFrom() == ComponentRectTransform::RectFrom::RECT)
	{
		parentCorners = App->ui->GetWHCorners();
		rectParent = App->ui->GetWHRect();
	}
	else
	{
		parentCorners = parent->cmp_rectTransform->GetCorners();
		rectParent = parent->cmp_rectTransform->GetRect();
	}
#else
	parentCorners = parent->cmp_rectTransform->GetCorners();
	rectParent = parent->cmp_rectTransform->GetRect();
#endif
	math::float3 xDirection = (parentCorners[ComponentRectTransform::Rect::RTOPLEFT] - parentCorners[ComponentRectTransform::Rect::RTOPRIGHT]).Normalized();
	math::float3 yDirection = (parentCorners[ComponentRectTransform::Rect::RBOTTOMLEFT] - parentCorners[ComponentRectTransform::Rect::RTOPLEFT]).Normalized();

	corners[4 + ComponentRectTransform::Rect::RTOPRIGHT] = parentCorners[ComponentRectTransform::Rect::RTOPRIGHT] + (xDirection * ((float)(rectTransform[ComponentRectTransform::Rect::X] - rectParent[ComponentRectTransform::Rect::X]) / WORLDTORECT)) + (yDirection * ((float)(rectTransform[ComponentRectTransform::Rect::Y] - rectParent[ComponentRectTransform::Rect::Y]) / WORLDTORECT));
	corners[4 + ComponentRectTransform::Rect::RTOPLEFT] = corners[4 + ComponentRectTransform::Rect::RTOPRIGHT] + (xDirection * ((float)actualSize / WORLDTORECT));
	corners[4 + ComponentRectTransform::Rect::RBOTTOMLEFT] = corners[4 + ComponentRectTransform::Rect::RTOPLEFT] + (yDirection * ((float)rectTransform[ComponentRectTransform::Rect::YDIST] / WORLDTORECT));
	corners[4 + ComponentRectTransform::Rect::RBOTTOMRIGHT] = corners[4 + ComponentRectTransform::Rect::RBOTTOMLEFT] - (xDirection * ((float)actualSize / WORLDTORECT));

	math::float3 zDirection = xDirection.Cross(yDirection);
	uint tmp;
	float z = ModuleUI::FindCanvas(parent, tmp)->cmp_canvas->GetZ(parent, GetType());
	corners[4 + ComponentRectTransform::Rect::RTOPRIGHT] -= zDirection * z;
	corners[4 + ComponentRectTransform::Rect::RTOPLEFT] -= zDirection * z;
	corners[4 + ComponentRectTransform::Rect::RBOTTOMLEFT] -= zDirection * z;
	corners[4 + ComponentRectTransform::Rect::RBOTTOMRIGHT] -= zDirection * z;
}
