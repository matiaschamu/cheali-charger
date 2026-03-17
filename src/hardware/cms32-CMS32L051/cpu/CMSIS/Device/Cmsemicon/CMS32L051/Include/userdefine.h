#ifndef _USER_DEF_H
#define _USER_DEF_H

typedef unsigned short MD_STATUS;
/* Status list definition */
#define MD_STATUSBASE        (0x00U)
#define MD_OK                (MD_STATUSBASE + 0x00U) /* register setting OK */
#define MD_SPT               (MD_STATUSBASE + 0x01U) /* IIC stop */
#define MD_NACK              (MD_STATUSBASE + 0x02U) /* IIC no ACK */
#define MD_BUSY1             (MD_STATUSBASE + 0x03U) /* busy 1 */
#define MD_BUSY2             (MD_STATUSBASE + 0x04U) /* busy 2 */
#define MD_OVERRUN           (MD_STATUSBASE + 0x05U) /* IIC OVERRUN occur */

/* Error list definition */
#define MD_ERRORBASE         (0x80U)
#define MD_ERROR             (MD_ERRORBASE + 0x00U)  /* error */
#define MD_ARGERROR          (MD_ERRORBASE + 0x01U)  /* error agrument input error */
#define MD_ERROR1            (MD_ERRORBASE + 0x02U)  /* error 1 */
#define MD_ERROR2            (MD_ERRORBASE + 0x03U)  /* error 2 */
#define MD_ERROR3            (MD_ERRORBASE + 0x04U)  /* error 3 */
#define MD_ERROR4            (MD_ERRORBASE + 0x05U)  /* error 4 */
#define MD_ERROR5            (MD_ERRORBASE + 0x06U)  /* error 5 */

typedef enum 
{
    SPI_MODE_0 = 0,  // Mode 0: CPOL = 0, CPHA = 0; i.e. CKP = 1, DAP = 1 
    SPI_MODE_1 = 1,  // Mode 1: CPOL = 0, CPHA = 1; i.e. CKP = 1, DAP = 0 
    SPI_MODE_2 = 2,  // Mode 2: CPOL = 1, CPHA = 0; i.e. CKP = 0, DAP = 1 
    SPI_MODE_3 = 3,  // Mode 3: CPOL = 1, CPHA = 1; i.e. CKP = 0, DAP = 0 
} spi_mode_t;


#define ADC_PORT_SETTING() do {} while(0)
#define SCL00_PORT_SETTING() do {} while(0)
#define SDA00_PORT_SETTING() do {} while(0)
#define TXD0_PORT_SETTING() do {} while(0)
#define RXD0_PORT_SETTING() do {} while(0)


#define TO00_PORT_SETTING() do {} while(0)
#define TO01_PORT_SETTING() do {} while(0)
#define TO02_PORT_SETTING() do {} while(0)
#define TO03_PORT_SETTING() do {} while(0)
#define TI00_PORT_SETTING() do {} while(0)
#define TI01_PORT_SETTING() do {} while(0)
#define TI02_PORT_SETTING() do {} while(0)
#define TI03_PORT_SETTING() do {} while(0)

#define TO10_PORT_SETTING() do {} while(0)
#define TO11_PORT_SETTING() do {} while(0)
#define TO12_PORT_SETTING() do {} while(0)
#define TO13_PORT_SETTING() do {} while(0)
#define TI10_PORT_SETTING() do {} while(0)
#define TI11_PORT_SETTING() do {} while(0)
#define TI12_PORT_SETTING() do {} while(0)
#define TI13_PORT_SETTING() do {} while(0)

#define SS00_PORT_SETTING()   do {} while(0)
#define SCLKO00_PORT_SETTING() do {} while(0)
#define SDI00_PORT_SETTING()   do {} while(0)
#define SDO00_PORT_SETTING()   do {} while(0)
#define SS00_PORT_CLR()        do {} while(0)
#define SS00_PORT_SET()        do {} while(0)
#define SCLKI00_PORT_SETTING() do {} while(0)

#define SS01_PORT_SETTING()   do {} while(0)
#define SCLKO01_PORT_SETTING() do {} while(0)
#define SDI01_PORT_SETTING()   do {} while(0)
#define SDO01_PORT_SETTING()   do {} while(0)
#define SS01_PORT_CLR()        do {} while(0)
#define SS01_PORT_SET()        do {} while(0)
#define SCLKI01_PORT_SETTING() do {} while(0)

#define SCL01_PORT_SETTING()   do {} while(0)
#define SDA01_PORT_SETTING()   do {} while(0)

#define TXD1_PORT_SETTING()    do {} while(0)
#define RXD1_PORT_SETTING()    do {} while(0)

#define SS10_PORT_SETTING()   do {} while(0)
#define SCLKO10_PORT_SETTING() do {} while(0)
#define SDI10_PORT_SETTING()   do {} while(0)
#define SDO10_PORT_SETTING()   do {} while(0)
#define SS10_PORT_CLR()        do {} while(0)
#define SS10_PORT_SET()        do {} while(0)
#define SCLKI10_PORT_SETTING() do {} while(0)

#define SS11_PORT_SETTING()   do {} while(0)
#define SCLKO11_PORT_SETTING() do {} while(0)
#define SDI11_PORT_SETTING()   do {} while(0)
#define SDO11_PORT_SETTING()   do {} while(0)
#define SS11_PORT_CLR()        do {} while(0)
#define SS11_PORT_SET()        do {} while(0)
#define SCLKI11_PORT_SETTING() do {} while(0)

#define SCL10_PORT_SETTING()   do {} while(0)
#define SDA10_PORT_SETTING()   do {} while(0)
#define SCL11_PORT_SETTING()   do {} while(0)
#define SDA11_PORT_SETTING()   do {} while(0)

#define TXD2_PORT_SETTING()    do {} while(0)
#define RXD2_PORT_SETTING()    do {} while(0)

#define SS20_PORT_SETTING()   do {} while(0)
#define SCLKO20_PORT_SETTING() do {} while(0)
#define SDI20_PORT_SETTING()   do {} while(0)
#define SDO20_PORT_SETTING()   do {} while(0)
#define SS20_PORT_CLR()        do {} while(0)
#define SS20_PORT_SET()        do {} while(0)
#define SCLKI20_PORT_SETTING() do {} while(0)

#define SS21_PORT_SETTING()   do {} while(0)
#define SCLKO21_PORT_SETTING() do {} while(0)
#define SDI21_PORT_SETTING()   do {} while(0)
#define SDO21_PORT_SETTING()   do {} while(0)
#define SS21_PORT_CLR()        do {} while(0)
#define SS21_PORT_SET()        do {} while(0)
#define SCLKI21_PORT_SETTING() do {} while(0)

#define SCL20_PORT_SETTING()   do {} while(0)
#define SDA20_PORT_SETTING()   do {} while(0)
#define SCL21_PORT_SETTING()   do {} while(0)
#define SDA21_PORT_SETTING()   do {} while(0)

#endif
