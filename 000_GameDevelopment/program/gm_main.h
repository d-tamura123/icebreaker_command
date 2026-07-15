#pragma once

void gameStart();
void gameMain( float delta_time );
void gameEnd();


/*
float col[3] = { 0 };
float slider = 0;

//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

    DrawCircle(300, 300, 200, dxe::Colors::Red, true);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    //  デモウィンドウの描画
    //ImGui::ShowDemoWindow();

    //  新しいウィンドウの作成
    ImGui::SetWindowSize({ 200, 300 });
    ImGui::StyleColorsClassic();
    ImGui::Begin("TestWindow");
    {
        //  テキストの表示
        ImGui::Text("Hello, world!");

        if (ImGui::Button("TestButton"))
        {
            //  ボタンを押されたときの処理
        }
        if (ImGui::ColorPicker3("test", col)) {
        }
        ImGui::SliderFloat("label", &slider, 0, 100);
    }
    ImGui::End();


    //  ImGuiの描画処理
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


*/