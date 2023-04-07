#include "hunter_wifi.h"

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include <utility>

namespace esphome {
namespace hunterwifi {

static const char *const TAG = "hunterwifi";
  
void HunterZoneSwitch::loop() {
  if (!this->f_.has_value())
    return;
  auto s = (*this->f_)();
  if (!s.has_value())
    return;

  this->publish_state(*s);
}

void HunterZoneSwitch::write_state(bool state) {
  hunter_roam_ = new HunterRoam(pin_->get_pin());  // NOLINT(cppcoreguidelines-owning-memory)
  byte a_zone = zone_;
  byte a_duration = max_duration_;
  byte result;
  
  if (state) {
    result = hunter_roam.startZone(a_zone, a_duration);
  } else {
    result = hunter_roam.stopZone(a_zone);
  }
  
  if (result == 0)
    {
      //Accomplish change
      this->publish_state(state);
    }
  else
  {
    ESP_LOGW(TAG, "Failed message setup for Hunter controller: %s", hunter_roam.errorHint(result));
  }
}

void HunterZoneSwitch::set_state_lambda(std::function<optional<bool>()> &&f) { this->f_ = f; }
float HunterZoneSwitch::get_setup_priority() const { return setup_priority::HARDWARE; }

void HunterZoneSwitch::setup() { this->state = false; }

void HunterZoneSwitch::dump_config() { LOG_SWITCH("", "HunterWifi Switch", this); }
  
void HunterWifiComponent ::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HunterWifiComponent ...");

  pin_->setup();
}

void HunterWifiComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "HunterWifiComponent  :");
  LOG_PIN("  Pin: ", this->pin_);
  for (size_t valve_number = 0; valve_number < this->number_of_valves(); valve_number++) {
    ESP_LOGCONFIG(TAG, "  Valve %u:", valve_number);
    ESP_LOGCONFIG(TAG, "    Name: %s", this->valve_[valve_number].valve_switch->get_name().c_str());
    ESP_LOGCONFIG(TAG, "    Zone: %u", this->valve_[valve_number].zone_number);
    ESP_LOGCONFIG(TAG, "    Max_Duration: %u", this->valve_[valve_number].max_duration);
    }
  }
  
// add_valve(HunterZoneSwitch *valve_sw, uint16_t zone_number); 
void HunterWifiComponent::add_valve(HunterZoneSwitch  *valve_sw, uint16_t zone_number, uint16_t max_duration) {
  auto new_valve_number = this->number_of_valves();
  this->valve_.resize(new_valve_number + 1);
  HunterValve *new_valve = &this->valve_[new_valve_number];

  new_valve->valve_switch = valve_sw;
  new_valve->zone_number = zone_number;
  new_valve->max_duration = max_duration;
}

size_t HunterWifiComponent::number_of_valves() { return this->valve_.size(); }


}  // namespace dallas
}  // namespace esphome
