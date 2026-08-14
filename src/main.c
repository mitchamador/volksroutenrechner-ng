#include "core.h"
#include "ui_16x2_legacy.h"

// Entry point. Everything that used to live in this file's giant main()
// has moved out:
//   - core.c            HW/EEPROM/power/sensor logic (core_init/core_tick
//                        and everything they call)
//   - ui_16x2_legacy.c   16x2 menu navigation, screens, editors
//                        (ui_16x2_legacy_update() and everything it calls)
//
// main() itself is now just: start up, then repeatedly tick the core and
// update the UI -- the Arduino setup()/loop() shape.

void main(void) {
    core_init();

#ifdef SERVICE_COUNTERS_CHECKS_SUPPORT
    if (config.settings.service_alarm) {
        uint8_t warn = check_service_counters();
        print_warning_service_counters(warn);
    }
#endif

    // wait for the first ADC conversion before doing anything else
    while (adc_voltage.current == 0 && screen_refresh == 0)
        ;

    // force an initial fill_live_data()/fill_misc_values() pass
    screen_refresh = 1;

    for (;;) {
        core_tick();
        ui_16x2_legacy_update();
    }
}
