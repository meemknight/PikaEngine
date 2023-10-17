#include <pluggins/sushiViewer/sushiViewer.h>
#include <engineLibraresSupport/sushi/engineSushiSupport.h>
#include <pikaImgui/pikaImgui.h>
#include <imgui_stdlib.h>
#include <engineLibraresSupport/engineGL2DSupport.h>

//todo user can request imgui ids; shortcut manager context; allocators

ContainerStaticInfo SushiViewer::containerInfo()
{
	ContainerStaticInfo info = {};
	info.defaultHeapMemorySize = pika::MB(50);

	info.requestImguiFbo = true;
	info.pushAnImguiIdForMe = true;

	return info;
}

bool SushiViewer::create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
{
	renderer.create(requestedInfo.requestedFBO.fbo);
	font = pika::gl2d::loadFont(PIKA_RESOURCES_PATH "arial.ttf", requestedInfo);

	return true;
}

void SushiViewer::displaySushiBackgroundImgui(::sushi::Background &e, int id)
{
	if (ImGui::BeginChildFrame(id, {0, 100}, true))
	{
		ImGui::Text("Background element editor");

		pika::pikaImgui::ColorEdit4Swatches("Background color", &e.color[0]);

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

		const char *names[] = {
			"topLeft", "top Middle", "top Right", "middle Left", "center", "middle Right", 
			"bottom Left", "bottom Middle", "bottom Right", "absolute",
		};

		{
			bool selected[3 * 3 + 1] = {};
			for (int y = 0; y < 3; y++)
			{
				for (int x = 0; x < 3; x++)
				{
					ImVec2 alignment = ImVec2((float)x, (float)y);
				
					if (x > 0) ImGui::SameLine();
					ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
					if (ImGui::Button(names[x + y * 3], ImVec2(40, 40)))
					{
						copy.anchorPoint = x + y * 3;
					}
					ImGui::PopStyleVar();
				}
			}

			ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {1,3});
			if (ImGui::Button(names[sushi::Transform::absolute], ImVec2(40, 40)))
			{
				copy.anchorPoint = sushi::Transform::absolute;
			}
			ImGui::PopStyleVar();
		}

		ImGui::DragFloat2("Pos pixels", &e.positionPixels[0]);
		ImGui::DragFloat2("Pos percentage", &e.positionPercentage[0], 0.01);

		

		ImGui::Combo("Size calculation", &e.sizeCalculationType, "normal"
			"\0aspect ratio on x"
			"\0aspect ratio on y"
			"\0aspect ratio on X keep minimum"
			"\0aspect ratio on y keep minimum"
			"\0");

		if (e.sizeCalculationType == sushi::Transform::normalSize)
		{
			ImGui::DragFloat2("Size pixels", &e.sizePixels[0]);
			ImGui::DragFloat2("Size percentage", &e.sizePercentage[0], 0.01);
		}
		else if (e.sizeCalculationType == sushi::Transform::useAspectRatioOnY)
		{
			ImGui::DragFloat("Size pixels", &e.sizePixels[0]);
			ImGui::DragFloat("Size percentage x", &e.sizePercentage[0], 0.01);
			ImGui::DragFloat("Aspect ratio", &e.aspectRation, 0.01, 0);
		}
		else if (e.sizeCalculationType == sushi::Transform::useAspectRatioOnX)
		{
			ImGui::DragFloat("Size pixels", &e.sizePixels[1]);
			ImGui::DragFloat("Size percentage y", &e.sizePercentage[1], 0.01);
			ImGui::DragFloat("Aspect ratio", &e.aspectRation, 0.01, 0);
		}
		else if (e.sizeCalculationType == sushi::Transform::useAspectRatioOnXKeepMinimum)
		{
			ImGui::DragFloat2("Size pixels", &e.sizePixels[0]);
			ImGui::DragFloat2("Size percentage", &e.sizePercentage[0], 0.01);
			ImGui::DragFloat("Aspect ratio", &e.aspectRation, 0.01, 0);
		}
		else if (e.sizeCalculationType == sushi::Transform::useAspectRatioOnYKeepMinimum)
		{
			ImGui::DragFloat2("Size pixels", &e.sizePixels[0]);
			ImGui::DragFloat2("Size percentage", &e.sizePercentage[0], 0.01);
			ImGui::DragFloat("Aspect ratio", &e.aspectRation, 0.01, 0);
		}


		e.changeSettings(copy, parent);

	}
	ImGui::EndChildFrame();
}

void SushiViewer::displaySushiTextElementImgui(::sushi::Text &e, glm::vec4 parent, int id)
{
	ImGui::PushID(id);

	ImGui::Text("Text element:");

	ImGui::InputText("text content", &e.text);

	::pika::pikaImgui::ColorEdit4Swatches("Text color: ", &e.color[0]);

	displaySushiTransformImgui(e.transform, parent, id);

	ImGui::PopID();

}

void SushiViewer::displaySushiParentElementImgui(::sushi::SushiParent &e, glm::vec4 parent,
	bool displayChildren)
{
	ImGui::PushID(e.id);

	glm::vec4 color = {0.2f,0.2f,0.2f,1.f};
	if (displayChildren) { color = {0.2,0.3,0.7,0.9}; }

	if (pika::pikaImgui::BeginChildFrameColoured(e.id, color, { 0, 700 }, true))
	{

		ImGui::Text("Parent editor: %s, id: %u", e.name, e.id);

		if (img.elementId != e.id)
		{
			if (pika::pikaImgui::blueButton("Select"))
			{
				img.elementId = e.id;
			}
		}

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

		ImGui::Combo("Layout type", &e.layout.layoutType, "Free\0Horizonta\0Vertical\0");

		displaySushiTransformImgui(e.transform, parent, e.id + 20000);
		ImGui::Separator();
		displaySushiBackgroundImgui(e.background, e.id + 10000);
		ImGui::Separator();

		displaySushiTextElementImgui(e.text, e.outData.absTransform, e.id);

		ImGui::Separator();


		if (displayChildren)
		{

			if (pika::pikaImgui::greenButton("Add parent"))
			{
				sushi::Transform transform;
				transform.anchorPoint = sushi::Transform::center;
				transform.sizePercentage = glm::vec2(0.5f);

				//img.elementId = sushiContext.addParent(e, "New Item", transform,
				//	sushi::Background({0.5,0.2,0.2,1.f}));

				sushiContext.addParent(e, "New Item", transform,
					sushi::Background({0.5,0.2,0.2,1.f}), sushi::Layout{});
			}

			if (!e.parents.empty())
			{
				ImGui::Separator();
				{ ImGui::Text("Children:"); }
				for (auto &i : e.parents)
				{
					displaySushiParentElementImgui(i, e.outData.absTransform, 0);
				}
			}
		};

	}
	ImGui::EndChildFrame();

	ImGui::PopID();

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

void visit(sushi::SushiParent &parent, unsigned int id,
sushi::SushiParent* &selectedParent
	)
{
	if (parent.id == id)
	{
		selectedParent = &parent;
		return;
	}

	for (auto &i : parent.parents)
	{
		visit(i, id, selectedParent);
		if (selectedParent) { break; }
	}

	return;
};

void visitSelect(sushi::SushiParent &parent, unsigned int &id,
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

	for (auto &i : parent.parents)
	{
		visitSelect(i, id, selectedParent, mousePos);
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
		sushiContext.update(renderer, in, font);
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

		if (img.elementId)
		{
			if (ImGui::Button("Load and append from selected"))
			{
				sushi::SushyBinaryFormat data;

				if (requestedInfo.readEntireFileBinary(img.fileSelector.file, data.data))
				{
					if (!sushiContext.load(data, img.elementId))
					{
						requestedInfo.consoleWrite("Couldn't parse file");
					}
				}
				else
				{
					requestedInfo.consoleWrite("Couldn't open file");
				}
			}
		}

		if (ImGui::Button("Save"))
		{

			if (img.fileSelector.file[0] != 0)
			{
				auto rez = sushiContext.save();

				requestedInfo.writeEntireFileBinary(img.fileSelector.file, rez.data.data(),
					rez.data.size() * sizeof(rez.data[0]));
			}

		}

	#pragma region get selected parent
		auto &c = sushiContext;
		if (img.elementId == 0)
		{
			img.elementId = c.root.id;
		}

		if (input.lMouse.held() && !img.dragging)
		{
			visitSelect(sushiContext.root, img.elementId, selectedParent,
				{input.mouseX, input.mouseY});
		}
		else
		{
			visit(sushiContext.root, img.elementId, selectedParent);
		}
	#pragma endregion


		if (selectedParent != 0 && selectedParent != &sushiContext.root
			&& ImGui::Button("Save from current children"))
		{
			if (img.fileSelector.file[0] != 0)
			{
				auto rez = sushiContext.saveFromParent(selectedParent);

				requestedInfo.writeEntireFileBinary(img.fileSelector.file, rez.data.data(),
					rez.data.size() * sizeof(rez.data[0]));
			}
		}


		ImGui::Text("Current selected id: %u", img.elementId);

	
		//ImGui::InputInt("Element: ", &img.elementSelected);
		//
		//size_t elementsSize = c.root.allUiElements.size();
		//img.elementSelected = glm::clamp(img.elementSelected, -1, (int)(elementsSize - 1));
		//

		if (selectedParent)
		{
			parentOfParent = findParentOfParent(sushiContext.root, img.elementId);
			glm::vec4 parentRect = {0, 0, renderer.windowW, renderer.windowH};

			if (parentOfParent)
			{
				parentRect = parentOfParent->outData.absTransform;
				
				if (::pika::pikaImgui::blueButton("Select parent"))
				{
					img.elementId = parentOfParent->id;
				}
			}

			displaySushiParentElementImgui(*selectedParent, parentRect, true);
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

	//render parent gyzmos
	if (selectedParent) 
	{
		if (parentOfParent)
		{
			renderer.renderRectangleOutline(parentOfParent->outData.absTransform,
				{0.5,0.5,0,0.5}, 4.0f);
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
