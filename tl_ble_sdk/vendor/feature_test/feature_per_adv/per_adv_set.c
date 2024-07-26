/********************************************************************************************************
 * @file    per_adv_set.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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


#if (FEATURE_TEST_MODE == TEST_PER_ADV)




/*********************************** Extended ADV data buffer allocation, Begin ************************************/
#define APP_EXT_ADV_SETS_NUMBER         1   //user set value
#define APP_EXT_ADV_DATA_LENGTH         1024 //2048//1664//1024   //user set value
#define APP_EXT_SCANRSP_DATA_LENGTH     1024 //2048//1664//1024   //user set value
_attribute_iram_bss_            u8  app_extAdvSetParam_buf[ADV_SET_PARAM_LENGTH * APP_EXT_ADV_SETS_NUMBER];
_attribute_iram_noinit_data_    u8  app_extAdvData_buf[APP_EXT_ADV_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];
_attribute_iram_noinit_data_    u8  app_extScanRspData_buf[APP_EXT_SCANRSP_DATA_LENGTH * APP_EXT_ADV_SETS_NUMBER];

/*********************************** Extended ADV data buffer allocation, End** ************************************/

/*********************************** Periodic ADV data buffer allocation, Begin ************************************/
#define APP_PERID_ADV_SETS_NUMBER       1   //1//EBQ test need to change it to the supported value
#define APP_PERID_ADV_DATA_LENGTH       100 //1024

_attribute_iram_bss_            u8  app_peridAdvSet_buffer[PERD_ADV_PARAM_LENGTH * APP_PERID_ADV_SETS_NUMBER];
_attribute_iram_noinit_data_    u8  app_peridAdvData_buffer[APP_PERID_ADV_DATA_LENGTH * APP_PERID_ADV_SETS_NUMBER];
/*********************************** Periodic ADV data buffer allocation, End** ************************************/

#if (APP_PAWR_ADV_TEST_EN)
/*********************************** Periodic ADV with Response data buffer allocation, Begin ************************************/
#define APP_PAWR_SUBEVT_DATA_LENGTH     100
#define APP_PAWR_SUBEVT_DATA_COUNT      4
_attribute_iram_noinit_data_    u8  app_peridAdvWrData_buffer[APP_PERID_ADV_SETS_NUMBER * APP_PAWR_SUBEVT_DATA_LENGTH * APP_PAWR_SUBEVT_DATA_COUNT];
/*********************************** Periodic ADV with Response data buffer allocation, End** ************************************/
#endif

_attribute_ble_data_retention_  ble_sts_t   ble_status = BLE_SUCCESS;
_attribute_ble_data_retention_  u8  testAdvData[APP_EXT_ADV_DATA_LENGTH];
_attribute_ble_data_retention_  u8  testScanRspData[APP_EXT_SCANRSP_DATA_LENGTH];

void app_periodic_adv_test(void)
{
    //////////// Extended ADV Initialization  Begin ///////////////////////
    /* Extended ADV module and ADV Set Parameters buffer initialization */
    blc_ll_initExtendedAdvModule_initExtendedAdvSetParamBuffer(app_extAdvSetParam_buf, APP_EXT_ADV_SETS_NUMBER);
    blc_ll_initExtendedAdvDataBuffer(app_extAdvData_buf, APP_EXT_ADV_DATA_LENGTH);
    blc_ll_initExtendedScanRspDataBuffer(app_extScanRspData_buf, APP_EXT_SCANRSP_DATA_LENGTH);
    u32  my_adv_interval_min = ADV_INTERVAL_200MS;
    u32  my_adv_interval_max = ADV_INTERVAL_200MS;
    // Extended, None_Connectable_None_Scannable undirected, with auxiliary packet
    blc_ll_setExtAdvParam( ADV_HANDLE0,         ADV_EVT_PROP_EXTENDED_NON_CONNECTABLE_NON_SCANNABLE_UNDIRECTED, my_adv_interval_min,            my_adv_interval_max,
                           BLT_ENABLE_ADV_ALL,  OWN_ADDRESS_PUBLIC,                                             BLE_ADDR_PUBLIC,                NULL,
                           ADV_FP_NONE,         TX_POWER_3dBm,                                                  BLE_PHY_1M,                     0,
                           BLE_PHY_1M,          ADV_SID_0,                                                      0);
    for(int i=0;i<1024;i++){
        testAdvData[i] = i;
    }

    blc_ll_setExtAdvData( ADV_HANDLE0,    31, (u8 *)testAdvData);
//  blc_ll_setExtAdvData( ADV_HANDLE0,   200, testAdvData);
//  blc_ll_setExtAdvData( ADV_HANDLE0,   400, testAdvData);
//  blc_ll_setExtAdvData( ADV_HANDLE0,   600, testAdvData);
//  blc_ll_setExtAdvData( ADV_HANDLE0,  1024, testAdvData);
    blc_ll_setExtAdvEnable( BLC_ADV_ENABLE, ADV_HANDLE0, 0 , 0);
    //////////////// Extended ADV Initialization End ///////////////////////




    //////////// Periodic Adv Initialization  Begin ////////////////////////
    blc_ll_initPeriodicAdvModule_initPeriodicdAdvSetParamBuffer(app_peridAdvSet_buffer, APP_PERID_ADV_SETS_NUMBER);
    blc_ll_initPeriodicAdvDataBuffer(app_peridAdvData_buffer, APP_PERID_ADV_DATA_LENGTH);
    u32  my_per_adv_itvl_min = PERADV_INTERVAL_200MS;
    u32  my_per_adv_itvl_max = PERADV_INTERVAL_200MS;

    blc_ll_setPeriodicAdvParam( ADV_HANDLE0, my_per_adv_itvl_min, my_per_adv_itvl_max, PERD_ADV_PROP_MASK_TX_POWER_INCLUDE);
    blc_ll_setPeriodicAdvData( ADV_HANDLE0, 100, testAdvData);
    blc_ll_setPeriodicAdvEnable( BLC_ADV_ENABLE, ADV_HANDLE0);
    //////////////// Periodic Adv Initialization End ///////////////////////





//  blc_ll_setAuxAdvChnIdxByCustomers(1); //debug chn_2406
//  blc_ll_setMaxAdvDelay_for_AdvEvent(MAX_DELAY_0MS);


#if 0
    /* enable BaseBand debug function */
    {
        ble_dbg_port_init(0);
       /*
        #define GPIO_CHN0                           GPIO_PE1
        #define GPIO_CHN1                           GPIO_PE2
        #define GPIO_CHN2                           GPIO_PA0
        #define GPIO_CHN3                           GPIO_PA4
        #define GPIO_CHN4                           GPIO_PA3
        #define GPIO_CHN5                           GPIO_PB0
        #define GPIO_CHN6                           GPIO_PB2  UART TX
        #define GPIO_CHN7                           GPIO_PE0
        #define GPIO_CHN8                           GPIO_PA2
        #define GPIO_CHN9                           GPIO_PA1
        #define GPIO_CHN10                          GPIO_PB1
        #define GPIO_CHN11                          GPIO_PB3  UART RX
        #define GPIO_CHN12                          GPIO_PC7
        #define GPIO_CHN13                          GPIO_PC6
        #define GPIO_CHN14                          GPIO_PC5
        #define GPIO_CHN15                          GPIO_PC4
       */
        /* 1. dbg_sel0: 0x80140803[5],  debug port switch
         * 2. dbg_sel1: 0x80140803[7],  debug port switch
         *
         *         dbg_sel0 = 1            dbg_sel0 = 0                dbg_sel1 = 1
         *
         *  PA0:    tx_en                   tx_en
         *  PA1:    tx_on                   tx_on
         *  PA2:    rx_en                   rx_en
         *  PA3:    clk_bb                  clk_bb
         *  PA4:    hit_sync                hit_sync
         *  PB0:    sclk                    sclk
         *  PB1:    tx_data                 tx_data
         *  PB2:    rx_data_vld             rx_data_vld
         *  PB3:    rx_data0                rx_data0                rx_symb0(for zigbee)
         *  PB4:                                                    rx_symb1(for zigbee)
         *  PB5:                                                    rx_symb2(for zigbee)
         *  PB6:                                                    rx_symb3(for zigbee)
         *  PB7:    ll_ss[0]                ll_ss[0]
         *  PC0:    ll_ss[1]                ll_ss[1]
         *  PC1:    ll_ss[2]                ll_ss[2]
         *  PC2:    ss[0]                   ss[0]
         *  PC3:    ss[1]                   ss[1]
         *  PC4:    ss[2]                   ss[2]
         *  PC5:    dma_ack_tx              dma_ack_rx
         *  PC6:    reg_wr                  reg_rd
         *  PC7:    dma_eof                 dma_err
         *  PD0:    dma_sof                 dma_cyc
         *  PD1:    dma_rdy_tx              dma_rdy_rx
         *
         *  PD4:    XXX: ?
         */

        /* ll_ss    state
            0       IDLE
            1       ACTIVE
            2       TXSTL
            3       TX
            4       RXWAIT
            5       RX
            6       TXWAIT

           bb_ss    state
            0       IDLE
            1       SYNC
            2       DEC
            3       HD
            4       FOOT
        */
        bt_dbg_set_pin(GPIO_PA0);  //tx_en          CHN2
        bt_dbg_set_pin(GPIO_PA1);  //tx_on          CHN9
        bt_dbg_set_pin(GPIO_PA2);  //rx_en          CHN8

        //bt_dbg_set_pin(GPIO_PA3);  //clk_bb         CHN4
        //bt_dbg_set_pin(GPIO_PA4);  //hit_sync       CHN3
        //bt_dbg_set_pin(GPIO_PB0);  //sclk           CHN5
        bt_dbg_set_pin(GPIO_PB1);  //tx_data        CHN10
        bt_dbg_set_pin(GPIO_PB3);  //rx_data0        CHN11

        /* ll_ss    state
            0       IDLE
            1       ACTIVE
            2       TXSTL
            3       TX
            4       RXWAIT
            5       RX
            6       TXWAIT
        */
    //    bt_dbg_set_pin(GPIO_PB7);  //ll_ss[0]       xxx
    //    bt_dbg_set_pin(GPIO_PC0);  //ll_ss[1]       xxx
    //    bt_dbg_set_pin(GPIO_PC1);  //ll_ss[2]       xxx

        /*  bb_ss    state
            0       IDLE
            1       SYNC
            2       DEC
            3       HD
            4       FOOT
        */
    //    bt_dbg_set_pin(GPIO_PC2);  //ss[0]          xxx
    //    bt_dbg_set_pin(GPIO_PC3);  //ss[1]          xxx
    //    bt_dbg_set_pin(GPIO_PC4);  //ss[2]          CHN15
    //
    //    bt_dbg_set_pin(GPIO_PC5);  //dma_ack_rx(dbg_sel0=0)     CHN14
    //
    //    bt_dbg_set_pin(GPIO_PC6);  //reg_rd         CHN13
    //    bt_dbg_set_pin(GPIO_PC7);  //dma_err        CHN12
    //    bt_dbg_set_pin(GPIO_PD0);  //dma_cyc        xxx
    //    bt_dbg_set_pin(GPIO_PD1);  //dma_rdy_rx     xxx
    }
#endif

    DBG_CHN0_TOGGLE;
    DBG_CHN1_TOGGLE;
    DBG_CHN2_TOGGLE;
    DBG_CHN3_TOGGLE;
    DBG_CHN4_TOGGLE;
    DBG_CHN5_TOGGLE;
    DBG_CHN6_TOGGLE;
    DBG_CHN7_TOGGLE;
    DBG_CHN8_TOGGLE;
    DBG_CHN9_TOGGLE;
    DBG_CHN10_TOGGLE;
    DBG_CHN11_TOGGLE;
    DBG_CHN12_TOGGLE;
    DBG_CHN13_TOGGLE;
    DBG_CHN14_TOGGLE;
    DBG_CHN15_TOGGLE;

    sleep_ms(500);
    DBG_CHN0_TOGGLE;
    DBG_CHN1_TOGGLE;
    DBG_CHN2_TOGGLE;
    DBG_CHN3_TOGGLE;
    DBG_CHN4_TOGGLE;
    DBG_CHN5_TOGGLE;
    DBG_CHN6_TOGGLE;
    DBG_CHN7_TOGGLE;
    DBG_CHN8_TOGGLE;
    DBG_CHN9_TOGGLE;
    DBG_CHN10_TOGGLE;
    DBG_CHN11_TOGGLE;
    DBG_CHN12_TOGGLE;
    DBG_CHN13_TOGGLE;
    DBG_CHN14_TOGGLE;
    DBG_CHN15_TOGGLE;

}


#endif // end of (FEATURE_TEST_MODE == TEST_PER_ADV)


