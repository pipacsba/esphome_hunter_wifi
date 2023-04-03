#include "hunter_wifi.h.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hunterwifi  {

static const char *const TAG = "hunterwifi.switch";

void HunterWifiComponent ::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HunterWifiComponent ...");

  pin_->setup();

  smartPort = new HunterRoam(pin_);  // NOLINT(cppcoreguidelines-owning-memory)
}

void HunterWifiComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "HunterWifiComponent  :");
  LOG_PIN("  Pin: ", this->pin_);
  LOG_UPDATE_INTERVAL(this);
}

void HunterWifiComponent::register_switch(HunterZoneSwitch  *switch) { this->switches_.push_back(switch); }

void HunterWifiComponent::update() {

}

void HunterZoneSwitch::set_address(uint64_t address) { this->address_ = address; }
uint8_t HunterZoneSwitch::get_resolution() const { return this->resolution_; }
void HunterZoneSwitch::set_resolution(uint8_t resolution) { this->resolution_ = resolution; }
optional<uint8_t> HunterZoneSwitch::get_index() const { return this->index_; }
void HunterZoneSwitch::set_index(uint8_t index) { this->index_ = index; }
uint8_t *HunterZoneSwitch::get_address8() { return reinterpret_cast<uint8_t *>(&this->address_); }
const std::string &HunterZoneSwitch::get_address_name() {
  if (this->address_name_.empty()) {
    this->address_name_ = std::string("0x") + format_hex(this->address_);
  }

  return this->address_name_;
}
bool IRAM_ATTR HunterZoneSwitch::read_scratch_pad() {
  auto *wire = this->parent_->one_wire_;

  {
    InterruptLock lock;

    if (!wire->reset()) {
      return false;
    }
  }

  {
    InterruptLock lock;

    wire->select(this->address_);
    wire->write8(DALLAS_COMMAND_READ_SCRATCH_PAD);

    for (unsigned char &i : this->scratch_pad_) {
      i = wire->read8();
    }
  }

  return true;
}
bool HunterZoneSwitch::setup_sensor() {
  bool r = this->read_scratch_pad();

  if (!r) {
    ESP_LOGE(TAG, "Reading scratchpad failed: reset");
    return false;
  }
  if (!this->check_scratch_pad())
    return false;

  if (this->scratch_pad_[4] == this->resolution_)
    return false;

  if (this->get_address8()[0] == DALLAS_MODEL_DS18S20) {
    // DS18S20 doesn't support resolution.
    ESP_LOGW(TAG, "DS18S20 doesn't support setting resolution.");
    return false;
  }

  switch (this->resolution_) {
    case 12:
      this->scratch_pad_[4] = 0x7F;
      break;
    case 11:
      this->scratch_pad_[4] = 0x5F;
      break;
    case 10:
      this->scratch_pad_[4] = 0x3F;
      break;
    case 9:
    default:
      this->scratch_pad_[4] = 0x1F;
      break;
  }

  auto *wire = this->parent_->one_wire_;
  {
    InterruptLock lock;
    if (wire->reset()) {
      wire->select(this->address_);
      wire->write8(DALLAS_COMMAND_WRITE_SCRATCH_PAD);
      wire->write8(this->scratch_pad_[2]);  // high alarm temp
      wire->write8(this->scratch_pad_[3]);  // low alarm temp
      wire->write8(this->scratch_pad_[4]);  // resolution
      wire->reset();

      // write value to EEPROM
      wire->select(this->address_);
      wire->write8(0x48);
    }
  }

  delay(20);  // allow it to finish operation
  wire->reset();
  return true;
}
bool HunterZoneSwitch::check_scratch_pad() {
  bool chksum_validity = (crc8(this->scratch_pad_, 8) == this->scratch_pad_[8]);
  bool config_validity = false;

  switch (this->get_address8()[0]) {
    case DALLAS_MODEL_DS18B20:
      config_validity = ((this->scratch_pad_[4] & 0x9F) == 0x1F);
      break;
    default:
      config_validity = ((this->scratch_pad_[4] & 0x10) == 0x10);
  }

#ifdef ESPHOME_LOG_LEVEL_VERY_VERBOSE
  ESP_LOGVV(TAG, "Scratch pad: %02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X (%02X)", this->scratch_pad_[0],
            this->scratch_pad_[1], this->scratch_pad_[2], this->scratch_pad_[3], this->scratch_pad_[4],
            this->scratch_pad_[5], this->scratch_pad_[6], this->scratch_pad_[7], this->scratch_pad_[8],
            crc8(this->scratch_pad_, 8));
#endif
  if (!chksum_validity) {
    ESP_LOGW(TAG, "'%s' - Scratch pad checksum invalid!", this->get_name().c_str());
  } else if (!config_validity) {
    ESP_LOGW(TAG, "'%s' - Scratch pad config register invalid!", this->get_name().c_str());
  }
  return chksum_validity && config_validity;
}
float HunterZoneSwitch::get_temp_c() {
  int16_t temp = (int16_t(this->scratch_pad_[1]) << 11) | (int16_t(this->scratch_pad_[0]) << 3);
  if (this->get_address8()[0] == DALLAS_MODEL_DS18S20) {
    int diff = (this->scratch_pad_[7] - this->scratch_pad_[6]) << 7;
    temp = ((temp & 0xFFF0) << 3) - 16 + (diff / this->scratch_pad_[7]);
  }

  return temp / 128.0f;
}
std::string DallasTemperatureSensor::unique_id() { return "dallas-" + str_lower_case(format_hex(this->address_)); }

}  // namespace dallas
}  // namespace esphome
