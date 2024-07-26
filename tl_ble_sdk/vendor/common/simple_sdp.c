/********************************************************************************************************
 * @file    simple_sdp.c
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

#include "simple_sdp.h"
#include "device_manage.h"

#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)


typedef struct{
    u8  type;
    u8  rf_len;
    u16 l2capLen;
    u16 chanId;
    u8  opcode;
    u8  format;
    u8  data[1];            // character_handle / property / value_handle / value
}ble_att_findInfoRsp_t;


sdp_info_store_cb_t sdp_info_store_cb = NULL;

int central_sdp_pending = 0;            // SDP: service discovery


dev_char_info_t  cur_sdp_device;  //current connected device which is in SDP flow


main_service_t      main_service = 0;


static const  u8 my_OtaUUID[16]                     = WRAPPING_BRACES(TELINK_SPP_DATA_OTA);


ble_sts_t  host_att_discoveryService (u16 handle, att_db_uuid16_t *p16, int n16, att_db_uuid128_t *p128, int n128);



void simple_sdp_loop (void)
{
    if (main_service)
    {
        main_service ();
        main_service = 0;
    }
}


void simple_sdp_register_store_info_callback(sdp_info_store_cb_t cb)
{
    sdp_info_store_cb = cb;
}



/**
 * @brief   SDP handler.
 *          !!! Note: This is a simple SDP processing implemented by telink.
 * @param   none.
 * @return  none.
 */
void app_service_discovery (void)
{
    att_db_uuid16_t     db16[ATT_DB_UUID16_NUM];
    att_db_uuid128_t    db128[ATT_DB_UUID128_NUM];
    memset (db16, 0, ATT_DB_UUID16_NUM * sizeof (att_db_uuid16_t));
    memset (db128, 0, ATT_DB_UUID128_NUM * sizeof (att_db_uuid128_t));

    if ( central_sdp_pending && host_att_discoveryService (central_sdp_pending, db16, ATT_DB_UUID16_NUM, db128, ATT_DB_UUID128_NUM) == BLE_SUCCESS) // service discovery OK
    {
        cur_sdp_device.char_handle[2] = blm_att_findHandleOfUuid128 (db128, my_OtaUUID);            //OTA
        cur_sdp_device.char_handle[3] = blm_att_findHandleOfUuid16 (db16, CHARACTERISTIC_UUID_REPORT,
                    HID_REPORT_ID_CONSUME_CONTROL_INPUT | (HID_REPORT_TYPE_INPUT<<8));      //consume report(media key report)
        cur_sdp_device.char_handle[4] = blm_att_findHandleOfUuid16 (db16, CHARACTERISTIC_UUID_REPORT,
                    HID_REPORT_ID_KEYBOARD_INPUT | (HID_REPORT_TYPE_INPUT<<8));             //normal key report
        cur_sdp_device.char_handle[5] = blm_att_findHandleOfUuid16 (db16, CHARACTERISTIC_UUID_REPORT,
                    HID_REPORT_ID_MOUSE_INPUT | (HID_REPORT_TYPE_INPUT<<8));                //mouse report
        //cur_sdp_device.char_handle[6] = blm_att_findHandleOfUuid128 (db128, TelinkSppDataServer2ClientUUID);      //BLE Module, SPP Server to Client
        //cur_sdp_device.char_handle[7] = blm_att_findHandleOfUuid128 (db128, TelinkSppDataClient2ServerUUID);      //BLE Module, SPP Client to Server

        /* add the peer device att_handle value to conn_dev_list after service discovery is correctly finished */
        dev_char_info_add_peer_att_handle(&cur_sdp_device);

        /* peer device att_handle value store in flash */
        dev_char_info_store_peer_att_handle(&cur_sdp_device);

        tlkapi_send_string_data(APP_SIMPLE_SDP_LOG_EN,"[APP][SDP] OTA handle", (u8*)&cur_sdp_device.char_handle[2], 2);
        tlkapi_send_string_data(APP_SIMPLE_SDP_LOG_EN,"[APP][SDP] CMKEY handle", (u8*)&cur_sdp_device.char_handle[3], 2);
        tlkapi_send_string_data(APP_SIMPLE_SDP_LOG_EN,"[APP][SDP] KBKEY handle", (u8*)&cur_sdp_device.char_handle[4], 2);
        tlkapi_send_string_data(APP_SIMPLE_SDP_LOG_EN,"[APP][SDP] MOUSE handle", (u8*)&cur_sdp_device.char_handle[5], 2);
    }

    central_sdp_pending = 0;  //service discovery finish
}

/**
 * @brief       This function is used to register SDP handler.
 * @param[in]   p       - Pointer point to SDP handler.
 * @return      none.
 */
void app_register_service (void *p)
{
    main_service = p;
}










u8  *p_att_response = 0;

volatile u32    host_att_req_busy = 0;

/**
 * @brief       This function is used to process ATT packets related to SDP
 * @param[in]   connHandle  - connection handle
 * @param[in]   p           - Pointer point to ATT data buffer.
 * @return
 */
int host_att_client_handler (u16 connHandle, u8 *p)
{
    ble_att_readByTypeRsp_t *p_rsp = (ble_att_readByTypeRsp_t *) p;
    if (p_att_response)
    {
        if ((connHandle & 7) == (host_att_req_busy & 7) && p_rsp->chanId == 0x04 &&
            (p_rsp->opcode == 0x01 || p_rsp->opcode == ((host_att_req_busy >> 16) | 1)))
        {
            memcpy (p_att_response, p, p_rsp->l2capLen+6);
            host_att_req_busy = 0;
        }
    }
    return 0;
}

typedef int (*host_att_idle_func_t) (void);
host_att_idle_func_t host_att_idle_func = 0;

/**
 * @brief       This function is used to register ble stack mainloop function.
 * @param[in]   p           - Pointer point to ble stack mainloop function.
 * @return
 */
int host_att_register_idle_func (void *p)
{
    if (host_att_idle_func)
        return 1;

    host_att_idle_func = p;
    return 0;
}

int host_att_response (void)
{
    return host_att_req_busy == 0;
}


int host_att_service_wait_event (u16 handle, u8 *p, u32 timeout)
{
    host_att_req_busy = handle | (p[6] << 16);
    p_att_response = p;

    u32 t = clock_time ();
    while (!clock_time_exceed (t, timeout))
    {
        if (host_att_response ())
        {
            return 0;
        }
        if (host_att_idle_func)
        {
            if (host_att_idle_func ())
            {
                break;
            }
        }
    }
    return 1;
}

//Gaoqiu add ------------------------------------------------------------------
int app_char_discovery(u8* result, u16 connHandle, u16 startAttHandle, u16 endAttHandle, u8*uuid, u8 uuidLen)
{
    blc_gatt_pushReadByTypeRequest(connHandle, startAttHandle, endAttHandle, uuid, uuidLen);

    return host_att_service_wait_event(connHandle, result, 1000000);
}

int app_read_char_value(u8* result, u16 connHandle, u16 attHandle)
{
    blc_gatt_pushReadRequest(connHandle, attHandle);

    return host_att_service_wait_event(connHandle, result, 1000000);
}

int app_find_char_info(u8* result, u16 connHandle, u16 startAttHandle, u16 endAttHandle)
{
    blc_gatt_pushFindInformationRequest(connHandle, startAttHandle, endAttHandle);

    return host_att_service_wait_event(connHandle, result, 1000000);
}


u16 blm_att_findHandleOfUuid16 (att_db_uuid16_t *p, u16 uuid, u16 ref)
{
    for (int i=0; i<p->num; i++)
    {
        if (p[i].uuid == uuid && p[i].ref == ref)
        {
            return p[i].handle;
        }
    }
    return 0;
}

u16 blm_att_findHandleOfUuid128 (att_db_uuid128_t *p, const u8 * uuid)
{
    for (int i=0; i<p->num; i++)
    {
        if (memcmp (p[i].uuid, uuid, 16) == 0)
        {
            return p[i].handle;
        }
    }
    return 0;
}


ble_sts_t  host_att_discoveryService (u16 handle, att_db_uuid16_t *p16, int n16, att_db_uuid128_t *p128, int n128)
{
    att_db_uuid16_t *ps16 = p16;
    att_db_uuid128_t *ps128 = p128;
    int i16 = 0;
    int i128 = 0;

    ps16->num = 0;
    ps128->num = 0;

    // char discovery: att_read_by_type
    // hid discovery: att_find_info
    u8  dat[256];
    u16 s = 1;
    u16 uuid = DECLARATIONS_UUID_CHARACTERISTIC;
    do {
        dat[6] = ATT_OP_READ_BY_TYPE_REQ;
        if (app_char_discovery(dat, handle, s, 0xffff, (u8 *)&uuid, 2)) // 1s
        {
            return  GATT_ERR_SERVICE_DISCOVERY_TIMEOUT;         //timeout
        }

        // process response data
        ble_att_readByTypeRsp_t *p_rsp = (ble_att_readByTypeRsp_t *) dat;
        if (p_rsp->opcode != ATT_OP_READ_BY_TYPE_RSP)
        {
            break;
        }

        if (p_rsp->datalen == 21)       //uuid128
        {
            u8 *pd = p_rsp->data;
            while(p_rsp->l2capLen > 21){
                s = pd[3] + pd[4] * 256;
                if (i128 < n128)
                {
                    p128->property = pd[2];
                    p128->handle = s;
                    memcpy (p128->uuid, pd + 5, 16);
                    i128++;
                    p128++;
                }
                p_rsp->l2capLen -= 21;
                pd += 21;
            }
        }
        else if (p_rsp->datalen == 7) //uuid16
        {
            u8 *pd = p_rsp->data;
            while (p_rsp->l2capLen > 7)
            {
                s = pd[3] + pd[4] * 256;
                if (i16 < n16)
                {
                    p16->property = pd[2];
                    p16->handle = s;
                    p16->uuid = pd[5] | (pd[6] << 8);
                    p16->ref = 0;
                    i16 ++;
                    p16++;
                }
                p_rsp->l2capLen -= 7;
                pd += 7;
            }
        }
    } while (1);

    ps16->num = i16;
    ps128->num = i128;

    //--------- use att_find_info to find the reference property for hid ----------
    p16 = ps16;
    for (int i=0; i<i16; i++)
    {
        if (p16->uuid == CHARACTERISTIC_UUID_REPORT)        //find reference
        {
            dat[6] = ATT_OP_FIND_INFORMATION_REQ;
            if (app_find_char_info(dat, handle, p16->handle, 0xffff)) // 1s
            {
                return  GATT_ERR_SERVICE_DISCOVERY_TIMEOUT;         //timeout
            }

            ble_att_findInfoRsp_t *p_rsp = (ble_att_findInfoRsp_t *) dat;
            if (p_rsp->opcode == ATT_OP_FIND_INFO_RSP && p_rsp->format == 1)
            {
                int n = p_rsp->l2capLen - 2;
                u8 *pd = p_rsp->data;
                while (n > 0)
                {
                    if ((pd[2]==U16_LO(DECLARATIONS_UUID_CHARACTERISTIC) && pd[3]==U16_HI(DECLARATIONS_UUID_CHARACTERISTIC)) ||
                        (pd[2]==U16_LO(DECLARATIONS_UUID_PRIMARY_SERVICE) && pd[3]==U16_HI(DECLARATIONS_UUID_PRIMARY_SERVICE))  )
                    {
                        break;
                    }

                    if (pd[2]==U16_LO(DESCRIPTOR_UUID_REPORT_REFERENCE) && pd[3]==U16_HI(DESCRIPTOR_UUID_REPORT_REFERENCE))
                    {
                        //-----------       read attribute ----------------
                        dat[6] = ATT_OP_READ_REQ;
                        if(app_read_char_value(dat, handle, pd[0])){
                            return  GATT_ERR_SERVICE_DISCOVERY_TIMEOUT;         //timeout
                        }

                        ble_att_readRsp_t *pr = (ble_att_readRsp_t *) dat;
                        if (pr->opcode == ATT_OP_READ_RSP)
                        {
                            p16->ref = pr->value[0] | (pr->value[1] << 8);
                        }

                        break;
                    }
                    n -= 4;
                    pd += 4;
                }
            }
        } //----- end for if CHARACTERISTIC_UUID_REPORT

        p16++;
    }

    return  BLE_SUCCESS;
}













/**
 * @brief       Used for add peer device service ATThandle.
 * @param[in]   dev_char_info       - Pointer point to data buffer.
 * @return      0: success
 *              1: failed
 */
int dev_char_info_add_peer_att_handle (dev_char_info_t* dev_char_info)
{
    int i;
    for(i=0; i< acl_conn_central_num; i++){
        if( conn_dev_list[i].conn_handle == dev_char_info->conn_handle){
            break;
        }
    }

    if( i < acl_conn_central_num){
        for(int j=0; j<CHAR_HANDLE_MAX;  j++){
            conn_dev_list[i].char_handle[j] = dev_char_info->char_handle[j];
        }

        conn_dev_list[i].char_handle_valid = 1;

        return 0;   //success
    }
    else{
        return 1;   //fail
    }
}




/**
 * @brief       Use for store peer device att handle to flash.
 * @param[in]   dev_char_info    Pointer point to peer device ATT handle info.
 * @return      0: failed
 *             !0: return flash address
 */
int     dev_char_info_store_peer_att_handle(dev_char_info_t* pdev_char)
{
    u8 mark;
    u32 current_flash_adr;
    for(current_flash_adr = flash_sector_simple_sdp_att; current_flash_adr < (flash_sector_simple_sdp_att + FLASH_SDP_ATT_MAX_SIZE); current_flash_adr += sizeof(dev_att_t) )
    {
        flash_read_page(current_flash_adr, 1, &mark);

        if( mark == U8_MAX){

            flash_write_page( current_flash_adr + OFFSETOF(dev_att_t, adr_type),  7, (u8 *)&pdev_char->peer_adrType);  // peer_adrType(1)+peer_addr(6)

#if (PEER_SLAVE_USE_RPA_EN)
            if( IS_RESOLVABLE_PRIVATE_ADDR(pdev_char->peer_adrType, pdev_char->peer_addr) ){
                //TODO, store irk to flash
            }
#endif

//           char_handle[0] :  MIC
//           char_handle[1] :  Speaker
//           char_handle[2] :  OTA
//           char_handle[3] :  Consume Report
//           char_handle[4] :  Key Report
//           char_handle[5] :  mouse report
//           char_handle[6] :  BLE Module, SPP Server to Client
//           char_handle[7] :  BLE Module, SPP Client to Server
            flash_write_page( current_flash_adr + OFFSETOF(dev_att_t, char_handle) + 2*2,  2, (u8 *)&pdev_char->char_handle[2]);   //save OTA att_handle
            flash_write_page( current_flash_adr + OFFSETOF(dev_att_t, char_handle) + 3*2,  2, (u8 *)&pdev_char->char_handle[3]);   //save Consume Report att_handle
            flash_write_page( current_flash_adr + OFFSETOF(dev_att_t, char_handle) + 4*2,  2, (u8 *)&pdev_char->char_handle[4]);   //save Key Report att_handle
            flash_write_page( current_flash_adr + OFFSETOF(dev_att_t, char_handle) + 5*2,  2, (u8 *)&pdev_char->char_handle[5]);   //save Mouse Report att_handle

            if(sdp_info_store_cb){
                sdp_info_store_cb(current_flash_adr, pdev_char);
            }

            mark = ATT_BOND_MARK;
            flash_write_page( current_flash_adr,  1, (u8 *)&mark);

            tlkapi_send_string_data(APP_SIMPLE_SDP_LOG_EN,"[APP][SDP] save simple SDP info", &current_flash_adr, 4);

            return current_flash_adr;  //Store Success
        }
    }






    return 0;  //Store Fail
}


/**
 * @brief       Get peer device att handle info by peer address
 * @param[in]   adr_type         address type
 * @param[in]   addr             Pointer point to peer address buffer
 * @param[out]  dev_att          Pointer point to dev_att_t
 * @return      0: failed
 *             !0: return flash address
 */
int     dev_char_info_search_peer_att_handle_by_peer_mac(u8 adr_type, u8* addr, dev_att_t* pdev_att)
{


    u8 mark;
    u32 current_flash_adr;
    for(current_flash_adr = flash_sector_simple_sdp_att; current_flash_adr < (flash_sector_simple_sdp_att + FLASH_SDP_ATT_MAX_SIZE); current_flash_adr += sizeof(dev_att_t) )
    {
        flash_read_page(current_flash_adr, 1, &mark);

        if(mark == U8_MAX){
            return 0;   //Search Fail
        }
        else if(mark == ATT_ERASE_MARK){
            continue;   //Search for next unit
        }
        else if( mark == ATT_BOND_MARK)
        {
            flash_read_page(current_flash_adr, sizeof(dev_att_t), (u8 *)pdev_att);

            int addr_match = 0;
#if (PEER_SLAVE_USE_RPA_EN)
            if( IS_RESOLVABLE_PRIVATE_ADDR(pdev_att->adr_type, pdev_att->addr) ){
                //TODO, resolve address by irk
                if(0){
                    addr_match = 1;
                }

            }
            else
#endif
            {
                if( adr_type == pdev_att->adr_type && !memcmp(addr, pdev_att->addr, 6 )){  //match
                    addr_match = 1;
                }
            }

            if(addr_match){
                return current_flash_adr;
            }


        }


    }

    return 0;   //Search Fail

}

/**
 * @brief       Delete peer device att handle info by peer address
 * @param[in]   adr_type         address type
 * @param[in]   addr             Pointer point to peer address buffer
 * @return      0: success
 *              1: not find
 */
int dev_char_info_delete_peer_att_handle_by_peer_mac(u8 addrType, u8 *addr)
{
    dev_att_t dev_info;

    for(u32 cur_flash_addr = flash_sector_simple_sdp_att;
        cur_flash_addr < flash_sector_simple_sdp_att + FLASH_SDP_ATT_MAX_SIZE;
        cur_flash_addr += sizeof(dev_att_t))
    {
        u8 flag;
        flash_read_page(cur_flash_addr, 1, &flag);

        //have no device information
        if(flag == 0xff)
            return 1;//not find

        if(flag == ATT_BOND_MARK){
            //only read per device MAC address type and MAC address
            flash_read_page(cur_flash_addr, 8, (u8*)&dev_info);
        #if(PEER_SLAVE_USE_RPA_EN)
            if(IS_RESOLVABLE_PRIVATE_ADDR(addrType, addr)){
                //todo: resolve private address using IRK
            }
            else
        #endif
            {
                if(dev_info.adr_type == addrType && !memcmp(dev_info.addr, addr, 6)){
                    u8 temp = ATT_ERASE_MARK;
                    flash_write_page(cur_flash_addr, 1, (u8*)&temp);

                    tlkapi_send_string_data(APP_SIMPLE_SDP_LOG_EN,"[APP][SDP] delete peer ATT handle", addr, 6);

                    return 0;//find
                }
            }
        }
    }
    return 1;//not find
}


#endif    //end of ACL_CENTRAL_SIMPLE_SDP_ENABLE
