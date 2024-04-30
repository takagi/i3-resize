#include <cassert>
#include <getopt.h>
#include <iostream>

#include "i3_ipc.hpp"
#include "i3_containers.hpp"

#define IS_ONE_OF(expr, ...) [](auto&& expect, auto&&... value) { return ((expect == value) || ...); }((expr), __VA_ARGS__)

using node_layout = i3_containers::node_layout;

void print_help() {
    std::cout <<
        "Usage: i3-resize OPTION\n"
        "Resize the focused i3wm window horizontally or vertically.\n"
        "\n"
        "Options:\n"
        "  -h, --horizontal      horizontally resize the focused window\n"
        "  -v, --vertical        vertically resize the focused window\n";
    exit(1);
}

std::optional<float> get_percent_of_focused_node(i3_containers::node_layout layout, const i3_containers::node& a_root, std::optional<float> percent = std::nullopt) {
    assert(IS_ONE_OF(layout, node_layout::splith, node_layout::splitv));

    if (a_root.is_focused) {
        return percent;
    }

    if (a_root.focus.empty()) {
        return std::nullopt;
    }

    std::uint64_t ID_of_focused_child = a_root.focus.front();
    for (const auto& node : a_root.nodes) {
        if (node.id == ID_of_focused_child) {
            if (a_root.layout == layout && a_root.nodes.size() > 1) {
                percent = node.percent;
            }
            return get_percent_of_focused_node(layout, node, percent);
        }
    }

    return std::nullopt;
}

void _resize_window(node_layout layout, std::string msg, float threshold, std::string widen_command, std::string narrow_command) {
    i3_ipc i3;

    i3_containers::node tree = i3.get_tree();

    std::optional<float> percent = get_percent_of_focused_node(layout, tree);

    std::cout << msg << "percent of the focused window: "
              << (percent.has_value() ? std::to_string(*percent) : "Unknown to i3")
              << std::endl;
    if (!percent.has_value()) {
        return;
    }

    if (*percent < threshold) {
        i3.execute_commands(widen_command);
    } else {
        i3.execute_commands(narrow_command);
    }
}

void resize_window_horizontally() {
    _resize_window(node_layout::splith, "Horizontally", 0.67, "resize set width 67ppt", "resize set width 50ppt");
}

void resize_window_vertically() {
    _resize_window(node_layout::splitv, "Vertically", 0.80, "resize set height 80ppt", "resize set height 50ppt");
}

int main(int argc, char *argv[]) {
    const char* shortopts = "hv";
    const option longopts[] = {
        {"horizontal", no_argument, nullptr, 'h'},
        {"vertical", no_argument, nullptr, 'v'},
        {nullptr, no_argument, nullptr, 0},
    };

    const auto opt = getopt_long(argc, argv, shortopts, longopts, nullptr);
    switch (opt) {
        case 'h':
            resize_window_horizontally();
            break;
        case 'v':
            resize_window_vertically();
            break;
        default:
            print_help();
            break;
    }

    return 0;
}