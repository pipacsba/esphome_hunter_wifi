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

// define struct for zone valves
struct HunterValve {
  HunterZoneSwitch *valve_switch;
  uint16_t zone_number;
  uint16_t max_duration;
}; 
 
//main hunterwifi component (controller)
//this also maybe not even needed, not used for anything
class HunterWifiComponent  : public Component {
 public:
  // add pin
  void set_pin(InternalGPIOPin *pin) { pin_ = pin; }
  void setup() override;
  void dump_config() override;
  void add_valve(HunterZoneSwitch *valve_sw, uint16_t zone_number, uint16_t max_duration); 
 
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

//create switch for zones
class HunterZoneSwitch : public switch_::Switch, public Component {
 public:
  //HunterZoneSwitch();

  void setup() override;
  void dump_config() override;
  //define pin number to use for REM communication
  void set_pin(InternalGPIOPin *pin) { pin_ = pin; }
  //define the zone number as used in Huner device pinout
  void set_zone(byte zone) { zone_ = zone; }
  //set maximum zone sprinkler duration
  void set_max_duration(byte max_duration) { max_duration_ = max_duration; }
  
  //flexibility is everythin
  void set_state_lambda(std::function<optional<bool>()> &&f);
  void loop() override;

  float get_setup_priority() const override;

 protected:
  /** Write the given state to hardware. You should implement this
   * abstract method if you want to create your own switch.
   *
   * In the implementation of this method, you should also call
   * publish_state to acknowledge that the state was written to the hardware.
   *
   * @param state The state to write. Inversion is already applied if user specified it.
   */
  void write_state(bool state) override;

  InternalGPIOPin *pin_;
  HunterRoam *hunter_roam_;
  byte zone_;
  byte max_duration_;

  optional<std::function<optional<bool>()>> f_;
};

 
}  // namespace hunterwifi
}  // namespace esphome
