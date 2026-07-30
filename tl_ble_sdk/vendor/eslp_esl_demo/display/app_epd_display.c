/********************************************************************************************************
 * @file    app_epd_display.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    01,2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "app_epd_display.h"

#if APP_EPD_DISPLAY

#define APP_EPD_DISPLAY_IMAGE_SIZE 4736
#define DISPLAY_EPD_WIDTH          296
#define DISPLAY_EPD_HEIGHT         128
#define DISPLAY_EPD_TYPE           BLC_ESLS_DISPLAY_TYPE_BLACK_WHITE

#define Simulation                 1

#if HARDWARE_BOARD_SELECT == HW_C1T335A78
    #define EPD_BS1_PIN      GPIO_PE7
    #define EPD_BUSY_PIN     GPIO_PE3
    #define EPD_RESET_PIN    GPIO_PE2
    #define EPD_DC_PIN       GPIO_PE4
    #define EPD_SPI_CS_PIN   GPIO_PE5
    #define EPD_SPI_CK_PIN   GPIO_PB2
    #define EPD_SPI_DO_PIN   GPIO_PA1
    #define EPD_PWR_CTRL_PIN GPIO_PA2
#endif

#define EPD_ENABLE_WRITE_CMD()  gpio_write(EPD_DC_PIN, 0)
#define EPD_ENABLE_WRITE_DATA() gpio_write(EPD_DC_PIN, 1)
#define EPD_IS_BUSY()           (!gpio_read(EPD_BUSY_PIN))

#define EPD_POWER_ON()          gpio_write(EPD_PWR_CTRL_PIN, 0)

#define EPD_POWER_OFF()            \
    do {                           \
        gpio_write(EPD_DC_PIN, 0); \
    } while (0)

#define APP_EPD_DISPLAY_RESET_STAGE_1_TIMEOUT_MS   1400
#define APP_EPD_DISPLAY_RESET_STAGE_2_TIMEOUT_MS   1400
#define APP_EPD_DISPLAY_BUSY_CHECK_1_TIMEOUT_MS    240
#define APP_EPD_DISPLAY_BUSY_CHECK_2_TIMEOUT_MS    240
#define APP_EPD_DISPLAY_LOAD_IMAGE_TIMEOUT_MS      2
#define APP_EPD_DISPLAY_REFRESH_DISPLAY_TIMEOUT_MS 140
#define APP_EPD_DISPLAY_IMAGE_LOAD_CHUNK_SIZE      64

typedef enum
{
    APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_IDLE = 0,
    APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_RESET_STAGE_1,
    APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_RESET_STAGE_2,
    APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_BUSY_CHECK_1,
    APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_CLEAR_IMAGE_CONTINUE,
    APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_LOAD_IMAGE_CONTINUE,
    APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_LOAD_IMAGE_DONE,
    APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_REFRESH_DISPLAY,
    APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_BUSY_CHECK_2,
    APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_TEARDOWN,
} app_epd_display_update_display_state_t;

typedef struct
{
    app_epd_display_update_display_state_t state;
    u8                                     image[APP_EPD_DISPLAY_IMAGE_SIZE];
    u16                                    offset;
    u32                                    last_tick;
    u32                                    timeout;
} app_epd_display_state_t;

_attribute_data_sec_ app_epd_display_state_t state;

static void EPD_SPI_Write(unsigned char value)
{
#ifdef Simulation
    unsigned char i;

    delay_us(10);
    for (i = 0; i < 8; i++) {
        gpio_write(EPD_SPI_CK_PIN, 0);
        delay_us(1);
        if (value & 0x80) {
            gpio_write(EPD_SPI_DO_PIN, 1);
        } else {
            gpio_write(EPD_SPI_DO_PIN, 0);
        }
        value = (value << 1);
        delay_us(1);
        delay_us(1);
        gpio_write(EPD_SPI_CK_PIN, 1);
        delay_us(1);
    }
#else
    spi_write(&value, 1, 0, 0, EPD_SPI_CS_PIN);
#endif
}

static void EPD_WriteCmd(unsigned char cmd)
{
    //delay_us(10);
    gpio_write(EPD_SPI_CS_PIN, 0);
    EPD_ENABLE_WRITE_CMD();
    EPD_SPI_Write(cmd);
    gpio_write(EPD_SPI_CS_PIN, 1);
}

static void EPD_WriteData(unsigned char data)
{
    gpio_write(EPD_SPI_CS_PIN, 0);
    EPD_ENABLE_WRITE_DATA();
    EPD_SPI_Write(data);
    gpio_write(EPD_SPI_CS_PIN, 1);
}

static void EPD_Init(void)
{
    //reset pin configuration
    gpio_function_en(EPD_RESET_PIN);
    gpio_output_en(EPD_RESET_PIN);
    gpio_input_dis(EPD_RESET_PIN);
    gpio_set_level(EPD_RESET_PIN, 0);
    //BS1 pin configuration
    //select 4-lines SPI interface
    gpio_function_en(EPD_BS1_PIN);
    gpio_output_en(EPD_BS1_PIN);
    gpio_input_dis(EPD_BS1_PIN);
    gpio_set_level(EPD_BS1_PIN, 0);
    //EPD power supply control pin configuration
    gpio_function_en(EPD_PWR_CTRL_PIN);
    gpio_output_en(EPD_PWR_CTRL_PIN);
    gpio_input_dis(EPD_PWR_CTRL_PIN);
    gpio_set_level(EPD_PWR_CTRL_PIN, 1);
    //EPD Data/Command control pin configuration
    gpio_function_en(EPD_DC_PIN);
    gpio_output_en(EPD_DC_PIN);
    gpio_input_dis(EPD_DC_PIN);
    gpio_set_level(EPD_DC_PIN, 0);
    //EPD Busy Status pin configuratioin
    gpio_function_en(EPD_BUSY_PIN);
    gpio_output_dis(EPD_BUSY_PIN);
    gpio_input_en(EPD_BUSY_PIN);
    gpio_set_up_down_res(EPD_BUSY_PIN, GPIO_PIN_PULLUP_1M);

#ifdef Simulation

    //cs pin configuration
    gpio_function_en(EPD_SPI_CS_PIN);
    gpio_output_en(EPD_SPI_CS_PIN);
    gpio_input_dis(EPD_SPI_CS_PIN);
    gpio_set_level(EPD_SPI_CS_PIN, 1);
    //clk pin configuration
    gpio_function_en(EPD_SPI_CK_PIN);
    gpio_output_en(EPD_SPI_CK_PIN);
    gpio_input_dis(EPD_SPI_CK_PIN);
    gpio_set_level(EPD_SPI_CK_PIN, 0);
    //DO pin configuration
    gpio_function_en(EPD_SPI_DO_PIN);
    gpio_output_en(EPD_SPI_DO_PIN);
    gpio_input_dis(EPD_SPI_DO_PIN);
    gpio_set_level(EPD_SPI_DO_PIN, 1);
#else
    spi_master_init(SPI_CLK_1M, SPI_MODE0);
    spi_master_gpio_set(SPI_GPIO_GROUP_B6B7D2D7);
    spi_masterCSpin_select(EPD_SPI_CS_PIN);

#endif
}

static void EPD_Close(void)
{
    gpio_set_level(EPD_PWR_CTRL_PIN, 1);

    //need to disable PULLUP, or it will enlarge the deep retention current.
    //for other IO, no need to care
    gpio_shutdown(EPD_BUSY_PIN);
    gpio_set_up_down_res(EPD_BUSY_PIN, GPIO_PIN_UP_DOWN_FLOAT);


    //GPIO_AllTurnOff();
    //    gpio_function_en(EPD_RESET_PIN);
    //    //gpio_set_up_down_res(EPD_RESET_PIN,GPIO_PIN_PULLUP_10K);
    //    gpio_output_dis(EPD_RESET_PIN);
    //    gpio_input_en(EPD_RESET_PIN);
    //    //gpio_set_up_down_res(EPD_RESET_PIN,GPIO_PIN_PULLUP_10K);
    //
    //    //BS1 pin configuration
    //    gpio_function_en(EPD_BS1_PIN);
    //    gpio_output_dis(EPD_BS1_PIN);
    //    gpio_input_en(EPD_BS1_PIN);
    //    gpio_set_up_down_res(EPD_BS1_PIN,GPIO_PIN_PULLDOWN_100K);
    //
    //    //EPD power supply control pin configuration
    //    gpio_function_en(EPD_PWR_CTRL_PIN);
    //    gpio_set_up_down_res(EPD_PWR_CTRL_PIN,GPIO_PIN_PULLDOWN_100K);
    //    gpio_output_dis(EPD_PWR_CTRL_PIN);
    //    gpio_input_en(EPD_PWR_CTRL_PIN);
    //    gpio_set_up_down_res(EPD_PWR_CTRL_PIN,GPIO_PIN_PULLDOWN_100K);
    //
    //
    //    //EPD Data/Command control pin configuration
    //    gpio_function_en(EPD_DC_PIN);
    //    gpio_output_dis(EPD_DC_PIN);
    //    gpio_input_en(EPD_DC_PIN);
    //    //gpio_set_up_down_res(EPD_DC_PIN,GPIO_PIN_PULLUP_1M);
    //
    //
    //    //EPD Busy Status pin configuratioin
    //    gpio_function_en(EPD_BUSY_PIN);
    //    gpio_output_dis(EPD_BUSY_PIN);
    //    gpio_input_en(EPD_BUSY_PIN);
    //    gpio_set_up_down_res(EPD_BUSY_PIN,GPIO_PIN_PULLDOWN_100K); //turn off pull-up resistor
    //
    //
    //    //Turn off all SPI IOs
    //    gpio_function_en(EPD_SPI_CS_PIN);
    //    gpio_output_dis(EPD_SPI_CS_PIN);
    //    gpio_input_en(EPD_SPI_CS_PIN);
    //    //gpio_set_up_down_res(EPD_SPI_CS_PIN,GPIO_PIN_PULLUP_1M);
    //
    //
    //    gpio_function_en(EPD_SPI_CK_PIN);
    //    gpio_output_dis(EPD_SPI_CK_PIN);
    //    gpio_input_en(EPD_SPI_CK_PIN);
    //    gpio_set_up_down_res(EPD_SPI_CK_PIN,GPIO_PIN_PULLDOWN_100K);


    //    gpio_function_en(EPD_SPI_DO_PIN);
    //    gpio_output_dis(EPD_SPI_DO_PIN);
    //    gpio_input_en(EPD_SPI_DO_PIN);
    //    gpio_set_up_down_res(EPD_SPI_DO_PIN,GPIO_PIN_PULLDOWN_100K);
}

static void EPD_ClearImageStart(int size)
{
    EPD_WriteCmd(0x10);
    for (int i = 0; i < size; i++) {
        EPD_WriteData(0xff);
    }
}

static void EPD_ClearImageContinue(int size)
{
    for (int i = 0; i < size; i++) {
        EPD_WriteData(0xff);
    }
}

static void EPD_LoadImageStart(unsigned char *image, int size)
{
    EPD_WriteCmd(0x13);
    for (int i = 0; i < size; i++) {
        EPD_WriteData(image[i]);
    }
}

static void EPD_LoadImageContinue(unsigned char *image, int size)
{
    for (int i = 0; i < size; i++) {
        EPD_WriteData(image[i]);
    }
}

static bool app_display_epd_is_busy(void)
{
    return state.state != APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_IDLE;
}

static bool app_display_epd_get_buffer(u8 **ptr)
{
    *ptr = state.image;

    return true;
}

static bool app_display_epd_image(void)
{
    if (state.state != APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_IDLE) {
        return false;
    }

    EPD_Init();
    EPD_POWER_ON();

    //Reset the EPD driver IC
    gpio_write(EPD_BS1_PIN, 0);
    gpio_write(EPD_RESET_PIN, 0);

    state.state     = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_RESET_STAGE_1;
    state.last_tick = clock_time();
    state.timeout   = 1000 * APP_EPD_DISPLAY_RESET_STAGE_1_TIMEOUT_MS;
    state.offset    = 0;

    return true;
}

static bool app_display_epd_clean(void)
{
    if (state.state != APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_IDLE) {
        return false;
    }

    EPD_Init();

    EPD_POWER_ON();
    //Reset the EPD driver IC
    gpio_write(EPD_BS1_PIN, 0);
    gpio_write(EPD_RESET_PIN, 0);

    state.state     = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_RESET_STAGE_1;
    state.last_tick = clock_time();
    state.timeout   = 1000 * APP_EPD_DISPLAY_RESET_STAGE_1_TIMEOUT_MS;
    state.offset    = 0;
    memset(state.image, 0xff, sizeof(state.image));

    return true;
}

static void app_display_epd_loop(void)
{
    u16 to_write, to_clear;

    if (state.state == APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_IDLE || !clock_time_exceed(state.last_tick, state.timeout)) {
        return;
    }

    state.last_tick = clock_time();
    switch (state.state) {
    case APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_RESET_STAGE_1:
        gpio_write(EPD_RESET_PIN, 1);
        state.state   = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_RESET_STAGE_2;
        state.timeout = 1000 * APP_EPD_DISPLAY_RESET_STAGE_2_TIMEOUT_MS;
        break;
    case APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_RESET_STAGE_2:
        EPD_WriteCmd(0x06);
        EPD_WriteData(0x17);
        EPD_WriteData(0x17);
        EPD_WriteData(0x17);
        //power on
        EPD_WriteCmd(0x04);
        state.timeout = 0;
        state.state   = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_BUSY_CHECK_1;
        break;
    case APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_BUSY_CHECK_1:
    {
        if (EPD_IS_BUSY()) {
            state.timeout = 1000 * APP_EPD_DISPLAY_BUSY_CHECK_1_TIMEOUT_MS;
            break;
        }
        //panel setting
        EPD_WriteCmd(0x00);
        EPD_WriteData(0x1f);
        EPD_WriteData(0x0f);

        //resolution setting
        EPD_WriteCmd(0x61);

        EPD_WriteData(0x80);
        EPD_WriteData(0x01);
        EPD_WriteData(0x28);


        //Vcom and data interval setting
        EPD_WriteCmd(0X50);
        EPD_WriteData(0x97);

        // Begin clear
        to_clear = (sizeof(state.image) - state.offset) > APP_EPD_DISPLAY_IMAGE_LOAD_CHUNK_SIZE ? APP_EPD_DISPLAY_IMAGE_LOAD_CHUNK_SIZE : sizeof(state.image) - state.offset;
        EPD_ClearImageStart(to_clear);
        state.offset += to_clear;
        state.state   = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_CLEAR_IMAGE_CONTINUE;
        state.timeout = 0;

        break;
    }
    case APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_CLEAR_IMAGE_CONTINUE:
    {
        to_clear = sizeof(state.image) - state.offset > APP_EPD_DISPLAY_IMAGE_LOAD_CHUNK_SIZE ? APP_EPD_DISPLAY_IMAGE_LOAD_CHUNK_SIZE : sizeof(state.image) - state.offset;
        if (to_clear) {
            EPD_ClearImageContinue(to_clear);
            state.offset += to_clear;
            state.timeout = 0;
        } else {
            // Begin write
            state.offset = 0;
            to_write     = sizeof(state.image) - state.offset > APP_EPD_DISPLAY_IMAGE_LOAD_CHUNK_SIZE ? APP_EPD_DISPLAY_IMAGE_LOAD_CHUNK_SIZE : sizeof(state.image) - state.offset;
            EPD_LoadImageStart(&state.image[state.offset], to_write);
            state.offset += to_write;
            state.state   = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_LOAD_IMAGE_CONTINUE;
            state.timeout = 0;
        }

        break;
    }
    case APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_LOAD_IMAGE_CONTINUE:
    {
        to_write = sizeof(state.image) - state.offset > APP_EPD_DISPLAY_IMAGE_LOAD_CHUNK_SIZE ? APP_EPD_DISPLAY_IMAGE_LOAD_CHUNK_SIZE : sizeof(state.image) - state.offset;
        if (to_write) {
            EPD_LoadImageContinue(&state.image[state.offset], to_write);
            state.offset += to_write;
            state.timeout = 0;
        } else {
            state.state   = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_LOAD_IMAGE_DONE;
            state.timeout = 1000 * APP_EPD_DISPLAY_LOAD_IMAGE_TIMEOUT_MS;
        }

        break;
    }
    case APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_LOAD_IMAGE_DONE:
        EPD_WriteCmd(0x12);
        state.state   = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_REFRESH_DISPLAY;
        state.timeout = 1000 * APP_EPD_DISPLAY_REFRESH_DISPLAY_TIMEOUT_MS;

        break;
    case APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_REFRESH_DISPLAY:
        state.state   = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_BUSY_CHECK_2;
        state.timeout = 0;

        break;
    case APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_BUSY_CHECK_2:
        if (EPD_IS_BUSY()) {
            state.timeout = 1000 * APP_EPD_DISPLAY_BUSY_CHECK_1_TIMEOUT_MS;
        } else {
            state.timeout = 0;
            state.state   = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_TEARDOWN;
        }

        break;
    case APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_TEARDOWN:
        //Vcom and data interval setting
        EPD_WriteCmd(0X50);
        EPD_WriteData(0xf7);

        //power off
        EPD_WriteCmd(0X02);

        //deep sleep
        EPD_WriteCmd(0X07);
        EPD_WriteData(0xa5);

        EPD_POWER_OFF();

        state.state = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_IDLE;

        EPD_Close();
        break;
    case APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_IDLE:
        // Do nothing
        break;
    }
}

static void app_display_epd_init(void)
{
    state.state = APP_EPD_DISPLAY_UPDATE_DISPLAY_STATE_IDLE;
}

static const app_display_iface_t app_display_epd_cb = {
    .type       = DISPLAY_EPD_TYPE,
    .width      = DISPLAY_EPD_WIDTH,
    .heigth     = DISPLAY_EPD_HEIGHT,
    .image_size = APP_EPD_DISPLAY_IMAGE_SIZE,
    .is_busy    = app_display_epd_is_busy,
    .image      = app_display_epd_image,
    .loop       = app_display_epd_loop,
    .clean      = app_display_epd_clean,
    .get_buffer = app_display_epd_get_buffer,
    .init       = app_display_epd_init,
};

const app_display_iface_t *app_display_epd_get_iface(void)
{
    return &app_display_epd_cb;
}

#endif
