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

    // 2. Inicializar o GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return -1;
    }

    // Configurar versão do OpenGL (3.3 Core)
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // 3. Criar a janela gráfica do simulador
    GLFWwindow* window = glfwCreateWindow(1024, 720, "Simulador de ECU - Painel de Telemetria (PDS2 / UFMG)", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Ativa V-Sync (60 FPS cravados)

    // 4. Inicializar o contexto do Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Tema visual escuro profissional
    ImGui::StyleColorsDark();

    // Inicializar backends (ligação ImGui <-> GLFW / OpenGL3)
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --- Variáveis de Estado da Simulação ---
    bool motor_ligado = false;
    float tps_acelerador = 0.0f;       // Posição do acelerador (0 a 100%)
    float rpm_atual = 0.0f;            // Rotações por minuto
    float temp_arrefecimento = 25.0f;  // Temperatura da água (°C)
    float pressao_map = 101.3f;        // Pressão no coletor (kPa)
    float tempo_injecao_ms = 0.0f;     // Tempo de abertura do bico injetor (ms)
    
    // Buffer circular para o gráfico de RPM em tempo real
    const int HISTORICO_TAM = 200;
    std::vector<float> historico_rpm(HISTORICO_TAM, 0.0f);
    int offset_grafico = 0;

    // 5. Loop Principal (60 FPS)
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Iniciar novo frame do ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- Dinâmica simples de teste ---
        if (motor_ligado) {
            float rpm_alvo = 850.0f + (tps_acelerador / 100.0f) * 6150.0f; // Marcha lenta 850, Corte 7000
            rpm_atual += (rpm_alvo - rpm_atual) * 0.08f; // Inércia do motor
            
            // Aquecimento gradual do motor até 92°C
            if (temp_arrefecimento < 92.0f) {
                temp_arrefecimento += 0.02f + (rpm_atual / 7000.0f) * 0.05f;
            }
            pressao_map = 35.0f + (tps_acelerador / 100.0f) * 66.3f;
            tempo_injecao_ms = 2.2f + (tps_acelerador / 100.0f) * 12.5f;
        } else {
            rpm_atual += (0.0f - rpm_atual) * 0.1f;
            if (rpm_atual < 10.0f) rpm_atual = 0.0f;
            if (temp_arrefecimento > 25.0f) temp_arrefecimento -= 0.01f;
            pressao_map = 101.3f;
            tempo_injecao_ms = 0.0f;
        }

        // Guardar valor no histórico do gráfico
        historico_rpm[offset_grafico] = rpm_atual;
        offset_grafico = (offset_grafico + 1) % HISTORICO_TAM;

        // ====================================================================
        // DESENHO DA INTERFACE GRÁFICA (JANELAS IMGUI)
        // ====================================================================
        
        // Janela 1: Controlo de Bancada (Simulação de Entradas/Sensores)
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(420, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Bancada de Controlo (Entradas da ECU)");

        ImGui::Text("Estado do Sistema:");
        ImGui::Checkbox("Chave de Ignicao / Motor Ligado", &motor_ligado);
        
        ImGui::Separator();
        ImGui::Text("Controlo de Aceleracao (TPS):");
        ImGui::SliderFloat("Pedal (%)", &tps_acelerador, 0.0f, 100.0f, "%.1f %%");
        
        if (ImGui::Button("Golpe de Acelerador (Blip)", ImVec2(200, 30))) {
            if (motor_ligado) rpm_atual = std::min(7000.0f, rpm_atual + 1800.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Zerar Pedal", ImVec2(120, 30))) {
            tps_acelerador = 0.0f;
        }

        ImGui::Separator();
        ImGui::Text("Ajuste Manual de Sensores:");
        ImGui::SliderFloat("Temp. Agua (ECT)", &temp_arrefecimento, 0.0f, 130.0f, "%.1f C");

        ImGui::End();

        // Janela 2: Telemetria em Tempo Real (Saídas e Gráficos)
        ImGui::SetNextWindowPos(ImVec2(460, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(540, 480), ImGuiCond_FirstUseEver);
        ImGui::Begin("Monitor de Telemetria da ECU");

        // Barra de Progresso de RPM com alerta de Redline
        ImGui::Text("Rotacao do Motor (RPM):");
        float fracao_rpm = rpm_atual / 7000.0f;
        if (rpm_atual > 6200.0f) {
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.9f, 0.1f, 0.1f, 1.0f)); // Vermelho no corte
        } else {
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.7f, 0.3f, 1.0f)); // Verde normal
        }
        char rpm_buf[32];
        snprintf(rpm_buf, sizeof(rpm_buf), "%.0f RPM", rpm_atual);
        ImGui::ProgressBar(fracao_rpm, ImVec2(-1.0f, 30.0f), rpm_buf);
        ImGui::PopStyleColor();

        ImGui::Spacing();

        // Gráfico Osciloscópio de RPM
        ImGui::Text("Curva de Rotacao (Tempo Real):");
        ImGui::PlotLines("##GraficoRPM", historico_rpm.data(), HISTORICO_TAM, offset_grafico, 
                         "Historico de RPM", 0.0f, 7500.0f, ImVec2(-1, 140));

        ImGui::Separator();
        ImGui::Text("Leituras e Atuadores da Injetora:");
        
        if (ImGui::BeginTable("TabelaSensores", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Pressao do Coletor (MAP)");
            ImGui::TableSetColumnIndex(1); ImGui::Text("%.1f kPa", pressao_map);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Temperatura do Motor (ECT)");
            ImGui::TableSetColumnIndex(1); 
            if (temp_arrefecimento > 105.0f) {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%.1f C (SOBREAQUECIMENTO!)", temp_arrefecimento);
            } else {
                ImGui::Text("%.1f C", temp_arrefecimento);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Tempo de Injecao (Pulso PWM)");
            ImGui::TableSetColumnIndex(1); ImGui::Text("%.2f ms", tempo_injecao_ms);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Taxa de Atualizacao (UI)");
            ImGui::TableSetColumnIndex(1); ImGui::Text("%.1f FPS", io.Framerate);

            ImGui::EndTable();
        }

        ImGui::End();

        // 6. Renderizar tudo no ecrã via OpenGL
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.10f, 0.12f, 0.14f, 1.00f); // Fundo cinza-azulado escuro
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 7. Limpeza de memória ao fechar a janela
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}