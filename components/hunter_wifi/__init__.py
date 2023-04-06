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

hunterwifi_ns = cg.esphome_ns.namespace("hunterwifi")
HunterWifiComponent = hunterwifi_ns.class_("HunterWifiComponent", cg.Component)
HunterControllerSwitch = hunterwifi_ns.class_(
    "HunterControllerSwitch", switch.Switch, cg.Component
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
        if CONF_NUMBER not in valve:
            raise cv.Invalid(f"{CONF_NUMBER} is required for each valve")
    return config

HUNTERWIFI_VALVE_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_NUMBER): cv.maybe_simple_value(
            number.NUMBER_SCHEMA.extend(
                {
                    #cv.GenerateID(): cv.declare_id(HunterWifiControllerNumber),
                    cv.Optional(
                        CONF_ENTITY_CATEGORY, default=ENTITY_CATEGORY_CONFIG
                    ): cv.entity_category,
                    cv.Optional(CONF_INITIAL_VALUE, default=1): cv.positive_int,
                    cv.Optional(CONF_MAX_VALUE, default=48): cv.positive_int,
                    cv.Optional(CONF_MIN_VALUE, default=1): cv.positive_int,
                    cv.Optional(CONF_RESTORE_VALUE, default=True): cv.boolean,
                    cv.Optional(CONF_STEP, default=1): cv.positive_int,
                }
            ).extend(cv.COMPONENT_SCHEMA),
            validate_min_max,
            key=CONF_NAME,
        ),
        cv.Required(CONF_ID): cv.maybe_simple_value(
            switch.switch_schema(HunterControllerSwitch),
            key=CONF_ID,
        ),
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
    var = cg.new_Pvariable(hunterwifi_controller[CONF_ID])
    await cg.register_component(var, hunterwifi_controller)
        
    for valve_index, valve in enumerate(hunterwifi_controller[CONF_VALVES]):
        sw_valve_var = await switch.new_switch(valve[CONF_ID])
        await cg.register_component(sw_valve_var, valve[CONF_ID])
        cg.add(var.add_valve(sw_valve_var))
        cg.add(
            var.configure_valve_switch(
                valve_index, sw_valve_var, valve[CONF_NUMBER]
            )
        )
    pin = await cg.gpio_pin_expression(hunterwifi_controller[CONF_PIN])
    cg.add(var.set_pin(pin))
        
  # assign hunter zone number to switches (valve number/id)
  #for hunterwifi_controller in config:
  #  var = await cg.get_variable(hunterwifi_controller[CONF_ID])
  #  for valve_index, valve in enumerate(hunterwifi_controller[CONF_VALVES]):
  #        valve_switch = await cg.get_variable(valve[CONF_ID])
  #        cg.add(
  #            var.configure_valve_switch(
  #                valve_index, valve_switch, valve[CONF_NUMBER]
  #            )
  #        )
   # I do not know what this part does, copied and adopted from sprinkler component
  for hunterwifi_controller in config:
      var = await cg.get_variable(hunterwifi_controller[CONF_ID])
      for controller_to_add in config:
          if hunterwifi_controller[CONF_ID] != controller_to_add[CONF_ID]:
              cg.add(
                  var.add_controller(
                      await cg.get_variable(controller_to_add[CONF_ID])
                  )
              )
