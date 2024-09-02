/*
 * PressureSensing.h
 *
 *  Created on: Feb 11, 2023
 *      Author: Admin
 */

#ifndef INC_PRESSURE_PRESSURESENSING_H_
#define INC_PRESSURE_PRESSURESENSING_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <retarget.h>
#include "stm32l4xx_hal.h"

//platform
#define STM32_USE 2

#ifndef DRV_BYTE_ORDER
#ifndef __BYTE_ORDER__

#define DRV_LITTLE_ENDIAN 1234
#define DRV_BIG_ENDIAN    4321

/** if _BYTE_ORDER is not defined, choose the endianness of your architecture
  * by uncommenting the define which fits your platform endianness
  */
//#define DRV_BYTE_ORDER    DRV_BIG_ENDIAN
#define DRV_BYTE_ORDER    DRV_LITTLE_ENDIAN

#else /* defined __BYTE_ORDER__ */

#define DRV_LITTLE_ENDIAN  __ORDER_LITTLE_ENDIAN__
#define DRV_BIG_ENDIAN     __ORDER_BIG_ENDIAN__
#define DRV_BYTE_ORDER     __BYTE_ORDER__

#endif /* __BYTE_ORDER__*/
#endif /* DRV_BYTE_ORDER */

/**
  * @}
  *
  */

/** @defgroup STMicroelectronics sensors common types
  * @{
  *
  */

#ifndef MEMS_SHARED_TYPES
#define MEMS_SHARED_TYPES

typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0       : 1;
    uint8_t bit1       : 1;
    uint8_t bit2       : 1;
    uint8_t bit3       : 1;
    uint8_t bit4       : 1;
    uint8_t bit5       : 1;
    uint8_t bit6       : 1;
    uint8_t bit7       : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7       : 1;
    uint8_t bit6       : 1;
    uint8_t bit5       : 1;
    uint8_t bit4       : 1;
    uint8_t bit3       : 1;
    uint8_t bit2       : 1;
    uint8_t bit1       : 1;
    uint8_t bit0       : 1;
#endif /* DRV_BYTE_ORDER */
} bitwise_t;

#define PROPERTY_DISABLE                (0U)
#define PROPERTY_ENABLE                 (1U)

#endif /* MEMS_SHARED_TYPES */

/**
  * @}
  *
  */

/** @defgroup LPS28DFW_Infos
  * @{
  *
  */

/** I2C Device Address 8 bit format **/
#define LPS28DFW_I2C_ADD_L               0xB9U
#define LPS28DFW_I2C_ADD_H               0xBBU

/** Device Identification (Who am I) **/
#define LPS28DFW_WHOAMI                  0xB4U

/**
  * @}
  *
  */

#define LPS28DFW_INTERRUPT_CFG           0x0BU
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t phe              : 1;
    uint8_t ple              : 1;
    uint8_t lir              : 1;
    uint8_t not_used_01      : 1;
    uint8_t reset_az         : 1;
    uint8_t autozero         : 1;
    uint8_t reset_arp        : 1;
    uint8_t autorefp         : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t autorefp         : 1;
    uint8_t reset_arp        : 1;
    uint8_t autozero         : 1;
    uint8_t reset_az         : 1;
    uint8_t not_used_01      : 1;
    uint8_t lir              : 1;
    uint8_t ple              : 1;
    uint8_t phe              : 1;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_interrupt_cfg_t;

#define LPS28DFW_THS_P_L                 0x0CU
typedef struct
{
    uint8_t ths              : 8;
} lps28dfw_ths_p_l_t;

#define LPS28DFW_THS_P_H                 0x0DU
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t ths              : 7;
    uint8_t not_used_01      : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01      : 1;
    uint8_t ths              : 7;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_ths_p_h_t;

#define LPS28DFW_IF_CTRL                 0x0EU
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t not_used_01      : 2;
    uint8_t int_pd_dis      : 1;
    uint8_t not_used_02      : 1;
    uint8_t sda_pu_en        : 1;
    uint8_t not_used_03      : 2;
    uint8_t int_en_i3c       : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t int_en_i3c       : 1;
    uint8_t not_used_03      : 2;
    uint8_t sda_pu_en        : 1;
    uint8_t not_used_02      : 1;
    uint8_t int_pd_dis      : 1;
    uint8_t not_used_01      : 2;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_if_ctrl_t;

#define LPS28DFW_WHOAMI_ADDR             0x0FU
#define LPS28DFW_CTRL_REG1               0x10U
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t avg              : 3;
    uint8_t odr              : 4;
    uint8_t not_used_01      : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01      : 1;
    uint8_t odr              : 4;
    uint8_t avg              : 3;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_ctrl_reg1_t;

#define LPS28DFW_CTRL_REG2               0x11U
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t oneshot          : 1;
    uint8_t not_used_01      : 1;
    uint8_t swreset          : 1;
    uint8_t bdu              : 1;
    uint8_t en_lpfp          : 1;
    uint8_t lfpf_cfg         : 1;
    uint8_t fs_mode          : 1;
    uint8_t boot             : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t boot             : 1;
    uint8_t fs_mode          : 1;
    uint8_t lfpf_cfg         : 1;
    uint8_t en_lpfp          : 1;
    uint8_t bdu              : 1;
    uint8_t swreset          : 1;
    uint8_t not_used_01      : 1;
    uint8_t oneshot          : 1;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_ctrl_reg2_t;

#define LPS28DFW_CTRL_REG3               0x12U
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t if_add_inc       : 1;
    uint8_t pp_od            : 1;
    uint8_t not_used_02      : 1;
    uint8_t int_h_l          : 1;
    uint8_t not_used_01      : 4;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01      : 4;
    uint8_t int_h_l          : 1;
    uint8_t not_used_02      : 1;
    uint8_t pp_od            : 1;
    uint8_t if_add_inc       : 1;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_ctrl_reg3_t;

#define LPS28DFW_CTRL_REG4               0x13U
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t int_f_ovr        : 1;
    uint8_t int_f_wtm        : 1;
    uint8_t int_f_full       : 1;
    uint8_t not_used_02      : 1;
    uint8_t int_en           : 1;
    uint8_t drdy             : 1;
    uint8_t drdy_pls         : 1;
    uint8_t not_used_01      : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01      : 1;
    uint8_t drdy_pls         : 1;
    uint8_t drdy             : 1;
    uint8_t int_en           : 1;
    uint8_t not_used_02      : 1;
    uint8_t int_f_full       : 1;
    uint8_t int_f_wtm        : 1;
    uint8_t int_f_ovr        : 1;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_ctrl_reg4_t;

#define LPS28DFW_FIFO_CTRL               0x14U
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t f_mode           : 2;
    uint8_t trig_modes       : 1;
    uint8_t stop_on_wtm      : 1;
    uint8_t not_used_01      : 4;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01      : 4;
    uint8_t stop_on_wtm      : 1;
    uint8_t trig_modes       : 1;
    uint8_t f_mode           : 2;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_fifo_ctrl_t;

#define LPS28DFW_FIFO_WTM                0x15U
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t wtm              : 7;
    uint8_t not_used_01      : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01      : 1;
    uint8_t wtm              : 7;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_fifo_wtm_t;

#define LPS28DFW_REF_P_L                 0x16U
typedef struct
{
    uint8_t refp             : 8;
} lps28dfw_ref_p_l_t;

#define LPS28DFW_REF_P_H                 0x17U
typedef struct
{
    uint8_t refp             : 8;
} lps28dfw_ref_p_h_t;

#define LPS28DFW_I3C_IF_CTRL_ADD         0x19U
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t I3C_Bus_Avb_Sel  : 2;
    uint8_t not_used_02      : 3;
    uint8_t asf_on           : 1;
    uint8_t not_used_01      : 2;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01      : 2;
    uint8_t asf_on           : 1;
    uint8_t not_used_02      : 3;
    uint8_t I3C_Bus_Avb_Sel  : 2;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_i3c_if_ctrl_add_t;

#define LPS28DFW_RPDS_L                  0x1AU
#define LPS28DFW_RPDS_H                  0x1BU
#define LPS28DFW_INT_SOURCE              0x24U
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t ph               : 1;
    uint8_t pl               : 1;
    uint8_t ia               : 1;
    uint8_t not_used_01      : 4;
    uint8_t boot_on          : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t boot_on          : 1;
    uint8_t not_used_01      : 4;
    uint8_t ia               : 1;
    uint8_t pl               : 1;
    uint8_t ph               : 1;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_int_source_t;

#define LPS28DFW_FIFO_STATUS1            0x25U
typedef struct
{
    uint8_t fss              : 8;
} lps28dfw_fifo_status1_t;

#define LPS28DFW_FIFO_STATUS2            0x26U
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t not_used_01      : 5;
    uint8_t fifo_full_ia     : 1;
    uint8_t fifo_ovr_ia      : 1;
    uint8_t fifo_wtm_ia      : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t fifo_wtm_ia      : 1;
    uint8_t fifo_ovr_ia      : 1;
    uint8_t fifo_full_ia     : 1;
    uint8_t not_used_01      : 5;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_fifo_status2_t;

#define LPS28DFW_STATUS                  0x27U
typedef struct
{
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t p_da             : 1;
    uint8_t t_da             : 1;
    uint8_t not_used_01      : 2;
    uint8_t p_or             : 1;
    uint8_t t_or             : 1;
    uint8_t not_used_02      : 2;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_02      : 2;
    uint8_t t_or             : 1;
    uint8_t p_or             : 1;
    uint8_t not_used_01      : 2;
    uint8_t t_da             : 1;
    uint8_t p_da             : 1;
#endif /* DRV_BYTE_ORDER */
} lps28dfw_status_t;

#define LPS28DFW_PRESS_OUT_XL            0x28U
#define LPS28DFW_PRESS_OUT_L             0x29U
#define LPS28DFW_PRESS_OUT_H             0x2AU
#define LPS28DFW_TEMP_OUT_L              0x2BU
#define LPS28DFW_TEMP_OUT_H              0x2CU
#define LPS28DFW_FIFO_DATA_OUT_PRESS_XL  0x78U
#define LPS28DFW_FIFO_DATA_OUT_PRESS_L   0x79U
#define LPS28DFW_FIFO_DATA_OUT_PRESS_H   0x7AU

typedef union
{
    lps28dfw_interrupt_cfg_t    interrupt_cfg;
    lps28dfw_ths_p_l_t          ths_p_l;
    lps28dfw_ths_p_h_t          ths_p_h;
    lps28dfw_if_ctrl_t          if_ctrl;
    lps28dfw_ctrl_reg1_t        ctrl_reg1;
    lps28dfw_ctrl_reg2_t        ctrl_reg2;
    lps28dfw_ctrl_reg3_t        ctrl_reg3;
    lps28dfw_ctrl_reg4_t        ctrl_reg4;
    lps28dfw_fifo_ctrl_t        fifo_ctrl;
    lps28dfw_fifo_wtm_t         fifo_wtm;
    lps28dfw_ref_p_l_t          ref_p_l;
    lps28dfw_ref_p_h_t          ref_p_h;
    lps28dfw_i3c_if_ctrl_add_t  i3c_if_ctrl_add;
    lps28dfw_int_source_t       int_source;
    lps28dfw_fifo_status1_t     fifo_status1;
    lps28dfw_fifo_status2_t     fifo_status2;
    lps28dfw_status_t           status;
    bitwise_t                  bitwise;
    uint8_t                    byte;
} lps28dfw_reg_t;

typedef struct
{
    uint8_t whoami;
} lps28dfw_id_t;

#define LPS28DFW_DRV_RDY	0x00
#define LPS28DFW_BOOT    	0x01
#define LPS28DFW_RESET   	0x02

typedef struct
{
    uint8_t sw_reset  : 1; /* Restoring configuration registers. */
    uint8_t boot      : 1; /* Restoring calibration parameters. */
    uint8_t drdy_pres : 1; /* Pressure data ready. */
    uint8_t drdy_temp : 1; /* Temperature data ready. */
    uint8_t ovr_pres  : 1; /* Pressure data overrun. */
    uint8_t ovr_temp  : 1; /* Temperature data overrun. */
    uint8_t end_meas  : 1; /* Single measurement is finished. */
    uint8_t ref_done  : 1; /* Auto-Zero value is set. */
} lps28dfw_stat_t;

typedef struct
{
    uint8_t int_push_pull : 1; /* 1 = push-pull / 0 = open-drain*/
    uint8_t int_pull_down : 1; /* 1 = pull-down always disabled (0=auto) */
    uint8_t sda_pull_up : 1; /* 1 = pull-up always disabled */
} lps28dfw_pin_conf_t;

typedef struct
{
    uint8_t drdy_pres   :  1; /* Pressure data ready */
    uint8_t drdy_temp   :  1; /* Temperature data ready */
    uint8_t over_pres   :  1; /* Over pressure event */
    uint8_t under_pres  :  1; /* Under pressure event */
    uint8_t thrsld_pres :  1; /* Over/Under pressure event */
    uint8_t fifo_full   :  1; /* FIFO full */
    uint8_t fifo_ovr    :  1; /* FIFO overrun */
    uint8_t fifo_th     :  1; /* FIFO threshold reached */
} lps28dfw_all_sources_t;


#define fs_LPS28DFW_1260hPa 	0x00
#define fs_LPS28DFW_4000hPa 	0x01

#define odr_LPS28DFW_ONE_SHOT	0x00 /* Device in power down till software trigger */
#define odr_LPS28DFW_1Hz		0x01
#define odr_LPS28DFW_4Hz		0x02
#define odr_LPS28DFW_10Hz		0x03
#define odr_LPS28DFW_25Hz		0x04
#define odr_LPS28DFW_50Hz		0x05
#define odr_LPS28DFW_75Hz		0x06
#define odr_LPS28DFW_100Hz		0x07
#define odr_LPS28DFW_200Hz		0x08

#define avg_LPS28DFW_4_AVG		0
#define avg_LPS28DFW_8_AVG		1
#define avg_LPS28DFW_16_AVG		2
#define avg_LPS28DFW_32_AVG		3
#define avg_LPS28DFW_64_AVG		4
#define avg_LPS28DFW_128_AVG	5
#define avg_LPS28DFW_256_AVG	6
#define avg_LPS28DFW_512_AVG	7

#define lpf_LPS28DFW_LPF_DISABLE	0
#define lpf_LPS28DFW_LPF_ODR_DIV_4	1
#define lpf_LPS28DFW_LPF_ODR_DIV_9	3

typedef struct
{
	uint8_t 	fs;
	uint8_t 	odr;
	uint8_t 	avg;
	uint8_t 	lpf;

} lps28dfw_md_t;

typedef struct
{
    struct
    {
        // float_t hpa;
        float hpa;
        int32_t raw; /* 32 bit signed-left algned  format left  */
    } pressure;
    struct
    {
        // float_t deg_c;
        float deg_c;
        int16_t raw;
    } heat;
} lps28dfw_data_t;

#define fifoOperation_LPS28DFW_BYPASS           0
#define fifoOperation_LPS28DFW_FIFO             1
#define fifoOperation_LPS28DFW_STREAM           2
#define fifoOperation_LPS28DFW_STREAM_TO_FIFO   7 /* Dynamic-Stream, FIFO on Trigger */
#define fifoOperation_LPS28DFW_BYPASS_TO_STREAM 6 /* Bypass, Dynamic-Stream on Trigger */
#define fifoOperation_LPS28DFW_BYPASS_TO_FIFO   5
typedef struct
{
	uint8_t operation;
    uint8_t watermark; /* (0 disable) max 128.*/
} lps28dfw_fifo_md_t;

typedef struct
{
    // float_t hpa;
    float hpa;
    int32_t raw;
} lps28dfw_fifo_data_t;

typedef struct
{
    uint8_t int_latched  : 1; /* int events are: int on threshold, FIFO */
    uint8_t active_low   : 1; /* 1 = active low / 0 = active high */
    uint8_t drdy_latched : 1; /* pulsed ~5 μs with enabled drdy_pres " */
} lps28dfw_int_mode_t;

typedef struct
{
    uint8_t drdy_pres : 1; /* Pressure data ready */
    uint8_t fifo_th   : 1; /* FIFO threshold reached */
    uint8_t fifo_ovr  : 1; /* FIFO overrun */
    uint8_t fifo_full : 1; /* FIFO full */
} lps28dfw_pin_int_route_t;

typedef struct
{
    uint16_t threshold;   /* Threshold in hPa * 16 (@1260hPa)
                         * Threshold in hPa * 8  (@4000hPa)
                         */
    uint8_t over_th  : 1; /* Pressure data over threshold event */
    uint8_t under_th : 1; /* Pressure data under threshold event */
} lps28dfw_int_th_md_t;

#define ref_LPS28DFW_OUT_AND_INTERRUPT 0
#define ref_LPS28DFW_ONLY_INTERRUPT    1
#define ref_LPS28DFW_RST_REFS          2
typedef struct
{
	uint8_t apply_ref;
    uint8_t get_ref : 1; /* Use current pressure value as reference */
} lps28dfw_ref_md_t;



/**
  *@}
  *
  */

#define LPS28DFW_OK                 INT32_C(0)
#define LPS28DFW_E_NOT_CONNECTED    INT32_C(-1)
#define LPS28DFW_E_COM_FAIL         INT32_C(-2)

// Define I2C addresses. LPS28DFW_I2C_ADD_L/H are 8-bit values, need 7-bit
#define LPS28DFW_I2C_ADDRESS_DEFAULT (LPS28DFW_I2C_ADD_L >> 1)   // 0x5C
#define LPS28DFW_I2C_ADDRESS_SECONDARY (LPS28DFW_I2C_ADD_H >> 1) // 0x5D

typedef struct
{
	I2C_HandleTypeDef*	handle;
    uint8_t 			i2cAddress;
    lps28dfw_md_t 		modeConfig;
    lps28dfw_data_t 	data;

} lps28dfw_info_t;

int32_t lps28dfw_read_reg(I2C_HandleTypeDef*, uint8_t, uint8_t, uint8_t*, uint16_t);

int32_t lps28dfw_write_reg(I2C_HandleTypeDef*, uint8_t, uint8_t, uint8_t*, uint16_t);

float lps28dfw_from_fs1260_to_hPa(int32_t);
float lps28dfw_from_fs4000_to_hPa(int32_t);

float lps28dfw_from_lsb_to_celsius(int16_t);

// Sensor initialization, must specify communication interface
int32_t beginLPS28(lps28dfw_info_t*, uint8_t);

// Configuration control
int32_t initLPS28(lps28dfw_info_t*, uint8_t);
int32_t bootLPS28(lps28dfw_info_t*);
int32_t resetLPS28(lps28dfw_info_t*);
int32_t setModeConfig(lps28dfw_info_t*, lps28dfw_md_t*);
int32_t getModeConfig(lps28dfw_info_t*);
int32_t getStatus(lps28dfw_info_t*, lps28dfw_stat_t*);

// Data acquisistion
int32_t getSensorData(lps28dfw_info_t*);

// Interrupt control
int32_t setInterruptMode(lps28dfw_info_t*, lps28dfw_int_mode_t*);
int32_t enableInterrupts(lps28dfw_info_t*, lps28dfw_pin_int_route_t*);
int32_t getInterruptStatus(lps28dfw_info_t*, lps28dfw_all_sources_t*);

// FIFO control
int32_t setFIFOConfig(lps28dfw_info_t*, lps28dfw_fifo_md_t*);
int32_t getFIFOConfig(lps28dfw_info_t*, lps28dfw_fifo_md_t*);
int32_t getFIFOLength(lps28dfw_info_t*, uint8_t*);
int32_t getFIFOData(lps28dfw_info_t*, lps28dfw_fifo_data_t*, uint8_t);
int32_t flushFIFO(lps28dfw_info_t*);

// Reference mode control
int32_t setReferenceMode(lps28dfw_info_t*, lps28dfw_ref_md_t*);
int32_t setThresholdMode(lps28dfw_info_t*, lps28dfw_int_th_md_t*);
int32_t getReferencePressure(lps28dfw_info_t*, int16_t*);

#endif /* INC_PRESSURE_PRESSURESENSING_H_ */
