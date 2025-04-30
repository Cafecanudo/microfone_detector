#include <atomic>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <portaudio.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>
#include <unistd.h>
#include <iostream>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512
#define THRESHOLD 0.01 // Ajuste este valor conforme necessário
#define BORDER_THICKNESS 10 // Espessura da borda verde em pixels
#define BORDER_TRANSPARENCY 0.5 // Transparencia
#define BORDER_COLOR 0x00FF00 //Cor da borda

typedef struct {
    int frameIndex;
    int maxFrameIndex;
    float *recordedSamples;
    bool speechDetected;
    float lastSpeechTime;
} paTestData;

std::atomic g_border_showing(false);
std::atomic<Display *> g_display(nullptr);
std::atomic<Window> g_window(NULL);

static int audioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    auto *data = static_cast<paTestData *>(userData);
    const auto *in = static_cast<const float *>(inputBuffer);
    (void) outputBuffer;

    float rms = 0.0f;

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        rms += in[i] * in[i];
    }

    rms = std::sqrt(rms / framesPerBuffer);

    float displayVolume = rms * 10.0f;
    if (displayVolume > 1.0f) displayVolume = 1.0f;

    if (rms > THRESHOLD) {
        if (!data->speechDetected) {
            XMapWindow(g_display.load(), g_window.load());

            // printf("\nFala detectada! (Nível: %.6f - %d%%)\n", rms, (int) (displayVolume * 100));
            data->speechDetected = true;
            data->lastSpeechTime = timeInfo->currentTime;
        }
    } else {
        if (data->speechDetected && timeInfo->currentTime - data->lastSpeechTime > 0.3) {
            data->speechDetected = false;
            XUnmapWindow(g_display.load(), g_window.load());
        }
    }
    XFlush(g_display.load());
    return paContinue;
}

int main() {
    PaStreamParameters inputParameters;
    PaStream *stream;
    PaError err = paNoError;
    paTestData data;

    data.frameIndex = 0;
    data.maxFrameIndex = SAMPLE_RATE * 5;
    data.speechDetected = false;
    data.lastSpeechTime = 0;

    err = Pa_Initialize();
    if (err != paNoError) {
        printf("Erro ao inicializar PortAudio: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        printf("Erro: Nenhum dispositivo de entrada encontrado.\n");
        return 1;
    }

    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    Display *display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Não foi possível conectar ao X server" << std::endl;
        return 1;
    }

    int event_base, error_base;
    if (!XFixesQueryExtension(display, &event_base, &error_base)) {
        std::cerr << "Extensão XFixes não disponível" << std::endl;
        XCloseDisplay(display);
        return 1;
    }

    const Window root = DefaultRootWindow(display);
    const int screen = DefaultScreen(display);
    const int screen_width = DisplayWidth(display, screen);
    const int screen_height = DisplayHeight(display, screen);

    constexpr short int border_width = BORDER_THICKNESS;
    constexpr unsigned long border_color = BORDER_COLOR;
    constexpr float transparency = BORDER_TRANSPARENCY;

    XSetWindowAttributes attr;
    attr.override_redirect = True;
    attr.background_pixel = border_color;

    Window window = XCreateWindow(
        display, root,
        0, 0, screen_width, screen_height,
        0,
        CopyFromParent, InputOutput,
        CopyFromParent, CWOverrideRedirect | CWBackPixel,
        &attr
    );

    const Atom opacity_atom = XInternAtom(display, "_NET_WM_WINDOW_OPACITY", False);
    auto opacity = static_cast<unsigned long>(0xFFFFFFFF * transparency);
    XChangeProperty(display, window, opacity_atom, XA_CARDINAL, 32,
                    PropModeReplace, reinterpret_cast<unsigned char *>(&opacity), 1);

    XSelectInput(display, window, 0);

    XserverRegion region = XFixesCreateRegion(display, NULL, 0);
    XFixesSetWindowShapeRegion(display, window, ShapeInput, 0, 0, region);
    XFixesDestroyRegion(display, region);

    XRectangle rect_outer = {0, 0, static_cast<unsigned short>(screen_width), static_cast<unsigned short>(screen_height)};
    XRectangle rect_inner = {
        border_width, border_width,
        static_cast<unsigned short>(screen_width - 2 * border_width),
        static_cast<unsigned short>(screen_height - 2 * border_width)
    };

    Region region_outer = XCreateRegion();
    Region region_inner = XCreateRegion();
    Region region_border = XCreateRegion();

    XUnionRectWithRegion(&rect_outer, region_outer, region_outer);
    XUnionRectWithRegion(&rect_inner, region_inner, region_inner);
    XSubtractRegion(region_outer, region_inner, region_border);

    XShapeCombineRegion(display, window, ShapeBounding, 0, 0, region_border, ShapeSet);

    XDestroyRegion(region_outer);
    XDestroyRegion(region_inner);
    XDestroyRegion(region_border);

    XMapWindow(display, window);

    XRaiseWindow(display, window);

    g_display.store(display);
    g_window.store(window);

    err = Pa_OpenStream(&stream,
                        &inputParameters,
                        NULL,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff,
                        audioCallback,
                        &data);

    if (err != paNoError) {
        printf("Erro ao abrir stream: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        printf("Erro ao iniciar stream: %s\n", Pa_GetErrorText(err));
        return 1;
    }
    getchar();

    err = Pa_StopStream(stream);
    if (err != paNoError) {
        printf("Erro ao parar stream: %s\n", Pa_GetErrorText(err));
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        printf("Erro ao fechar stream: %s\n", Pa_GetErrorText(err));
    }

    Pa_Terminate();
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
