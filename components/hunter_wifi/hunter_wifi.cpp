#include "hunter_wifi.h"

#include "esphome/core/entity_base.h"
#include "esphome/core/application.h"
#include "esphome/components/number/number.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include <utility>

namespace esphome {
namespace hunterwifi {

static const char *const TAG = "hunterwifi";

//flexibility is everything
void HunterZoneSwitch::loop() {
  if (!this->f_.has_value())
    return;
  auto s = (*this->f_)();
  if (!s.has_value())
    return;

  this->publish_state(*s);
}

//send REM message in case of switch representing a zone is switched (changes state)
void HunterZoneSwitch::write_state(bool state) {
  hunter_roam_ = new HunterRoam(pin_->get_pin());  // NOLINT(cppcoreguidelines-owning-memory)
  byte a_zone = zone_;
  byte result;
    
  if (state) {
    byte a_duration = max_duration_;
    byte b_duration = 240;
    //for (number::Number *obj : App.get_numbers()) {
    if (this->duration_number_name_)
    {
      //if (obj->get_name().c_str() != duration_number_name_) {
      //  ESP_LOGVV(TAG, "%s do not match %s", obj->get_name().c_str(), duration_number_name_);
      // continue;
      //}
      //b_duration = obj->state;
      b_duration = (byte) this->duration_number_name_->state;
      ESP_LOGVV(TAG, "Requested duration for Hunter controller for zone %d for %d minutes.", a_zone, b_duration);
    }
    byte duration = min(a_duration, b_duration);
    
    result = hunter_roam_->startZone(a_zone, duration);
    ESP_LOGVV(TAG, "Message setup for Hunter controller is started on pin %d for zone %d for %d minutes.",pin_->get_pin(), a_zone, duration);
  } else {
    result = hunter_roam_->stopZone(a_zone);
    ESP_LOGVV(TAG, "Message setup for Hunter controller is started on pin %d to stop zone %d.",pin_->get_pin(), a_zone);
  }
  
  if (result == 0)
    {
      //Acknowledge change
      this->publish_state(state);
      ESP_LOGVV(TAG, "Message setup for Hunter controller is successfull");
    }
  else
  {
    ESP_LOGW(TAG, "Failed message setup for Hunter controller: %s", hunter_roam_->errorHint(result));
  }
}

void HunterZoneSwitch::set_state_lambda(std::function<optional<bool>()> &&f) { this->f_ = f; }
float HunterZoneSwitch::get_setup_priority() const { return setup_priority::HARDWARE; }

void HunterZoneSwitch::setup() { this->state = false; }

void HunterZoneSwitch::dump_config() { LOG_SWITCH("", "HunterWifi Switch", this); }
  
void HunterWifiComponent::setup() {
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
    if (this->valve_[valve_number].duration_number_name)
    {
      ESP_LOGCONFIG(TAG, "    Duration Number Name: %s", this->valve_[valve_number].duration_number_name->get_name().c_str());
    }
  }
}
  
// add valves to the component
void HunterWifiComponent::add_valve(HunterZoneSwitch  *valve_sw, uint16_t zone_number, uint16_t max_duration, number::Number *duration_number_name) {
  auto new_valve_number = this->number_of_valves();
  this->valve_.resize(new_valve_number + 1);
  HunterValve *new_valve = &this->valve_[new_valve_number];

  new_valve->valve_switch = valve_sw;
  new_valve->zone_number = zone_number;
  new_valve->max_duration = max_duration;
  new_valve->duration_number_name = duration_number_name;
  ESP_LOGW(TAG, "Valve added: %d", zone_number);
}

// add valves to the component
void HunterWifiComponent::add_valve(HunterZoneSwitch  *valve_sw, uint16_t zone_number, uint16_t max_duration) {
  auto new_valve_number = this->number_of_valves();
  this->valve_.resize(new_valve_number + 1);
  HunterValve *new_valve = &this->valve_[new_valve_number];

  new_valve->valve_switch = valve_sw;
  new_valve->zone_number = zone_number;
  new_valve->max_duration = max_duration;
  ESP_LOGW(TAG, "Valve added: %d", zone_number);
}


size_t HunterWifiComponent::number_of_valves() { return this->valve_.size(); }


}  // namespace dallas
}  // namespace esphome
