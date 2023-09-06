#include <pluggins/sushiViewer/sushiViewer.h>
#include <engineLibraresSupport/sushi/engineSushiSupport.h>


//todo user can request imgui ids; shortcut manager context; allocators

ContainerStaticInfo SushiViewer::containerInfo()
{
	ContainerStaticInfo info = {};
	info.defaultHeapMemorySize = pika::MB(10);

	info.requestImguiFbo = true;
	info.pushAnImguiIdForMe = true;

	return info;
}

bool SushiViewer::create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
{
	renderer.create(requestedInfo.requestedFBO.fbo);

	return true;
}

void SushiViewer::displaySushiBackgroundImgui(::sushi::Background &e, int id)
{
	if (ImGui::BeginChildFrame(id, {0, 100}, true))
	{
		ImGui::Text("Background element editor");

		ImGui::ColorEdit4("Background color", &e.color[0]);

		//todo texture stuff
	}
	ImGui::EndChildFrame();
}

void SushiViewer::displaySushiTransformImgui(::sushi::Transform & e, glm::vec4 parent, int id)
{
	if (ImGui::BeginChildFrame(id, {0, 200}, true))
	{
		auto copy = e;

		ImGui::Text("Transform element editor");

		ImGui::Combo("Pos type", &copy.placementType, "Relative\0Absolute\0");
		ImGui::Combo("Size type", &copy.dimensionsType, "Percentage\0Absolute\0");

		ImGui::DragFloat2("Pos", &e.pos[0]);
		ImGui::DragFloat2("Size", &e.size[0]);

		e.changeSettings(copy, parent);

	}
	ImGui::EndChildFrame();
}

void SushiViewer::displaySushiUiElementImgui(::sushi::SushiUiElement &e, glm::vec4 parent, int id)
{
	if (ImGui::BeginChildFrame(id, {0, 500}, true))
	{
		ImGui::Text("Ui element editor: %s, id: %u", e.name, e.id);

		displaySushiTransformImgui(e.transform, parent, id + 1000);
		ImGui::Separator();
		displaySushiBackgroundImgui(e.background, id + 500);

	}
	ImGui::EndChildFrame();
}

void SushiViewer::displaySushiParentElementImgui(::sushi::SushiParent &e, glm::vec4 parent)
{
	if (ImGui::BeginChildFrame(2, {0, 700}, true))
	{
		ImGui::Text("Parent editor: %s, id: %u", e.name, e.id);

		displaySushiTransformImgui(e.transform, parent, 3);
		ImGui::Separator();
		displaySushiBackgroundImgui(e.background, 4);
		ImGui::Separator();

		sushi::Transform transform;
		transform.relativeTransformPixelSize({10, 10, 100, 100});

		if (ImGui::Button("Add element"))
		{
			sushiContext.addElement(e, "New Element", transform, sushi::Background());
		}

		if (ImGui::Button("Add parent"))
		{
			sushiContext.addParent(e, "New Parent", transform, sushi::Background({0.5,0.2,0.2,1.f}));
		}

		ImGui::Separator();
		int id = 30;
		for (auto &i : e.allUiElements)
		{
			ImGui::PushID(id++);
			displaySushiUiElementImgui(i, e.outData.absTransform, id+500);
			ImGui::PopID();
		}

	}
	ImGui::EndChildFrame();
}

//takes the id of a parent and returns his parent
sushi::SushiParent *findParentOfParent(sushi::SushiParent &parent, unsigned int id)
{

	for (auto &i : parent.subElements)
	{
		if (i.id == id)
		{
			return &parent;
		}
	}

	for (auto &i : parent.subElements)
	{
		auto rez = findParentOfParent(i, id);
		if (rez) { return rez; }
	}

	return 0;
}

void visit(sushi::SushiParent &parent, unsigned int id, sushi::SushiUiElement* &selectedElement,
sushi::SushiParent* &selectedParent
	)
{
	if (parent.id == id)
	{
		selectedParent = &parent;
		return;
	}

	for (auto &i : parent.allUiElements)
	{
		if (i.id == id)
		{
			selectedElement = &i;
			selectedParent = &parent;
			return;
		}
	}

	for (auto &i : parent.subElements)
	{
		visit(i, id, selectedElement, selectedParent);
		if (selectedElement || selectedParent) { break; }
	}

	return;
};

void visitSelect(sushi::SushiParent &parent, unsigned int &id, sushi::SushiUiElement *&selectedElement,
	sushi::SushiParent *&selectedParent, glm::vec2 mousePos
)
{
	if (sushi::pointInBox(mousePos, parent.outData.absTransform))
	{
		id = parent.id;
	}

	if (parent.id == id)
	{
		selectedParent = &parent;
	}

	for (auto &i : parent.allUiElements)
	{
		if (sushi::pointInBox(mousePos, i.outData.absTransform))
		{
			id = i.id;
		}

		if (i.id == id)
		{
			selectedElement = &i;
			selectedParent = &parent;
		}
	}

	for (auto &i : parent.subElements)
	{
		visitSelect(i, id, selectedElement, selectedParent, mousePos);
		if (selectedElement || selectedParent) { break; }
	}

	return;
};


bool SushiViewer::update(pika::Input input, pika::WindowState windowState, RequestedContainerInfo &requestedInfo)
{
	bool leftCtrlHeld = input.buttons[pika::Button::LeftCtrl].held();

#pragma region update stuff
	glClear(GL_COLOR_BUFFER_BIT);
	renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);

	if (leftCtrlHeld)
	{
		sushi::SushiInput in;
		in.mouseX = input.mouseX;
		in.mouseY = input.mouseY;
		sushiContext.update(renderer, in);
	}
	else
	{
		sushiContext.update(renderer, pika::toSushi(input));
	}
#pragma endregion


	sushi::SushiUiElement *selectedElement = 0;
	sushi::SushiParent *selectedParent = 0;

	ImGui::Begin("Sushi editor");
	{
		ImGui::Text("Current selected id: %u", img.elementId);

		auto &c = sushiContext;
		

		if (img.elementId == 0)
		{
			img.elementId = c.root.id;
		}
		
		if (leftCtrlHeld && input.lMouse.held())
		{		
			visitSelect(sushiContext.root, img.elementId, selectedElement, selectedParent,
				{input.mouseX, input.mouseY});
		}
		else
		{
			visit(sushiContext.root, img.elementId, selectedElement, selectedParent);
		}

		//ImGui::InputInt("Element: ", &img.elementSelected);
		//
		//size_t elementsSize = c.root.allUiElements.size();
		//img.elementSelected = glm::clamp(img.elementSelected, -1, (int)(elementsSize - 1));
		//


		if (selectedElement && selectedParent)
		{
			if (ImGui::Button("Select parent"))
			{
				img.elementId = selectedParent->id;
			}

			displaySushiUiElementImgui(*selectedElement, selectedParent->outData.absTransform, 100);
		}else
		if (selectedParent)
		{
			auto parentOfParentId = findParentOfParent(sushiContext.root, img.elementId);
			glm::vec4 parentRect = {0, 0, renderer.windowW, renderer.windowH};

			if (parentOfParentId)
			{
				parentRect = parentOfParentId->outData.absTransform;
			}

			displaySushiParentElementImgui(*selectedParent, parentRect);
		}


	
	}
	ImGui::End();


	if (selectedElement)
	{
		renderer.renderRectangleOutline(selectedElement->outData.absTransform,
			{1,0,0,0.5}, 4.0f);
	}else
	if (selectedParent)
	{
		//todo add alias render box
		renderer.renderRectangleOutline(selectedParent->outData.absTransform, 
			{1,0,0,0.5}, 4.0f);
	}


	renderer.flush();

	return true;
}

//optional

void SushiViewer::destruct(RequestedContainerInfo &requestedInfo)
{

}
