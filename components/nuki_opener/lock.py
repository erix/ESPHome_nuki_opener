import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import lock, binary_sensor, text_sensor, sensor, switch
from esphome.const import CONF_ID, CONF_BATTERY_LEVEL, DEVICE_CLASS_CONNECTIVITY, DEVICE_CLASS_BATTERY, DEVICE_CLASS_DOOR, UNIT_PERCENT, ENTITY_CATEGORY_CONFIG, DEVICE_CLASS_PRESENCE

AUTO_LOAD = ["binary_sensor", "sensor", "switch"]

CONF_NUKI_ID = "opener_id"
CONF_IS_CONNECTED = "is_connected"
CONF_IS_PAIRED = "is_paired"
CONF_UNPAIR = "unpair"
CONF_BATTERY_CRITICAL = "battery_critical"
CONF_BATTERY_LEVEL = "battery_level"
CONF_DOORBELL = "doorbell"

nuki_opener_ns = cg.esphome_ns.namespace('nuki_opener')
NukiOpener = nuki_opener_ns.class_('NukiOpenerComponent', lock.Lock, switch.Switch, cg.Component)

CONFIG_SCHEMA = lock.LOCK_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(NukiOpener),
    cv.Required(CONF_NUKI_ID): cv.int_,
    cv.Required(CONF_IS_CONNECTED): binary_sensor.binary_sensor_schema(
                    device_class=DEVICE_CLASS_CONNECTIVITY,
                ),
    cv.Required(CONF_IS_PAIRED): binary_sensor.binary_sensor_schema(
                    device_class=DEVICE_CLASS_CONNECTIVITY,
                ),
    cv.Optional(CONF_BATTERY_CRITICAL): binary_sensor.binary_sensor_schema(
                    device_class=DEVICE_CLASS_BATTERY,
                ),
    cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
                    device_class=DEVICE_CLASS_BATTERY,
                    unit_of_measurement=UNIT_PERCENT,
                ),
    cv.Optional(CONF_DOORBELL): binary_sensor.binary_sensor_schema(
                ),
    cv.Optional(CONF_UNPAIR, default=False): cv.boolean,
}).extend(cv.polling_component_schema("500ms"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await lock.register_lock(var, config)

    if CONF_NUKI_ID in config:
        cg.add(var.set_nuki_id(config[CONF_NUKI_ID]))
    if CONF_IS_CONNECTED in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_IS_CONNECTED])
        cg.add(var.set_is_connected(sens))

    if CONF_IS_PAIRED in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_IS_PAIRED])
        cg.add(var.set_is_paired(sens))

    if CONF_BATTERY_CRITICAL in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_BATTERY_CRITICAL])
        cg.add(var.set_battery_critical(sens))

    if CONF_BATTERY_LEVEL in config:
        sens = await sensor.new_sensor(config[CONF_BATTERY_LEVEL])
        cg.add(var.set_battery_level(sens))

    if CONF_DOORBELL in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_DOORBELL])
        cg.add(var.set_doorbell(sens))

    if CONF_UNPAIR in config:
        cg.add(var.set_unpair(config[CONF_UNPAIR]))
    
