// https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BME280-DS002.pdf
// https://github.com/adafruit/Adafruit_BME280_Library
// https://github.com/espressif/esp-idf/blob/8bc19ba893e5544d571a753d82b44a84799b94b1/examples/peripherals/i2c/i2c_tools/main/cmd_i2ctools.c

#include "main.h"

#include "esp_system.h"
#include "esp_log.h"

#include "driver/i2c.h"

static const char *LTAG = "meteostanice bme280";

#define WS_I2C_PORT I2C_NUM_0
#define WS_I2C_ADDR 0x76
#define WS_I2C_FREQ 10000
#define WS_I2C_PULLUPS false
#define WS_I2C_SDA 21
#define WS_I2C_SCL 22
#define WS_I2C_WRITE_WAIT 1000 / portTICK_RATE_MS
#define WS_I2C_READ_WAIT 1000 / portTICK_RATE_MS
#define WS_I2C_MAX_RETRIES 16

#define WS_I2C_ACK 0
#define WS_I2C_NACK 1

#define WS_MEASURE_WAITLOOP_DELAY 1000 / portTICK_PERIOD_MS
#define WS_MEASURE_MAX_ITERS 1000

#define WS_BME280_REG_CHIP_ID 0xD0
#define WS_BME280_REG_RESET 0xE0
#define WS_BME280_REG_CTRL_HUM 0xF2
#define WS_BME280_REG_STATUS 0xF3
#define WS_BME280_REG_CTRL_MEAS 0xF4
#define WS_BME280_REG_CONFIG 0xF5
#define WS_BME280_REG_COMP1 0x88
#define WS_BME280_REG_COMP2 0xE1
#define WS_BME280_REG_RESULT 0xF7
//#define WS_BME280_REG_ 0x

#define WS_BME280_RESET_VALUE 0xB6
#define WS_BME280_STATUS_MEASURING_MASK 0b1000
#define WS_BME280_STATUS_IM_UPDATE_MASK 1
#define WS_BME280_MEAS_VALUE 0b00100100

typedef struct {
  bool measuring;
  bool im_update;
} ws_bme280_status_t;

typedef struct {
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;

  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;

  int16_t dig_H2;
  int16_t dig_H4;
  int16_t dig_H5;

  uint8_t dig_H1;
  uint8_t dig_H3;
  int8_t dig_H6;
} ws_bme280_compensation_t;
typedef struct {
  int32_t hum;
  int32_t temp;
  int32_t pres;
} ws_bme280_measurement_raw_t;


//TODO: memory leaky ve write a read

ws_bme280_compensation_t ws_bme280_compensation;

/**
 * Zapsat do registrů WS_BME280.
 *
 * Buffer musí mít sudou velikost - vždy jde o
 * bajt adresy následovaný bajtem zapisované hodnoty.
 */
static esp_err_t ws_bme280_write(const uint8_t *pairs, size_t length) {
  assert(length % 2 == 0);

  esp_err_t err;
  uint32_t ntry = 0;

  for (; ntry < WS_I2C_MAX_RETRIES; ntry++) {
    i2c_cmd_handle_t link = i2c_cmd_link_create();

    // Tato volání by neměla selhat
    ESP_ERROR_CHECK(
      i2c_master_start(link)
    );
    ESP_ERROR_CHECK(
      i2c_master_write_byte(link, (WS_I2C_ADDR << 1) | I2C_MASTER_WRITE, true)
    );
    ESP_ERROR_CHECK(
      i2c_master_write(link, pairs, length, true)
    );
    ESP_ERROR_CHECK(
      i2c_master_stop(link)
    );

    err = i2c_master_cmd_begin(WS_I2C_PORT, link, WS_I2C_WRITE_WAIT);
    i2c_cmd_link_delete(link);
    if (err == ESP_OK) {
      break;
    }
  }

  if (ntry > 0) {
    ESP_LOGW(LTAG, "ws_bme280_write had to try %u times!", ntry);
  }

  return err;
}

/**
 * Číst z registrů WS_BME280.
 *
 * Čtení začíná na adrese _reg_ a přečte _bytes_ bajtů.
 */
static esp_err_t ws_bme280_read(uint8_t reg, uint8_t *output_buffer, size_t bytes) {
  assert(bytes > 0);

  esp_err_t err;
  uint32_t ntry = 0;

  for (; ntry < WS_I2C_MAX_RETRIES; ntry++) {
    i2c_cmd_handle_t link = i2c_cmd_link_create();

    // Tato volání by neměla selhat
    ESP_ERROR_CHECK(
      i2c_master_start(link)
    );

    ESP_ERROR_CHECK(
      i2c_master_write_byte(link, (WS_I2C_ADDR << 1) | I2C_MASTER_WRITE, true)
    );
    ESP_ERROR_CHECK(
      i2c_master_write_byte(link, reg, true)
    );
    ESP_ERROR_CHECK(
      i2c_master_start(link)
    );
    ESP_ERROR_CHECK(
      i2c_master_write_byte(link, (WS_I2C_ADDR << 1) | I2C_MASTER_READ, true)
    );
    if (bytes > 1) {
      ESP_ERROR_CHECK(
        i2c_master_read(link, output_buffer, bytes - 1, WS_I2C_ACK)
      );
    }
    ESP_ERROR_CHECK(
      i2c_master_read_byte(link, output_buffer + (bytes - 1), WS_I2C_NACK)
    );
    ESP_ERROR_CHECK(
      i2c_master_stop(link)
    );

    err = i2c_master_cmd_begin(WS_I2C_PORT, link, WS_I2C_READ_WAIT);
    i2c_cmd_link_delete(link);
    if (err == ESP_OK) {
      break;
    }
  }

  if (ntry > 0) {
    ESP_LOGW(LTAG, "ws_bme280_read had to try %u times!", ntry);
  }

  return err;
}

/**
 * Přečíst stavový registr BME280
 */
static esp_err_t ws_bme280_read_status(ws_bme280_status_t *out) {
  uint8_t status;

  esp_err_t err = ws_bme280_read(WS_BME280_REG_STATUS, &status, 1);
  if (err != ESP_OK) {
    return err;
  }

  *out = (ws_bme280_status_t){
    !!(status & WS_BME280_STATUS_MEASURING_MASK),
    !!(status & WS_BME280_STATUS_IM_UPDATE_MASK)
  };
  return ESP_OK;
}

static const i2c_config_t i2c_config = {
  .mode = I2C_MODE_MASTER,
  .sda_io_num = WS_I2C_SDA,
  .scl_io_num = WS_I2C_SCL,
  .sda_pullup_en = WS_I2C_PULLUPS,
  .scl_pullup_en = WS_I2C_PULLUPS,
  .master.clk_speed = WS_I2C_FREQ
};

static esp_err_t ws_bme280_read_compensation(ws_bme280_compensation_t *c) {
  // https://github.com/finitespace/BME280/blob/master/src/BME280.cpp
  uint8_t buf[25];
  esp_err_t err;

  err = ws_bme280_read(WS_BME280_REG_COMP1, buf, 25);
  if (err != ESP_OK) return err;

  c->dig_T1 = (buf[1] << 8) | buf[0];
  c->dig_T2 = (buf[3] << 8) | buf[2];
  c->dig_T3 = (buf[5] << 8) | buf[4];

  c->dig_P1 = (buf[7] << 8) | buf[6];
  c->dig_P2 = (buf[9] << 8) | buf[8];
  c->dig_P3 = (buf[11] << 8) | buf[10];
  c->dig_P4 = (buf[13] << 8) | buf[12];
  c->dig_P5 = (buf[15] << 8) | buf[14];
  c->dig_P6 = (buf[17] << 8) | buf[16];
  c->dig_P7 = (buf[19] << 8) | buf[18];
  c->dig_P8 = (buf[21] << 8) | buf[20];
  c->dig_P9 = (buf[23] << 8) | buf[22];

  c->dig_H1 = buf[24];

  err = ws_bme280_read(WS_BME280_REG_COMP2, buf, 7);
  if (err != ESP_OK) return err;

  c->dig_H2 = (buf[1] << 8) | buf[0];
  c->dig_H3 = buf[2];
  c->dig_H4 = (buf[3] << 4) | (buf[4] & 0b1111);
  c->dig_H5 = (buf[5] << 4) | ((buf[4] >> 4) & 0b1111);
  c->dig_H6 = buf[6];

  return ESP_OK;
}

static esp_err_t ws_bme280_read_measurement(ws_bme280_measurement_raw_t *out) {
  uint8_t buf[8];

  esp_err_t err = ws_bme280_read(WS_BME280_REG_RESULT, buf, sizeof(buf));
  if (err != ESP_OK) {
    return err;
  }

  *out = (ws_bme280_measurement_raw_t){
    .hum = (buf[6] << 8) | buf[7],
    .temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4),
    .pres = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4)
  };

  return ESP_OK;
}

static int32_t ws_bme280_compensate_temp(int32_t input, int32_t *t_fine, ws_bme280_compensation_t *c) {
  const int32_t x1 = (
    (((input >> 3) - ((int32_t)(c->dig_T1) << 1)))
    * (int32_t)(c->dig_T2)
  ) >> 11;
  const int32_t x2 = (
    ((
      ((input >> 4) - (int32_t)(c->dig_T1))
      *
      ((input >> 4) - (int32_t)(c->dig_T1))
    ) >> 12)
    * (int32_t)(c->dig_T3)
  ) >> 14;

  *t_fine = x1 + x2;
  const int32_t res = (*t_fine * 5 + 128) >> 8;

  ESP_LOGD(LTAG, "compensate_temp input=%i,x1=%i,x2=%i,res=%i,t1=%u,t2=%i,t3=%i", input,x1,x2,res,c->dig_T1,c->dig_T2,c->dig_T3);

  return res;
}
static uint32_t ws_bme280_compensate_hum(int32_t input, int32_t t_fine, ws_bme280_compensation_t *c) {
  // https://github.com/BoschSensortec/BME280_driver/blob/cf40d00b0b5139e287b670881c433c0041d98d9f/bme280.c

  int32_t x;

  x = (t_fine - ((int32_t)76800));
	x = (
    (((
      (input << 14) - ((int32_t)(c->dig_H4) << 20) - (((int32_t)c->dig_H5) * x)
    ) + ((int32_t)16384)) >> 15)
	  * ((
        (((
            ((x * ((int32_t)c->dig_H6)) >> 10)
            * (((x * ((int32_t)c->dig_H3)) >> 11) + ((int32_t)32768))
        ) >> 10) + ((int32_t)2097152))
        * ((int32_t)c->dig_H2) + 8192
      ) >> 14)
  );
	x = (x - ((
    (((x >> 15) * (x >> 15)) >> 7)
    * (int32_t)c->dig_H1)	>> 4
  ));
	x = (x < 0 ? 0 : x);
	x = (x > 419430400 ? 419430400 : x);
	return (uint32_t)(x >> 12);
}
static uint32_t ws_bme280_compensate_pres(int32_t input, int32_t t_fine, ws_bme280_compensation_t *c) {
  // https://github.com/BoschSensortec/BME280_driver/blob/cf40d00b0b5139e287b670881c433c0041d98d9f/bme280.c

  int64_t x1, x2, pressure;

	x1 = (int64_t)t_fine - 128000;
	x2 = x1 * x1 *
	(int64_t)c->dig_P6;
	x2 = x2 + ((x1 *
	(int64_t)c->dig_P5)
	<< 17);
	x2 = x2 +
	(((int64_t)c->dig_P4)
	<< 35);
	x1 = ((x1 * x1 *
	(int64_t)c->dig_P3)
	>> 8) +
	((x1 * (int64_t)c->dig_P2)
	<< 12);
	x1 = (((((int64_t)1)
	<< 47) + x1)) *
	((int64_t)c->dig_P1)
	>> 33;
	pressure = 1048576 - input;
	/* Avoid exception caused by division by zero */
	if (x1 != 0)
			pressure = (((pressure
			<< 31) - x2)
			* 3125) / x1;
	else
		return 0;
	x1 = (((int64_t)c->dig_P9) *
	(pressure >> 13) *
	(pressure >> 13))
	>> 25;
	x2 = (((int64_t)c->dig_P8) *
	pressure) >> 19;
	pressure = (((pressure + x1 +
	x2) >> 8) +
	(((int64_t)c->dig_P7)
	<< 4));

	return (uint32_t)pressure;
}


esp_err_t ws_bme280_init() {
  esp_err_t err;

  ESP_ERROR_CHECK(
    i2c_param_config(WS_I2C_PORT, &i2c_config)
  );
  ESP_ERROR_CHECK(
    i2c_driver_install(
      WS_I2C_PORT,
      I2C_MODE_MASTER,
      false,
      false,
      0
    )
  );
  ESP_LOGI(LTAG, "I2C initalized");

  uint8_t reset_buf[] = { WS_BME280_REG_RESET, WS_BME280_RESET_VALUE };
  err = ws_bme280_write(reset_buf, sizeof(reset_buf));
  if (err != ESP_OK) return err;

  uint8_t init_buf[] = {
    // Vypnout oversampling vlhkosti
    WS_BME280_REG_CTRL_HUM, 1,
    // Vypnout oversampling teploty a tlaku; zapnout sleep mode
    WS_BME280_REG_CTRL_MEAS, WS_BME280_MEAS_VALUE,
    // Vypnout IIR filtr
    WS_BME280_REG_CONFIG, 0
  };
  err = ws_bme280_write(init_buf, sizeof(init_buf));
  if (err != ESP_OK) return err;

  err = ws_bme280_read_compensation(&ws_bme280_compensation);
  if (err != ESP_OK) return err;

  uint8_t chip_id = 0;
  err = ws_bme280_read(WS_BME280_REG_CHIP_ID, &chip_id, 1);
  // Tohle není fatální chyba.
  if (err != ESP_OK) chip_id = 0xEE;

  ESP_LOGI(
    LTAG, "BME280 initalized (CHIP_ID=0x%X)", chip_id
  );

  return ESP_OK;
}

int32_t ws_bme280_measure(ws_bme280_measurement_t *measurement) {
  ws_bme280_status_t status;
  esp_err_t err;

  uint8_t conf_buf[] = {
    // Vypnout oversampling teploty a tlaku; zapnout forced mode
    WS_BME280_REG_CTRL_MEAS, 1 | WS_BME280_MEAS_VALUE
  };
  err = ws_bme280_write(conf_buf, sizeof(conf_buf));
  if (err != ESP_OK) {
    ESP_LOGW(LTAG, "Failed to enter forced mode");
    return err;
  }

  for(uint32_t i = 1;;i++) {
    err = ws_bme280_read_status(&status);
    if (err != ESP_OK) {
      ESP_LOGW(LTAG, "Failed to read status");
      return err;
    }

    if (!status.measuring) {
      break;
    }

    if (i == WS_MEASURE_MAX_ITERS) {
      ESP_LOGW(LTAG, "Measure cycle took too long!");
      return -1;
    }

    vTaskDelay(WS_MEASURE_WAITLOOP_DELAY);
  }

  ws_bme280_measurement_raw_t raw;
  err = ws_bme280_read_measurement(&raw);
  if (err != ESP_OK) {
    ESP_LOGW(LTAG, "Failed to read measurement");
    return err;
  }
  ESP_LOGD(LTAG, "RAW measurement temp=%u,pres=%u,hum=%u", raw.temp, raw.pres, raw.hum);

  int32_t t_fine;
  measurement->temp = ws_bme280_compensate_temp(raw.temp, &t_fine, &ws_bme280_compensation);
  measurement->hum = ws_bme280_compensate_hum(raw.hum, t_fine, &ws_bme280_compensation);
  measurement->pres = ws_bme280_compensate_pres(raw.pres, t_fine, &ws_bme280_compensation);
  ESP_LOGD(LTAG, "measurement temp=%u,pres=%u,hum=%u", measurement->temp, measurement->pres, measurement->hum);

  return ESP_OK;
}
