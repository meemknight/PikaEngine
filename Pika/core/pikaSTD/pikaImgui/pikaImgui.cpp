#include <glad/glad.h>
#include <pikaImgui/pikaImgui.h>
#include <GLFW/glfw3.h>

void pika::initImgui(PikaContext &pikaContext)
{
	auto context = ImGui::CreateContext();
	//ImGui::StyleColorsDark();
	imguiThemes::embraceTheDarkness();
	
	ImGuiIO &io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	
	ImGuiStyle &style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		//style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 0.f;
		style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
	}
	
	ImGui_ImplGlfw_InitForOpenGL(pikaContext.wind, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	pikaContext.ImGuiContext = context;
}

void pika::setImguiContext(PikaContext pikaContext)
{
	ImGui::SetCurrentContext(pikaContext.ImGuiContext);
}

void pika::imguiStartFrame(PikaContext pikaContext)
{
	setImguiContext(pikaContext);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}


void pika::imguiEndFrame(PikaContext pikaContext)
{
	setImguiContext(pikaContext);
	ImGui::Render();
	int display_w = 0, display_h = 0;
	glfwGetFramebufferSize(pikaContext.wind, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO &io = ImGui::GetIO();

	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		//GLFWwindow *backup_current_context = glfwGetCurrentContext();
		//ImGui::UpdatePlatformWindows();
		//ImGui::RenderPlatformWindowsDefault();
		//glfwMakeContextCurrent(backup_current_context);

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		pikaContext.glfwMakeContextCurrentPtr(pikaContext.wind); //idea create a class with some functions
	
	}
}
