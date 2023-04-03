#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "HunterRoam.h"

#include <vector>

namespace esphome {
namespace hunterwifi {

class HunterZoneSwitch;

class HunterWifiComponent  : public Component {
 public:
  void set_pin(InternalGPIOPin *pin) { pin_ = pin; }
  void register_switch(HunterZoneSwitch *switch);

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void update() override;

 protected:
  friend HunterZoneSwitch;

  InternalGPIOPin *pin_;
  HunterRoam *hunter_roam_;

};

/// Internal class that helps us create multiple sensors for one Dallas hub.
class HunterZoneSwitch : public switch::Switch {
 public:
  void set_parent(HunterWifiComponent *parent) { parent_ = parent; }
  /// Helper to get a pointer to the address as uint8_t.
  /// uint8_t *get_address8();  - ez a Dallas sensor cime vaj'h?
  /// Helper to create (and cache) the name for this switch. For example "Front Garden".
  const std::string &get_zone_name();

  /// Get the index of this sensor.
  void uint8_t get_index() const;
  /// Set the index of this sensor.
  void set_index(uint8_t index);

  bool setup_switch();

  int get_result();

  std::string unique_id() override;

 protected:
  HunterWifiComponent *parent_;
  uint8_t index_;

  std::string zone_name_;
};

}  // namespace dallas
}  // namespace esphome
