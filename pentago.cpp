#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <list>
#include <thread>

#include <GLFW/glfw3.h>

#include "board.h"
#include "minimax.h"

struct Action {
    std::string desc;
};

struct State {
    bool showMenuWindow = true;
    bool showGameWindow = false;

    bool ai = false;
    bool thinking = false;
    bool stale = false;
    bool rotate = false;

    int side = 0;
    int currentSide = 0;
    int winner = 0;

    Board board;
    std::list<Action> history;
    std::thread aiThread;
};

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "Glfw Error " << error << ":" << description << '\n';
}

void resetGameState(State& state) {
    state.showMenuWindow = true;
    state.showGameWindow = false;

    state.thinking = false;
    state.stale = false;
    state.rotate = false;

    state.side = 0;
    state.currentSide = 0;
    state.winner = 0;

    state.board = {};
    state.history.clear();
}

void switchSide(State& state) {
    assert(state.currentSide != 0);
    state.currentSide = -state.currentSide;
}

void check(State& state) {
    state.winner = state.board.winner();
    state.stale = state.board.stale();
}

void menuWindow(State& state) {
    ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("The object is to get five marbles in a row before your opponent does\n"
        "Each player has to twist one of the four game blocks 90 degrees, clockwise or counter clockwise, as part of each turn.\n"
        "A player is free to twist any of the game blocks, regardless of which game block the player placed the marble on.\n"
        "A winning row of five marbles can occur vertically, horizontally or diagonally, anywhere on the board and will span two or three game blocks.");

    ImGui::Checkbox("AI", &state.ai);

    if (state.side == 0) {
        if (state.ai) {
            if (ImGui::Button("Play first")) {
                resetGameState(state);
                state.side = -1;
                state.currentSide = -1;
                state.showGameWindow = true;
            }

            if (ImGui::Button("Play second")) {
                resetGameState(state);
                state.side = 1;
                state.currentSide = -1;
                state.showGameWindow = true;
            }
        }
        else {
            if (ImGui::Button("Play")) {
                resetGameState(state);
                state.side = -1;
                state.currentSide = -1;
                state.showGameWindow = true;
            }
        }
    }
    else {
        if (state.winner == 0 || state.stale) {
            if (ImGui::Button("Concede")) {
                resetGameState(state);
            }
        }
        else {
            if (ImGui::Button("Reset")) {
                resetGameState(state);
            }
        }
    }

    ImGui::End();
}

void postMove(State& state) {
    switchSide(state);
}

void rotateButton(State& state, int rotation) {
    state.board.rotate(rotation);
    check(state);
    state.rotate = false;

    switch (rotation) {
    case 0: state.history.push_back({ "Q1 C C W" }); break;
    case 1: state.history.push_back({ "Q1 C W" }); break;
    case 2: state.history.push_back({ "Q2 C C W" }); break;
    case 3: state.history.push_back({ "Q2 C W" }); break;
    case 4: state.history.push_back({ "Q3 C C W" }); break;
    case 5: state.history.push_back({ "Q3 C W" }); break;
    case 6: state.history.push_back({ "Q4 C C W" }); break;
    case 7: state.history.push_back({ "Q5 C W" }); break;
    default: state.history.push_back({ "Unknown rotation, should not happen" });
    }
    
    postMove(state);
}

void aiMove(State& state) {
    std::cout << "aiMove\n";
    auto moves = state.board.getMoves();
    while (!moves.empty()) {
        auto move = moves.top();
        moves.pop();
        std::cout << std::format("pos: {} rotation: {} score: {}\n", move.second.first, move.second.second, move.first);
    }
    std::cout << "/aiMove\n";

    state.thinking = true;
    auto bestMove = mm::bestMove(state.board, 5);
    state.thinking = false;

    state.board.put(bestMove.first);
    state.history.push_back({ std::format("M{}", bestMove.first) });
    check(state);
    state.rotate = !state.stale && state.winner == 0;
    if (state.rotate)
        rotateButton(state, bestMove.second);
}

void gameWindow(State& state) {
    ImGui::Begin("Game", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::SetItemAllowOverlap();
    ImGui::BeginGroup();

    for (uint8_t i = 1; i <= 36; i++) {
        auto owner = state.board.get(36 - i);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 50);

        if (owner == -1) { // blue
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(20, 60, 220, 255));

            ImGui::Button(std::to_string(37 - i).c_str(), { 50, 50 });

            ImGui::PopStyleColor();
        }
        else if (owner == 1) { // red
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(220, 20, 60, 255));

            ImGui::Button(std::to_string(37 - i).c_str(), { 50, 50 });

            ImGui::PopStyleColor();
        }
        else { // none
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(100, 100, 100, 255));

            assert(state.currentSide != 0);

            if (state.currentSide == -1) { // blue
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(20, 60, 220, 255));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(20, 60, 220, 255));
            }
            else if (state.currentSide == 1) { // red
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(220, 20, 60, 255));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(220, 20, 60, 255));
            }

            if (ImGui::Button(std::to_string(37 - i).c_str(), { 50, 50 })) {
                if (!state.stale && state.winner == 0) {
                    state.board.put(36 - i);
                    state.history.push_back({ std::format("M{}", 37 - i) });
                    check(state);
                    state.rotate = true && !state.stale && state.winner == 0;
                }
            }

            ImGui::PopStyleColor(3);
        }

        ImGui::PopStyleVar();

        if (i % 6)
            ImGui::SameLine();

        if (i % 6 == 3) {
            ImGui::Dummy({ 20, 0 });
            ImGui::SameLine();
        }

        if (i == 18)
            ImGui::Dummy({ 0, 20 });
    }

    ImGui::EndGroup();

    if (state.rotate) {
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());
        ImGui::BeginChild(1, ImGui::GetItemRectSize(), false, 0);

        ImGui::Dummy({ 0, 66 });
        if (ImGui::ArrowButton("q0left", ImGuiDir_Left)) {
            rotateButton(state, 0);
        }

        ImGui::SameLine();
        ImGui::Dummy({ 111, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q0right", ImGuiDir_Right)) rotateButton(state, 1);

        ImGui::SameLine();
        ImGui::Dummy({ 22, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q1left", ImGuiDir_Left)) rotateButton(state, 2);

        ImGui::SameLine();
        ImGui::Dummy({ 111, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q1right", ImGuiDir_Right)) rotateButton(state, 3);

        ImGui::Dummy({ 0, 158 });
        if (ImGui::ArrowButton("q2left", ImGuiDir_Left)) rotateButton(state, 4);

        ImGui::SameLine();
        ImGui::Dummy({ 111, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q2right", ImGuiDir_Right)) rotateButton(state, 5);

        ImGui::SameLine();
        ImGui::Dummy({ 22, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q3left", ImGuiDir_Left)) rotateButton(state, 6);

        ImGui::SameLine();
        ImGui::Dummy({ 111, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q3right", ImGuiDir_Right)) rotateButton(state, 7);

        ImGui::EndChild();
    }

    if (state.ai && state.side != state.currentSide && !state.thinking) {
        state.aiThread = std::thread([&]() {
            aiMove(state);
        });
    }

    if (state.thinking) {
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());
        ImGui::BeginChild(1, ImGui::GetItemRectSize(), false, 0);
        ImGui::EndChild();
    }
    else if (state.aiThread.joinable()) {
        state.aiThread.join();
    }

    ImGui::End();
}

void gameStatsWindow(State& state) {
    ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::BeginListBox("##history", { 200, 450 });

    for (auto& action : state.history) {
        ImGui::BulletText(action.desc.c_str());
    }

    if (state.winner != 0) {
        std::string text = std::format("{} has won", state.winner == -1 ? "blue" : "red");
        ImGui::BulletText(text.c_str());
    }

    ImGui::EndListBox();

    ImGui::End();
}

int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "5go", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    static State state;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (state.showMenuWindow) {
            menuWindow(state);
        }

        if (state.showGameWindow) {
            gameStatsWindow(state);
            gameWindow(state);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
