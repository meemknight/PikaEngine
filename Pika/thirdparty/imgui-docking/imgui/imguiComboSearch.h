#include <cstdint>
#include <cctype>
#include <utility>
#include <vector>
#include <string>
#include <algorithm>

//https://github.com/ocornut/imgui/issues/1658#issuecomment-886171438

namespace ImGui
{



    bool ComboWithFilter(const char *label, int *current_item, const std::vector<std::string> &items);

    bool ComboWithFilter(const char *label, int *current_item, const std::vector<char *> &items);

    bool ComboWithFilter(const char *label, int *current_item, std::vector<const char *> &items);

    void ListWithFilter(const char *label, int *current_item,
        char *filter, size_t filterSize,
        std::vector<std::string> &items, ImVec2 size = {0,0});


}
