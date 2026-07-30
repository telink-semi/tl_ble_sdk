/********************************************************************************************************
 * @file    app_esl.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    10,2023
 *
 * @par     Copyright (c) 2023, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "app_image_storage.h"
#include "app_esl.h"

#include "display/app_display.h"
#include "display/app_display_dummy.h"
#include "display/app_epd_display.h"

#include "led/app_led.h"
#include "led/app_led_mono_green.h"
#include "led/app_led_mono_red.h"
#include "led/app_led_rgb_dummy.h"

#include "sensor/app_sensor.h"
#include "sensor/app_sensor_dummy_type0.h"
#include "sensor/app_sensor_dummy_type1.h"

#include "vendor_image/app_vendor_image.h"

#define IMPLAUSIBLE_TIME_OFFSET_MS (48 * 24 * 60 * 60 * 100)

typedef struct
{
    // Pending timed command
    bool pendingUpdate : 1;
    // Absolute time in timed command
    u32 scheduledCommandTime;
    // Scheduled timed command
    app_led_command_t scheduledCommand;
} ledControlData_t;

typedef struct
{
    bool active;
    u32  timer;
    u32  lastTick;
} currentTime_t;

typedef struct
{
    // Display has been set by the user
    bool active : 1;
    // Pending timed command
    bool pendingUpdate : 1;
    // Display has been requested
    bool newImage : 1;
    // Last requested image index
    u8 imageIdx;
    // Image index in timed command
    u8 pendingUpdateImageIdx;
    // Absolute time in timed command
    u32 pendingUpdateAbsoluteTime;
} displayControl_t;

typedef struct
{
    bool             active;
    u32              start;
    u32              timeout;
    blc_esl_handle_t handle;
} unassociateTimer_t;

_attribute_data_retention_ static blc_esls_displayData_t       displayData[ESLS_DISPLAYS_SUPPORTED];
_attribute_data_retention_ static blc_esls_sensorInformation_t sensorInformation[ESLS_SENSORS_SUPPORTED];
_attribute_data_retention_ static blc_esls_ledInformation_t    ledInformation[ESLS_LEDS_SUPPORTED];
_attribute_data_retention_ static blc_eslss_regParam_t         eslsRegParam;
static const electronicShelfLabelParam_t                       electronicShelfLabelParams = {
                          .eslssParam = &eslsRegParam,
};
static const blc_otss_regParam_t ots_params = {
    .oacp_features = BLC_OTS_OACP_FEATURE_OACP_READ_OPCODE_SUPPORTED | BLC_OTS_OACP_FEATURE_OACP_WRITE_OPCODE_SUPPORTED | BLC_OTS_OACP_FEATURE_PATCHING_OF_OBJECTS_SUPPORTED | BLC_OTS_OACP_FEATURE_TRUNCATION_OF_OBJECTS_SUPPORTED,
    .olcp_features = BLC_OTS_OLCP_FEATURE_OLCP_GO_TO_OPCODE_SUPPORTED,
};
_attribute_data_retention_ static blc_ots_object_id_t objectIds[APP_IMAGE_STORAGE_MAX_IMAGES];
#if APP_VENDOR_IMAGE
_attribute_data_retention_ static blc_ots_object_id_t vendorObjectId;
#endif
_attribute_data_retention_ static blc_esls_state_t   state                                       = BLC_ESLS_STATE_UNASSOCIATED;
_attribute_data_retention_ static ledControlData_t   ledControlData[ARRAY_SIZE(ledInformation)]  = {0};
_attribute_data_retention_ static displayControl_t   displayControlData[ARRAY_SIZE(displayData)] = {0};
_attribute_data_retention_ static currentTime_t      currentTime                                 = {0};
_attribute_data_retention_ static bool               serviceNeeded                               = 0;
_attribute_data_retention_ static bool               pendingUnassociate                          = 0;
_attribute_data_retention_ static u32                serviceNeededTick                           = 0;
_attribute_data_retention_ static unassociateTimer_t unTimer                                     = {0};
#if APP_VENDOR_IMAGE
_attribute_data_retention_ static u8 app_vendor_image_idx = 0;
#endif

static void reloadUnassociateTimer(u32 timeout, blc_esl_handle_t handle)
{
    unTimer.active  = true;
    unTimer.handle  = handle;
    unTimer.timeout = timeout;
    unTimer.start   = clock_time();
}

static void stopUnassociateTimer(void)
{
    unTimer.active = false;
}

static void checkUnassociateTimer(void)
{
    if (unTimer.active && clock_time_exceed(unTimer.start, unTimer.timeout)) {
        if (unTimer.handle.type == BLC_ESL_TRANSPORT_TYPE_ACL) {
            blc_ll_disconnect(unTimer.handle.connHandle, HCI_ERR_REMOTE_USER_TERM_CONN);
        } else {
            pendingUnassociate = false;
            blc_eslp_esl_unassociate();
        }

        unTimer.active = false;
    }
}

static u16 updateBasicState(blc_eslss_controlPointResponseHdr_t *rsp)
{
    blc_eslss_controlPointResponseBasicState_t *basicState = (blc_eslss_controlPointResponseBasicState_t *)rsp;

    basicState->hdr.opcode = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_BASIC_STATE;

    basicState->serviceNeeded        = serviceNeeded;
    basicState->synchronized         = ble_eslp_esl_is_pa_synchronized() ? 1 : 0;
    basicState->pendingLedUpdate     = 0;
    basicState->pendingDisplayUpdate = 0;
    basicState->activeLed            = 0;

    for (u8 i = 0; i < ARRAY_SIZE(ledControlData); i++) {
        ledControlData_t *ledControl = &ledControlData[i];
        bool              ledActive  = false;

        ledActive = app_led_is_active(i);
        if (ledActive) {
            basicState->activeLed = 1;
        }

        if (ledControl->pendingUpdate) {
            basicState->pendingLedUpdate = 1;
        }
    }

    for (u8 i = 0; i < ARRAY_SIZE(displayControlData); i++) {
        displayControl_t *displayControl = &displayControlData[i];

        if (displayControl->pendingUpdate) {
            basicState->pendingDisplayUpdate = 1;
        }
    }

    return sizeof(*basicState);
}

static void displayControlDataLoop(u8 displayId)
{
    displayControl_t *displayCtrl = &displayControlData[displayId];
    if (displayCtrl->newImage && !app_display_is_busy(displayId)) {
        u16 imageSize;
        u8 *image;

        if (!app_display_get_buffer(displayId, &image) ||
            !app_display_get_info(displayId, NULL, NULL, NULL, &imageSize)) {
            goto done;
        }

#if APP_VENDOR_IMAGE
        if (displayCtrl->imageIdx == app_vendor_image_idx) {
            if (app_vendor_image_get_image_size() == imageSize) {
                app_vendor_image_get_image(image);
                app_display_image(displayId);
            }
        } else {
#endif
            app_image_storage_get_image_data(displayCtrl->imageIdx, 0, imageSize, image);
            app_display_image(displayId);
#if APP_VENDOR_IMAGE
        }
#endif
done:
        displayCtrl->newImage = false;
    }
}

static bool currentTimeCheckTimedCommand(u32 time)
{
    if (!currentTime.active) {
        return false;
    }

    if (currentTime.timer <= time) {
        return (time - currentTime.timer) <= IMPLAUSIBLE_TIME_OFFSET_MS;
    }

    return (0xFFFFFFFF - currentTime.timer) + time;
}

static void app_esl_leds_fill_led_information(void)
{
    for (u8 i = 0; i < min(app_led_get_num_leds(), ARRAY_SIZE(ledInformation)); i++) {
        app_led_config_t config;

        app_led_get_info(i, &config);
        ledInformation[i].red   = config.colorRed;
        ledInformation[i].green = config.colorGreen;
        ledInformation[i].blue  = config.colorBlue;
        ledInformation[i].type  = config.type == APP_LED_TYPE_MONO ? BLC_ESLS_LED_INFORMATION_MONOCHROME : BLC_ESLS_LED_INFORMATION_SRGB;
    }
}

static void app_esl_handleControlPointCmds(blc_eslp_esl_controlPointCommant_t *cmd)
{
    u8                                   rspBuf[MAX_ESL_PAYLOAD_SIZE];
    blc_eslss_controlPointResponseHdr_t *response = (blc_eslss_controlPointResponseHdr_t *)rspBuf;
    blc_eslss_controlPointCommandHdr_t  *command  = (blc_eslss_controlPointCommandHdr_t *)cmd->cmds;
    u8                                   rspIndex = 0;
    u8                                   rspLeft  = 0;

    for (u8 i = 0; i < cmd->numCmds; i++) {
        if (command->eslId != BLC_ESLS_ESL_ID_BROADCAST) {
            rspLeft++;
        }

        command = (blc_eslss_controlPointCommandHdr_t *)(((u8 *)command) + blc_esl_getCommandSize(command));
    }

    if (rspLeft * BLC_ESLS_CMD_RSP_MIN_LENGTH > sizeof(rspBuf)) {
        return;
    }

    command = (blc_eslss_controlPointCommandHdr_t *)cmd->cmds;

    for (u8 i = 0; i < cmd->numCmds; i++) {
        u16 rspLen = 0;
        u8  rsp[BLC_ESLS_CMD_RSP_MAX_LENGTH];

        switch (command->opcode) {
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_PING:
            if (command->eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                rspLen = updateBasicState((blc_eslss_controlPointResponseHdr_t *)rsp);
            }
            break;
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_LED_CONTROL:
        {
            blc_eslss_controlPointCommandLedControl_t *ledCommand      = (blc_eslss_controlPointCommandLedControl_t *)command;
            u8                                         ledId           = ledCommand->ledId;
            app_led_command_t                          app_led_command = {
                                         .settings.colorRed   = ledCommand->colorRed,
                                         .settings.colorGreen = ledCommand->colorGreen,
                                         .settings.colorBlue  = ledCommand->colorBlue,
                                         .settings.brightness = ledCommand->brightness,
                                         .bit_off_period      = ledCommand->flashingPattern[5],
                                         .bit_on_period       = ledCommand->flashingPattern[6],
                                         .repeat_duration     = ledCommand->repeatDuration,
                                         .repeat_type         = ledCommand->repeatType,
            };
            app_led_config_t config;

            memcpy(app_led_command.pattern, ledCommand->flashingPattern, sizeof(app_led_command.pattern));

            app_led_command_apply(ledCommand->ledId, &app_led_command);
            tlkapi_printf(APP_LOG_EN, "LED [%d] Applying new command", ledId);
            if (app_led_get_info(ledCommand->ledId, &config) && config.type == APP_LED_TYPE_SRGB) {
                // Need to update LED in case of RGB LED
                app_esl_leds_fill_led_information();
                blc_eslss_updateEslLedInformation(0xFFFF, min(app_led_get_num_leds(), ARRAY_SIZE(ledInformation)), ledInformation);
            }

            if (command->eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                blc_eslss_controlPointResponseLedState_t *ledState = (blc_eslss_controlPointResponseLedState_t *)rsp;
                ledState->hdr.opcode                               = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_LED_STATE,
                ledState->ledId                                    = ledId;
                rspLen                                             = sizeof(blc_eslss_controlPointResponseLedState_t);
            }
            break;
        }
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_LED_TIMED_CONTROL:
        {
            blc_eslss_controlPointCommandLedTimedControl_t *ledTimedCommand = (blc_eslss_controlPointCommandLedTimedControl_t *)command;
            u8                                              ledId           = ledTimedCommand->ledId;

            if (ledControlData[ledId].pendingUpdate) {
                if (ledTimedCommand->absoluteTime == 0) {
                    // ESLS spec 3.9.2.11.1 Handling multiple LED Timed Control commands "If the value of the Absolute
                    // Time parameter is zero (0x00000000), then the pending LED Timed Control command shall be deleted."
                    tlkapi_printf(APP_LOG_EN, "LED [%d] pending update: true -> false", ledId);

                    ledControlData[ledId].pendingUpdate = false;
                    if (ledTimedCommand->hdr.eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                        blc_eslss_controlPointResponseLedState_t *ledState = (blc_eslss_controlPointResponseLedState_t *)rsp;
                        ledState->hdr.opcode                               = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_LED_STATE,
                        ledState->ledId                                    = ledId;
                        rspLen                                             = sizeof(blc_eslss_controlPointResponseLedState_t);
                    }
                    break;
                }

                if (ledTimedCommand->absoluteTime != ledControlData[ledId].scheduledCommandTime) {
                    // ESLS spec 3.9.2.11.1 Handling multiple LED Timed Control commands "Otherwise,
                    // the ESL shall send the Error response: Queue Full, as described in Section 3.9.3.1, as it
                    // is not permitted to have more than one LED Timed Control command pending. In this case, the LED
                    // Timed Control command that was already pending remains unchanged.
                    if (ledTimedCommand->hdr.eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                        blc_eslss_controlPointResponseError_t *errorRsp = (blc_eslss_controlPointResponseError_t *)rsp;
                        errorRsp->hdr.opcode                            = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_ERROR;
                        errorRsp->error                                 = BLC_ESLSS_ERROR_CODE_QUEUE_FULL;
                        rspLen                                          = blc_esl_getResponseSize(&errorRsp->hdr);
                    }
                    break;
                }

                // ESLS spec 3.9.2.11.1 Handling multiple LED Timed Control commands "If the value of the
                // Absolute_Time parameter in the new command is equal to the value of the Absolute_Time
                // parameter in the pending LED Timed Control command, then the newly received command
                // shall replace the old pending command.
            }

            if (!currentTimeCheckTimedCommand(ledTimedCommand->absoluteTime)) {
                blc_eslss_controlPointResponseError_t *errorRsp = (blc_eslss_controlPointResponseError_t *)rsp;
                errorRsp->hdr.opcode                            = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_ERROR;
                errorRsp->error                                 = BLC_ESLSS_ERROR_CODE_IMPLAUSIBLE_ABSOLUTE_TIME;
                rspLen                                          = blc_esl_getResponseSize(&errorRsp->hdr);
                break;
            }

            tlkapi_printf(APP_LOG_EN, "LED [%d] pending update: %s -> true, time: %d", ledId, ledControlData[ledId].pendingUpdate ? "true" : "false", ledTimedCommand->absoluteTime);

            ledControlData[ledId].scheduledCommand.settings.colorRed   = ledTimedCommand->colorRed;
            ledControlData[ledId].scheduledCommand.settings.colorGreen = ledTimedCommand->colorGreen;
            ledControlData[ledId].scheduledCommand.settings.colorBlue  = ledTimedCommand->colorBlue;
            ledControlData[ledId].scheduledCommand.settings.brightness = ledTimedCommand->brightness;
            ledControlData[ledId].scheduledCommand.bit_off_period      = ledTimedCommand->flashingPattern[5];
            ledControlData[ledId].scheduledCommand.bit_on_period       = ledTimedCommand->flashingPattern[6];
            ledControlData[ledId].scheduledCommand.repeat_duration     = ledTimedCommand->repeatDuration;
            ledControlData[ledId].scheduledCommand.repeat_type         = ledTimedCommand->repeatType;
            memcpy(ledControlData[ledId].scheduledCommand.pattern, ledTimedCommand->flashingPattern, 5);
            ledControlData[ledId].scheduledCommandTime = ledTimedCommand->absoluteTime;
            ledControlData[ledId].pendingUpdate        = true;

            if (command->eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                blc_eslss_controlPointResponseLedState_t *ledState = (blc_eslss_controlPointResponseLedState_t *)rsp;
                ledState->hdr.opcode                               = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_LED_STATE,
                ledState->ledId                                    = ledId;
                rspLen                                             = blc_esl_getResponseSize(&ledState->hdr);
            }
            break;
        }
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_READ_SENSOR_DATA:
        {
            if (command->eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                blc_eslss_controlPointCommandReadSensorData_t *sensorCmd = (blc_eslss_controlPointCommandReadSensorData_t *)command;
                blc_eslss_controlPointResponseSensorValue_t   *sensorRsp = (blc_eslss_controlPointResponseSensorValue_t *)rsp;
                u8                                             val[15];
                u8                                             valLength = sizeof(val);

                app_sensor_read(sensorCmd->sensorId, &valLength, val);

                sensorRsp->hdr.opcode = BLC_ESLSS_SENSOR_VALUE_OPCODE(valLength);
                sensorRsp->sensorId   = sensorCmd->sensorId;
                memcpy(sensorRsp->sensorData, val, valLength);
                rspLen = blc_esl_getResponseSize(&sensorRsp->hdr);
            }
            break;
        }
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_REFRESH_DISPLAY:
        {
            blc_eslss_controlPointCommandRefreshDisplay_t *refreshCmd     = (blc_eslss_controlPointCommandRefreshDisplay_t *)command;
            displayControl_t                              *displayControl = &displayControlData[refreshCmd->displayId];

            if (!displayControl->active) {
                if (refreshCmd->hdr.eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                    blc_eslss_controlPointResponseError_t *errorRsp = (blc_eslss_controlPointResponseError_t *)rsp;
                    errorRsp->hdr.opcode                            = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_ERROR;
                    errorRsp->error                                 = BLC_ESLSS_ERROR_CODE_IMAGE_NOT_AVAILABLE;
                    rspLen                                          = blc_esl_getResponseSize(&errorRsp->hdr);
                }

                break;
            }

            if (refreshCmd->hdr.eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                blc_eslss_controlPointResponseDisplayState_t *displayState = (blc_eslss_controlPointResponseDisplayState_t *)rsp;
                displayState->hdr.opcode                                   = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_DISPLAY_STATE;
                displayState->displayId                                    = refreshCmd->displayId;
                displayState->imageId                                      = displayControl->imageIdx;
                rspLen                                                     = blc_esl_getResponseSize(&displayState->hdr);
            }

            break;
        }
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_DISPLAY_IMAGE:
        {
            blc_eslss_controlPointCommandDisplayImage_t *displayImage        = (blc_eslss_controlPointCommandDisplayImage_t *)command;
            displayControl_t                            *displayControl      = &displayControlData[displayImage->displayId];
            u32                                          imageLength         = 0;
            u16                                          expectedImageLength = 0;

            tlkapi_printf(APP_LOG_EN, "DISPLAY [%d] IMAGE [%d]", displayImage->displayId, displayImage->imageId);

            app_image_storage_get_image_length(displayImage->imageId, &imageLength);
            app_display_get_info(displayImage->displayId, NULL, NULL, NULL, &expectedImageLength);

#if APP_VENDOR_IMAGE
            if (displayImage->imageId == app_vendor_image_idx) {
                imageLength = app_vendor_image_get_image_size();
            }
#endif

            if (imageLength != expectedImageLength) {
                if (displayImage->hdr.eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                    blc_eslss_controlPointResponseError_t *errorRsp = (blc_eslss_controlPointResponseError_t *)rsp;
                    errorRsp->hdr.opcode                            = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_ERROR;
                    errorRsp->error                                 = BLC_ESLSS_ERROR_CODE_IMAGE_NOT_AVAILABLE;
                    rspLen                                          = blc_esl_getResponseSize(&errorRsp->hdr);
                }

                break;
            }
            displayControl->active   = true;
            displayControl->newImage = true;
            displayControl->imageIdx = displayImage->imageId;

            if (displayImage->hdr.eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                blc_eslss_controlPointResponseDisplayState_t *displayState = (blc_eslss_controlPointResponseDisplayState_t *)rsp;
                displayState->hdr.opcode                                   = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_DISPLAY_STATE;
                displayState->displayId                                    = displayImage->displayId;
                displayState->imageId                                      = displayImage->imageId;
                rspLen                                                     = blc_esl_getResponseSize(&displayState->hdr);
            }

            break;
        }
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_DISPLAY_TIMED_IMAGE:
        {
            blc_eslss_controlPointCommandDisplayTimedImage_t *displayTimedImage   = (blc_eslss_controlPointCommandDisplayTimedImage_t *)command;
            displayControl_t                                 *displayControl      = &displayControlData[displayTimedImage->displayId];
            u32                                               imageLength         = 0;
            u16                                               expectedImageLength = 0;

            app_image_storage_get_image_length(displayTimedImage->imageId, &imageLength);
            app_display_get_info(displayTimedImage->displayId, NULL, NULL, NULL, &expectedImageLength);

#if APP_VENDOR_IMAGE
            if (displayTimedImage->imageId == app_vendor_image_idx) {
                imageLength = app_vendor_image_get_image_size();
            }
#endif

            if (imageLength != expectedImageLength) {
                if (displayTimedImage->hdr.eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                    blc_eslss_controlPointResponseError_t *errorRsp = (blc_eslss_controlPointResponseError_t *)rsp;
                    errorRsp->hdr.opcode                            = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_ERROR;
                    errorRsp->error                                 = BLC_ESLSS_ERROR_CODE_IMAGE_NOT_AVAILABLE;
                    rspLen                                          = blc_esl_getResponseSize(&errorRsp->hdr);
                }

                break;
            }

            if (displayControl->pendingUpdate) {
                if (displayTimedImage->absoluteTime == 0) {
                    // 3.9.2.9.1 Handling multiple Display Timed Image commands "If the value of the Absolute Time parameter
                    // is zero (0x00000000), then the pending Display Timed Image command shall be deleted.
                    displayControl->pendingUpdate = true;
                    if (displayTimedImage->hdr.eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                        blc_eslss_controlPointResponseDisplayState_t *displayState = (blc_eslss_controlPointResponseDisplayState_t *)rsp;
                        displayState->hdr.opcode                                   = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_DISPLAY_STATE;
                        displayState->displayId                                    = displayTimedImage->displayId;
                        displayState->imageId                                      = displayTimedImage->imageId;
                        rspLen                                                     = blc_esl_getResponseSize(&displayState->hdr);
                    }
                    break;
                }

                if (displayTimedImage->absoluteTime != displayControl->pendingUpdateAbsoluteTime) {
                    // 3.9.2.9.1 Handling multiple Display Timed Image commands "Otherwise, the ESL shall send the Error response:
                    // Queue Full, as described in Section 3.9.3.1, because it is not permitted to have more than one
                    // Display Timed Image command pending. In this case, the Display Timed Image command that was already
                    // pending remains unchanged.
                    if (displayTimedImage->hdr.eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                        blc_eslss_controlPointResponseError_t *errorRsp = (blc_eslss_controlPointResponseError_t *)rsp;
                        errorRsp->hdr.opcode                            = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_ERROR;
                        errorRsp->error                                 = BLC_ESLSS_ERROR_CODE_QUEUE_FULL;
                        rspLen                                          = blc_esl_getResponseSize(&errorRsp->hdr);
                    }
                    break;
                }

                // If the value of the Absolute_Time parameter in the new command is equal to the value of the
                // Absolute_Time parameter in the pending Display Timed Image command, then the newly received
                // command shall replace the old pending command.
            }

            if (!currentTimeCheckTimedCommand(displayTimedImage->absoluteTime)) {
                if (displayTimedImage->hdr.eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                    blc_eslss_controlPointResponseError_t *errorRsp = (blc_eslss_controlPointResponseError_t *)rsp;
                    errorRsp->hdr.opcode                            = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_ERROR;
                    errorRsp->error                                 = BLC_ESLSS_ERROR_CODE_IMPLAUSIBLE_ABSOLUTE_TIME;
                    rspLen                                          = blc_esl_getResponseSize(&errorRsp->hdr);
                }
                break;
            }

            displayControl->pendingUpdate             = true;
            displayControl->pendingUpdateImageIdx     = displayTimedImage->imageId;
            displayControl->pendingUpdateAbsoluteTime = displayTimedImage->absoluteTime;

            if (displayTimedImage->hdr.eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                blc_eslss_controlPointResponseDisplayState_t *displayState = (blc_eslss_controlPointResponseDisplayState_t *)rsp;
                displayState->hdr.opcode                                   = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_DISPLAY_STATE;
                displayState->displayId                                    = displayTimedImage->displayId;
                displayState->imageId                                      = displayTimedImage->imageId;
                rspLen                                                     = blc_esl_getResponseSize(&displayState->hdr);
            }
            break;
        }
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_0:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_1:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_2:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_3:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_4:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_5:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_6:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_7:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_8:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_9:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_A:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_B:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_C:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_D:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_E:
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_VENDOR_F:
        {
            blc_eslss_controlPointCommandVendorSpecific_t *v = (blc_eslss_controlPointCommandVendorSpecific_t *)command;
#if APP_VENDOR_IMAGE
            if (!app_vendor_image_handle_vendor_cmd(v, (blc_eslss_controlPointResponseHdr_t *)rsp)) {
#endif
                blc_eslss_controlPointResponseError_t *errorRsp = (blc_eslss_controlPointResponseError_t *)rsp;
                errorRsp->hdr.opcode                            = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_ERROR;
                errorRsp->error                                 = BLC_ESLSS_ERROR_CODE_INVALID_OPCODE;
#if APP_VENDOR_IMAGE
            }
#endif

            if (command->eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                rspLen = blc_esl_getResponseSize((blc_eslss_controlPointResponseHdr_t *)rsp);
            }
            break;
        }
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_SERVICE_RESET:
        {
            serviceNeededTick = clock_time();
            serviceNeeded     = false;

            if (command->eslId != BLC_ESLS_ESL_ID_BROADCAST) {
                rspLen = updateBasicState((blc_eslss_controlPointResponseHdr_t *)rsp);
            }

            break;
        }
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_UNASSOCIATE_FROM_AP:
            rspLen = updateBasicState((blc_eslss_controlPointResponseHdr_t *)rsp);
            // TODO: We should disconnect after sending notification. Currently set timer and discnnect then
            pendingUnassociate = true;
            reloadUnassociateTimer(1000 * 1000, cmd->handle);
            break;
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_FACTORY_RESET:
            // No response required, we should disconnect
            pendingUnassociate = true;
            blc_ll_disconnect(cmd->handle.connHandle, HCI_ERR_REMOTE_USER_TERM_CONN);
            break;
        case BLC_ESLSS_CONTROL_POINT_COMMAND_OPCODE_UPDATE_COMPLETE:
            // No response required
            break;
        }

        if (rspLen) {
            rspLeft--;
            rspIndex++;

            if ((unsigned int)(rspLen + (rspLeft * BLC_ESLS_CMD_RSP_MIN_LENGTH)) > (unsigned int)(sizeof(rspBuf) - (((u8 *) response) - rspBuf))) {
                // We can't fit this response, send error instead
                blc_eslss_controlPointResponseError_t *errorRsp = (blc_eslss_controlPointResponseError_t *)rsp;
                errorRsp->hdr.opcode                            = BLC_ESLSS_CONTROL_POINT_RESPONSE_OPCODE_ERROR;
                errorRsp->error                                 = BLC_ESLSS_ERROR_CODE_CAPACITY_LIMIT;
                rspLen                                          = blc_esl_getResponseSize(&errorRsp->hdr);
            }

            memcpy((u8 *)response, rsp, rspLen);
        }

        response = (blc_eslss_controlPointResponseHdr_t *)(((u8 *)response) + rspLen);
        command  = (blc_eslss_controlPointCommandHdr_t *)(((u8 *)command) + blc_esl_getCommandSize(command));
    }

    if (rspIndex) {
        blc_eslp_esl_controlPointResponse(cmd->handle, rspIndex, (blc_eslss_controlPointResponseHdr_t *)rspBuf);
    }

    extern void app_process_power_management(void);
    app_process_power_management();
}

static int app_esl_prfEvtCb(u16 aclHandle, int evtID, u8 *pData, u16 dataLen)
{
    (void)dataLen;
    (void)aclHandle;

    switch (evtID) {
    case ESL_EVT_ESLP_ESL_CURRENT_ABSOLUTE_TIME:
    {
        blc_eslp_esl_currentAbsoluteTimeEvt_t *pEvt = (blc_eslp_esl_currentAbsoluteTimeEvt_t *)pData;

        currentTime.active   = true;
        currentTime.timer    = pEvt->time;
        currentTime.lastTick = clock_time();
        break;
    }
    case ESL_EVT_ESLP_ESL_CONTROL_POINT_CMD:
        app_esl_handleControlPointCmds((blc_eslp_esl_controlPointCommant_t *)pData);
        break;
    case ESL_EVT_ESLP_ESL_STATE:
    {
        blc_eslp_esl_stateEvt_t *pEvt = (blc_eslp_esl_stateEvt_t *)pData;

        if (pEvt->state == BLC_ESLS_STATE_UNSYNCHRONIZED) {
#if (LEGACY_ADV_SEND)
            blc_ll_setAdvEnable(BLC_ADV_ENABLE); //ADV enable
#else
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
#endif
        } else if (pEvt->state == BLC_ESLS_STATE_UNASSOCIATED) {
            // Remove bonding information
            blc_smp_eraseAllBondingInfo();
#if (LEGACY_ADV_SEND)
            blc_ll_setAdvEnable(BLC_ADV_ENABLE); //ADV enable
#else
            blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);
#endif
        } else if (pEvt->state == BLC_ESLS_STATE_UPDATING ||
                   pEvt->state == BLC_ESLS_STATE_CONFIGURING ||
                   pEvt->state == BLC_ESLS_STATE_SYNCHRONIZED) {
#if (LEGACY_ADV_SEND)
            blc_ll_setAdvEnable(BLC_ADV_DISABLE); //ADV enable
#else
            blc_ll_setExtAdvEnable(BLC_ADV_DISABLE, ADV_HANDLE0, 0, 0);
#endif
        }

        state = pEvt->state;
        break;
    }
    default:
        break;
    }

    return 0;
}

static bool currentTimeExpired(u32 prev, u32 next, u32 expirationTime)
{
    if (next > prev) {
        return (expirationTime > prev) && (expirationTime <= next);
    }

    // roll over
    return (expirationTime <= next) || (expirationTime > prev);
}

static void currentTimeLoop(void)
{
    if (currentTime.active && clock_time_exceed(currentTime.lastTick, 1000)) {
        // increase timer and check if any pending command timer expired
        u32 prev = currentTime.timer;
        u32 tick = clock_time();

        currentTime.timer += ((tick - currentTime.lastTick) / (SYSTEM_TIMER_TICK_1US * 1000));
        currentTime.lastTick = tick;

        for (u8 i = 0; i < ARRAY_SIZE(ledControlData); i++) {
            ledControlData_t *ledControl = &ledControlData[i];
            app_led_config_t  config;

            if (ledControl->pendingUpdate && currentTimeExpired(prev, currentTime.timer, ledControl->scheduledCommandTime)) {
                // Move pending command to current command
                app_led_command_apply(i, &ledControl->scheduledCommand);
                tlkapi_printf(APP_LOG_EN, "LED [%d] Applying new command", i);
                if (app_led_get_info(i, &config) && config.type == APP_LED_TYPE_SRGB) {
                    // Need to update LED in case of RGB LED
                    app_esl_leds_fill_led_information();
                    blc_eslss_updateEslLedInformation(0xFFFF, min(app_led_get_num_leds(), ARRAY_SIZE(ledInformation)), ledInformation);
                }
                ledControl->pendingUpdate = false;
            }
        }

        for (u8 i = 0; i < ARRAY_SIZE(displayControlData); i++) {
            displayControl_t *displayControl = &displayControlData[i];

            if (displayControl->pendingUpdate && currentTimeExpired(prev, currentTime.timer, displayControl->pendingUpdateAbsoluteTime)) {
                displayControl->newImage      = true;
                displayControl->active        = true;
                displayControl->imageIdx      = displayControl->pendingUpdateImageIdx;
                displayControl->pendingUpdate = false;
            }
        }
    }

    if (!serviceNeeded && clock_time_exceed(serviceNeededTick, 30 * 1000 * 1000)) {
        serviceNeeded = true;
    }
}

void app_esl_handle_controller_event(u32 h, u8 *p, int n)
{
    blc_eslp_esl_stackEventCallback(h, p, n);

    if (h & HCI_FLAG_EVENT_BT_STD) {
        u8 evtCode = h & 0xff;
        if (evtCode == HCI_EVT_DISCONNECTION_COMPLETE) {
            stopUnassociateTimer();
            if (state == BLC_ESLS_STATE_UNASSOCIATED || state == BLC_ESLS_STATE_UNSYNCHRONIZED) {
#if (LEGACY_ADV_SEND)
                blc_ll_setAdvEnable(BLC_ADV_ENABLE); //ADV enable
#else
                blc_ll_setExtAdvEnable(BLC_ADV_ENABLE, ADV_HANDLE0, 0, 0);

#endif
            }
            if (pendingUnassociate) {
                pendingUnassociate = false;
                stopUnassociateTimer();
                blc_eslp_esl_unassociate();
            }
        }
    }
}

void app_esl_handle_host_event(u32 h, u8 *p, int n)
{
    blc_eslp_esl_hostEventCallback(h, p, n);
    blc_otss_hostEventCallback(h, p, n);
}

static u16 ots_read_cb(blc_ots_object_id_t *id, u16 offset, u16 length, u8 **outData)
{
    _attribute_data_retention_ static u8 buf_temp[OTSS_L2CAP_MTU];

    foreach_arr(i, objectIds)
    {
        if (blc_ots_object_id_equal(&objectIds[i], id)) {
            u32 ret  = app_image_storage_get_image_data(i, offset, min(length, sizeof(buf_temp)), buf_temp);
            *outData = buf_temp;
            return ret;
        }
    }

    return 0;
}

static u16 ots_write_cb(blc_ots_object_id_t *id, u8 mode, u16 offset, u16 length, u8 *data)
{
    foreach_arr(i, objectIds)
    {
        if (blc_ots_object_id_equal(&objectIds[i], id)) {
            u16 ret = app_image_storage_image_write(i, length, offset, data, !!(mode & BLC_OTS_OACP_WRITE_MODE_TRUNCATE));
            if (mode & BLC_OTS_OACP_WRITE_MODE_TRUNCATE) {
                app_image_storage_update_image_info(i, offset + length);
            }

            return ret;
        }
    }

    return 0;
}

static const blc_otss_object_callbacks_t ots_callbacks = {
    .read_cb  = ots_read_cb,
    .write_cb = ots_write_cb,
};

static void app_esl_displays_init(void)
{
#if (HARDWARE_BOARD_SELECT != HW_EVK)
    app_display_register(app_display_epd_get_iface(), NULL);
#else
    app_display_register(app_display_dummy_get_iface(), NULL);
#endif

    for (u8 i = 0; i < app_display_get_num_displays(); i++) {
        app_display_clean(i);
    }

    for (u8 i = 0; i < min(app_display_get_num_displays(), ARRAY_SIZE(displayData)); i++) {
        app_display_get_info(i, &displayData[i].width, &displayData[i].height, &displayData[i].displayType, NULL);
    }

    eslsRegParam.displayData    = displayData;
    eslsRegParam.displayDataNum = min(app_display_get_num_displays(), ARRAY_SIZE(displayData));
}

static void app_esl_leds_init(void)
{
    app_led_register(app_led_mono_red_get_iface(), NULL);
    app_led_register(app_led_mono_green_get_iface(), NULL);
    //    app_led_register(app_led_rgb_dummy_get_iface(), NULL);

    app_esl_leds_fill_led_information();

    eslsRegParam.ledInfo            = ledInformation;
    eslsRegParam.ledInformationsNum = min(app_led_get_num_leds(), ARRAY_SIZE(ledInformation));
}

static void app_esl_sensors_init(void)
{
    u8 sensor_num = 0;

    app_sensor_register(app_sensor_dummy_type0_get_iface(), NULL);
    //  app_sensor_register(app_sensor_dummy_type1_get_iface(), NULL);

    for (u8 i = 0; i < min(app_sensor_get_num_sensors(), ARRAY_SIZE(sensorInformation)); i++) {
        app_sensor_config_t config;

        if (!app_sensor_get_info(i, &config)) {
            continue;
        }

        sensorInformation[i].size = config.size == APP_SENSOR_SIZE_0 ? BLC_ESLS_SENSOR_INFORMATION_SIZE_0 : BLC_ESLS_SENSOR_INFORMATION_SIZE_1;
        if (sensorInformation[i].size == BLC_ESLS_SENSOR_INFORMATION_SIZE_0) {
            sensorInformation[i].sensorType0 = config.size0;
        } else {
            sensorInformation[i].sensorType1 = config.size1;
        }

        sensor_num++;
    }

    eslsRegParam.sensorInfo            = sensorInformation;
    eslsRegParam.sensorInformationsNum = min(sensor_num, ARRAY_SIZE(sensorInformation));
}

static void app_esl_add_objects(void)
{
    u8 maxImageNum = 0;

    for (u8 i = 0; i < APP_IMAGE_STORAGE_MAX_IMAGES; i++) {
        uuid_t type = {
            .uuidLen     = ATT_16_UUID_LEN,
            .uuidVal.u16 = 0xabcd, //TODO: set type of image
        };
        u32 props = BLC_OTS_OBJECT_PROPERTIES_READ |
                    BLC_OTS_OBJECT_PROPERTIES_WRITE |
                    BLC_OTS_OBJECT_PROPERTIES_PATCH |
                    BLC_OTS_OBJECT_PROPERTIES_TRUNCATE;
        blc_ots_object_size_t size = {
            .currentSize   = 0,
            .allocatedSize = app_image_storage_get_max_image_length(),
        };

        u32 temp_current_size = 0;
        app_image_storage_get_image_length(i, &temp_current_size);
        size.currentSize = temp_current_size;

        if (blc_otss_objectAdd(&size, &type, props, &objectIds[i]) == BLE_SUCCESS) {
            u64 obj_id = bstream_to_u48_le(objectIds[i].objectId);
            u8  objectName[0x20];
            u16 objectNameLen;

            //objectNameLen = snprintf(objectName, sizeof(objectName), "image obj_id:%llu idx:%d", obj_id, i);
            objectNameLen = snprintf((char *)objectName, sizeof(objectName), "id:%llu", obj_id);
            blc_otss_objectSetName(&objectIds[i], objectName, objectNameLen);

            maxImageNum++;
        } else {
            tlkapi_printf(APP_LOG_EN, "Failed to add object");
        }
    }

#if APP_VENDOR_IMAGE
    // Add one more object to represent the vendor image
    uuid_t type = {
        .uuidLen     = ATT_16_UUID_LEN,
        .uuidVal.u16 = 0xabcd, //TODO: set type of image
    };
    u32                   props = 0;
    blc_ots_object_size_t size  = {
         .currentSize   = app_vendor_image_get_image_size(),
         .allocatedSize = app_vendor_image_get_image_size(),
    };


    if (blc_otss_objectAdd(&size, &type, props, &vendorObjectId) == BLE_SUCCESS) {
        u64 obj_id = bstream_to_u48_le(vendorObjectId.objectId);
        u8  objectName[0x20];
        u16 objectNameLen;

        objectNameLen = snprintf((char *)objectName, sizeof(objectName), "id:%llu", obj_id);
        blc_otss_objectSetName(&vendorObjectId, objectName, objectNameLen);

        app_vendor_image_idx = maxImageNum;
        maxImageNum++;
    } else {
        tlkapi_printf(APP_LOG_EN, "Failed to add vendor image object");
    }
#endif

    eslsRegParam.maxImageIndex = maxImageNum > 0 ? maxImageNum - 1 : maxImageNum;
}
#define HOST_MALLOC_BUFF_SIZE      (512)

static _attribute_data_retention_ u8 hostMallocBuffer[HOST_MALLOC_BUFF_SIZE];
void app_esl_init(void)
{
    app_esl_displays_init();
    app_esl_leds_init();
    app_esl_sensors_init();
    app_image_storage_init();

    blc_esl_registerOTSControlServer(&ots_params);
    blc_otss_registerCallbacks(&ots_callbacks);

    app_esl_add_objects();

    blc_eslp_esl_registerElectronicShelfLabel(&electronicShelfLabelParams);
    blc_basic_registerDISControlServer(NULL);
    blc_svc_addCoreGroup();

    blc_prf_initialModule(app_esl_prfEvtCb,hostMallocBuffer,HOST_MALLOC_BUFF_SIZE);
}

void app_esl_loop(void)
{
    blc_eslp_esl_loop();

    currentTimeLoop();
    checkUnassociateTimer();

    for (u8 i = 0; i < ARRAY_SIZE(displayControlData); i++) {
        displayControlDataLoop(i);
    }

    app_display_loop();
    app_led_loop();
    app_sensor_loop();
}

bool app_esl_task_isBusy(void)
{
    bool LED_BUSY_FLAG     = false;
    bool DISPLAY_BUSY_FLAG = false;

    for (u8 i = 0; i < APP_DISPLAY_MAX_DISPLAYS; i++) {
        DISPLAY_BUSY_FLAG |= app_display_is_busy(i);
    }

    for (u8 i = 0; i < APP_LED_MAX_LEDS; i++) {
        if (app_led_is_active(i)) {
            LED_BUSY_FLAG = true;
            break;
        }
    }

    return (LED_BUSY_FLAG || DISPLAY_BUSY_FLAG);
}
