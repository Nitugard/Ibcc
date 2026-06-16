#include <stdio.h>
#include "Src/Window.h"

int main(int argc, char** argv) {

    window_config config = {
            .vsync = false,
            .fullscreen = false,
            .width = 1360,
            .height = 768,
            .title = "Graficki prikaz manipulatora",
            .resizable = true,
    };

    window_init(&config);
    window_run();
    window_finalize();

    return 0;
}
