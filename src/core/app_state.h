#pragma once

#include <string>
#include <vector>

struct AppState {
    int selected_row = -1;
    int selected_pair_index = 0;
    int last_selected_row = -1;
    std::string selected_pair;
    std::vector<std::string> pairs;
    std::vector<size_t> filtered_indices;
};
