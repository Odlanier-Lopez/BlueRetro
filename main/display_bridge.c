/*
 * display_bridge.c
 *
 * Archivo NUEVO, no reemplaza ni modifica nada existente de BlueRetro.
 * Corre en el nucleo 2 (el que BlueRetro deja libre a proposito) y hace
 * una sola cosa: cada 500ms revisa el GameID que BlueRetro ya recibio de
 * Swiss (via gid_get(), que ya viene incluido en el proyecto), lo
 * convierte de hexadecimal a texto legible, y si cambio lo manda por
 * UART a un segundo ESP32 (el que maneja la pantalla).
 *
 * Formato enviado por UART: el ID en texto plano + salto de linea.
 * Ejemplo: "GM4E01\n"
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "adapter/gameid.h"

/* ------------------------------------------------------------------ */
/* AJUSTA ESTOS 3 VALORES SEGUN TU HARDWARE:                          */
/* - Elige un GPIO que NO este usado por BlueRetro. Los que BlueRetro */
/*   ya usa para hablar con la consola (GameCube) son: 19, 5, 26, 27. */
/*   No uses esos. GPIO1/GPIO3 tampoco (son el UART de programacion). */
/* ------------------------------------------------------------------ */
#define DISPLAY_UART_PORT   UART_NUM_1
#define DISPLAY_UART_TX_PIN 25   /* GPIO25 - confirmado libre en el esquema del usuario */
#define DISPLAY_UART_BAUD    115200

static char last_sent_id[16] = {0};

/* Convierte "474D344530310000" (hex) de vuelta a "GM4E01" (texto real).
   Se detiene en el primer byte no imprimible (los ceros de relleno). */
static void hex_gameid_to_text(const char *hex, char *out, size_t out_len) {
    size_t out_idx = 0;
    size_t hex_len = strlen(hex);

    for (size_t i = 0; i + 1 < hex_len && out_idx < out_len - 1; i += 2) {
        char byte_str[3] = {hex[i], hex[i + 1], 0};
        uint8_t byte_val = (uint8_t)strtol(byte_str, NULL, 16);

        if (byte_val < 0x20 || byte_val > 0x7E) {
            /* byte no imprimible (relleno con ceros u otro dato) -> cortar aqui */
            break;
        }
        out[out_idx++] = (char)byte_val;
    }
    out[out_idx] = '\0';
}

static void display_bridge_task(void *arg) {
    char text_id[16];

    while (1) {
        const char *hex_id = gid_get();

        if (hex_id[0] != '\0') {
            hex_gameid_to_text(hex_id, text_id, sizeof(text_id));

            if (text_id[0] != '\0' && strcmp(text_id, last_sent_id) != 0) {
                strncpy(last_sent_id, text_id, sizeof(last_sent_id) - 1);

                char line[24];
                int len = snprintf(line, sizeof(line), "%s\n", text_id);
                uart_write_bytes(DISPLAY_UART_PORT, line, len);

                printf("[display_bridge] GameID enviado: %s\n", text_id);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void display_bridge_init(void) {
    uart_config_t uart_config = {
        .baud_rate = DISPLAY_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_param_config(DISPLAY_UART_PORT, &uart_config);
    uart_set_pin(DISPLAY_UART_PORT, DISPLAY_UART_TX_PIN, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(DISPLAY_UART_PORT, 256, 0, 0, NULL, 0);

    /* Se fija al nucleo 2 a proposito, para no interferir con el
       nucleo 1 que BlueRetro usa para Bluetooth. */
    xTaskCreatePinnedToCore(display_bridge_task, "display_bridge", 2048, NULL, 1, NULL, 1);
}
