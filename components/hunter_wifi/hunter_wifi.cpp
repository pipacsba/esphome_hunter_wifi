#include "hunter_wifi.h"

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include <utility>

namespace esphome {
namespace hunterwifi  {

static const char *const TAG = "hunterwifi";
  
HunterZoneSwitch::HunterZoneSwitch() {}
HunterZoneSwitch::HunterZoneSwitch(switch_::Switch *hunterwifi_switch) : on_switch_(hunterwifi_switch) {}
HunterZoneSwitch::HunterZoneSwitch(switch_::Switch *off_switch, switch_::Switch *on_switch) : off_switch_(off_switch), on_switch_(on_switch) {}

void HunterZoneSwitch::turn_off() {
  if (!this->state()) {  // do nothing if we're already in the requested state
    return;
  }
  if (this->off_switch_ != nullptr) {  // latching valve, start a pulse
    if (!this->off_switch_->state) {
      this->off_switch_->turn_on();
    }
    this->pinned_millis_ = millis();
  } else if (this->on_switch_ != nullptr) {  // non-latching valve
    this->on_switch_->turn_off();
  }
  this->state_ = false;
}

void HunterZoneSwitch::turn_on() {
  if (this->state()) {  // do nothing if we're already in the requested state
    return;
  }
  if (this->off_switch_ != nullptr) {  // latching valve, start a pulse
    if (!this->on_switch_->state) {
      this->on_switch_->turn_on();
    }
    this->pinned_millis_ = millis();
  } else if (this->on_switch_ != nullptr) {  // non-latching valve
    this->on_switch_->turn_on();
  }
  this->state_ = true;
}  
  
  
  
void HunterWifiComponent ::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HunterWifiComponent ...");

  pin_->setup();

  smartPort = new HunterRoam(pin_);  // NOLINT(cppcoreguidelines-owning-memory)
}

void HunterWifiComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "HunterWifiComponent  :");
  LOG_PIN("  Pin: ", this->pin_);
  for (size_t valve_number = 0; valve_number < this->number_of_valves(); valve_number++) {
    ESP_LOGCONFIG(TAG, "  Valve %u:", valve_number);
    ESP_LOGCONFIG(TAG, "    Name: %s", this->valve_name(valve_number));
    ESP_LOGCONFIG(TAG, "    Zone: %u", this->valve_zone(valva_number));
    }
  }
}

void HunterWifiComponent::register_switch(HunterZoneSwitch  *switch) { this->switches_.push_back(switch); }

void HunterWifiComponent::update() {

}


//  auto *wire = this->parent_->one_wire_;
//  {
//    InterruptLock lock;
//    if (wire->reset()) {
//      wire->select(this->address_);
//      wire->write8(DALLAS_COMMAND_WRITE_SCRATCH_PAD);
//      wire->write8(this->scratch_pad_[2]);  // high alarm temp
//      wire->write8(this->scratch_pad_[3]);  // low alarm temp
//      wire->write8(this->scratch_pad_[4]);  // resolution
//      wire->reset();
//
//      // write value to EEPROM
//      wire->select(this->address_);
//      wire->write8(0x48);
//    }






}  // namespace dallas
}  // namespace esphome
