#pragma once





#ifdef __cplusplus
extern "C" {
#endif

#include <esp_types.h>
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
#include "driver/gpio.h"
#include "soc/soc_caps.h"
#include "hal/i2c_types.h"

#define I2C_APB_CLK_FREQ  APB_CLK_FREQ /*!< I2C source clock is APB clock, 80MHz */

#define I2C_NUM_MAX            (SOC_I2C_NUM) /*!< I2C port max */
#define I2C_NUM_0              (0) /*!< I2C port 0 */
#if SOC_I2C_NUM >= 2
#define I2C_NUM_1              (1) /*!< I2C port 1 */
#endif

// I2C clk flags for users to use, can be expanded in the future.
#define I2C_SCLK_SRC_FLAG_FOR_NOMAL       (0)         /*!< Any one clock source that is available for the specified frequency may be choosen*/
#define I2C_SCLK_SRC_FLAG_AWARE_DFS       (1 << 0)    /*!< For REF tick clock, it won't change with APB.*/
#define I2C_SCLK_SRC_FLAG_LIGHT_SLEEP     (1 << 1)    /*!< For light sleep mode.*/

#define I2C_INTERNAL_STRUCT_SIZE (24)
#define I2C_LINK_RECOMMENDED_SIZE(TRANSACTIONS)     (2 * I2C_INTERNAL_STRUCT_SIZE + I2C_INTERNAL_STRUCT_SIZE * \
                                                        (5 * TRANSACTIONS)) /* Make the assumption that each transaction
                                                                             * of the user is surrounded by a "start", device address
                                                                             * and a "nack/ack" signal. Allocate one more room for
                                                                             * "stop" signal at the end.
                                                                             * Allocate 2 more internal struct size for headers.
                                                                             */

typedef struct{
    i2c_mode_t mode;     /*!< I2C mode */
    int sda_io_num;      /*!< GPIO number for I2C sda signal */
    int scl_io_num;      /*!< GPIO number for I2C scl signal */
    bool sda_pullup_en;  /*!< Internal GPIO pull mode for I2C sda signal*/
    bool scl_pullup_en;  /*!< Internal GPIO pull mode for I2C scl signal*/

    union {
        struct {
            uint32_t clk_speed;      /*!< I2C clock frequency for master mode, (no higher than 1MHz for now) */
        } master;                    /*!< I2C master config */
        struct {
            uint8_t addr_10bit_en;   /*!< I2C 10bit address mode enable for slave mode */
            uint16_t slave_addr;     /*!< I2C address for slave mode */
            uint32_t maximum_speed;  /*!< I2C expected clock speed from SCL. */
        } slave;                     /*!< I2C slave config */
    };
    uint32_t clk_flags;              /*!< Bitwise of ``I2C_SCLK_SRC_FLAG_**FOR_DFS**`` for clk source choice*/
} i2c_config_t;


typedef void *i2c_cmd_handle_t;    /*!< I2C command handle  */


esp_err_t wire_driver_install(i2c_port_t i2c_num, i2c_mode_t mode, size_t slv_rx_buf_len, size_t slv_tx_buf_len, int intr_alloc_flags);
esp_err_t wire_driver_delete(i2c_port_t i2c_num);
esp_err_t wire_param_config(i2c_port_t i2c_num, const i2c_config_t *i2c_conf);
esp_err_t wire_reset_tx_fifo(i2c_port_t i2c_num);
esp_err_t wire_reset_rx_fifo(i2c_port_t i2c_num);
esp_err_t wire_isr_register(i2c_port_t i2c_num, void (*fn)(void *), void *arg, int intr_alloc_flags, intr_handle_t *handle);
esp_err_t wire_isr_free(intr_handle_t handle);
esp_err_t wire_set_pin(i2c_port_t i2c_num, int sda_io_num, int scl_io_num,
                      bool sda_pullup_en, bool scl_pullup_en, i2c_mode_t mode);

esp_err_t wire_master_write_device(i2c_port_t i2c_num, uint8_t device_address,
                                     const uint8_t* write_buffer, size_t write_size,
                                     TickType_t ticks_to_wait);

esp_err_t wire_master_read_device(i2c_port_t i2c_num, uint8_t device_address,
                                      uint8_t* read_buffer, size_t read_size,
                                      TickType_t ticks_to_wait);

esp_err_t wire_master_write_read_device(i2c_port_t i2c_num, uint8_t device_address,
                                       const uint8_t* write_buffer, size_t write_size,
                                       uint8_t* read_buffer, size_t read_size,
                                       TickType_t ticks_to_wait);

i2c_cmd_handle_t wire_cmd_link_create_static(uint8_t* buffer, uint32_t size);
i2c_cmd_handle_t wire_cmd_link_create(void);
void wire_cmd_link_delete_static(i2c_cmd_handle_t cmd_handle);
void wire_cmd_link_delete(i2c_cmd_handle_t cmd_handle);
esp_err_t wire_master_start(i2c_cmd_handle_t cmd_handle);
esp_err_t wire_master_write_byte(i2c_cmd_handle_t cmd_handle, uint8_t data, bool ack_en);
esp_err_t wire_master_write(i2c_cmd_handle_t cmd_handle, const uint8_t *data, size_t data_len, bool ack_en);
esp_err_t wire_master_read_byte(i2c_cmd_handle_t cmd_handle, uint8_t *data, i2c_ack_type_t ack);
esp_err_t wire_master_read(i2c_cmd_handle_t cmd_handle, uint8_t *data, size_t data_len, i2c_ack_type_t ack);
esp_err_t wire_master_stop(i2c_cmd_handle_t cmd_handle);
esp_err_t wire_master_cmd_begin(i2c_port_t i2c_num, i2c_cmd_handle_t cmd_handle, TickType_t ticks_to_wait);
int wire_slave_write_buffer(i2c_port_t i2c_num, const uint8_t *data, int size, TickType_t ticks_to_wait);
int wire_slave_read_buffer(i2c_port_t i2c_num, uint8_t *data, size_t max_size, TickType_t ticks_to_wait);
esp_err_t wire_set_period(i2c_port_t i2c_num, int high_period, int low_period);
esp_err_t wire_get_period(i2c_port_t i2c_num, int *high_period, int *low_period);
esp_err_t wire_filter_enable(i2c_port_t i2c_num, uint8_t cyc_num);
esp_err_t wire_filter_disable(i2c_port_t i2c_num);
esp_err_t wire_set_start_timing(i2c_port_t i2c_num, int setup_time, int hold_time);
esp_err_t wire_get_start_timing(i2c_port_t i2c_num, int *setup_time, int *hold_time);
esp_err_t wire_set_stop_timing(i2c_port_t i2c_num, int setup_time, int hold_time);
esp_err_t wire_get_stop_timing(i2c_port_t i2c_num, int *setup_time, int *hold_time);
esp_err_t wire_set_data_timing(i2c_port_t i2c_num, int sample_time, int hold_time);
esp_err_t wire_get_data_timing(i2c_port_t i2c_num, int *sample_time, int *hold_time);
esp_err_t wire_set_timeout(i2c_port_t i2c_num, int timeout);
esp_err_t wire_get_timeout(i2c_port_t i2c_num, int *timeout);
esp_err_t wire_set_data_mode(i2c_port_t i2c_num, i2c_trans_mode_t tx_trans_mode, i2c_trans_mode_t rx_trans_mode);
esp_err_t wire_get_data_mode(i2c_port_t i2c_num, i2c_trans_mode_t *tx_trans_mode, i2c_trans_mode_t *rx_trans_mode);

#ifdef __cplusplus
}
#endif
