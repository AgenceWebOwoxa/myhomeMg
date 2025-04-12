#include <windows.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <algorithm> // Pour std::max et std::min

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define IDC_UPDATE_BUTTON 1001
#define IDC_RED_EDIT 1002
#define IDC_GREEN_EDIT 1003
#define IDC_BLUE_EDIT 1004
#define IDC_RADIUS_EDIT 1005

// Framebuffer
static uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
static int sphere_red = 255, sphere_green = 0, sphere_blue = 0;
static int sphere_radius = 50;

// Convertir R,G,B en uint32_t
uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

// Dessiner un pixel
void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        framebuffer[y * SCREEN_WIDTH + x] = color;
    }
}

// Dessiner un cercle (futur sphère)
void draw_circle(int center_x, int center_y, int radius, uint32_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                draw_pixel(center_x + x, center_y + y, color);
            }
        }
    }
}

// Calcul FPS
float calculate_fps() {
    static DWORD last_time = 0;
    DWORD current_time = GetTickCount();
    float fps = 1000.0f / (current_time - last_time);
    last_time = current_time;
    return fps;
}

// Gestion de la fenêtre
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hRedEdit, hGreenEdit, hBlueEdit, hRadiusEdit;

    switch (uMsg) {
    case WM_CREATE: {
        // Créer bouton Update
        CreateWindowA("BUTTON", "Update",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 200, 80, 30, hwnd, (HMENU)IDC_UPDATE_BUTTON, NULL, NULL);

        // Créer champs texte
        CreateWindowA("EDIT", "255",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
            100, 170, 50, 20, hwnd, (HMENU)IDC_RED_EDIT, NULL, NULL);
        CreateWindowA("EDIT", "0",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
            100, 190, 50, 20, hwnd, (HMENU)IDC_GREEN_EDIT, NULL, NULL);
        CreateWindowA("EDIT", "0",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
            100, 210, 50, 20, hwnd, (HMENU)IDC_BLUE_EDIT, NULL, NULL);
        CreateWindowA("EDIT", "50",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
            100, 230, 50, 20, hwnd, (HMENU)IDC_RADIUS_EDIT, NULL, NULL);

        // Stocker handles
        hRedEdit = GetDlgItem(hwnd, IDC_RED_EDIT);
        hGreenEdit = GetDlgItem(hwnd, IDC_GREEN_EDIT);
        hBlueEdit = GetDlgItem(hwnd, IDC_BLUE_EDIT);
        hRadiusEdit = GetDlgItem(hwnd, IDC_RADIUS_EDIT);
        return 0;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == IDC_UPDATE_BUTTON) {
            // Lire les champs
            char buffer[16];
            GetWindowTextA(hRedEdit, buffer, 16);
            sphere_red = atoi(buffer);
            GetWindowTextA(hGreenEdit, buffer, 16);
            sphere_green = atoi(buffer);
            GetWindowTextA(hBlueEdit, buffer, 16);
            sphere_blue = atoi(buffer);
            GetWindowTextA(hRadiusEdit, buffer, 16);
            sphere_radius = atoi(buffer);

            // Limiter les valeurs
            sphere_red = std::max(0, std::min(255, sphere_red));
            sphere_green = std::max(0, std::min(255, sphere_green));
            sphere_blue = std::max(0, std::min(255, sphere_blue));
            sphere_radius = std::max(10, std::min(100, sphere_radius));

            InvalidateRect(hwnd, NULL, TRUE); // Redessiner
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Config bitmap
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = SCREEN_WIDTH;
        bmi.bmiHeader.biHeight = -SCREEN_HEIGHT;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        // Dessiner
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            framebuffer[i] = rgb(0, 0, 0); // Fond noir
        }
        draw_circle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, sphere_radius,
            rgb(sphere_red, sphere_green, sphere_blue));

        // Afficher FPS
        char fps_text[32];
        sprintf(fps_text, "FPS: %.1f", calculate_fps());
        for (int i = 0; fps_text[i]; i++) {
            draw_pixel(10 + i * 8, 10, rgb(255, 255, 255)); // "Police" simpliste
        }

        SetDIBitsToDevice(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, SCREEN_HEIGHT,
            framebuffer, &bmi, DIB_RGB_COLORS);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Enregistrer la classe
    const char CLASS_NAME[] = "Retro3DEngine";
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassA(&wc);

    // Créer la fenêtre
    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "Retro 3D Engine",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);

    // Boucle principale
    MSG msg = { 0 };
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}