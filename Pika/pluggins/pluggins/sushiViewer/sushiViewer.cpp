#include <pluggins/sushiViewer/sushiViewer.h>
#include <engineLibraresSupport/sushi/engineSushiSupport.h>
#include <pikaImgui/pikaImgui.h>

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

void SushiViewer::displaySushiUiElementImgui(::sushi::SushiUiElement &e, glm::vec4 parent)
{
	if (ImGui::BeginChildFrame(e.id, {0, 500}, true))
	{
		ImGui::PushID(e.id);
		ImGui::Text("Ui element editor: %s id: %u", e.name, e.id);

		char data[16] = {};
		static_assert(sizeof(data) == sizeof(e.name));
		std::strncpy(data, e.name, sizeof(e.name) - 1);
		ImGui::InputText("Rename", data, sizeof(e.name));
		if (std::strcmp(data, e.name))
		{
			sushiContext.rename(&e, data);
		}

		ImGui::Separator();
		if (pika::pikaImgui::redButton("Delete"))
		{
			toDelete.push_back(e.id);
		}
		ImGui::Separator();

		displaySushiTransformImgui(e.transform, parent, e.id + 20000);
		ImGui::Separator();
		displaySushiBackgroundImgui(e.background, e.id + 10000);
		ImGui::PopID();
	}
	ImGui::EndChildFrame();
}

void SushiViewer::displaySushiParentElementImgui(::sushi::SushiParent &e, glm::vec4 parent)
{
	if (pika::pikaImgui::BeginChildFrameColoured(2, {0.2,0.3,0.7,0.9}, { 0, 700 }, true))
	{
		ImGui::PushID(e.id);

		ImGui::Text("Parent editor: %s, id: %u", e.name, e.id);

		char data[16] = {};
		static_assert(sizeof(data) == sizeof(e.name));
		std::strncpy(data, e.name, sizeof(e.name) - 1);
		ImGui::InputText("Rename", data, sizeof(e.name));
		if (std::strcmp(data, e.name))
		{
			sushiContext.rename(&e, data);
		}

		ImGui::Separator();
		if (pika::pikaImgui::redButton("Delete"))
		{
			toDelete.push_back(e.id);
		}
		ImGui::Separator();

		ImGui::NewLine();

		ImGui::Combo("Layout type", &e.layoutType, "Free\0Horizonta\0Vertical\0");

		displaySushiTransformImgui(e.transform, parent, e.id + 20000);
		ImGui::Separator();
		displaySushiBackgroundImgui(e.background, e.id + 10000);
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
		if(!e.allUiElements.empty())
			{ImGui::Text("Ui Elements:");}
		int id = 30;
		for (auto &i : e.allUiElements)
		{
			ImGui::PushID(id++);
			displaySushiUiElementImgui(i, e.outData.absTransform);
			ImGui::PopID();
		}

		ImGui::PopID();
	}
	ImGui::EndChildFrame();
}

//takes the id of a parent and returns his parent
sushi::SushiParent *findParentOfParent(sushi::SushiParent &parent, unsigned int id)
{

	for (auto &i : parent.parents)
	{
		if (i.id == id)
		{
			return &parent;
		}
	}

	for (auto &i : parent.parents)
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

	for (auto &i : parent.parents)
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

	for (auto &i : parent.parents)
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

	while (!toDelete.empty())
	{
		auto id = toDelete.back();
		toDelete.pop_back();
		sushiContext.deleteById(id);
	}


	sushi::SushiUiElement *selectedElement = 0;
	sushi::SushiParent *selectedParent = 0;
	sushi::SushiParent *parentOfParent = 0;

	ImGui::Begin("Sushi editor");
	{

		if (img.fileSelector.run(9000325))
		{

		};

		if (ImGui::Button("Load"))
		{
			sushi::SushyBinaryFormat data;

			if (requestedInfo.readEntireFileBinary(img.fileSelector.file, data.data))
			{
				if (!sushiContext.load(data))
				{
					requestedInfo.consoleWrite("Couldn't parse file");
				}
			}
			else
			{
				requestedInfo.consoleWrite("Couldn't open file");
			}
		}

		else if (ImGui::Button("Save"))
		{

			if (img.fileSelector.file[0] != 0)
			{
				auto rez = sushiContext.save();

				requestedInfo.writeEntireFileBinary(img.fileSelector.file, rez.data.data(),
					rez.data.size() * sizeof(rez.data[0]));
			}

		}

		ImGui::Text("Current selected id: %u", img.elementId);

		auto &c = sushiContext;
		

		if (img.elementId == 0)
		{
			img.elementId = c.root.id;
		}
		
		if (input.lMouse.held() && !img.dragging)
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

			displaySushiUiElementImgui(*selectedElement, selectedParent->outData.absTransform);
		}else
		if (selectedParent)
		{
			parentOfParent = findParentOfParent(sushiContext.root, img.elementId);
			glm::vec4 parentRect = {0, 0, renderer.windowW, renderer.windowH};

			if (parentOfParent)
			{
				parentRect = parentOfParent->outData.absTransform;
			}

			displaySushiParentElementImgui(*selectedParent, parentRect);
		}


	
	}
	ImGui::End();

	auto drawDisplacement = [&](glm::vec2 from2, glm::vec2 to2, glm::vec4 color)
	{
		float thickness = 10;

		glm::vec4 from = {from2 - glm::vec2(thickness,thickness), thickness, thickness};
		glm::vec4 to = {to2 - glm::vec2(thickness,thickness), thickness, thickness};

		if (to.x != from.x || to.y != from.y)
		{
			renderer.renderRectangle(to, color);

			renderer.renderLine(glm::vec2(to) + glm::vec2(to.z,to.w)/2.f,
				glm::vec2(from) + glm::vec2(from.z, from.w) / 2.f, color, 3);
		}
		
		renderer.renderRectangleOutline(from, color, 3.f);
	};

	auto drawGyzmos = [&](glm::vec4 pos, sushi::Transform &t, glm::vec4 parentPos, float opacity = 0.5)
	{
		//todo add alias render box
		renderer.renderRectangleOutline(pos,
			{0,1,0,0.5}, 4.0f);

		glm::vec2 anchorPoiont = {};
		glm::vec2 anchorPoiontMoved = {};

		switch (t.anchorPoint)
		{

			case ::sushi::Transform::topLeft:
			{
				anchorPoiont = {glm::vec2(parentPos)};
				anchorPoiontMoved = {glm::vec2(pos)};
			}
			break;

			case ::sushi::Transform::topMiddle:
			{
				anchorPoiont = {glm::vec2(parentPos)
					+ glm::vec2(parentPos.z / 2.f, 0)};
				anchorPoiontMoved = {glm::vec2(pos)
					+ glm::vec2(pos.z / 2.f, 0)};
			}
			break;

			case ::sushi::Transform::topRight:
			{
				anchorPoiont = {glm::vec2(parentPos)
					+ glm::vec2(parentPos.z, 0)};
				anchorPoiontMoved = {glm::vec2(pos)
					+ glm::vec2(pos.z, 0)};
			}
			break;

			case ::sushi::Transform::middleLeft:
			{
				anchorPoiont = {glm::vec2(parentPos)
					+ glm::vec2(0, parentPos.w/2.f)};
				anchorPoiontMoved = {glm::vec2(pos)
					+ glm::vec2(0, pos.w / 2.f)};
			}
			break;

			case ::sushi::Transform::center:
			{
				anchorPoiont = {glm::vec2(parentPos)
					+ glm::vec2(parentPos.z, parentPos.w) / 2.f};
				anchorPoiontMoved = {glm::vec2(pos)
					+ glm::vec2(pos.z, pos.w) / 2.f};
			}
			break;

			case ::sushi::Transform::middleRight:
			{
				anchorPoiont = {glm::vec2(parentPos)
					+ glm::vec2(parentPos.z, parentPos.w/2.f)};
				anchorPoiontMoved = {glm::vec2(pos)
					+ glm::vec2(pos.z, pos.w / 2.f)};
			}
			break;

			case ::sushi::Transform::bottomLeft:
			{
				anchorPoiont = {glm::vec2(parentPos)
					+ glm::vec2(0, parentPos.w)};
				anchorPoiontMoved = {glm::vec2(pos)
					+ glm::vec2(0, pos.w)};
			}
			break;

			case ::sushi::Transform::bottomMiddle:
			{
				anchorPoiont = {glm::vec2(parentPos)
					+ glm::vec2(parentPos.z / 2.f , parentPos.w)};
				anchorPoiontMoved = {glm::vec2(pos)
					+ glm::vec2(pos.z / 2.f , pos.w)};
			}
			break;

			case ::sushi::Transform::bottomRight:
			{
				anchorPoiont = {glm::vec2(parentPos)
					+ glm::vec2(parentPos.z, parentPos.w)};
				anchorPoiontMoved = {glm::vec2(pos)
					+ glm::vec2(pos.z, pos.w)};
			}
			break;

			case ::sushi::Transform::absolute:
			{
				anchorPoiont = {};
				anchorPoiontMoved = {glm::vec2(pos)};
			}
			break;
		
		}

		glm::vec2 anchorPoiontMovedFinal = anchorPoiontMoved;
		anchorPoiontMoved -= t.positionPixels;

		drawDisplacement(anchorPoiont, anchorPoiontMoved, {0.8,0.5,0,opacity}); //percentage move

		drawDisplacement(anchorPoiontMoved, anchorPoiontMovedFinal, {0,1,0,opacity}); //pixel move
	};

	sushi::Transform *selectedTransform = 0;

	//render element
	if (selectedElement)
	{
		glm::vec4 parentTransform = {0,0,renderer.windowW, renderer.windowH};
		if (selectedParent) { parentTransform = selectedParent->outData.absTransform; }

		drawGyzmos(selectedElement->outData.absTransform, selectedElement->transform, 
			parentTransform);

		if (selectedParent)
		{
			//todo add alias render box
			renderer.renderRectangleOutline(selectedParent->outData.absTransform,
				{0.5,0.5,0,0.5}, 4.0f);
		}

		selectedTransform = &selectedElement->transform;
	}else
	if (selectedParent) //rende
	{

		for (auto &i : selectedParent->allUiElements)
		{
			drawGyzmos(i.outData.absTransform, i.transform,
				selectedParent->outData.absTransform, 0.2);
		}

		for (auto &i : selectedParent->parents)
		{
			drawGyzmos(i.outData.absTransform, i.transform,
				selectedParent->outData.absTransform, 0.2);
		}

		selectedTransform = &selectedParent->transform;

		glm::vec4 parentTransform = {0,0,renderer.windowW, renderer.windowH};
		if (parentOfParent) { parentTransform = parentOfParent->outData.absTransform; }

		drawGyzmos(selectedParent->outData.absTransform, 
			selectedParent->transform, parentTransform);
	}

	if (leftCtrlHeld && input.lMouse.held() && selectedTransform)
	{
		if (!img.dragging)
		{
			img.dragging = true;
			img.dragBegin = {input.mouseX, input.mouseY};
			img.originalPos = selectedTransform->positionPixels;
		}
		else
		{
			glm::vec2 newPos = {input.mouseX, input.mouseY};
			glm::vec2 delta = newPos - img.dragBegin;

			selectedTransform->positionPixels = img.originalPos + delta;
		}

	}
	else
	{
		img.dragging = 0;
		img.dragBegin = {};
	}


	renderer.flush();

	return true;
}

//optional

void SushiViewer::destruct(RequestedContainerInfo &requestedInfo)
{

}
