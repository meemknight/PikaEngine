#include <glad/glad.h>
#include <pikaImgui/pikaImgui.h>
#include <GLFW/glfw3.h>
#include "IconsForkAwesome.h"
#include <pikaAllocator/freeListAllocator.h>
#include <logs/assert.h>

void *pika::imguiCustomAlloc(size_t sz, void *user_data)
{
	pika::memory::FreeListAllocator* allocator = (pika::memory::FreeListAllocator *)user_data;
	PIKA_DEVELOPMENT_ONLY_ASSERT(allocator, "no allocator for imgui");

	return allocator->allocate(sz);
}

void pika::imguiCustomFree(void *ptr, void *user_data)
{
	pika::memory::FreeListAllocator *allocator = (pika::memory::FreeListAllocator *)user_data;
	PIKA_DEVELOPMENT_ONLY_ASSERT(allocator, "no allocator for imgui");
	
	allocator->free(ptr);
}

void pika::setImguiAllocator(pika::memory::FreeListAllocator &allocator)
{
	ImGui::SetAllocatorFunctions(imguiCustomAlloc, imguiCustomFree, &allocator);
}


void pika::initImgui(PikaContext &pikaContext)
{
	setImguiAllocator(pikaContext.imguiAllocator);

	auto context = ImGui::CreateContext();
	//ImGui::StyleColorsDark();
	imguiThemes::embraceTheDarkness();
	
	ImGuiIO &io = ImGui::GetIO();
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


	//https://pixtur.github.io/mkdocs-for-imgui/site/FONTS/
	//https://github.com/juliettef/IconFontCppHeaders
	//https://fontawesome.com/v4/icons/
	io.Fonts->AddFontFromFileTTF(PIKA_RESOURCES_PATH "arial.ttf", 16);

	//ImVector<ImWchar> ranges;
	//ImFontGlyphRangesBuilder builder;
	//builder.AddText("Hello world");                        // Add a string (here "Hello world" contains 7 unique characters)
	//builder.AddChar(0x7262);                               // Add a specific character
	//builder.AddRanges(io.Fonts->GetGlyphRangesJapanese()); // Add one of the default ranges
	//builder.BuildRanges(&ranges);


	ImFontConfig config;
	config.MergeMode = true;
	config.GlyphMinAdvanceX = 16.0f; // Use if you want to make the icon monospaced
	static const ImWchar icon_ranges[] = {ICON_MIN_FK, ICON_MAX_FK, 0};
	io.Fonts->AddFontFromFileTTF(PIKA_RESOURCES_PATH "fontawesome-webfont.ttf", 16.0f, &config, icon_ranges);
	io.Fonts->Build();


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

void pika::addErrorSymbol()
{
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
	ImGui::Text(ICON_FK_TIMES_CIRCLE " ");
	ImGui::PopStyleColor();
}

void pika::addWarningSymbol()
{
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
	ImGui::Text(ICON_FK_EXCLAMATION_TRIANGLE " ");
	ImGui::PopStyleColor();
}

void pika::helpMarker(const char *desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}
