#include "ps2.h"
#include "priv_ps2.h"

#include "utils/native.h"
#include "utils/common.h"

uint8_t isDualChannel = 0;

uint8_t PS2_Initialize()
{
        PS2_SetEnabled(1, 0);
        PS2_SetEnabled(2, 0);
        inb(PS2_DATA_PORT); //Flush the IO port

        PS2_SetControllerConfig(0, 0, 0); //Disable IRQs and translation
        if(!PS2_TestDevice(0)) return -1; //Self test failed

        //Check if controller is dual channel
        if(PS2_IsDualChannel()) {
                PS2_SetEnabled(2, 1);
                PS2_SetControllerConfig(0,0,0);
                isDualChannel = !isDualChannel; //If bit is set, not dual channel, if clear, is dual channel
                PS2_SetEnabled(2, 0);
        }

        //Make dual channel status permanent
        isDualChannel = isDualChannel << 1;

        uint8_t p1_test_res = PS2_TestDevice(1);
        uint8_t p2_test_res = isDualChannel ? PS2_TestDevice(2) : 0;

        if(!p1_test_res && !p2_test_res) return -1; //No PS2 devices supported

        //Enable these devices if their tests passed
        PS2_SetEnabled(1, p1_test_res);
        PS2_SetEnabled(2, p2_test_res);

        PS2_SetControllerConfig(p1_test_res, p2_test_res, 0); //Enable IRQs

        if(PS2Keyboard_Initialize() < 0) {
                PS2_SetEnabled(1, 0); //If initialization failed, disable
                p1_test_res = 0;
        }

        return p1_test_res | (p2_test_res << 1) | (isDualChannel << 1);
}

uint8_t PS2_SendCommand(uint16_t port, uint8_t val, uint8_t *response)
{
        uint8_t resentCount = 0;
        uint8_t ret = 0;

        do {
                if(resentCount == 3) break;
                while(IS_INPUTBUF_FULL) ; //Wait until Input buffer is empty
                outb(port, val);
                resentCount++;
                ret = PS2_ReadData(PS2_DATA_PORT);
        }
        while(ret == PS2_RETVAL_RESEND_REQ);  //Repeat if requested

        if(response != NULL) *response = ret;
        return resentCount-1;
}

uint8_t PS2_ReadData(uint16_t port)
{
        int waits = 0;
        while(!IS_OUTPUTBUF_FULL) {
                if(waits++ == 10) return PS2_RETVAL_ECHO;
        }
        return inb(port);
}

uint8_t PS2_GetStatus()
{
        return inb(PS2_CTRL_PORT);
}

void PS2_SetEnabled(uint8_t deviceIndex, uint8_t status)
{
        if(deviceIndex < 1 || deviceIndex > 2) return;
        if(status)
        {
                outb(PS2_CTRL_PORT, (deviceIndex == 1) ? PS2_P1_ENABLE : PS2_P2_ENABLE);
        }else{
                outb(PS2_CTRL_PORT, (deviceIndex == 1) ? PS2_P1_DISABLE : PS2_P2_DISABLE);
        }
}

uint8_t PS2_TestDevice(uint8_t deviceIndex)
{
        uint8_t val = 0;
        if(deviceIndex == 0) val = PS2_CTRL_TEST;
        else if(deviceIndex == 1) val = PS2_P1_TEST;
        else if (deviceIndex == 2) val = PS2_P2_TEST;

        uint8_t result = 0;

        PS2_SendCommand(PS2_CTRL_PORT, val, &result);
        if(result >= 0x55) result -= 0x55;
        return !result;
}

uint8_t PS2_SetControllerConfig(uint8_t P1_IRQ, uint8_t P2_IRQ, uint8_t translation)
{
        uint8_t configByte = 0, configCopy = 0;
        PS2_SendCommand(PS2_CTRL_PORT, PS2_READ_CONFIG, &configByte);

        configCopy = configByte;

        if(P1_IRQ)
        {
                configByte = SET_BIT(configByte, 0);
        }else{
                configByte = CLEAR_BIT(configByte, 0);
        }

        if(P2_IRQ)
        {
                configByte = SET_BIT(configByte, 1);
        }else{
                configByte = CLEAR_BIT(configByte, 1);
        }

        if(translation)
        {
                configByte = SET_BIT(configByte, 6);
        }else{
                configByte = CLEAR_BIT(configByte, 6);
        }

        //Set the temporary part of the dual channel bit
        int dualChnl = CHECK_BIT(configByte, 5);
        if(dualChnl) isDualChannel = SET_BIT(isDualChannel, 0);
        else isDualChannel = CLEAR_BIT(isDualChannel, 0);

        while(IS_INPUTBUF_FULL) ;
        outb(PS2_CTRL_PORT, PS2_WRITE_CONFIG);
        while(IS_OUTPUTBUF_FULL) ;
        outb(PS2_DATA_PORT, configByte);

        //PS2_SendCommand(PS2_CTRL_PORT, PS2_WRITE_CONFIG, NULL);
        //PS2_SendCommand(PS2_CTRL_PORT, configByte, NULL);
        return configCopy;
}

uint8_t PS2_IsDualChannel(){
        return (isDualChannel >> 1) & 1;
}
