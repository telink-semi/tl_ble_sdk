/********************************************************************************************************
 * @file    app_audio.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "app_config.h"
#include "application/audio/audio_config.h"
#include "stack/ble/ble.h"
#include "app.h"
#include "app_att.h"
#include "app_audio.h"
#include "vendor/common/blt_led.h"
#include "battery_check.h"






#if (BLE_AUDIO_ENABLE)
_attribute_data_retention_  u8      audio_start = 0;
extern u8       buffer_mic_pkt_wptr;
extern u8       buffer_mic_pkt_rptr;
_attribute_data_retention_  u8      ui_mic_enable = 0;
_attribute_data_retention_  u8      key_voice_press = 0;
_attribute_data_retention_  int     ui_mtu_size_exchange_req = 0;

_attribute_data_retention_  u32     audioProcStart_tick = 0;
_attribute_data_retention_  u32     audioProcDelay_us = 0;

_attribute_data_retention_ u32  key_voice_pressTick = 0;
u32     audio_stick = 0;

extern s16 buffer_mic[TL_MIC_BUFFER_SIZE >> 1] __attribute__((aligned(4)));

void proc_mic_encoder(void);
int *mic_encoder_data_buffer(void);
void mic_encoder_data_read_ok(void);
#if ((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) \
               || (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID) )   //HID Service,ADPCM
    void audio_mic_param_init(void);
#endif

audio_codec_stream0_input_t audio_codec_stream0_input =
    {
        .input_src     = AMIC_STREAM0_MONO_L,
        .sample_rate   = AUDIO_16K,
        .data_width    = CODEC_BIT_16_DATA,
        .fifo_chn      = FIFO0,
        .dma_num       = DMA2,
        .data_buf      = buffer_mic,
        .data_buf_size = TL_MIC_BUFFER_SIZE,
};

/**
 * @brief      the func serve to init amic
 * @param[in]  none
 * @return     none
 */
void amic_gpio_reset (void)
{
    gpio_set_input_en(GPIO_AMIC_BIAS, 0);
    gpio_set_output_en(GPIO_AMIC_BIAS, 1);
    gpio_write(GPIO_AMIC_BIAS, 0);

    gpio_set_input_en(GPIO_AMIC_SP, 0);
    gpio_set_output_en(GPIO_AMIC_SP, 1);
    gpio_write(GPIO_AMIC_SP, 0);

    gpio_set_input_en(GPIO_AMIC_SN, 0);
    gpio_set_output_en(GPIO_AMIC_SN, 1);
    gpio_write(GPIO_AMIC_SN, 0);

}


/**
 * @brief       This function serves to fade audio pga gain
 * @param[in]   gain          - audio channel select.
 * @return      none.
 */
void audio_fade_pga_gain(codec_in_pga_gain_e gain)
{
    codec_in_pga_gain_e value = audio_get_adc_pga_gain();

    if (gain > value) {
        for (value++; (signed int)value <= (signed int)gain; value++) {
            delay_ms(1);
            audio_set_adc_pga_gain(value);
        }
    } else if (gain < value) {
        for (value--; (signed int)value >= (signed int)gain; value--) {
            delay_ms(1);
            audio_set_adc_pga_gain(value);
        }
    }
}

/**
 * @brief       This function serves to fade stream digital gain
 * @param[in]   gain          - audio channel select.
 * @return      none.
 */
void audio_stream0_fade_dig_gain(codec_in_path_digital_gain_e gain)
{
    codec_in_path_digital_gain_e value = audio_get_stream0_dig_gain();

    if (gain > value) {
        for (value += 4; (signed int)value <= (signed int)gain; value += 4) {
            delay_ms(1);
            audio_set_stream0_dig_gain(value);
        }
    } else if (gain < value) {
        for (value -= 4; (signed int)value >= (signed int)gain; value -= 4) {
            delay_ms(1);
            audio_set_stream0_dig_gain(value);
        }
    }
}

/**
 * @brief      This function is the microphone delay function.
 * @param[in]  delay_time: microphone delay duration, unit is us.
 * @return     none
 */
void audio_proc_delay(u32 delay_time_us)
{
    audioProcDelay_us = delay_time_us;
}

#if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TELINK)                   //GATT Telink
/**
 * @brief      for open the audio and mtu size exchange
 * @param[in]  none
 * @return     none
 */
void voice_press_proc(void)
{
    key_voice_press = 0;
    audio_stick = clock_time()|1;
    ui_enable_mic (1);
    for (int i = ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++) { //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
        if (ui_mtu_size_exchange_req && conn_dev_list[i].conn_state) {
            ui_mtu_size_exchange_req = 0;
            blc_att_requestMtuSizeExchange(conn_dev_list[i].conn_handle, 0x009e);
        }
    }
}


#elif (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)                 // GATT GOOGLE

#include "application/audio/gl_audio.h"
extern u16      app_audio_sync_serial;
extern u32      app_audio_timer;

_attribute_data_retention_ u8   audio_send_index = 0;
#elif ((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) \
       || (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID) )   //HID Service,ADPCM

u8      audio_start_status = 0;
u8      audio_end_status = 0;
u8      audio_send_idx = 0;
u32     audio_end_tick = 0;

_attribute_data_retention_  u8  audio_bt_status = 0;
/**
 * @brief      this function is call back function of audio measurement from server to client
 * @param[in]  p:data pointer.
 * @return     will always return 0
 */
int server2client_auido_proc(void* p)
{
    rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;

    if (pw->dat[0] == APP_AUDIO_BT_OPEN){  //audio start
        if(!ui_mic_enable){
            audio_bt_status = APP_AUDIO_BT_OPEN;
        }

    }
    else if(pw->dat[0] == APP_AUDIO_BT_CLOSE){
        audio_bt_status = APP_AUDIO_BT_CLOSE;
    }
    return 0;
}
#endif


/**
 * @brief      for open the audio and mtu size exchange
 * @param[in]  en   0:close the micphone  1:open the micphone
 * @return     none
 */
void ui_enable_mic (int en)
{
#if (BATT_CHECK_ENABLE)
    battery_set_detect_enable(!en);
#endif
    ui_mic_enable = en;

    #if (UI_LED_ENABLE)
        extern const led_cfg_t led_cfg[];
        device_led_setup(led_cfg[en ? 1 : 2]);
    #endif

    if(en){  //audio on
        #if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
            app_audio_sync_serial = 0;
            bls_pm_setManualLatency(0);
            blc_pm_setSleepMask(PM_SLEEP_DISABLE);
            app_audio_timer = clock_time() | 1;
            audio_stick = clock_time() | 1;
            extern u16  adpcm_serial_num;
            adpcm_serial_num = 0;
            audio_send_index = 0;
            extern u16 buffer_mic_rptr;
            buffer_mic_rptr = 0;
            memset(buffer_mic, 0, TL_MIC_BUFFER_SIZE);
        #elif ((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) \
               || (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID) )   //HID Service,ADPCM
            audio_mic_param_init();
            audio_send_idx = 0;
            audio_end_tick = 0;
        #endif
        ///////////////////// AUDIO initialization///////////////////
        audio_init();
        buffer_mic_pkt_rptr = buffer_mic_pkt_wptr = 0;
        audio_set_amic_bias_pin(GPIO_AMIC_BIAS);
        gpio_set_output(GPIO_AMIC_SP, 0);
        gpio_set_output(GPIO_AMIC_SN, 0);

        /****stream0 amic/dmic init****/
        audio_codec_stream0_input_init(&audio_codec_stream0_input);

        /****rx  dma init****/
        audio_rx_dma_chain_init(audio_codec_stream0_input.fifo_chn, audio_codec_stream0_input.dma_num, (unsigned short *)audio_codec_stream0_input.data_buf, audio_codec_stream0_input.data_buf_size);

        audio_mic_mute_en();                                             /* Step1 - mute audio*/
        audio_codec_stream0_input_en(audio_codec_stream0_input.dma_num); /* Step2 - enable audio codec */
        audio_codec_clr_input_pop(20);                                 /* Step3 - Clear codec input pop and dis mute audio */
        audio_codec_input_path_en(audio_codec_stream0_input.fifo_chn); /* Step4 - enable codec input path, codec data come in */
        audio_fade_pga_gain(CODEC_IN_GAIN_9P0_DB);

    #if defined(IIR_FILTER_ENABLE) && IIR_FILTER_ENABLE
        //only used for debugging EQ Filter parameters, removed after mass production
        extern void filter_setting(void);
        filter_setting();
    #endif
        if(audioProcDelay_us)
        {
            audioProcStart_tick = clock_time()|1;
        }
    }
    else{  //audio off
//        audio_stream0_fade_dig_gain(CODEC_IN_D_GAIN_m48_DB);
        audio_codec_stream0_input_dis(audio_codec_stream0_input.dma_num);
        audio_codec_adc_power_down();
        audio_power_down();
        amic_gpio_reset();
        buffer_mic_pkt_rptr = buffer_mic_pkt_wptr = 0;
        #if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
            audio_stick = 0;
            app_audio_timer = 0;
            audio_start = 0;
        #elif ((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) \
               || (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID) )   //HID Service,ADPCM
            audio_start_status = 0;
            audio_end_status = 0;
            audio_stick = 0;
            audio_end_tick = 0;
            audio_bt_status = APP_AUDIO_BT_CLOSE;
        #endif
    }
}

/**
 * @brief      this function is used to define what to do when voice key is pressed
 * @param[in]  none
 * @return     none
 */
void key_voice_is_press(void)
{
#if(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TELINK)
    if(ui_mic_enable){  //if voice on, voice off
        ui_enable_mic (0);
    }
    else{ //if voice not on, mark voice key press tick
        key_voice_press = 1;
        key_voice_pressTick = clock_time();
    }
#elif(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
    for (int i = ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++) { //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
        if (!ui_mic_enable && conn_dev_list[i].conn_state) {
            #if GOOGLE_AUDIO_DLE
                u8 ret = blc_att_requestMtuSizeExchange(conn_dev_list[i].conn_handle, 0x009e);
            #endif
            if(app_audio_key_start(conn_dev_list[i].conn_handle, 1) == APP_AUDIO_ENABLE){
                ui_enable_mic(1);
            }
        }
    }
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID))
    if(!key_voice_press)
    {
        key_voice_pressTick = clock_time() | 1;//first press tick
        key_voice_press = 1;
        for (int i = ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++) { //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
            if (!ui_mic_enable && conn_dev_list[i].conn_state) {
                if(audio_start_status == 0)
                {
                    u8 value[20]={0x99, 0x99, 0x99, 0x21, };// AC_SEARCH
                    if(blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
                        audio_start_status = 2; //push notify fail
                    }
                    else{
                        audio_start_status = 1; //push notify success
                    }
                }
            }
        }
    }
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID))
    for (int i = ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++) { //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
        if (!ui_mic_enable && conn_dev_list[i].conn_state) {
            if(audio_start_status == 0){
                u8 value[20]={0x99, 0x99, 0x99, 0x31, };// AC_SEARCH
                if(blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
                    audio_start_status = 2; //push notify fail
                }
                else{
                    audio_start_status = 1; //push notify success
                }
            }
        }
    }
#endif
}
/**
 * @brief      this function is used to define what to do when voice key is released
 * @param[in]  none
 * @return     none
 */
void key_voice_is_release(void)
{
#if(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TELINK)
    if(ui_mic_enable){
        ui_enable_mic(0);
    }
#elif(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
    extern u8 app_audio_key_flags;
    extern u8 htt_audio_model_key_press_flags;

    if((app_audio_key_flags & APP_AUDIO_KEY_FLAG_PRESS) || htt_audio_model_key_press_flags)
    {
        app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_PRESS;

        for (int i = ACL_CENTRAL_MAX_NUM; i < (ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM); i++) { //peripheral index is from "ACL_CENTRAL_MAX_NUM" to "ACL_CENTRAL_MAX_NUM + ACL_PERIPHR_MAX_NUM - 1"
            if (ui_mic_enable && conn_dev_list[i].conn_state) {
                if(app_audio_key_start(conn_dev_list[i].conn_handle, 0) == APP_AUDIO_DISABLE){
                    ui_enable_mic(0);
                }
            }
        }
    }

#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID))
    if(key_voice_press)
    {
        key_voice_pressTick = 0;
        key_voice_press = 0;
        if(ui_mic_enable && (audio_end_status == 0)){
            u8 value[20]={0x99, 0x99, 0x99, 0x24, };// AC_SEARCH
            for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
                if(conn_dev_list[i].conn_state != 0){
                    if (blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
                        audio_end_status = 2;   //push notify fail
                    }
                    else{
                        audio_end_status = 1;   //push notify success
                    }
                    audio_end_tick = clock_time() | 1;
                }
            }
        }
    }
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID))
    if(ui_mic_enable && (audio_end_status == 0)){
        u8 value[20]={0x99, 0x99, 0x99, 0x34, };// AC_SEARCH
        for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
            if(conn_dev_list[i].conn_state != 0){
                if (blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
                    audio_end_status = 2;   //push notify fail
                }
                else{
                    audio_end_status = 1;   //push notify success
                }
                audio_end_tick = clock_time() | 1;
            }
        }
    }
#endif
}

/**
 * @brief      this function is used to check audio state
 * @param[in]  none
 * @return     none
 */
void audio_state_check(void)
{
#if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TELINK)
     //long press voice 1 second
    for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
        if(conn_dev_list[i].conn_state != 0){
            if(key_voice_press && !ui_mic_enable && clock_time_exceed(key_voice_pressTick, 1000000)){
                voice_press_proc();
            }
        }
    }

#elif(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
    // Do nothing
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID))
    if (audio_start_status == 2){//press
        u8 value[20]={0x99, 0x99, 0x99, 0x21, };// AC_SEARCH
        for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
            if(conn_dev_list[i].conn_state != 0){
                if(!blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
                    audio_start_status = 2; //push notify success
                }
            }
        }
    }
    if(audio_end_status == 2){//release
        u8 value[20]={0x99, 0x99, 0x99, 0x24, };// AC_SEARCH
        for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
            if(conn_dev_list[i].conn_state != 0){
                if(!blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
                    audio_start_status = 0; //push notify success
                }
            }
        }
    }
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID))
    if (audio_start_status == 2){//press
        u8 value[20]={0x99, 0x99, 0x99, 0x31, };// AC_SEARCH
        for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
            if(conn_dev_list[i].conn_state != 0){
                if(!blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
                    audio_start_status = 2; //push notify success
                }
            }
        }
    }
    if(audio_end_status == 2){//release
        u8 value[20]={0x99, 0x99, 0x99, 0x34, };// AC_SEARCH
        for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
            if(conn_dev_list[i].conn_state != 0){
                if(!blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
                    audio_start_status = 0; //push notify success
                }
            }
        }
    }
#endif
}

/**
 * @brief      audio task in loop for encode and transmit encode data
 * @param[in]  none
 * @return     none
 */
void task_audio (void)
{
#if(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TELINK)
    static u32 audioProcTick = 0;
    if(clock_time_exceed(audioProcTick, 500)){
        audioProcTick = clock_time();
    }
    else{
        return;
    }
    if(audioProcStart_tick&&clock_time_exceed(audioProcStart_tick, audioProcDelay_us)){
        memset(buffer_mic, 0, TL_MIC_BUFFER_SIZE);
        audioProcStart_tick = 0;
    }
    else if(audioProcStart_tick!=0){
        return;
    }
    ///////////////////////////////////////////////////////////////
    proc_mic_encoder ();

    //////////////////////////////////////////////////////////////////
    for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
        if(conn_dev_list[i].conn_state != 0){
            if(blc_ll_getTxFifoNumber(conn_dev_list[i].conn_handle) < 9)
            {
                int *p = mic_encoder_data_buffer ();
                if (p)                  //around 3.2 ms @16MHz clock
                {
                    if( BLE_SUCCESS ==  blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, AUDIO_MIC_INPUT_DP_H, (u8*)p, ADPCM_PACKET_LEN)){
                        mic_encoder_data_read_ok();
                    }
                }
            }
            break;
        }
    }

#elif(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
    static u32 audioProcTick = 0;
        if(clock_time_exceed(audioProcTick, 500)){
            audioProcTick = clock_time();
        }
        else{
            return;
        }

        if(audioProcStart_tick&&clock_time_exceed(audioProcStart_tick, audioProcDelay_us)){
            memset(buffer_mic, 0, TL_MIC_BUFFER_SIZE);
            audioProcStart_tick = 0;
        }
        else if(audioProcStart_tick!=0){
            return;
        }


        ///////////////////////////////////////////////////////////////

        if(app_audio_timeout_proc()){
            return;
        }
        proc_mic_encoder ();
        ///////////////////////////////0.4e///////////////////////////////////
    #if (GOOGLE_AUDIO_VERSION == GOOGLE_AUDIO_V0P4)
        u8 audio_send_length;
        for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
            if(conn_dev_list[i].conn_state != 0){
                if(blc_ll_getTxFifoNumber(conn_dev_list[i].conn_handle) < 9) {
                    int *p = mic_encoder_data_buffer ();
                    if(p)
                    {
                        for(u8 j=0; j<7; j++)
                        {
                            if(audio_send_index < 6)
                            {
                                audio_send_length = 20;
                            }
                            else if(audio_send_index == 6)
                            {
                                audio_send_length = 14;
                            }
                            else
                            {
                                audio_send_length = 0;
                            }
                            if(BLE_SUCCESS == blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle,AUDIO_GOOGLE_RX_DP_H, (u8*)p+audio_send_index*20, audio_send_length))
                            {
                                audio_send_index++;
                            }
                            else
                            {
                                return ;
                            }
                            if(audio_send_index == 7)
                            {
                                audio_send_index = 0;
                                mic_encoder_data_read_ok();
                            }
                        }
                    }
                }
            }
        }
    #endif

        ////////////////////////////v 1.0////////////////////////////////////////////
    #if (GOOGLE_AUDIO_VERSION == GOOGLE_AUDIO_V1P0)

        // can send data 20 bytes or dle 120 bytes
    #if GOOGLE_AUDIO_DLE
        for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
            if(conn_dev_list[i].conn_state != 0){
                if(blc_ll_getTxFifoNumber(conn_dev_list[i].conn_handle) < 9) {
                    int *p = mic_encoder_data_buffer();
                    u8 dataLen = 120;
                    if(p)
                    {
                        u8 ret = blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle,AUDIO_GOOGLE_RX_DP_H, (u8*)p, dataLen);
                        if(BLE_SUCCESS == ret){
                            mic_encoder_data_read_ok();
                        } else{
                            return;
                        }
                    }
                }

            }
        }
    #else
        for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
            if(conn_dev_list[i].conn_state != 0){
                if(blc_ll_getTxFifoNumber(conn_dev_list[i].conn_handle) < 9)
                {
                    int *p = mic_encoder_data_buffer ();
                    u8 dataLen = 20;
                    if (p)
                    {
                        for(int j = audio_send_index; j < 6; j++)
                        {
                            if(BLE_SUCCESS == blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle,AUDIO_GOOGLE_RX_DP_H, (u8*)p+audio_send_index*dataLen, dataLen))
                            {
                                audio_send_index++;
                            } else {
                                return;
                            }
                            if(audio_send_index == 6)
                            {
                                audio_send_index = 0;
                                mic_encoder_data_read_ok();
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }
    #endif  //#if GOOGLE_AUDIO_DLE
    #endif  //#if (GOOGLE_AUDIO_VERSION == GOOGLE_AUDIO_V1P0)
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID))
    static u32 audioProcTick = 0;
    if(clock_time_exceed(audioProcTick, 500)){
        audioProcTick = clock_time();
    }
    else{
        return;
    }
    if(audioProcStart_tick&&clock_time_exceed(audioProcStart_tick, audioProcDelay_us)){
        memset(buffer_mic, 0, TL_MIC_BUFFER_SIZE);
        audioProcStart_tick = 0;
    }
    else if(audioProcStart_tick!=0){
        return;
    }

    ///////////////////////////////////////////////////////////////


    proc_mic_encoder ();

    //////////////////////////////////////////////////////////////////
    for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
        if(conn_dev_list[i].conn_state != 0){
            if(blc_ll_getTxFifoNumber(conn_dev_list[i].conn_handle) < 8 + audio_send_idx){
                int *p = mic_encoder_data_buffer ();
                if (p)                  //around 3.2 ms @16MHz clock
                {
                    u8 j =0;
                    u8 handle;
                    u8 *buff;
                    for(j=0;j<12;j++){
                        handle = HID_AUDIO_REPORT_INPUT_FIRST_DP_H + (audio_send_idx%3)*4;//+ (audio_send_idx%3)*4
                        buff = ((u8*)(p))+20*audio_send_idx;
                        if(BLE_SUCCESS == blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle,handle, buff, 20)){
                            audio_send_idx++;
                        }
                        if(audio_send_idx == 6){
                            audio_send_idx = 0;
                            buffer_mic_pkt_rptr++;
                            return;
                        }
                    }
                }
            }
        }
    }
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID))
    static u32 audioProcTick = 0;
    if(clock_time_exceed(audioProcTick, 500)){
        audioProcTick = clock_time();
    }
    else{
        return;
    }

    if(audioProcStart_tick&&clock_time_exceed(audioProcStart_tick, audioProcDelay_us)){
        memset(buffer_mic, 0, TL_MIC_BUFFER_SIZE);
        audioProcStart_tick = 0;
    }
    else if(audioProcStart_tick!=0){
        return;
    }

    ///////////////////////////////////////////////////////////////
    proc_mic_encoder ();

    //////////////////////////////////////////////////////////////////
    for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
        if(conn_dev_list[i].conn_state != 0){
            if(blc_ll_getTxFifoNumber(conn_dev_list[i].conn_handle) < 12){
                int *p = mic_encoder_data_buffer ();
                if (p)                  //around 3.2 ms @16MHz clock
                {
                    u8 j =0;
                    u8 handle;
                    for(j=0;j<3;j++){
                        handle = HID_AUDIO_REPORT_INPUT_FIRST_DP_H + (audio_send_idx%3)*4;//+ (audio_send_idx%3)*4
                        #if((TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID))
                        if(BLE_SUCCESS == blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, handle, (u8*)p +3, 20)){
                        #elif(TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID)
                            if(BLE_SUCCESS == blc_gatt_pushHandleValueNotify (conn_dev_list[i].conn_handle, handle, (u8*)p, ADPCM_PACKET_LEN)){
                        #endif
                            audio_send_idx++;
                            buffer_mic_pkt_rptr++;
                            if(audio_send_idx == 3){
                                audio_send_idx = 0;
                            }
                            return;
                        }
                    }
                }
            }
        }
    }
#endif
}

/**
 * @brief      audio proc in main loop
 * @param[in]  none
 * @return     none
 */
void proc_audio(void){
#if ((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TELINK) || (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE) )
    if(ui_mic_enable){

        if(audio_start || (audio_stick && clock_time_exceed(audio_stick, 1*1000))){
            audio_start = 1;
            task_audio();
        }
    }
    else{
        audio_start = 0;
    }
#elif ((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) \
       || (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID) )   //HID Service,ADPCM
    if(!ui_mic_enable && key_voice_pressTick && clock_time_exceed(key_voice_pressTick,3*1000*1000)){
        key_voice_pressTick = 0;
        audio_start_status = 0;
        audio_end_status = 0;
        return;
    }
    if(ui_mic_enable && audio_stick && clock_time_exceed(audio_stick, 100*1000*1000)){
        ui_enable_mic (0);
        return;
    }
    if(!key_voice_press && ui_mic_enable && audio_end_tick && clock_time_exceed(audio_end_tick, 200*1000)){
        ui_enable_mic (0);
        return;
    }
    if((!ui_mic_enable) && audio_bt_status == APP_AUDIO_BT_OPEN){
        audio_stick = clock_time()|1;
        ui_enable_mic (1);
        for(int i = ACL_CENTRAL_MAX_NUM; i < ACL_PERIPHR_MAX_NUM+ACL_CENTRAL_MAX_NUM; i++){
            if(ui_mtu_size_exchange_req && (conn_dev_list[i].conn_state != 0)){
                ui_mtu_size_exchange_req = 0;
                blc_att_requestMtuSizeExchange(conn_dev_list[i].conn_handle, 0x009e);
            }
        }
    }
    if(ui_mic_enable){
        if(audio_start || (audio_stick && clock_time_exceed(audio_stick, 1*1000))){// for 8258
            audio_start = 1;
            task_audio();
        }
    }
    else{
        audio_start = 0;
    }
#endif
}

#endif  //(BLE_AUDIO_ENABLE)
