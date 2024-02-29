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
  hunter_roam_ = new HunterRoam(pin_);  // NOLINT(cppcoreguidelines-owning-memory)
  uint8_t a_zone = zone_;
  uint8_t result;
    
  if (state) {
    uint8_t a_duration = max_duration_;
    uint8_t b_duration = 240;
    //for (number::Number *obj : App.get_numbers()) {
    if (this->duration_number_name_)
    {
      //if (obj->get_name().c_str() != duration_number_name_) {
      //  ESP_LOGVV(TAG, "%s do not match %s", obj->get_name().c_str(), duration_number_name_);
      // continue;
      //}
      //b_duration = obj->state;
      b_duration = (uint8_t) this->duration_number_name_->state;
      ESP_LOGVV(TAG, "Requested duration for Hunter controller for zone %d for %d minutes.", a_zone, b_duration);
    }
    uint8_t duration = std::min(a_duration, b_duration);
    
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
    ESP_LOGW(TAG, "Failed message setup for Hunter controller: %s", hunter_roam_->errorHint(result).c_str());
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


/**
 * Library that implements the Hunter SmartPort protocol.
 * 
 * It has been adapted from
 * 		- Sebastien <https://github.com/seb821/OpenSprinkler-Firmware-Hunter>
 * 		- Dave Fleck 
 * 		- Scott Shumate 
 * Original post: <https://www.hackster.io/sshumate/hunter-sprinkler-wifi-remote-control-4ea918>
 * 
 * The modifications made are:
 * 		1) It is now a class, for easier use in case several Hunter products are controlled
 * 			by the same device.
 * 		2) Public functions now return a byte, to make it easier to programatically check
 * 			if there has been an error. Use HunterRoam::errorHint(byte error) to obtain
 * 			a user friendly description of the error.
 * 		3) Updated functions documentation format so that IDEs can parse them.
 * 
 * 			        ------ Eloi Codina Torras - July 2020 ------
 */

/**
 * Constructor for the object HunterRoam.
 * 
 * @param pin GPIO number where the REM wire is connected to.
 */
HunterRoam::HunterRoam(InternalGPIOPin *pin) {
	pin_ = pin;
	this->pin_->pin_mode(gpio::FLAG_OUTPUT);
}

/**
 * Function to convert the error number returned by any function
 * to a user-friendly description.
 * 
 * @param error number returned by a public function of this library
 */
std::string HunterRoam::errorHint(uint8_t error) {
	switch (error) {
		case 0:
			return to_string("No error.");
		case 1:
			return to_string("Invalid zone number.");
		case 2:
			return to_string("Invalid watering time.");
		case 3:
			return to_string("Invalid program number.");
		default:
			return to_string("Unknonwn error.");
	}
}

/**
 * Write low bit on the bus.
 */
void HunterRoam::sendLow() {
	this->pin_->digital_write(HIGH);
	delayMicroseconds(SHORT_INTERVAL);
	this->pin_->digital_write(LOW);
	delayMicroseconds(LONG_INTERVAL);
}

/**
 * Write high bit on the bus
 * 
 * Arguments: none
 */
void HunterRoam::sendHigh() {
	this->pin_->digital_write(HIGH);
	delayMicroseconds(LONG_INTERVAL);
	this->pin_->digital_write(LOW);
	delayMicroseconds(SHORT_INTERVAL);
}

/**
 * Write the bit sequence out of the bus
 * 
 * @param buffer blob containing the bits to transmit
 * @param extrabit if true, then write an extra 1 bit
 */
void HunterRoam::writeBus(std::vector<uint8_t> buffer, bool extrabit) {
	// Resetimpulse
	this->pin_->digital_write(HIGH);
	delay(325); //milliseconds
	this->pin_->digital_write(LOW);
	delay(65); //milliseconds

	// Startimpulse
	this->pin_->digital_write(HIGH);
	delayMicroseconds(START_INTERVAL);
	this->pin_->digital_write(LOW);
	delayMicroseconds(SHORT_INTERVAL);

	// Write the bits out
	for (auto &sendByte : buffer) {
		for (uint8_t inner = 0; inner < 8; inner++) {
			// Send high order bits first
			(sendByte & 0x80) ? sendHigh() : sendLow();
			sendByte <<= 1;
		}
	}

	// Include an extra 1 bit
	if (extrabit) {
		sendHigh();
	}

	// Write the stop pulse
	sendLow();
}

/**	
 * Set a value with an arbitrary bit width to a bit position within a blob.
 * 
 * @param bits blob to write the value to
 * @param pos position within the blob
 * @param val to write
 * @param len in bits of the value
 */
void HunterRoam::hunterBitfield(std::vector <uint8_t> &bits, uint8_t pos, uint8_t val, uint8_t len) {
	while (len > 0) {
		if (val & 0x1) {
			bits[pos / 8] = bits[pos / 8] | 0x80 >> (pos % 8);
		} else {
			bits[pos / 8] = bits[pos / 8] & ~(0x80 >> (pos % 8));
		}
		len--;
		val = val >> 1;
		pos++;
	}
}

/**
 * Start a zone
 * 
 * @param zone zone number (1-48)
 * @param time time in minutes (0-240)
 */
uint8_t HunterRoam::startZone(uint8_t zone, uint8_t time) {

	// Start out with a base frame
	std::vector<uint8_t> buffer = {0xff,0x00,0x00,0x00,0x10,0x00,0x00,0x04,0x00,0x00,0x01,0x00,0x01,0xb8,0x3f};

	if (zone < 1 || zone > 48) {
		return 1;
	}

	if (time > 240) {
		return 2;
	}

	// The bus protocol is a little bizzare, not sure why

	// Bits 9:10 are 0x1 for zones > 12 and 0x2 otherwise
	if (zone > 12) {
		hunterBitfield(buffer, 9, 0x1, 2);
	} else {
		hunterBitfield(buffer, 9, 0x2, 2);
	}

	// Zone + 0x17 is at bits 23:29 and 36:42
	hunterBitfield(buffer, 23, zone + 0x17, 7);
	hunterBitfield(buffer, 36, zone + 0x17, 7);

	// Zone + 0x23 is at bits 49:55 and 62:68
	hunterBitfield(buffer, 49, zone + 0x23, 7);
	hunterBitfield(buffer, 62, zone + 0x23, 7);

	// Zone + 0x2f is at bits 75:81 and 88:94
	hunterBitfield(buffer, 75, zone + 0x2f, 7);
	hunterBitfield(buffer, 88, zone + 0x2f, 7);

	// Time is encoded in three places and broken up by nibble
	// Low nibble: bits 31:34, 57:60, and 83:86
	// High nibble: bits 44:47, 70:73, and 96:99
	hunterBitfield(buffer, 31, time, 4);
	hunterBitfield(buffer, 44, time >> 4, 4);
	hunterBitfield(buffer, 57, time, 4);
	hunterBitfield(buffer, 70, time >> 4, 4);
	hunterBitfield(buffer, 83, time, 4);
	hunterBitfield(buffer, 96, time >> 4, 4);

	// Bottom nibble of zone - 1 is at bits 109:112
	hunterBitfield(buffer, 109, zone - 1, 4);

	// Write the bits out of the bus
	writeBus(buffer, true);

	return 0;
}

/**
 * Stop a zone
 * 
 * @param zone - zone number (1-48)
 */
uint8_t HunterRoam::stopZone(uint8_t zone) {
	return startZone(zone, 0);
}

/**
 * Run a program
 * 
 * @param num - program number (1-4)
 */
uint8_t HunterRoam::startProgram(uint8_t num) {
	// Start with a basic program frame
	std::vector<uint8_t> buffer = {0xff, 0x40, 0x03, 0x96, 0x09 ,0xbd ,0x7f};

	if (num < 1 || num > 4) {
		return 3;
	}

	// Program number - 1 is at bits 31:32
	hunterBitfield(buffer, 31, num - 1, 2);
	writeBus(buffer, false);

	return 0;
}


}  // namespace dallas
}  // namespace esphome
