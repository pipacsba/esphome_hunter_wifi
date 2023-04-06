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
  auto *hunter_roam_ = this->parent_->smartPort;
  byte a_zone = this->zone_number;
  
  byte result = hunter_roam.stopZone(a_zone);
  if (result == 0)
    {
      this->state_ = false;
    }
  else
  {
    //errorHint
    ESP_LOGW(TAG, "Failed message setup for Hunter controller: %s", hunter_roam.errorHint(result));
  }
  
}

void HunterZoneSwitch::turn_on() {

  auto *hunter_roam_ = this->parent_->smartPort;
  byte a_zone = this->zone_number;
  
  byte result = hunter_roam.startZone(a_zone, 5);
  if (result == 0)
    {
      this->state_ = false;
    }
  else
  {
    //errorHint
    ESP_LOGW(TAG, "Failed message setup for Hunter controller: %s", hunter_roam.errorHint(result));
  }
  
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
    ESP_LOGCONFIG(TAG, "    Zone: %u", this->valve_zone(valve_number));
    }
  }
}
  
void HunterWifiComponent::configure_valve_switch(size_t valve_number, switch_::Switch *valve_switch, uint16_t zone_number) {
  if (this->is_a_valid_valve(valve_number)) {
    this->valve_[valve_number].valve_switch.set_on_switch(valve_switch);
    this->valve_[valve_number].zone_number = zone_number;
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
