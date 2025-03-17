#pragma once
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
public:
    lgfx::Panel_RM67162 panel_instance;  // RM67162 panel driver
    lgfx::Bus_Parallel16 bus_instance;   // QSPI requires Parallel Mode

    LGFX(void) {
        {
            auto cfg = bus_instance.config();

            // **QSPI Configuration (Parallel Mode)**
            cfg.freq_write = 80000000;   // 80MHz max write speed
            cfg.pin_wr = 47;             // WR (Write Clock)
            cfg.pin_rd = -1;             // No Read pin
            cfg.pin_rs = 0;              // RS (Command/Data Select)
            cfg.pin_d0 = 9;              // Data line 0
            cfg.pin_d1 = 46;
            cfg.pin_d2 = 3;
            cfg.pin_d3 = 8;
            cfg.pin_d4 = 18;
            cfg.pin_d5 = 17;
            cfg.pin_d6 = 16;
            cfg.pin_d7 = 15;

            bus_instance.config(cfg);
            panel_instance.setBus(&bus_instance);
        }

        {
            auto cfg = panel_instance.config();
            cfg.pin_cs = -1;   // No CS (Chip Select) for QSPI
            cfg.pin_rst = 4;   // Reset pin
            cfg.pin_busy = -1;
            cfg.memory_width = 536;
            cfg.memory_height = 240;
            cfg.panel_width = 536;
            cfg.panel_height = 240;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 2;
            panel_instance.config(cfg);
        }
        setPanel(&panel_instance);
    }
};
