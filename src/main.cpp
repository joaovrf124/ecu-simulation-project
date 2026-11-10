#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm> // Para std::min

// Headers do Dear ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Header do GLFW (inclui também o OpenGL do sistema)
#include <GLFW/glfw3.h>

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "Erro GLFW " << error << ": " << description << std::endl;
}

int main(int argc, char* argv[]) {
    // 1. Tratamento de argumentos da linha de comando (Opcional, mas previne avisos)
    if (argc > 1) {
        std::string first = argv[1];
        if (first == "--help" || first == "-h") {
            std::cout << "Simulador de ECU (Interface Grafica)\n";
            std::cout << "Uso: " << argv[0] << "\n";
            return 0;
        }
        std::cout << "[Info] Iniciando simulador grafico. Os cenarios de linha de comando estao desativados nesta versao.\n";
    }

