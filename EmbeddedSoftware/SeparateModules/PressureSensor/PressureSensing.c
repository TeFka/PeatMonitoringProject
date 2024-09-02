/*
 * PressureSensing.c
 *
 *  Created on: Feb 11, 2023
 *      Author: Admin
 */

#include "../Inc/Pressure/PressureSensing.h"

static void bytecpy(uint8_t *target, uint8_t *source)
{
    if ((target != NULL) && (source != NULL))
    {
        *target = *source;
    }
}

int32_t lps28dfw_read_reg(I2C_HandleTypeDef* handle, uint8_t wantedAddress, uint8_t reg, uint8_t *data,
                          uint16_t len)
{
	//printf("\r\nreading reg");
	HAL_StatusTypeDef ret;
	uint8_t transmitBuf[12];
	// Make sure the number of bytes is statusid
	if(len == 0)
	{
		return LPS28DFW_E_COM_FAIL;
	}
	uint8_t address = wantedAddress << 1;
	// Jump to desired register address
	transmitBuf[0] = reg;
	ret = HAL_I2C_Master_Transmit(handle, address, transmitBuf, 1, 1000);
	if ( ret != HAL_OK ) {
		//printf("\r\nHAL read transmit error");
		return LPS28DFW_E_COM_FAIL;
	}
	else{
		ret = HAL_I2C_Master_Receive(handle, address, transmitBuf, len, HAL_MAX_DELAY);
		if ( ret != HAL_OK ) {
			//printf("\r\nHAL read receive error");
			return LPS28DFW_E_COM_FAIL;
		}
		else{

			for(int i = 0;i<len;i++){

				data[i] = transmitBuf[i];

			}

		}
	}
	return LPS28DFW_OK;
}

int32_t lps28dfw_write_reg(I2C_HandleTypeDef* handle, uint8_t wantedAddress, uint8_t reg, uint8_t *data,
                           uint16_t len)
{
	HAL_StatusTypeDef ret;
	uint8_t transmitBuf[len+1];
	// Make sure the number of bytes is status id
	if(len == 0)
	{
		return LPS28DFW_E_COM_FAIL;
	}

	uint8_t address = wantedAddress << 1;
	transmitBuf[0] = reg;
	for(int i = 1; i<=len;i++){
		transmitBuf[i] = data[i-1];
	}
	ret = HAL_I2C_Master_Transmit(handle, address, transmitBuf, len+1, 1000);
	if ( ret != HAL_OK ) {
		return LPS28DFW_E_COM_FAIL;
	}

	return LPS28DFW_OK;
}

//conversions

float lps28dfw_from_fs1260_to_hPa(int32_t lsb)
{
    return ((float)lsb / 1048576.0f);   /* 4096.0f * 256 */
}

float lps28dfw_from_fs4000_to_hPa(int32_t lsb)
{
    return ((float)lsb /  524288.0f);   /* 2048.0f * 256 */
}

float lps28dfw_from_lsb_to_celsius(int16_t lsb)
{
    return ((float)lsb / 100.0f);
}

/// @brief Enables the BDU and IF_ADD_INC bits in the control registers
/// @return Error code. 0 means success, negative means failure
int32_t initLPS28(lps28dfw_info_t* sensorData, uint8_t status)
{
    lps28dfw_ctrl_reg2_t ctrl_reg2;
    lps28dfw_ctrl_reg3_t ctrl_reg3;
    uint8_t reg[2];
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG2, reg, 2);
    if (ret == 0)
    {
        bytecpy((uint8_t *)&ctrl_reg2, &reg[0]);
        bytecpy((uint8_t *)&ctrl_reg3, &reg[1]);

        switch (status)
        {
        case LPS28DFW_BOOT:
            ctrl_reg2.boot = PROPERTY_ENABLE;
            ret = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG2,
                                     (uint8_t *)&ctrl_reg2, 1);
            break;
        case LPS28DFW_RESET:
            ctrl_reg2.swreset = PROPERTY_ENABLE;
            ret = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG2,
                                     (uint8_t *)&ctrl_reg2, 1);
            break;
        case LPS28DFW_DRV_RDY:
            ctrl_reg2.bdu = PROPERTY_ENABLE;
            ctrl_reg3.if_add_inc = PROPERTY_ENABLE;
            bytecpy(&reg[0], (uint8_t *)&ctrl_reg2);
            bytecpy(&reg[1], (uint8_t *)&ctrl_reg3);
            ret = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG2, reg, 2);
            break;
        default:
            ctrl_reg2.swreset = PROPERTY_ENABLE;
            ret = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG2,
                                     (uint8_t *)&ctrl_reg2, 1);
            break;
        }

    }
    return ret;

}

/// @brief Begin communication with the sensor over I2C
/// @param address I2C address of sensor
/// @param wirePort I2C port to use for communication, defaults to Wire
/// @return Error code. 0 means success, negative means failure
int32_t beginLPS28(lps28dfw_info_t* sensorData, uint8_t address)
{
    // Variable to track errors returned by API calls
    int32_t err = LPS28DFW_OK;

    // Check whether the sensor is actually connected
    uint8_t chipID;
    err = lps28dfw_read_reg(sensorData->handle, address, LPS28DFW_WHOAMI_ADDR, &chipID, 1);
    if(err != LPS28DFW_OK)
    {
    	//printf("\r\nID read error");
        return err;
    }
    else{
    	//printf("\r\nRetrieved ID: %d", chipID);
    }
    if(chipID != LPS28DFW_WHOAMI)
    {
        return LPS28DFW_E_NOT_CONNECTED;
    }
    sensorData->i2cAddress = address;
    // Sensor is connected, send soft reset to clear any provious configuratio
    err = resetLPS28(sensorData);
    if(err != LPS28DFW_OK)
    {
        return err;
    }

    // Send init command
    return initLPS28(sensorData, LPS28DFW_DRV_RDY);
}

/// @brief Enables the BOOT bit in the control registers
/// @return Error code. 0 means success, negative means failure
int32_t bootLPS28(lps28dfw_info_t* sensorData)
{
    return initLPS28(sensorData, LPS28DFW_BOOT);
}

/// @brief Tells the sensor to reset itself
/// @return Error code. 0 means success, negative means failure
int32_t resetLPS28(lps28dfw_info_t* sensorData)
{
    // Variable to track errors returned by API calls
    int32_t err = LPS28DFW_OK;

    // Send reset command
    err = initLPS28(sensorData, LPS28DFW_RESET);

    lps28dfw_stat_t status;
    do
    {
        err = getStatus(sensorData, &status);
        if(err != LPS28DFW_OK)
        {
            return err;
        }
    }
    while(status.sw_reset);

    return LPS28DFW_OK;
}

/// @brief Sets operational parameters of the sensor, such as range and ODR
/// @param config Struct of configuration parameters, see lps28dfw_md_t
/// @return Error code. 0 means success, negative means failure
int32_t setModeConfig(lps28dfw_info_t* sensorData, lps28dfw_md_t* config)
{
    // Variable to track errors returned by API calls
    int32_t err = LPS28DFW_OK;

    lps28dfw_ctrl_reg1_t ctrl_reg1;
    lps28dfw_ctrl_reg2_t ctrl_reg2;
    uint8_t reg[2];

    err = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG1, reg, 2);
    if(err != LPS28DFW_OK)
    {
        return err;
    }
    if (err == 0)
    {
        bytecpy((uint8_t *)&ctrl_reg1, &reg[0]);
        bytecpy((uint8_t *)&ctrl_reg2, &reg[1]);

        ctrl_reg1.odr = (uint8_t)config->odr;
        ctrl_reg1.avg = (uint8_t)config->avg;
        ctrl_reg2.en_lpfp = ((uint8_t)config->lpf & 0x01U);
        ctrl_reg2.lfpf_cfg = ((uint8_t)config->lpf & 0x02U) >> 2;
        ctrl_reg2.fs_mode = (uint8_t)config->fs;

        bytecpy(&reg[0], (uint8_t *)&ctrl_reg1);
        bytecpy(&reg[1], (uint8_t *)&ctrl_reg2);

        err = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG1, reg, 2);
    }

    if(err != LPS28DFW_OK)
    {
        return err;
    }

    // Config set correctly, save config for use later
    memcpy(&(sensorData->modeConfig), config, sizeof(lps28dfw_md_t));

    return LPS28DFW_OK;
}

/// @brief Gets sensor status bits, such as data ready, overrun, etc.
/// @param status Struct of status bits, see lps28dfw_stat_t
/// @return Error code. 0 means success, negative means failure
int32_t getStatus(lps28dfw_info_t* sensorData, lps28dfw_stat_t* stat)
{

    lps28dfw_interrupt_cfg_t interrupt_cfg;
    lps28dfw_int_source_t int_source;
    lps28dfw_ctrl_reg2_t ctrl_reg2;
    lps28dfw_status_t status;
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG2,
                            (uint8_t *)&ctrl_reg2, 1);
    if (ret == 0)
    {
        ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_INT_SOURCE, (uint8_t *)&int_source, 1);
    }
    if (ret == 0)
    {
        ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_STATUS, (uint8_t *)&status, 1);
    }
    if (ret == 0)
    {
        ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_INTERRUPT_CFG,
                                (uint8_t *)&interrupt_cfg, 1);
    }
    stat->sw_reset  = ctrl_reg2.swreset;
    stat->boot      = int_source.boot_on;
    stat->drdy_pres = status.p_da;
    stat->drdy_temp = status.t_da;
    stat->ovr_pres  = status.p_or;
    stat->ovr_temp  = status.t_or;
    stat->end_meas  = ~ctrl_reg2.oneshot;
    stat->ref_done = ~interrupt_cfg.autozero;

    return ret;

}

int32_t lps28dfw_trigger_sw(lps28dfw_info_t* sensorData)
{
    lps28dfw_ctrl_reg2_t ctrl_reg2;
    int32_t ret = 0;

    if (sensorData->modeConfig.odr == odr_LPS28DFW_ONE_SHOT)
    {
        ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG2, (uint8_t *)&ctrl_reg2, 1);
        ctrl_reg2.oneshot = PROPERTY_ENABLE;
        if (ret == 0)
        {
            ret = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG2, (uint8_t *)&ctrl_reg2, 1);
        }
    }
    return ret;
}

int32_t lps28dfw_data_get(lps28dfw_info_t* sensorData)
{
    uint8_t buff[5];
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_PRESS_OUT_XL, buff, 5);

    /* pressure conversion */
    sensorData->data.pressure.raw = (int32_t)buff[2];
    sensorData->data.pressure.raw = (sensorData->data.pressure.raw * 256) + (int32_t) buff[1];
    sensorData->data.pressure.raw = (sensorData->data.pressure.raw * 256) + (int32_t) buff[0];
    sensorData->data.pressure.raw = sensorData->data.pressure.raw * 256;

    switch (sensorData->modeConfig.fs)
    {
    case fs_LPS28DFW_1260hPa:
        sensorData->data.pressure.hpa = lps28dfw_from_fs1260_to_hPa(sensorData->data.pressure.raw);
        break;
    case fs_LPS28DFW_4000hPa:
        sensorData->data.pressure.hpa = lps28dfw_from_fs4000_to_hPa(sensorData->data.pressure.raw);
        break;
    default:
        sensorData->data.pressure.hpa = 0.0f;
        break;
    }

    /* temperature conversion */
    sensorData->data.heat.raw = (int16_t)buff[4];
    sensorData->data.heat.raw = (sensorData->data.heat.raw * 256) + (int16_t) buff[3];
    sensorData->data.heat.deg_c = lps28dfw_from_lsb_to_celsius(sensorData->data.heat.raw);

    return ret;
}

/// @brief Gets pressure data from the sensor. This must be called to update
/// the data struct
/// @return Error code. 0 means success, negative means failure
int32_t getSensorData(lps28dfw_info_t* sensorData)
{
    // Variable to track errors returned by API calls
    int32_t err = LPS28DFW_OK;
    //printf("\r\nTest1:");
    // Check whether we're in one-shot mode
    if(sensorData->modeConfig.odr == odr_LPS28DFW_ONE_SHOT)
    {
        // We're in one-shot mode, trigger a measurement
        err = lps28dfw_trigger_sw(sensorData);
        if(err != LPS28DFW_OK)
        {
            return err;
        }
    }
    //printf("\r\nTest2:");
    // Wait for measurement to finish
    lps28dfw_stat_t status;
    do
    {
        err = getStatus(sensorData, &status);
        if(err != LPS28DFW_OK)
        {
            return err;
        }
    }
    while(!status.end_meas);
    //printf("\r\nTest3:");
    // Grab latest measurement
    return lps28dfw_data_get(sensorData);
}

/// @brief Sets interrupt pin to be active high/low and latched/pulsed
/// @param intMode Struct of config statusues, see lps28dfw_int_mode_t
/// @return Error code. 0 means success, negative means failure
int32_t setInterruptMode(lps28dfw_info_t* sensorData, lps28dfw_int_mode_t* intMode)
{

    lps28dfw_interrupt_cfg_t interrupt_cfg;
    lps28dfw_ctrl_reg3_t ctrl_reg3;
    lps28dfw_ctrl_reg4_t ctrl_reg4;
    uint8_t reg[2];
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG3, reg, 2);
    if (ret == 0)
    {
        bytecpy((uint8_t *)&ctrl_reg3, &reg[0]);
        bytecpy((uint8_t *)&ctrl_reg4, &reg[1]);

        ctrl_reg3.int_h_l = intMode->active_low;
        ctrl_reg4.drdy_pls = ~intMode->drdy_latched;

        bytecpy(&reg[0], (uint8_t *)&ctrl_reg3);
        bytecpy(&reg[1], (uint8_t *)&ctrl_reg4);

        ret = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG3, reg, 2);
    }
    if (ret == 0)
    {
        ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_INTERRUPT_CFG,
                                (uint8_t *)&interrupt_cfg, 1);
    }
    if (ret == 0)
    {
        interrupt_cfg.lir = intMode->int_latched ;
        ret = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_INTERRUPT_CFG,
                                 (uint8_t *)&interrupt_cfg, 1);
    }
    return ret;
}

/// @brief Enables the data ready and FIFO interrupt conditions
/// @param intRoute Struct of which conditions to enable, see lps28dfw_pin_int_route_t
/// @return Error code. 0 means success, negative means failure
int32_t enableInterrupts(lps28dfw_info_t* sensorData, lps28dfw_pin_int_route_t* intRoute)
{

    lps28dfw_ctrl_reg4_t ctrl_reg4;
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG4, (uint8_t *)&ctrl_reg4, 1);
    if (ret == 0)
    {
        ctrl_reg4.drdy = intRoute->drdy_pres;
        ctrl_reg4.int_f_wtm = intRoute->fifo_th;
        ctrl_reg4.int_f_ovr = intRoute->fifo_ovr;
        ctrl_reg4.int_f_full = intRoute->fifo_full;

        if ((intRoute->fifo_th != 0U) || (intRoute->fifo_ovr != 0U) || (intRoute->fifo_full != 0U))
        {
            ctrl_reg4.int_en = PROPERTY_ENABLE;
        }
        else
        {
            ctrl_reg4.int_en = PROPERTY_DISABLE;
        }

        ret = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG4, (uint8_t *)&ctrl_reg4, 1);
    }
    return ret;
}

/// @brief Gets interrupt status flags
/// @param status Interrupt status flags, see lps28dfw_all_sources_t
/// @return Error code. 0 means success, negative means failure
int32_t getInterruptStatus(lps28dfw_info_t* sensorData, lps28dfw_all_sources_t* status)
{
    lps28dfw_fifo_status2_t fifo_status2;
    lps28dfw_int_source_t int_source;
    lps28dfw_status_t statusVal;
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_STATUS, (uint8_t *)&statusVal, 1);
    if (ret == 0)
    {
        ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_INT_SOURCE,
                                (uint8_t *)&int_source, 1);
    }
    if (ret == 0)
    {
        ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_FIFO_STATUS2,
                                (uint8_t *)&fifo_status2, 1);
    }

    status->drdy_pres        = statusVal.p_da;
    status->drdy_temp        = statusVal.t_da;
    status->over_pres        = int_source.ph;
    status->under_pres       = int_source.pl;
    status->thrsld_pres      = int_source.ia;
    status->fifo_full        = fifo_status2.fifo_full_ia;
    status->fifo_ovr         = fifo_status2.fifo_ovr_ia;
    status->fifo_th          = fifo_status2.fifo_wtm_ia;

    return ret;
}

/// @brief Sets the FIFO config
/// @param fifoConfig FIFO config, see lps28dfw_fifo_md_t
/// @return Error code. 0 means success, negative means failure
int32_t setFIFOConfig(lps28dfw_info_t* sensorData, lps28dfw_fifo_md_t* fifoConfig)
{
    lps28dfw_fifo_ctrl_t fifo_ctrl;
    lps28dfw_fifo_wtm_t fifo_wtm;
    uint8_t reg[2];
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_FIFO_CTRL, reg, 2);
    if (ret == 0)
    {
        bytecpy((uint8_t *)&fifo_ctrl, &reg[0]);
        bytecpy((uint8_t *)&fifo_wtm, &reg[1]);

        fifo_ctrl.f_mode = (uint8_t)fifoConfig->operation & 0x03U;
        fifo_ctrl.trig_modes = ((uint8_t)fifoConfig->operation & 0x04U) >> 2;

        if (fifoConfig->watermark != 0x00U)
        {
            fifo_ctrl.stop_on_wtm = PROPERTY_ENABLE;
        }
        else
        {
            fifo_ctrl.stop_on_wtm = PROPERTY_DISABLE;
        }

        fifo_wtm.wtm = fifoConfig->watermark;

        bytecpy(&reg[0], (uint8_t *)&fifo_ctrl);
        bytecpy(&reg[1], (uint8_t *)&fifo_wtm);

        ret = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_FIFO_CTRL, reg, 2);
    }
    return ret;
}

/// @brief Gets the FIFO config
/// @param fifoConfig FIFO config, see lps28dfw_fifo_md_t
/// @return Error code. 0 means success, negative means failure
int32_t getFIFOConfig(lps28dfw_info_t* sensorData, lps28dfw_fifo_md_t* fifoConfig)
{
    lps28dfw_fifo_ctrl_t fifo_ctrl;
    lps28dfw_fifo_wtm_t fifo_wtm;
    uint8_t reg[2];
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_FIFO_CTRL, reg, 2);

    bytecpy((uint8_t *)&fifo_ctrl, &reg[0]);
    bytecpy((uint8_t *)&fifo_wtm, &reg[1]);

    switch ((fifo_ctrl.trig_modes << 2) | fifo_ctrl.f_mode)
    {
    case fifoOperation_LPS28DFW_BYPASS:
        fifoConfig->operation = fifoOperation_LPS28DFW_BYPASS;
        break;
    case fifoOperation_LPS28DFW_FIFO:
        fifoConfig->operation = fifoOperation_LPS28DFW_FIFO;
        break;
    case fifoOperation_LPS28DFW_STREAM:
        fifoConfig->operation = fifoOperation_LPS28DFW_STREAM;
        break;
    case fifoOperation_LPS28DFW_STREAM_TO_FIFO:
        fifoConfig->operation = fifoOperation_LPS28DFW_STREAM_TO_FIFO;
        break;
    case fifoOperation_LPS28DFW_BYPASS_TO_STREAM:
        fifoConfig->operation = fifoOperation_LPS28DFW_BYPASS_TO_STREAM;
        break;
    case fifoOperation_LPS28DFW_BYPASS_TO_FIFO:
        fifoConfig->operation = fifoOperation_LPS28DFW_BYPASS_TO_FIFO;
        break;
    default:
        fifoConfig->operation = fifoOperation_LPS28DFW_BYPASS;
        break;
    }

    fifoConfig->watermark = fifo_wtm.wtm;

    return ret;
}

/// @brief Gets the number of data samples stored in the FIFO buffer, up to 128
/// @param numData Number of data samples
/// @return Error code. 0 means success, negative means failure
int32_t getFIFOLength(lps28dfw_info_t* sensorData, uint8_t* numData)
{

    lps28dfw_fifo_status1_t fifo_status1;
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_FIFO_STATUS1,
                            (uint8_t *)&fifo_status1, 1);

    *numData = fifo_status1.fss;

    return ret;
}

/// @brief Gets pressure data out of FIFO buffer
/// @param data Array of data structs, see lps28dfw_fifo_data_t
/// @param numData Number of data samples to read
/// @return Error code. 0 means success, negative means failure
int32_t getFIFOData(lps28dfw_info_t* sensorData, lps28dfw_fifo_data_t* data, uint8_t numData)
{
    uint8_t fifo_data[3];
    uint8_t i;
    int32_t ret = 0;

    for (i = 0U; i < numData; i++)
    {
        ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_FIFO_DATA_OUT_PRESS_XL, fifo_data, 3);
        data[i].raw = (int32_t)fifo_data[2];
        data[i].raw = (data[i].raw * 256) + (int32_t)fifo_data[1];
        data[i].raw = (data[i].raw * 256) + (int32_t)fifo_data[0];
        data[i].raw = (data[i].raw * 256);

        switch (sensorData->modeConfig.fs)
        {
        case fs_LPS28DFW_1260hPa:
            data[i].hpa = lps28dfw_from_fs1260_to_hPa(data[i].raw);
            break;
        case fs_LPS28DFW_4000hPa:
            data[i].hpa = lps28dfw_from_fs4000_to_hPa(data[i].raw);
            break;
        default:
            data[i].hpa = 0.0f;
            break;
        }

    }

    return ret;
}

/// @brief Clears all data in FIFO buffer
/// @return Error code. 0 means success, negative means failure
int32_t flushFIFO(lps28dfw_info_t* sensorData)
{
    // Variable to track errors returned by API calls
    int8_t err = LPS28DFW_OK;

    // Get current FIFO config
    lps28dfw_fifo_md_t oldConfig;
    lps28dfw_fifo_md_t newConfig;
    err = getFIFOConfig(sensorData, &oldConfig);
    if(err != LPS28DFW_OK)
    {
        return err;
    }

    // FIFO can be flushed by setting to bypass mode
    memcpy(&newConfig, &oldConfig, sizeof(lps28dfw_fifo_md_t));
    newConfig.operation = fifoOperation_LPS28DFW_BYPASS;
    err = getFIFOConfig(sensorData, &newConfig);
    if(err != LPS28DFW_OK)
    {
        return err;
    }

    // Restore original configuration
    return setFIFOConfig(sensorData, &oldConfig);
}

/// @brief Sets reference mode, where measurements are relative to a reference
// pressure. If mode.get_ref is 1, this updates the reference pressure
/// @param mode Reference mode struct, see lps28dfw_ref_md_t
/// @return Error code. 0 means success, negative means failure
int32_t setReferenceMode(lps28dfw_info_t* sensorData, lps28dfw_ref_md_t* config)
{
    lps28dfw_interrupt_cfg_t interrupt_cfg;
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_INTERRUPT_CFG,
                            (uint8_t *)&interrupt_cfg, 1);
    if (ret == 0)
    {

        interrupt_cfg.autozero = config->get_ref;
        interrupt_cfg.autorefp = (uint8_t)config->apply_ref & 0x01U;

        interrupt_cfg.reset_az  = ((uint8_t)config->apply_ref & 0x02U) >> 1;
        interrupt_cfg.reset_arp = ((uint8_t)config->apply_ref & 0x02U) >> 1;

        // ret = lps28dfw_read_reg(ctx, LPS28DFW_INTERRUPT_CFG,
        //                         (uint8_t *)&interrupt_cfg, 1);
        ret = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_INTERRUPT_CFG,
                                 (uint8_t *)&interrupt_cfg, 1);
    }
    return ret;
}

/**
  * @brief  Configuration of Wake-up and Wake-up to Sleep .[set]
  *
  * @param  ctx   communication interface handler.(ptr)
  * @param  val   parameters of configuration.(ptr)
  * @retval       interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t lps28dfw_int_on_threshold_mode_set(lps28dfw_info_t* sensorData,
        lps28dfw_int_th_md_t *val)
{
    lps28dfw_interrupt_cfg_t interrupt_cfg;
    lps28dfw_ths_p_l_t ths_p_l;
    lps28dfw_ths_p_h_t ths_p_h;
    uint8_t reg[3];
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_INTERRUPT_CFG, reg, 3);
    if (ret == 0)
    {
        bytecpy((uint8_t *)&interrupt_cfg, &reg[0]);
        bytecpy((uint8_t *)&ths_p_l, &reg[1]);
        bytecpy((uint8_t *)&ths_p_h, &reg[2]);

        interrupt_cfg.phe = val->over_th;
        interrupt_cfg.ple = val->under_th;
        ths_p_h.ths = (uint8_t)(val->threshold / 256U);
        ths_p_l.ths = (uint8_t)(val->threshold - (ths_p_h.ths * 256U));

        bytecpy(&reg[0], (uint8_t *)&interrupt_cfg);
        bytecpy(&reg[1], (uint8_t *)&ths_p_l);
        bytecpy(&reg[2], (uint8_t *)&ths_p_h);

        ret = lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_INTERRUPT_CFG, reg, 3);
    }
    return ret;
}

/// @brief Sets threshold interrupt, setReferenceMode() must be called first
/// @param mode Threshold interrupt mode, see lps28dfw_int_th_md_t
/// @return Error code. 0 means success, negative means failure
int32_t setThresholdMode(lps28dfw_info_t* sensorData, lps28dfw_int_th_md_t* mode)
{
    // Variable to track errors returned by API calls
    int32_t err = LPS28DFW_OK;

    err = lps28dfw_int_on_threshold_mode_set(sensorData, mode);
    if(err != LPS28DFW_OK)
    {
        return err;
    }

    // lps28dfw_int_on_threshold_mode_set() doesn't set the INT_EN bit in
    // CTRL_REG4, so we need to do that manually. First grab the contents of the
    // CTRL_REG4 register
    lps28dfw_ctrl_reg4_t reg4;
    uint8_t reg[1];
    err = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG4, reg, 1);
    if(err != LPS28DFW_OK)
    {
        return err;
    }
    memcpy(&reg4, reg, 1);

    // Now set the INT_EN bit accordingly
    reg4.int_en = mode->under_th | mode->over_th;
    memcpy(reg, &reg4, 1);
    return lps28dfw_write_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_CTRL_REG4, reg, 1);

    // return LPS28DFW_OK;
}

/// @brief Gets the current reference pressure after calling setReferenceMode()
/// @param pressRaw Raw pressure statusue. Can convert to hPa by dividing by either
// 16 or 8 if the full scale range is 1260hPa or 4000hPa respectively
/// @return Error code. 0 means success, negative means failure
int32_t getReferencePressure(lps28dfw_info_t* sensorData, int16_t *status)
{
    uint8_t reg[2];
    int32_t ret;

    ret = lps28dfw_read_reg(sensorData->handle, sensorData->i2cAddress, LPS28DFW_REF_P_L, reg, 2);

    *status = (int16_t)reg[1];
    *status = *status * 256 + (int16_t)reg[0];

    return ret;
}

