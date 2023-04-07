#include "hunter_wifi.h"

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include <utility>

namespace esphome {
namespace hunterwifi {

static const char *const TAG = "hunterwifi";
  
//void HunterZoneSwitch::turn_off() {
//  auto *hunter_roam_ = this->parent_->smartPort;
//  byte a_zone = (byte) this->zone_number;
//  
//  byte result = hunter_roam.stopZone(a_zone);
//  if (result == 0)
//    {
//      this->state_ = false;
//    }
//  else
//  {
//    //errorHint
//    ESP_LOGW(TAG, "Failed message setup for Hunter controller: %s", hunter_roam.errorHint(result));
//  }
//}

//void HunterZoneSwitch::turn_on() {
//  auto *hunter_roam_ = this->parent_->smartPort;
//  byte a_zone = (byte) this->zone_number;
//  byte result = hunter_roam.startZone(a_zone, 5);
//  if (result == 0)
//    {
//      this->state_ = false;
//    }
//  else
//  {
//    //errorHint
//    ESP_LOGW(TAG, "Failed message setup for Hunter controller: %s", hunter_roam.errorHint(result));
//  }
//}  

HunterZoneSwitch::HunterZoneSwitch()
    : turn_on_trigger_(new Trigger<>()), turn_off_trigger_(new Trigger<>()) {}
    
void HunterZoneSwitch::loop() {
  if (!this->f_.has_value())
    return;
  auto s = (*this->f_)();
  if (!s.has_value())
    return;

  this->publish_state(*s);
}

void HunterZoneSwitch::write_state(bool state) {
  if (this->prev_trigger_ != nullptr) {
    this->prev_trigger_->stop_action();
  }

  if (state) {
    this->prev_trigger_ = this->turn_on_trigger_;
    this->turn_on_trigger_->trigger();
  } else {
    this->prev_trigger_ = this->turn_off_trigger_;
    this->turn_off_trigger_->trigger();
  }

  this->publish_state(state);
}

void HunterZoneSwitch::set_state_lambda(std::function<optional<bool>()> &&f) { this->f_ = f; }
float HunterZoneSwitch::get_setup_priority() const { return setup_priority::HARDWARE; }

Trigger<> *HunterZoneSwitch::get_turn_on_trigger() const { return this->turn_on_trigger_; }
Trigger<> *HunterZoneSwitch::get_turn_off_trigger() const { return this->turn_off_trigger_; }

void HunterZoneSwitch::setup() { this->state = false; }

void HunterZoneSwitch::dump_config() { LOG_SWITCH("", "HunterWifi Switch", this); }
  
void HunterWifiComponent ::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HunterWifiComponent ...");

  pin_->setup();

  hunter_roam_ = new HunterRoam(pin_->get_pin());  // NOLINT(cppcoreguidelines-owning-memory)
}

void HunterWifiComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "HunterWifiComponent  :");
  LOG_PIN("  Pin: ", this->pin_);
  for (size_t valve_number = 0; valve_number < this->number_of_valves(); valve_number++) {
    ESP_LOGCONFIG(TAG, "  Valve %u:", valve_number);
    ESP_LOGCONFIG(TAG, "    Name: %s", this->valve_[valve_number].valve_switch->name_);
    ESP_LOGCONFIG(TAG, "    Zone: %u", this->valve_[valve_number].zone_number);
    }
  }
  
// add_valve(HunterZoneSwitch *valve_sw, uint16_t zone_number); 
void HunterWifiComponent::add_valve(HunterZoneSwitch  *valve_sw, uint16_t zone_number) {
  auto new_valve_number = this->number_of_valves();
  this->valve_.resize(new_valve_number + 1);
  HunterValve *new_valve = &this->valve_[new_valve_number];

  new_valve->valve_switch = valve_sw;
  new_valve->zone_number = zone_number;
}

size_t HunterWifiComponent::number_of_valves() { return this->valve_.size(); }


}  // namespace dallas
}  // namespace esphome
