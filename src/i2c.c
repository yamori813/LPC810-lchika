// http://d.hatena.ne.jp/mikkabo/20150401/1427899147

#include "LPC8xx.h"
#include "mrt.h"

typedef int ErrorCode_t; // 本当はenumだけど今回は値は見ない
typedef int CHIP_I2C_MODE_T; // これも本当はenum
typedef void *I2C_HANDLE_T;
typedef void (*I2C_CALLBK_T)(uint32_t err_code, uint32_t n);

typedef struct I2C_PARAM {
    uint32_t        num_bytes_send;
    uint32_t        num_bytes_rec;
    uint8_t         *buffer_ptr_send;
    uint8_t         *buffer_ptr_rec;
    I2C_CALLBK_T    func_pt;
    uint8_t         stop_flag;
    uint8_t         dummy[3];
} I2C_PARAM_T;

typedef struct I2C_RESULT {
    uint32_t n_bytes_sent;
    uint32_t n_bytes_recd;
} I2C_RESULT_T;

typedef struct  I2CD_API {
    void (*i2c_isr_handler)(I2C_HANDLE_T *handle);
    ErrorCode_t (*i2c_master_transmit_poll)(I2C_HANDLE_T *handle, I2C_PARAM_T *param, I2C_RESULT_T *result);
    ErrorCode_t (*i2c_master_receive_poll)(I2C_HANDLE_T *handle, I2C_PARAM_T *param, I2C_RESULT_T *result);
    ErrorCode_t (*i2c_master_tx_rx_poll)(I2C_HANDLE_T *handle, I2C_PARAM_T *param, I2C_RESULT_T *result);
    ErrorCode_t (*i2c_master_transmit_intr)(I2C_HANDLE_T *handle, I2C_PARAM_T *param, I2C_RESULT_T *result);
    ErrorCode_t (*i2c_master_receive_intr)(I2C_HANDLE_T *handle, I2C_PARAM_T *param, I2C_RESULT_T *result);
    ErrorCode_t (*i2c_master_tx_rx_intr)(I2C_HANDLE_T *handle, I2C_PARAM_T *param, I2C_RESULT_T *result);
    ErrorCode_t (*i2c_slave_receive_poll)(I2C_HANDLE_T *handle, I2C_PARAM_T *param, I2C_RESULT_T *result);
    ErrorCode_t (*i2c_slave_transmit_poll)(I2C_HANDLE_T *handle, I2C_PARAM_T *param, I2C_RESULT_T *result);
    ErrorCode_t (*i2c_slave_receive_intr)(I2C_HANDLE_T *handle, I2C_PARAM_T *param, I2C_RESULT_T *result);
    ErrorCode_t (*i2c_slave_transmit_intr)(I2C_HANDLE_T *handle, I2C_PARAM_T *param, I2C_RESULT_T *result);
    ErrorCode_t (*i2c_set_slave_addr)(I2C_HANDLE_T *handle, uint32_t slave_addr_0_3, uint32_t slave_mask_0_3);
    uint32_t        (*i2c_get_mem_size)(void);
    I2C_HANDLE_T *  (*i2c_setup)( uint32_t  i2c_base_addr, uint32_t * start_of_ram);
    ErrorCode_t     (*i2c_set_bitrate)(I2C_HANDLE_T *handle, uint32_t  p_clk_in_hz, uint32_t bitrate_in_bps);
    uint32_t        (*i2c_get_firmware_version)(void);
    CHIP_I2C_MODE_T (*i2c_get_status)(I2C_HANDLE_T *handle);
    ErrorCode_t     (*i2c_set_timeout)(I2C_HANDLE_T *handle, uint32_t timeout);
} I2CD_API_T;

typedef struct ROM_API {
	const uint32_t    dummy1[5];
	const I2CD_API_T  *pI2CD;
	const uint32_t    dummy2[4];
} LPC_ROM_API_T;

#define LPC_I2CD_API ((*(LPC_ROM_API_T * *) 0x1FFF1FF8UL)->pI2CD)

static I2C_HANDLE_T *i2cHandleMaster;
static uint32_t i2cMasterHandleMEM[0x20];

static void i2ctrans(uint8_t addr, uint8_t d1, uint8_t d2)
{
    uint8_t data[3] = {addr, d1, d2};
    I2C_PARAM_T param;
    I2C_RESULT_T result;
	
    param.num_bytes_send  = 3; // 3byte送信
    param.buffer_ptr_send  = &data[0];
    param.num_bytes_rec  = 0; // 受信はしない(byte)
    param.stop_flag   = 1; // 送信終わったらstop
	
    // タイムアウト設定
    LPC_I2CD_API->i2c_set_timeout(i2cHandleMaster, 100000);
	
    // 送信
    LPC_I2CD_API->i2c_master_transmit_poll(i2cHandleMaster, &param, &result);
    // ここで戻り値が0(LP_OK)なら成功している。本例ではチェックしない。
}

int i2cinit(void) {
	// UM10601  16.3 Basic configuration
    // にしたがってI2Cの準備
    // In the SYSAHBCLKCTRL register, set bit 5 (Table 30) to enable the clock to theregister interface.
    LPC_SYSCON->SYSAHBCLKCTRL |= (0x1<<5);
	
    // Clear the I2C peripheral reset using the PRESETCTRL register (Table 19).
    LPC_SYSCON->PRESETCTRL &= ~(0x1<<6);
    LPC_SYSCON->PRESETCTRL |= (0x1<<6);
	
    //  Enable/disable the I2C interrupt in interrupt slots #8 in the NVIC.
    // -> defaultはdisableっぽいし、今回は割り込みいらないから略。
	
    //  Configure the I2C pin functions through the switch matrix. See Section 16.4.
    // -> 上の方ですでにやった。
	
    //  The peripheral clock for the I2C is the system clock (see Figure 32).
    //  -> これは単なる説明だよね？
    
    //  I2C-bus ROM APIを使って初期設定
    i2cHandleMaster = LPC_I2CD_API->i2c_setup(LPC_I2C_BASE, i2cMasterHandleMEM);
    LPC_I2CD_API->i2c_set_bitrate(i2cHandleMaster, 12000000, 100000); // 速度100KHz
	
    // 以下、LCDにコマンドを送る
    // (送信するコマンドの内容は、mbedにあったAQM0802ライブラリを真似した)
	mrtDelay(100);
	// Function set = 0x38
	i2ctrans(0x7c, 0x00, 0x38); mrtDelay(1);
	// Function set = 0x39
	i2ctrans(0x7c, 0x00, 0x39); mrtDelay(1);
	// Internal OSC frequency = 0x14
	i2ctrans(0x7c, 0x00, 0x14); mrtDelay(1);
	// Contrast set = 0x78
	i2ctrans(0x7c, 0x00, 0x78); mrtDelay(1);
	// Power/ICON/Contrast control = 0x55
	i2ctrans(0x7c, 0x00, 0x55);  mrtDelay(1);
	// Follower control = 0x6C
	i2ctrans(0x7c, 0x00, 0x6C); mrtDelay(200);
	// Function set = 0x38
	i2ctrans(0x7c, 0x00, 0x38); mrtDelay(1);
	// Display ON/OFF control = 0x0C
	i2ctrans(0x7c, 0x00, 0x0C); mrtDelay(1);
	// Clear Display = 0x01
	i2ctrans(0x7c, 0x00, 0x01); mrtDelay(2);
	
	// 'Hello'と表示
	i2ctrans(0x7c, 0x40, 'y');  i2ctrans(0x7c, 0x40, 'a');  i2ctrans(0x7c, 0x40, 'm');
	i2ctrans(0x7c, 0x40, 'o');  i2ctrans(0x7c, 0x40, 'r');  i2ctrans(0x7c, 0x40, 'i');
	
	return 0 ;
}
