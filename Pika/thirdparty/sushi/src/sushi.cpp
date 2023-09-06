#include <sushi/sushi.h>

namespace sushi
{


	void sushi::SushyContext::update(gl2d::Renderer2D &renderer,
		sushi::SushiInput &input)
	{

		if (renderer.windowH == 0 || renderer.windowW == 0)
		{
			return;
		}

		glm::vec4 drawRegion = {0, 0, renderer.windowW, renderer.windowH};

		renderer.pushCamera();
		{
			//backgrouund
			renderer.renderRectangle(drawRegion, background);


			auto uiElSize = allUiElements.size();
			for (int i = 0; i < uiElSize; i++)
			{
				glm::vec4 rectRez = allUiElements[i].transform.applyTransform(drawRegion);

				renderer.renderRectangle(rectRez, allUiElements[i].color);

			}

		}
		renderer.popCamera();
	}




};

