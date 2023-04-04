import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID, CONF_PIN, CONF_NAME

MULTI_CONF = False; #can be True in future if I understand the consequences
AUTO_LOAD = ["number", "switch"]

hunterwifi_ns = cg.esphome_ns.namespace("hunterwifi")
HunterWifiComponent = hunterwifi_ns.class_("HunterWifiComponent", cg.Component)

CONST_MAX_VALVES = 48
CONF_VALVES = "valves"
#CONF_VALVE_NAME = "name"
#CONF_VALVE_ID = "id"
CONF_VALVE_NUMBER = "number"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HunterWifiComponent),
        cv.Required(CONF_PIN): pins.internal_gpio_output_pin_schema,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))
