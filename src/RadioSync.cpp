//
// Uses the SoftDevice's radio notification: an interrupt before each radio event, which wakes a task running the
// callback (the Bluefruit API is not safe to be used from within an interrupt).
//

#include "RadioSync.hpp"

#include <Arduino.h>
#include <nrf_nvic.h>
#include <nrf_soc.h>


namespace {
    // low interrupt priority, allowed to use the FreeRTOS "FromISR" API (2..7) and not reserved by the SoftDevice (0, 1, 4)
    constexpr uint8_t irq_priority = 6;
    constexpr uint32_t task_stack_size = 1024;   // words, enough for the callback's (debug) output

    TaskHandle_t task = nullptr;
    RadioSync::Callback callback = nullptr;

    void run_callback_on_notification(void*) {
        while (true) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            callback();
        }
    }
}

extern "C" void RADIO_NOTIFICATION_IRQHandler() {
    BaseType_t higher_priority_task_woken = pdFALSE;
    vTaskNotifyGiveFromISR(task, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

namespace RadioSync {
    void begin(const uint8_t distance, const Callback callback_) {
        callback = callback_;
        xTaskCreate(run_callback_on_notification, "radio", task_stack_size, nullptr, TASK_PRIO_HIGH, &task);

        sd_nvic_ClearPendingIRQ(RADIO_NOTIFICATION_IRQn);
        sd_nvic_SetPriority(RADIO_NOTIFICATION_IRQn, irq_priority);
        sd_nvic_EnableIRQ(RADIO_NOTIFICATION_IRQn);
        sd_radio_notification_cfg_set(NRF_RADIO_NOTIFICATION_TYPE_INT_ON_ACTIVE, distance);
    }
}
