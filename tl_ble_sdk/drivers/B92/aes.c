/********************************************************************************************************
 * @file    aes.c
 *
 * @brief   This is the source file for B92
 *
 * @author  Driver Group
 * @date    2020
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
#include "aes.h"
#include "string.h"
#include "compiler.h"
#include "core.h"

/**********************************************************************************************************************
 *                                            local constants                                                       *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                              local macro                                                        *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                             local data type                                                     *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                              global variable                                                       *
 *********************************************************************************************************************/
_attribute_aes_data_sec_ unsigned int aes_data_buff[8];
_attribute_aes_data_sec_ unsigned char rpa_data_buff[256];
unsigned int aes_embase_addr = 0xc0000000;
static unsigned int embase_offset = 0;    //the embase address offset with IRAM head address.

/**
 * aes error timeout(us),a large value is set by default,can set it by aes_set_error_timeout().
 */
unsigned int g_aes_error_timeout_us  = 0xffffffff;
/**********************************************************************************************************************
 *                                              local variable                                                     *
 *********************************************************************************************************************/

/**********************************************************************************************************************
 *                                          local function prototype                                               *
 *********************************************************************************************************************/
/**
 * @brief     This function refer to wait aes encrypt/decrypt done.
 * @return    none.
 */
static bool aes_wait_done(void);
/**********************************************************************************************************************
 *                                         global function implementation                                             *
 *********************************************************************************************************************/
/**
 * @brief     This function serves to aes finite state machine reset(the configuration register is still there and does not need to be reconfigured).
 * @return    none.
 */
 void aes_hw_fsm_reset(void){
     reg_aes_rwbtcntl |= FLD_AES_SOFT_RESET;
 }

/**
 * @brief     This function serves to set the aes timeout(us).
 * @param[in] timeout_us - the timeout(us).
 * @return    none.
 * @note      The default timeout (g_aes_error_timeout_us) is the larger value.If the timeout exceeds the feed dog time and triggers a watchdog restart,
 *            g_aes_error_timeout_us can be changed to a smaller value via this interface, depending on the application.
 *            g_aes_error_timeout_us the minimum time must meet the following conditions:
 *            1. at least 100us;
 *            2. maximum interrupt processing time;
 *            3. Consider the conflict time of aes encryption and decryption by ble/bt;
 */
void aes_set_error_timeout(unsigned int timeout_us){
    g_aes_error_timeout_us = timeout_us;
}

/**
 * @brief     This function serves to record the api status.
 * @param[in] aes_error_timeout_code - aes_api_error_code_e.
 * @return    none.
 * @note      This function can be rewritten according to the application scenario,can read the parameters of the interface to obtain details about the timeout reason(aes_api_error_code_e),
 *            aes_hw_fsm_reset() must be called.
 */
__attribute__((weak)) void aes_timeout_handler(unsigned int aes_error_timeout_code)
{
    (void)aes_error_timeout_code;
    aes_hw_fsm_reset();
}


#define AES_WAIT(aes_api_error_code)           wait_condition_fails_or_timeout(aes_wait_done,g_aes_error_timeout_us,aes_timeout_handler,(unsigned int)aes_api_error_code)
/**
 * @brief     This function refer to set key and data for encryption/decryption. 
 * @param[in] key  - the key of encrypt/decrypt, big--endian.
 * @param[in] data - the data which to do encrypt/decrypt, big--endian. 
 * @return    none.
 * @note      The AES module register must be used by word and the key and data lengths must be 16 bytes.
 */
void aes_set_key_data(unsigned char *key, unsigned char* data)
{
    /*
        The reg_aes_ptr register is 32 bits, but only the lower 16 bits can be used. The actual access address is obtained by 
        reg_embase_addr (32bit) + reg_aes_ptr (16bit) which means the accessible space is only 64K.
        The reg_embase_addr can adjust by call aes_set_em_base_addr().
    */
    unsigned int temp;
    reg_embase_addr = aes_embase_addr;  //set the embase addr
    for (unsigned char i = 0; i < 4; i++) {
        temp = key[16-(4*i)-4]<<24 | key[16-(4*i)-3]<<16 | key[16-(4*i)-2]<<8 | key[16-(4*i)-1];
        reg_aes_key(i) = temp;
        temp = data[16-(4*i)-4]<<24 | data[16-(4*i)-3]<<16 | data[16-(4*i)-2]<<8 | data[16-(4*i)-1];
        aes_data_buff[i] = temp;
    }

    reg_aes_ptr = (unsigned int)aes_data_buff - embase_offset;  //the aes data ptr is base on embase address.
}

/**
 * @brief      This function refer to encrypt/decrypt to get result. AES module register must be used by word.
 * @param[out] result - the result of encrypt/decrypt, big--endian.
 * @return     none.
 */
void aes_get_result(unsigned char *result)
{
    /* read out the result */
    unsigned char *ptr = (unsigned char *)&aes_data_buff[4];
    for (unsigned char i=0; i<16; i++) {
        result[i] = ptr[15 - i];
    }
}

/**
 * @brief      This function servers to perform aes_128 encryption for 16-Byte input data with specific 16-Byte key.
 * @param[in]  key       - the key of encrypt, big--endian.
 * @param[in]  plaintext - the plaintext of encrypt, big--endian.
 * @param[out] result    - the result of encrypt, big--endian.
 * @return     1: operation successful;
 *             DRV_API_TIMEOUT: timeout exit(g_aes_error_timeout_us refer to the note for aes_set_error_timeout,the solution processing is already done in aes_timeout_handler, so just re-invoke the interface);
 */
int aes_encrypt(unsigned char *key, unsigned char* plaintext, unsigned char *result)
{
    aes_set_key_data(key, plaintext);   //set the key

    aes_set_mode(AES_ENCRYPT_MODE);      //cipher mode

   if(AES_WAIT(AES_API_ERROR_TIMEOUT_ENCRYPT)){
       return DRV_API_TIMEOUT;
   }

    aes_get_result(result);

    return 1;
}

/**
 * @brief      This function servers to perform aes_128 encryption for 16-Byte input data with specific 16-Byte key when BT is connected.
 * @param[in]  key       - the key of encrypt, big--endian.
 * @param[in]  plaintext - the plaintext of encrypt, big--endian.
 * @param[out] result    - the result of encrypt, big--endian.
 * @return     DRV_API_TIMEOUT: timeout exit(g_aes_error_timeout_us refer to the note for aes_set_error_timeout,the solution processing is already done in aes_timeout_handler, so just re-invoke the interface);
 * @note       Invoking this interface avoids the risk of AES conflicts when BT is connected.
 */
int aes_encrypt_bt_en(unsigned char* key, unsigned char* plaintext, unsigned char *result)
{
    int i, aes_correct = 0;
    unsigned char temp_result[AES_MAX_CNT][16];

    for(i=0; i<AES_MAX_CNT; i++)
    {
         if(aes_encrypt(key, plaintext, temp_result[i])==DRV_API_TIMEOUT){
             return DRV_API_TIMEOUT;
         }

        if(i > 0)
        {
            if(!memcmp(temp_result[i], temp_result[i-1], 16))
            {
                aes_correct = 1;
                break;
            }
            else
            {
                if(i >= 2)
                {
                    for(int j=0; j<i-1; j++)
                    {
                        if(!memcmp(temp_result[i], temp_result[j], 16))
                        {
                            aes_correct = 1;
                            break;
                        }
                    }
                }

            }
        }

        if(aes_correct){
            break;
        }
    }

    if(aes_correct)
    {
        memcpy(result, temp_result[i], 16);
        return 1;
    }
    return 0;
}

/**
 * @brief      This function servers to perform aes_128 decryption for 16-Byte input data with specific 16-Byte key.
 * @param[in]  key         - the key of decrypt, big--endian.
 * @param[in]  decrypttext - the text of decrypt, big--endian.
 * @param[out] result      - the result of decrypt, big--endian.
 * @return     1: operation successful;
 *             DRV_API_TIMEOUT: timeout exit(g_aes_error_timeout_us refer to the note for aes_set_error_timeout,the solution processing is already done in aes_timeout_handler, so just re-invoke the interface);
 */
int aes_decrypt(unsigned char *key, unsigned char* decrypttext, unsigned char *result)
{
    aes_set_key_data(key, decrypttext);    //set the key

    aes_set_mode(AES_DECRYPT_MODE);      //decipher mode

    if(AES_WAIT(AES_API_ERROR_TIMEOUT_DECRYPT)){
       return DRV_API_TIMEOUT;
    }

    aes_get_result(result);

    return 1;
}

/**
 * @brief      This function servers to perform aes_128 decryption for 16-Byte input data with specific 16-Byte key when BT is connected.
 * @param[in]  key         - the key of decrypt, big--endian.
 * @param[in]  decrypttext - the text of decrypt, big--endian.
 * @param[out] result      - the result of decrypt, big--endian.
 * @return     DRV_API_TIMEOUT: timeout exit(g_aes_error_timeout_us refer to the note for aes_set_error_timeout,the solution processing is already done in aes_timeout_handler, so just re-invoke the interface);
 * @note       Invoking this interface avoids the risk of AES conflicts when BT is connected.
 */
int aes_decrypt_bt_en(unsigned char* key, unsigned char* plaintext, unsigned char *result)
{
    int i, aes_correct = 0;
    unsigned char temp_result[AES_MAX_CNT][16];

    for(i=0; i<AES_MAX_CNT; i++)
    {
        if(aes_decrypt(key, plaintext, temp_result[i])==DRV_API_TIMEOUT){
            return DRV_API_TIMEOUT;
        }

        if(i > 0)
        {
            if(!memcmp(temp_result[i], temp_result[i-1], 16))
            {
                aes_correct = 1;
                break;
            }
            else
            {
                if(i >= 2)
                {
                    for(int j=0; j<i-1; j++)
                    {
                        if(!memcmp(temp_result[i], temp_result[j], 16))
                        {
                            aes_correct = 1;
                            break;
                        }
                    }
                }

            }
        }

        if(aes_correct){
            break;
        }
    }

    if(aes_correct)
    {
        memcpy(result, temp_result[i], 16);
        return 1;
    }
    return 0;
}

/**
 * @brief     This function refer to set the em base address.
 * @param[in] addr - The range of em base address that can be set is the address space of DLM and ILM, which can view the Memory Map of datasheets.
 *                   The current driver default setting is em_base_addr = 0xc0000000, if you call this function to modify the em base address,
 *                   you need to ensure that the _attribute_aes_data_sec_ section in the link file (AES-related functions will use this section)
 *                   is set in the following address range: [em_base_addr,em_base_addr+64KB] (chip design requirements)
 * @return    none.
 * @attention If you are using a BT-related SDK, you must follow the planning of BT's sdk to handle this address and not call this function
 */
void aes_set_em_base_addr(unsigned int addr){
    aes_embase_addr = addr;   //set the embase addr
    embase_offset = convert_ram_addr_bus2cpu(addr);
}

/**********************************************************************************************************************
  *                        This interface is only provided for ble and is not open to customers.                      *
  *                        This interface is not a function of the AES module, it only uses the AES module            *
  *                        and has been placed in this position for code maintenance convenience.                     *
  *********************************************************************************************************************/
/**
 * @brief     This function refer to match the rpa.
 * @param[in] irk         - the irk sequence, max 16 group(16byte a group). The address is 32 bits, but only the lower 16 bits are used.
 * @param[in] irk_len     - the irk group num.
 * @param[in] rpa         - the rpa which want to match.
 * @return    0xff-not matched, (0x0-0x0f)-the match group index.
 * @note      reg_embase_addr (32bit) +reg_aes_irk_ptr (16bit) is the actual access address.
 *            reg_aes_irk_ptr is only 16bit, so access space is only 64K. Adjusting reg_embase_addr changes the initial address of 64K.
 */
unsigned char aes_rpa_match(unsigned char *irk, unsigned char irk_len, unsigned char *rpa)
{
    unsigned int prand = rpa[3] | (rpa[4] << 8) | (rpa[5] << 16);
    unsigned int hash = rpa[0] | (rpa[1] << 8) | (rpa[2] << 16);

    for(unsigned int i = 0; i < irk_len * 16; i++){
        rpa_data_buff[i] = irk[i];
    }

    reg_embase_addr = aes_embase_addr;  //set the embase addr

    reg_aes_hash_status |= FLD_AES_RPASE_STA_CLR;
    reg_aes_rpase_cnt = (prand & FLD_AES_PRAND) | ((irk_len << 24) & FLD_AES_IRK_NUM) | FLD_AES_RPASE_EN;

    reg_aes_irk_ptr = (unsigned int)rpa_data_buff - embase_offset;

    reg_aes_hash_status = (hash & FLD_AES_HASH_STA);

    reg_aes_rpase_cnt |= FLD_AES_RPASE_START;

    while(BIT(30) != (reg_aes_hash_status & FLD_AES_RPASE_STA));

    unsigned int match_status = reg_aes_hash_status;

    if(match_status & FLD_AES_HASH_MATCH)
    {
        return ((match_status >> 24) & 0x0f);
    }
    return (0xff);
}

/**********************************************************************************************************************
  *                                         local function implementation                                             *
  *********************************************************************************************************************/
/**
 * @brief     This function refer to wait aes crypt done.
 * @return    0: done  1:no done.
 */
static bool aes_wait_done(void)
{
    return (FLD_AES_START == (reg_aes_mode & FLD_AES_START));
}
