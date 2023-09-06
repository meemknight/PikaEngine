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


	sushi::SushiUiElement element;
	element.background.color = Colors_Magenta;
	element.transform.absoluteTransformPixelSize({100,100,200,100});
	element.id = 2;

	sushiContext.root.allUiElements.push_back(element);

	return true;
}

void displaySushiBackgroundImgui(::sushi::Background &e)
{
	if (ImGui::BeginChildFrame(11, {0, 100}, true))
	{
		ImGui::Text("Background element editor");

		ImGui::ColorEdit4("Background color", &e.color[0]);

		//todo texture stuff
	}
	ImGui::EndChildFrame();
}

void displaySushiTransformImgui(::sushi::Transform &e)
{
	if (ImGui::BeginChildFrame(12, {0, 100}, true))
	{
		ImGui::Text("Transform element editor");



	}
	ImGui::EndChildFrame();
}

void displaySushiUiElementImgui(::sushi::SushiUiElement &e)
{
	if (ImGui::BeginChildFrame(13, {0, 500}, true))
	{
		ImGui::Text("Ui element editor: %s, id: %u", e.name, e.id);

		displaySushiTransformImgui(e.transform);
		ImGui::Separator();
		displaySushiBackgroundImgui(e.background);

	}
	ImGui::EndChildFrame();
}

void displaySushiParentElementImgui(::sushi::SushiParent &e)
{
	if (ImGui::BeginChildFrame(14, {0, 700}, true))
	{
		ImGui::Text("Parent editor: %s, id: %u", e.name, e.id);

		displaySushiTransformImgui(e.transform);
		ImGui::Separator();
		displaySushiBackgroundImgui(e.background);

		ImGui::Separator();
		for (auto &i : e.allUiElements)
		{
			displaySushiUiElementImgui(i);
		}

	}
	ImGui::EndChildFrame();
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

		if (selectedElement)
		{
			if (selectedParent)
			{
				if (ImGui::Button("Select parent"))
				{
					img.elementId = selectedParent->id;
				}
			}

			displaySushiUiElementImgui(*selectedElement);
		}else
		if (selectedParent)
		{
			displaySushiParentElementImgui(*selectedParent);
		}


	
	}
	ImGui::End();


	if (selectedParent)
	{
		//todo add alias render box
		renderer.renderRectangleOutline(selectedParent->outData.absTransform, 
			{1,0,0,0.5}, 4.0f);
	}

	if (selectedElement)
	{
		renderer.renderRectangleOutline(selectedElement->outData.absTransform,
			{1,0,0,0.5}, 4.0f);
	}


	renderer.flush();

	return true;
}

//optional

void SushiViewer::destruct(RequestedContainerInfo &requestedInfo)
{

}
