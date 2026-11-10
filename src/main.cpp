#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "Erro GLFW " << error << ": " << description << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string first = argv[1];
        if (first == "--help" || first == "-h") {
            std::cout << "Simulador de ECU EV (Interface Grafica)\n";
            return 0;
        }
    }

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return -1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1024, 720, "Simulador de ECU EV - Telemetria", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --- Variáveis de Estado da Simulação ---
    bool ts_active = false;
    float pedal_acelerador = 0.0f;
    float torque_aplicado = 0.0f;
    float temp_motor = 25.0f;
    float temp_bateria = 25.0f;
    float tensao_hv = 76.0f;
    
    const int HISTORICO_TAM = 200;
    std::vector<float> historico_torque(HISTORICO_TAM, 0.0f);
    int offset_grafico = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- Dinâmica do Veículo Elétrico ---
        // Regra do Shutdown baseada no SafetyMonitor (80 C)
        bool shutdown_aberto = (temp_motor >= 80.0f || temp_bateria >= 80.0f || tensao_hv < 60.0f);

        if (shutdown_aberto) {
            ts_active = false; 
        }

        if (ts_active) {
            float torque_alvo = (pedal_acelerador / 100.0f) * 240.0f; 
            torque_aplicado += (torque_alvo - torque_aplicado) * 0.15f; 
            
            if (temp_motor < 85.0f) temp_motor += (torque_aplicado / 240.0f) * 0.05f;
            if (temp_bateria < 65.0f) temp_bateria += (torque_aplicado / 240.0f) * 0.02f;
            
            tensao_hv = 76.0f - (torque_aplicado * 0.08f); 
        } else {
            torque_aplicado += (0.0f - torque_aplicado) * 0.2f;
            if (temp_motor > 25.0f) temp_motor -= 0.01f;
            if (temp_bateria > 25.0f) temp_bateria -= 0.005f;
            tensao_hv = 76.0f;
        }

        historico_torque[offset_grafico] = torque_aplicado;
        offset_grafico = (offset_grafico + 1) % HISTORICO_TAM;

        // --- JANELAS IMGUI ---
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(420, 320), ImGuiCond_FirstUseEver);
        ImGui::Begin("Bancada de Controle (Entradas)");

        ImGui::Text("Estado do Sistema:");
        ImGui::Checkbox("Tractive System Active (Drive)", &ts_active);
        
        ImGui::Separator();
        ImGui::Text("Controle de Acelerador:");
        ImGui::SliderFloat("Pedal (%)", &pedal_acelerador, 0.0f, 100.0f, "%.1f %%");
        
        if (ImGui::Button("Teste de Torque Maximo", ImVec2(200, 30))) {
            if (ts_active) torque_aplicado = 240.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Zerar Pedal", ImVec2(120, 30))) {
            pedal_acelerador = 0.0f;
        }

        ImGui::Separator();
        ImGui::Text("Injecao de Falhas (Sensores):");
        ImGui::SliderFloat("Temp. Motor (C)", &temp_motor, 0.0f, 100.0f, "%.1f C");
        ImGui::SliderFloat("Temp. Bateria (C)", &temp_bateria, 0.0f, 100.0f, "%.1f C");
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(460, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(540, 480), ImGuiCond_FirstUseEver);
        ImGui::Begin("Monitor de Telemetria EV");

        ImGui::Text("Torque Aplicado (Inversor):");
        float fracao_torque = torque_aplicado / 240.0f;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
        char tq_buf[32];
        snprintf(tq_buf, sizeof(tq_buf), "%.1f Nm", torque_aplicado);
        ImGui::ProgressBar(fracao_torque, ImVec2(-1.0f, 30.0f), tq_buf);
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Text("Curva de Demanda (Tempo Real):");
        ImGui::PlotLines("##GraficoTorque", historico_torque.data(), HISTORICO_TAM, offset_grafico, 
                         "Historico de Torque", 0.0f, 250.0f, ImVec2(-1, 140));

        ImGui::Separator();
        ImGui::Text("Leituras do Powertrain:");
        
        if (ImGui::BeginTable("TabelaEV", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Circuito de Shutdown");
            ImGui::TableSetColumnIndex(1); 
            if (shutdown_aberto) {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "ABERTO (FALHA)");
            } else {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "FECHADO (OK)");
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Tensao do Acumulador (HV)");
            ImGui::TableSetColumnIndex(1); 
            if (tensao_hv < 60.0f) {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%.1f V (UNDERVOLTAGE)");
            } else {
                ImGui::Text("%.1f V", tensao_hv);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Temperatura do Motor");
            ImGui::TableSetColumnIndex(1); 
            if (temp_motor >= 80.0f) {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%.1f C (FAULT)", temp_motor);
            } else {
                ImGui::Text("%.1f C", temp_motor);
            }
            
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Temperatura da Bateria");
            ImGui::TableSetColumnIndex(1); 
            if (temp_bateria >= 80.0f) {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%.1f C (FAULT)", temp_bateria);
            } else if (temp_bateria >= 60.0f) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%.1f C (WARN)", temp_bateria);
            } else {
                ImGui::Text("%.1f C", temp_bateria);
            }

            ImGui::EndTable();
        }
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.10f, 0.12f, 0.14f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}