#pragma once

#include "esphome/core/component.h"
#include "esphome/components/lock/lock.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/api/custom_api_device.h"

#include "NukiOpener.h"
#include "NukiConstants.h"
#include "BleScanner.h"

namespace esphome {
namespace nuki_opener {

static const char *TAG = "nukilock.lock";

class Handler: public Nuki::SmartlockEventHandler {
public:
  virtual ~Handler() {};
  Handler(bool *notified_p) { this->notified_p_ = notified_p; }
  void notify(Nuki::EventType eventType) {
    *(this->notified_p_) = true;
    ESP_LOGI(TAG, "event notified %d", eventType);      
  }
private:
  bool *notified_p_;
};

class NukiOpenerComponent : public lock::Lock, public PollingComponent, public api::CustomAPIDevice {
  static const uint8_t BLE_CONNECT_TIMEOUT_SEC = 3;
  static const uint8_t BLE_CONNECT_TIMEOUT_RETRIES = 1;

public:
  const uint32_t deviceId_ = 0x31d7ad1a; // get this ID from the Nuki app
  const std::string deviceName_ = "Door"; // get the name for the Nuki app 

  explicit NukiOpenerComponent() : Lock(), unpair_(false) { this->traits.set_supports_open(true); }

  void setup() override;
  void update() override;

  void set_is_connected(binary_sensor::BinarySensor *is_connected) { this->is_connected_ = is_connected; }
  void set_is_paired(binary_sensor::BinarySensor *is_paired) { this->is_paired_ = is_paired; }
  void set_battery_critical(binary_sensor::BinarySensor *battery_critical) { this->battery_critical_ = battery_critical; }
  void set_doorbell(binary_sensor::BinarySensor *doorbell) { this->doorbell_ = doorbell; }
  void set_battery_level(sensor::Sensor *battery_level) { this->battery_level_ = battery_level; }
  void set_unpair(bool unpair) {this->unpair_ = unpair; }

  float get_setup_priority() const override { return setup_priority::HARDWARE - 1.0f; }

  void dump_config() override;

  lock::LockState nuki_to_lock_state(NukiOpener::LockState);
  // bool nuki_doorsensor_to_binary(Nuki::DoorSensorState);
  // std::string nuki_doorsensor_to_string(Nuki::DoorSensorState nukiDoorSensorState);

protected:
  void control(const lock::LockCall &call) override;
  void update_status();

  binary_sensor::BinarySensor *is_connected_{nullptr};
  binary_sensor::BinarySensor *is_paired_{nullptr};
  binary_sensor::BinarySensor *battery_critical_{nullptr};
  binary_sensor::BinarySensor *doorbell_{nullptr};
  sensor::Sensor *battery_level_{nullptr};
  NukiOpener::NukiOpener *nukiLock_;
  BleScanner::Scanner scanner_;
  NukiOpener::OpenerState retrievedOpenerState_;
  Handler *handler_;
  bool status_update_;
  bool unpair_;
};

} //namespace nuki_lock
} //namespace esphome
