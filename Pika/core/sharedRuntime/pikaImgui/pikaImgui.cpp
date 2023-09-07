#include <glad/glad.h>
#include <pikaImgui/pikaImgui.h>
#include <GLFW/glfw3.h>
#include "IconsForkAwesome.h"
#include <pikaAllocator/freeListAllocator.h>
#include <logs/assert.h>
#include <compilerIntrinsics.h>
#include <stringManipulation/stringManipulation.h>

//todo macro to remove imgui impl

#if !(PIKA_SHOULD_REMOVE_IMGUI)

void *pika::pikaImgui::imguiCustomAlloc(size_t sz, void *user_data)
{
	pika::memory::FreeListAllocator *allocator = (pika::memory::FreeListAllocator *)user_data;
	PIKA_DEVELOPMENT_ONLY_ASSERT(allocator, "no allocator for imgui");

	return allocator->allocate(sz);
}

void pika::pikaImgui::imguiCustomFree(void *ptr, void *user_data)
{
	pika::memory::FreeListAllocator *allocator = (pika::memory::FreeListAllocator *)user_data;
	PIKA_DEVELOPMENT_ONLY_ASSERT(allocator, "no allocator for imgui");

	allocator->free(ptr);
}

void pika::pikaImgui::setImguiAllocator(pika::memory::FreeListAllocator &allocator)
{
	::ImGui::SetAllocatorFunctions(imguiCustomAlloc, imguiCustomFree, &allocator);
}


void pika::pikaImgui::initImgui(PikaContext &pikaContext)
{
	setImguiAllocator(pikaContext.imguiAllocator);

	auto context = ::ImGui::CreateContext();
	//ImGui::StyleColorsDark();
	imguiThemes::embraceTheDarkness();

	ImGuiIO &io = ::ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	ImGuiStyle &style = ::ImGui::GetStyle();
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

	{
		ImVector<ImWchar> ranges;
		ImFontGlyphRangesBuilder builder;
		builder.AddChar(0xf016);//ICON_FK_FILE_O
		builder.AddChar(0xf114);//ICON_FK_FOLDER_O
		builder.BuildRanges(&ranges);

		io.Fonts->AddFontFromFileTTF(PIKA_RESOURCES_PATH "fontawesome-webfont.ttf", 150, 0, ranges.Data);
	}
	io.Fonts->Build();


}

void pika::pikaImgui::setImguiContext(PikaContext pikaContext)
{
	::ImGui::SetCurrentContext(pikaContext.ImGuiContext);
}

void pika::pikaImgui::imguiStartFrame(PikaContext pikaContext)
{
	setImguiContext(pikaContext);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	::ImGui::NewFrame();
	::ImGui::DockSpaceOverViewport(::ImGui::GetMainViewport());
}


void pika::pikaImgui::imguiEndFrame(PikaContext pikaContext)
{
	setImguiContext(pikaContext);
	::ImGui::Render();
	int display_w = 0, display_h = 0;
	glfwGetFramebufferSize(pikaContext.wind, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());

	ImGuiIO &io = ::ImGui::GetIO();

	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		//GLFWwindow *backup_current_context = glfwGetCurrentContext();
		//ImGui::UpdatePlatformWindows();
		//ImGui::RenderPlatformWindowsDefault();
		//glfwMakeContextCurrent(backup_current_context);

		::ImGui::UpdatePlatformWindows();
		::ImGui::RenderPlatformWindowsDefault();
		pikaContext.glfwMakeContextCurrentPtr(pikaContext.wind); //idea create a class with some functions

	}
}

bool pika::pikaImgui::redButton(const char *label, const ImVec2 &size_arg)
{
	::ImGui::PushStyleColor(ImGuiCol_Button, {1,0,0,1});
	auto rez = ::ImGui::Button(label, size_arg);
	::ImGui::PopStyleColor();
	return rez;
}

bool pika::pikaImgui::BeginChildFrameColoured(ImGuiID id,
	glm::vec4 color, const ImVec2 &size, ImGuiWindowFlags extra_flags)
{
	ImGuiContext &g = *::ImGui::GetCurrentContext();
	const ImGuiStyle &style = ::ImGui::GetStyle();
	::ImGui::PushStyleColor(ImGuiCol_ChildBg, {color.x,color.y,color.z,color.w});
	::ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, style.FrameRounding);
	::ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, style.FrameBorderSize);
	::ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);
	bool ret = ::ImGui::BeginChild(id, size, true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysUseWindowPadding | extra_flags);
	::ImGui::PopStyleVar(3);
	::ImGui::PopStyleColor();
	return ret;
}

void pika::pikaImgui::addErrorSymbol()
{
	::ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
	::ImGui::Text(ICON_FK_TIMES_CIRCLE " ");
	::ImGui::PopStyleColor();
}

void pika::pikaImgui::addWarningSymbol()
{
	::ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
	::ImGui::Text(ICON_FK_EXCLAMATION_TRIANGLE " ");
	::ImGui::PopStyleColor();
}

void pika::pikaImgui::helpMarker(const char *desc)
{
	::ImGui::TextDisabled("(?)");
	if (::ImGui::IsItemHovered())
	{
		::ImGui::BeginTooltip();
		::ImGui::PushTextWrapPos(::ImGui::GetFontSize() * 35.0f);
		::ImGui::TextUnformatted(desc);
		::ImGui::PopTextWrapPos();
		::ImGui::EndTooltip();
	}
}

//https://github.com/ocornut/imgui/discussions/3862
void pika::pikaImgui::alignForWidth(float width, float alignment)
{
	ImGuiStyle &style = ::ImGui::GetStyle();
	float avail = ::ImGui::GetContentRegionAvail().x;
	float off = (avail - width) * alignment;
	if (off > 0.0f)
		::ImGui::SetCursorPosX(::ImGui::GetCursorPosX() + off);
}

static int sizesType = 0;

void pika::pikaImgui::displayMemorySizeValue(size_t value)
{

	switch (sizesType)
	{
	case 0:
		ImGui::Text("%" IM_PRIu64 " (bytes)", value);
		break;
	case 1:
		ImGui::Text("%f (KB)", pika::BYTES_TO_KB(value));
		break;
	case 2:
		ImGui::Text("%f (MB)", pika::BYTES_TO_MB(value));
		break;
	case 3:
		ImGui::Text("%f (GB)", pika::BYTES_TO_GB(value));
		break;
	default:
		PIKA_UNREACHABLE();
	}

	//ImGui::Text("%" IM_PRIu64 " (bytes)", value); 

}


void pika::pikaImgui::displayMemorySizeToggle()
{
	ImGui::Combo("Sizes type##pika", &sizesType, "Bytes\0KB\0MB\0GB\0");
}

void pika::pikaImgui::FileSelector::setInfo(std::string title, std::string pwd, std::vector<std::string> typeFilters)
{
	fileBrowser.SetTitle(std::move(title));
	fileBrowser.SetPwd(std::move(pwd));
	
	if (!typeFilters.empty())
	{
		fileBrowser.SetTypeFilters(std::move(typeFilters));
	}
}

bool pika::pikaImgui::FileSelector::run(int id)
{
	ImGui::PushID(id);
	
	bool r = ImGui::InputText(fileBrowser.getTitle().c_str(), this->file, sizeof(file), ImGuiInputTextFlags_EnterReturnsTrue);
	
	ImGui::SameLine();
	if (ImGui::Button("Select file"))
	{
		fileBrowser.Open();
	}
	
	fileBrowser.Display();
	
	if (fileBrowser.HasSelected())
	{
		pika::strlcpy(file, fileBrowser.GetSelected().string(), sizeof(file));
	
		fileBrowser.ClearSelected();
		fileBrowser.Close();

		r = true;
	}
	
	ImGui::PopID();
	
	return r;
}


#else

void pika::pikaImgui::alignForWidth(float width, float alignment) {};
void pika::pikaImgui::helpMarker(const char *desc) {};
void pika::pikaImgui::addWarningSymbol() {};
void pika::pikaImgui::addErrorSymbol() {};
void pika::pikaImgui::imguiEndFrame(PikaContext pikaContext) {};
void pika::pikaImgui::imguiStartFrame(PikaContext pikaContext) {};
void pika::pikaImgui::setImguiContext(PikaContext pikaContext) {};
void pika::pikaImgui::initImgui(PikaContext &pikaContext) {};
void pika::pikaImgui::setImguiAllocator(pika::memory::FreeListAllocator &allocator) {};
bool pika::pikaImgui::FileSelector::run(int id) { return false; };
void pika::pikaImgui::FileSelector::setInfo(std::string title, std::string pwd, std::vector<std::string> typeFilters) {};

#endif

