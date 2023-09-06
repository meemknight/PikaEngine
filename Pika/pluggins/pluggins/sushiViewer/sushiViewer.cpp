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

		ImGui::Combo("Anchor", &copy.anchorPoint, "topLeft"
			"\0top Middle"
			"\0top Right"
			"\0middle Left"
			"\0center"
			"\0middle Right"
			"\0bottom Left"
			"\0bottom Middle"
			"\0bottom Right"
			"\0absolute\0");

		ImGui::DragFloat2("Pos pixels", &e.positionPixels[0]);
		ImGui::DragFloat2("Pos percentage", &e.positionPercentage[0], 0.01);

		ImGui::DragFloat2("Size pixels", &e.sizePixels[0]);
		ImGui::DragFloat2("Size percentage", &e.sizePercentage[0], 0.01);

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
		transform.anchorPoint = sushi::Transform::center; 
		transform.sizePercentage = glm::vec2(0.5f);

		if (ImGui::Button("Add element"))
		{
			img.elementId = sushiContext.addElement(e, "New Element", transform, sushi::Background());
		}

		if (ImGui::Button("Add parent"))
		{
			img.elementId = sushiContext.addParent(e, "New Parent", transform, sushi::Background({0.5,0.2,0.2,1.f}));
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
	//bool shiftHeld = input.buttons[pika::Button::Shift].held();

#pragma region update stuff
	glClear(GL_COLOR_BUFFER_BIT);
	renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);

	//if (leftCtrlHeld)
	{
		sushi::SushiInput in;
		in.mouseX = input.mouseX;
		in.mouseY = input.mouseY;
		sushiContext.update(renderer, in);
	}
	//else
	//{
	//	sushiContext.update(renderer, pika::toSushi(input));
	//}
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
		
		if (input.lMouse.held())
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

	auto drawDisplacement = [&](glm::vec4 from, glm::vec4 to, glm::vec4 color)
	{
		if (to.x != from.x || to.y != from.y)
		{
			renderer.renderRectangle(to, color);

			renderer.renderLine(to, from, color);
		}
		
		renderer.renderRectangleOutline(from, color, 2.f);
	};

	auto drawGyzmos = [&](glm::vec4 pos, sushi::Transform &t, glm::vec4 parentPos)
	{
		//todo add alias render box
		renderer.renderRectangleOutline(pos,
			{0,1,0,0.5}, 4.0f);

		glm::vec4 center = {glm::vec2(pos) + glm::vec2(pos.z, pos.w) / 2.f - glm::vec2(3,3), 
			6.f,6.f};
	
		glm::vec4 originalCenter = center;
		originalCenter.x -= t.positionPixels.x;
		originalCenter.y -= t.positionPixels.y;

		if (t.anchorPoint != sushi::Transform::anchor::absolute &&
			(t.positionPixels.x != 0 || t.positionPixels.y != 0))
		{
			drawDisplacement(originalCenter, center, {0,1,0,0.5});
		}
		else
		{
			renderer.renderRectangleOutline(originalCenter, {0,1,0,0.5}, 2.f);
		}
				
		glm::vec4 newCenter = {};

		switch (t.anchorPoint)
		{

			case ::sushi::Transform::topLeft:
			{
				
			}
			break;

			case ::sushi::Transform::topMiddle:
			{
				
			}
			break;

			case ::sushi::Transform::toRight:
			{
				
			}
			break;

			case ::sushi::Transform::middleLeft:
			{
				
			}
			break;

			case ::sushi::Transform::center:
			{
				newCenter = {glm::vec2(parentPos)
					+ glm::vec2(parentPos.z, parentPos.w) / 2.f - glm::vec2(3, 3), 6.f, 6.f};
			}
			break;

			case ::sushi::Transform::middleRight:
			{
				
			}
			break;

			case ::sushi::Transform::bottomLeft:
			{
				
			}
			break;

			case ::sushi::Transform::bottomMiddle:
			{
				
			}
			break;

			case ::sushi::Transform::bottomRight:
			{
				
			}
			break;

			case ::sushi::Transform::absolute:
			{

				
			}
			break;
		
		}

		drawDisplacement(newCenter, originalCenter, {0.8,0.5,0,0.5});


	};

	if (selectedElement)
	{
		glm::vec4 parentTransform = {0,0,renderer.windowW, renderer.windowH};

		if (selectedParent) { parentTransform = selectedParent->outData.absTransform; }

		drawGyzmos(selectedElement->outData.absTransform, selectedElement->transform, parentTransform);

		if (selectedParent)
		{
			//todo add alias render box
			renderer.renderRectangleOutline(selectedParent->outData.absTransform,
				{0.5,0.5,0,0.5}, 4.0f);
		}

	}else
	if (selectedParent)
	{
		//todo add alias render box
		drawGyzmos(selectedParent->outData.absTransform, selectedParent->transform, selectedParent->outData.absTransform);
	}


	renderer.flush();

	return true;
}

//optional

void SushiViewer::destruct(RequestedContainerInfo &requestedInfo)
{

}
