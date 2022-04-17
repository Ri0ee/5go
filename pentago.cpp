#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <list>
#include <thread>
#include <array>

#include <GLFW/glfw3.h>

#include "board.h"
#include "minimax.h"
#include "table.h"
#include "stats.h"

struct Action {
    std::string desc;
};

struct State {
    bool showMenuWindow = true;
    bool showGameWindow = false;

    bool ai = false;
    bool thinking = false;
    bool aiFinished = false;
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
    tt::reset();
    stats::stats.clear();

    state.showMenuWindow = true;
    state.showGameWindow = false;

    state.thinking = false;
    state.stale = false;
    state.rotate = false;

    state.side = 0; // 1, 2
    state.currentSide = 0; // 1, 2
    state.winner = 0;

    state.board = {};
    state.history.clear();
}

void switchSide(State& state) {
    state.currentSide = state.currentSide == 1 ? 2 : 1;
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
                tt::init();
                state.side = 1;
                state.currentSide = 1;
                state.showGameWindow = true;
            }

            if (ImGui::Button("Play second")) {
                resetGameState(state);
                tt::init();
                state.side = 2;
                state.currentSide = 1;
                state.showGameWindow = true;
            }
        }
        else {
            if (ImGui::Button("Play")) {
                resetGameState(state);
                state.side = 1;
                state.currentSide = 1;
                state.showGameWindow = true;
            }
        }
    }
    else {
        if (!state.thinking) {
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
    }

    ImGui::End();
}

void postMove(State& state) {
    switchSide(state);
}

void rotateButton(State& state, int qidx, int dir) {
    state.board.rotate(qidx, dir);
    state.rotate = false;
    check(state);
    postMove(state);
}

void aiMove(State& state) {
    state.thinking = true;
    state.aiFinished = false;
    state.board = mm::bestMove(state.board, state.currentSide);
    state.thinking = false;
    state.aiFinished = true;
    switchSide(state);
}

void gameWindow(State& state) {
    ImGui::Begin("Game", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::SetItemAllowOverlap();
    ImGui::BeginGroup();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 50);

    auto board = state.board.toArray();
    int marbleIdx = 1;
    for (auto& row : board) {
        for (auto& cell : row) {
            if (cell == 1) { // blue
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(20, 60, 220, 255));
                ImGui::Button(std::to_string(marbleIdx - 1).c_str(), { 50, 50 });
                ImGui::PopStyleColor();
            }
            else if (cell == 2) { // red
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(220, 20, 60, 255));
                ImGui::Button(std::to_string(marbleIdx - 1).c_str(), { 50, 50 });
                ImGui::PopStyleColor();
            }
            else { // none
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(100, 100, 100, 255));

                if (state.currentSide == 1) { // blue
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(20, 60, 220, 255));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(20, 60, 220, 255));
                }
                else if (state.currentSide == 2) { // red
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(220, 20, 60, 255));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(220, 20, 60, 255));
                }

                if (ImGui::Button(std::to_string(marbleIdx - 1).c_str(), { 50, 50 })) {
                    if (!state.stale && state.winner == 0) {
                        cell = state.currentSide;
                        state.board.fromArray(board);
                        state.history.push_back({ std::format("M{}", marbleIdx) });
                        check(state);
                        state.rotate = true && !state.stale && state.winner == 0;
                    }
                }

                ImGui::PopStyleColor(3);
            }

            if (marbleIdx % 6)
                ImGui::SameLine();

            if (marbleIdx % 6 == 3) {
                ImGui::Dummy({ 20, 0 });
                ImGui::SameLine();
            }

            if (marbleIdx == 18)
                ImGui::Dummy({ 0, 20 });

            marbleIdx++;
        }
    }

    ImGui::PopStyleVar();
    ImGui::EndGroup();

    if (state.rotate) {
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());
        ImGui::BeginChild(1, ImGui::GetItemRectSize(), false, 0);

        ImGui::Dummy({ 0, 66 });
        if (ImGui::ArrowButton("q0left", ImGuiDir_Left)) rotateButton(state, 0, 0);
        ImGui::SameLine();
        ImGui::Dummy({ 111, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q0right", ImGuiDir_Right)) rotateButton(state, 0, 1);

        ImGui::SameLine();
        ImGui::Dummy({ 22, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q1left", ImGuiDir_Left)) rotateButton(state, 1, 0);

        ImGui::SameLine();
        ImGui::Dummy({ 111, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q1right", ImGuiDir_Right)) rotateButton(state, 1, 1);

        ImGui::Dummy({ 0, 158 });
        if (ImGui::ArrowButton("q2left", ImGuiDir_Left)) rotateButton(state, 2, 0);

        ImGui::SameLine();
        ImGui::Dummy({ 111, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q2right", ImGuiDir_Right)) rotateButton(state, 2, 1);

        ImGui::SameLine();
        ImGui::Dummy({ 22, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q3left", ImGuiDir_Left)) rotateButton(state, 3, 0);

        ImGui::SameLine();
        ImGui::Dummy({ 111, 0 });
        ImGui::SameLine();

        if (ImGui::ArrowButton("q3right", ImGuiDir_Right)) rotateButton(state, 3, 1);

        ImGui::EndChild();
    }

    if (state.ai && state.side != state.currentSide && !state.thinking && state.winner == 0) {
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

    if (state.aiFinished) {
        state.aiThread.join();
        state.aiFinished = false;
    }

    ImGui::End();
}

void gameStatsWindow(State& state) {
    ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("static evaluation"); 
    ImGui::SameLine(150); 
    ImGui::Text(std::to_string(bb::eval(state.board.bitboard)).c_str());

    if (tt::valid()) {
        ImGui::Text("deep evaluation"); 
        ImGui::SameLine(150); 
        ImGui::Text(std::to_string(tt::get(state.board.bitboard).score).c_str());
    }

    for (const auto& [stat, value] : stats::stats) {
        ImGui::Text(stat.c_str());
        ImGui::SameLine(150);
        ImGui::Text(std::to_string(value).c_str());
    }

    if (ImGui::BeginListBox("##history", { 200, 450 })) {
        for (auto& action : state.history) {
            ImGui::BulletText(action.desc.c_str());
        }

        switch (state.winner) {
        case 1: ImGui::BulletText("blue has won"); break;
        case 2: ImGui::BulletText("red has won"); break;
        case 3: ImGui::BulletText("draw"); break;
        }

        ImGui::EndListBox();
    }

    ImGui::End();
}

void test() {
    std::array<std::array<int, 6>, 6> board{};
    board[0] = { 0, 0, 0, 0, 0, 0 };
    board[1] = { 0, 0, 1, 0, 0, 0 };
    board[2] = { 0, 0, 1, 0, 0, 0 };
    board[3] = { 0, 0, 1, 0, 0, 0 };
    board[4] = { 0, 0, 1, 0, 0, 0 };
    board[5] = { 0, 0, 1, 0, 0, 0 };

    Board brd;
    brd.fromArray(board);
    std::cout << std::format("board: {:020} winner: {}\n", brd.bitboard, bb::winner(brd.bitboard));

    for (const auto& [staticEval, move] : bb::advances(brd.bitboard)) {
        Board moveBoard(move);
        std::cout << "score: " << staticEval << "\n";
        moveBoard.debugPrint(); std::cout << "\n";
    }

    auto moves = bb::advances(brd.bitboard);
    for (auto rit = moves.rbegin(); rit != moves.rend(); ++rit) {
        auto& [staticEval, move] = *rit;
        Board moveBoard(move);
        std::cout << "score: " << staticEval << "\n";
        moveBoard.debugPrint(); std::cout << "\n";
    }
}

int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 666, "5go", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwSetWindowPos(window, 1, 30);

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

    //test();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (state.showMenuWindow) {
            menuWindow(state);
        }

        if (state.showGameWindow) {
            gameWindow(state);
            gameStatsWindow(state);
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
