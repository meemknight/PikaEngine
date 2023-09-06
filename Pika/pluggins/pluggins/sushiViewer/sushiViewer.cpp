#include <pluggins/sushiViewer/sushiViewer.h>
#include <engineLibraresSupport/sushi/engineSushiSupport.h>


//todo user can request imgui ids; shortcut manager context; allocators

ContainerStaticInfo SushiViewer::containerInfo()
{
	ContainerStaticInfo info = {};
	info.defaultHeapMemorySize = pika::MB(10);

	info.requestImguiFbo = true; //todo this should not affect the compatibility of input recording



	return info;
}

bool SushiViewer::create(RequestedContainerInfo &requestedInfo, pika::StaticString<256> commandLineArgument)
{
	renderer.create(requestedInfo.requestedFBO.fbo);

	sushi::SushiUiElement element;
	element.color = Colors_Magenta;
	element.transform.absoluteTransformPixelSize({100,100,200,100});

	sushiContext.allUiElements.push_back(element);

	return true;
}

bool SushiViewer::update(pika::Input input, pika::WindowState windowState, RequestedContainerInfo &requestedInfo)
{
	glClear(GL_COLOR_BUFFER_BIT);
	renderer.updateWindowMetrics(windowState.windowW, windowState.windowH);


	sushiContext.update(renderer, pika::toSushi(input));


	renderer.flush();

	return true;
}

//optional

void SushiViewer::destruct(RequestedContainerInfo &requestedInfo)
{

}
