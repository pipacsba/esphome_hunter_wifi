#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/hal.h"
#include "esphome/components/number/number.h"
#include "HunterRoam.h"

#include <vector>

namespace esphome {
namespace hunterwifi {

class HunterWifiComponent;      // this component
class HunterZoneSwitch;         // switches representing any valve / zones

struct HunterValve {
  HunterZoneSwitch *valve_switch;
  uint16_t zone_number;
}; 
 
class HunterWifiComponent  : public Component {
 public:
  void set_pin(InternalGPIOPin *pin) { pin_ = pin; }

  void setup() override;
  void dump_config() override;
  //void set_name(const std::string &name) { this->name_ = name; }
  void add_valve(HunterZoneSwitch *valve_sw, uint16_t zone_number); 
 
  /// returns a pointer to a valve's switch object
  HunterZoneSwitch *valve_switch(size_t valve_number);
  /// returns the number of valves the controller is configured with
  size_t number_of_valves();
  
 protected:
 
  /// Sprinkler valve objects
  std::vector<HunterValve> valve_;
 
  friend HunterZoneSwitch;

  InternalGPIOPin *pin_;
  HunterRoam *hunter_roam_;

};

//class HunterZoneSwitch : public switch_::Switch, public Component  {
// public:
//  HunterZoneSwitch();
//
//  bool state();  // returns the switch's current state
//  void set_off_switch(switch_::Switch *off_switch) { this->off_switch_ = off_switch; }
//  void set_on_switch(switch_::Switch *on_switch) { this->on_switch_ = on_switch; }
//  void sync_valve_state();  // syncs internal state to switch;
//  void turn_off();        // sets internal flag and actuates the switch
//  void turn_on();         // sets internal flag and actuates the switch
//  switch_::Switch *off_switch() { return this->off_switch_; }
//  switch_::Switch *on_switch() { return this->on_switch_; }
//
//protected:
//  bool state_{false};
//  uint32_t zone_number_{0};
//  //switch_::Switch *off_switch_{nullptr};  // only used for latching valves
//  switch_::Switch *on_switch_{nullptr};   // used for both latching and non-latching valves
//};
  
class HunterZoneSwitch : public switch_::Switch, public Component {
 public:
  HunterZoneSwitch();

  void setup() override;
  void dump_config() override;

  void set_state_lambda(std::function<optional<bool>()> &&f);
  Trigger<> *get_turn_on_trigger() const;
  Trigger<> *get_turn_off_trigger() const;
  void loop() override;

  float get_setup_priority() const override;

 protected:
  void write_state(bool state) override;

  optional<std::function<optional<bool>()>> f_;
  Trigger<> *turn_on_trigger_;
  Trigger<> *turn_off_trigger_;
  Trigger<> *prev_trigger_{nullptr};
};

 
}  // namespace hunterwifi
}  // namespace esphome
