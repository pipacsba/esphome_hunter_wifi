import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import number # ez valoszinu nem fog kelleni
from esphome.components import switch
from esphome.const import (
    CONF_ID, 
    CONF_PIN, 
    CONF_NAME, 
    CONF_NUMBER, 
    CONF_ENTITY_CATEGORY, 
    ENTITY_CATEGORY_CONFIG, 
    CONF_INITIAL_VALUE,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_RESTORE_VALUE,
    CONF_STEP,
)


MULTI_CONF = False; #can be True in future if I understand the consequences
AUTO_LOAD = ["number", "switch"]
CODEOWNERS = ["pipacsba"]

CONF_VALVES = "valves"
CONF_ZONE = "zone"
CONF_MAX_DURATION_MINUTES = "max_duration_minutes"
CONF_DURATION_NUMBER_ID = "duration_id"

hunterwifi_ns = cg.esphome_ns.namespace("hunterwifi")
HunterWifiComponent = hunterwifi_ns.class_("HunterWifiComponent", cg.Component)
HunterZoneSwitch = hunterwifi_ns.class_(
    "HunterZoneSwitch", switch.Switch, cg.Component
)

def validate_min_max(config):
    if config[CONF_MAX_VALUE] <= config[CONF_MIN_VALUE]:
        raise cv.Invalid(f"{CONF_MAX_VALUE} must be greater than {CONF_MIN_VALUE}")

    if (config[CONF_INITIAL_VALUE] > config[CONF_MAX_VALUE]) or (
        config[CONF_INITIAL_VALUE] < config[CONF_MIN_VALUE]
    ):
        raise cv.Invalid(
            f"{CONF_INITIAL_VALUE} must be a value between {CONF_MAX_VALUE} and {CONF_MIN_VALUE}"
        )
    return config

def validate_hunterwifi(config):
  for hunterwifi_controller_index, hunterwifi_controller in enumerate(config):
    if len(hunterwifi_controller[CONF_VALVES]) <= 1:
        raise cv.Invalid(f"At least valve is required for {hunterwifi_controller_index}")
    for valve in hunterwifi_controller[CONF_VALVES]:
        if CONF_ID not in valve:
            raise cv.Invalid(f"{CONF_ID} is required for each valve")
        if CONF_ZONE not in valve:
            raise cv.Invalid(f"{CONF_ZONE} is required for each valve")
        if CONF_MAX_DURATION_MINUTES not in valve:
            raise cv.Invalid(f"{CONF_MAX_DURATION_MINUTES} is required for each valve")
    return config

HUNTERWIFI_VALVE_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ZONE): cv.int_range(1, 48),
        cv.Required(CONF_MAX_DURATION_MINUTES): cv.int_range(1, 240),
        cv.Required(CONF_ID): cv.maybe_simple_value(
            switch.switch_schema(HunterZoneSwitch),
            key=CONF_ID,
        ),
        cv.Optional(CONF_DURATION_NUMBER_ID, default = "None"): cv.string,
    }
)

HUNTERWIFI_CONTROLLER_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HunterWifiComponent),
        cv.Required(CONF_PIN): pins.internal_gpio_output_pin_schema,
        cv.Required(CONF_VALVES): cv.ensure_list(HUNTERWIFI_VALVE_SCHEMA)
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.All(
    cv.ensure_list(HUNTERWIFI_CONTROLLER_SCHEMA),
    validate_hunterwifi,
)

async def to_code(config):
  for hunterwifi_controller in config:
    # create and registaer hunterwifi controller
    var = cg.new_Pvariable(hunterwifi_controller[CONF_ID])
    await cg.register_component(var, hunterwifi_controller)
    
    #create pin
    pin = await cg.gpio_pin_expression(hunterwifi_controller[CONF_PIN])
    #add pin to hunterwifi controller
    cg.add(var.set_pin(pin))
        
    # for each valve
    for valve_index, valve in enumerate(hunterwifi_controller[CONF_VALVES]):
        # create a switch with the defined name
        sw_valve_var = await switch.new_switch(valve[CONF_ID])
        await cg.register_component(sw_valve_var, valve[CONF_ID])
        #add pin number to the switch
        cg.add(sw_valve_var.set_pin(pin))
        #add zone number to the switch
        zone_number = int(valve[CONF_ZONE])
        cg.add(sw_valve_var.set_zone(zone_number))
        #add max duration to the switch
        max_duration = int(valve[CONF_MAX_DURATION_MINUTES])
        cg.add(sw_valve_var.set_max_duration(max_duration))
        #add duration number id to the switch
        duration_id = valve[CONF_DURATION_NUMBER_ID]
        cg.add(sw_valve_var.set_duration_number_id(duration_id))
        
        #add valve to hunterwifi controller
        cg.add(var.add_valve(sw_valve_var, zone_number, max_duration, duration_id))
        

  # this is only valid for multiple hunterwifi controllers
  # let the hunterwificontroller know about each other
  # not tested functionality, and maybe no even needed as there is no hunterwifi main switch
  for hunterwifi_controller in config:
      var = await cg.get_variable(hunterwifi_controller[CONF_ID])
      for controller_to_add in config:
          if hunterwifi_controller[CONF_ID] != controller_to_add[CONF_ID]:
              cg.add(
                  var.add_controller(
                      await cg.get_variable(controller_to_add[CONF_ID])
                  )
              )
