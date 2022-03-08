#include <stdio.h>
#include "Src/Window.h"

int main(int argc, char** argv) {

    window_config config = {
            .vsync = true,
            .fullscreen = false,
            .width = 1360,
            .height = 768,
            .title = "Device",
            .resizable = true,
    };

    window_init(&config);
    window_run();
    window_finalize();

    return 0;
}
