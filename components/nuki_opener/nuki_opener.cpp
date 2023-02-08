#include "esphome/core/log.h"
#include "nuki_opener.h"

namespace esphome {
namespace nuki_opener {

lock::LockState NukiOpenerComponent::nuki_to_lock_state(NukiOpener::LockState nukiLockState) {
  switch(nukiLockState) {
    case NukiOpener::LockState::Locked:
      return lock::LOCK_STATE_LOCKED;
    case NukiOpener::LockState::Open:
      return lock::LOCK_STATE_UNLOCKED;
    case NukiOpener::LockState::RTOactive:
      return lock::LOCK_STATE_UNLOCKING;
    default:
      return lock::LOCK_STATE_NONE;
  }
}


void NukiOpenerComponent::update_status()
{
  this->status_update_ = false;
  Nuki::CmdResult result;
  if (this->nukiLock_->isPairedWithLock()) {
    result = this->nukiLock_->requestOpenerState(&(this->retrievedOpenerState_));
  } else {
    ESP_LOGE(TAG, "Nuki not paired");
  }

  if (result == Nuki::CmdResult::Success) {
    // ESP_LOGI(TAG, "Bat state: %#x, Bat crit: %d, Bat perc:%d lock state: %d %d:%d:%d",
    ESP_LOGI(TAG, "Bat state: %#x, lock state: %d %d:%d:%d",
             this->retrievedOpenerState_.criticalBatteryState,
             // this->nukiLock_->isBatteryCritical(), this->nukiLock_->getBatteryPerc(), 
             this->retrievedOpenerState_.lockState, 
             this->retrievedOpenerState_.currentTimeHour,
             this->retrievedOpenerState_.currentTimeMinute, 
             this->retrievedOpenerState_.currentTimeSecond);

    lock::LockState lockState = this->nuki_to_lock_state(this->retrievedOpenerState_.lockState);
    if (this->state == lockState) {
      // Ringer is on
      this->doorbell_->publish_state(true);
    } else {
      this->publish_state(this->nuki_to_lock_state(this->retrievedOpenerState_.lockState));
    }
    this->publish_state(this->nuki_to_lock_state(this->retrievedOpenerState_.lockState));
    this->is_connected_->publish_state(true);
    if (this->battery_critical_ != nullptr)
        this->battery_critical_->publish_state(this->nukiLock_->isBatteryCritical());
    if (this->battery_level_ != nullptr)
        this->battery_level_->publish_state((this->retrievedOpenerState_.criticalBatteryState & 0b11111100) >> 1);
  } else {
    ESP_LOGE(TAG, "requestOpenerState failed: %d", result);
    this->is_connected_->publish_state(false);
    this->publish_state(lock::LOCK_STATE_NONE);
    this->status_update_ = true;
  }  
}

void NukiOpenerComponent::setup() {

  ESP_LOGI(TAG, "Starting NUKI Lock...");
  this->nukiLock_ = new NukiOpener::NukiOpener(this->deviceName_, this->deviceId_);
  this->handler_ = new nuki_opener::Handler(&(this->status_update_));

  this->traits.set_supported_states(std::set<lock::LockState> {lock::LOCK_STATE_NONE,
                                      lock::LOCK_STATE_LOCKED, 
                                      lock::LOCK_STATE_UNLOCKED, 
                                      lock::LOCK_STATE_UNLOCKING});
  this->scanner_.initialize();
  this->nukiLock_->registerBleScanner(&this->scanner_);
  this->nukiLock_->initialize();
  this->nukiLock_->setConnectTimeout(BLE_CONNECT_TIMEOUT_SEC);
  this->nukiLock_->setConnectRetries(BLE_CONNECT_TIMEOUT_RETRIES);

  if (this->unpair_) {
    ESP_LOGW(TAG, "Unpair requested");
    this->nukiLock_->unPairNuki();
  }

  if (this->nukiLock_->isPairedWithLock()) {
    this->status_update_ = true;
    ESP_LOGI(TAG, "%s Nuki paired", this->deviceName_); 
    this->is_paired_->publish_initial_state(true);
    this->nukiLock_->setEventHandler(this->handler_);
  }
else {
    ESP_LOGW(TAG, "%s Nuki is not paired", this->deviceName_); 
    this->is_paired_->publish_initial_state(false);
  }

  this->publish_state(lock::LOCK_STATE_NONE);
  this->doorbell_->publish_state(false);

  // register_service(&NukiLockComponent::lock_n_go, "lock_n_go");
}

void NukiOpenerComponent::update() {

  this->scanner_.update();

  if (this->nukiLock_->isPairedWithLock()) {
    this->is_paired_->publish_state(true);
    if (this->status_update_) {
      this->update_status();
    }
    if (this->doorbell_->state) {
      this->doorbell_->publish_state(false);
    }
  }
else if (! this->unpair_) {
    bool paired = (this->nukiLock_->pairNuki() == Nuki::PairingResult::Success);
    if (paired) {
      ESP_LOGI(TAG, "Nuki paired");
      this->nukiLock_->setEventHandler(this->handler_);
      this->update_status();
    }
    this->is_paired_->publish_state(paired);
  }
}

void NukiOpenerComponent::control(const lock::LockCall &call) {
  if (!this->nukiLock_->isPairedWithLock()) {
    ESP_LOGE(TAG, "Lock/Unlock action called for unpaired nuki");
    return;
  }
  ESP_LOGE(TAG, "Lock/Unlock action called");

  auto state = *call.get_state();
  uint8_t result;

  NukiOpener::LockAction action = NukiOpener::LockAction::ElectricStrikeActuation;
  result = this->nukiLock_->lockAction(action);

  if (result == Nuki::CmdResult::Success) {
    this->publish_state(state);
  } else {
    ESP_LOGE(TAG, "lockAction failed: %d", result);
    this->is_connected_->publish_state(false);
    this->publish_state(lock::LOCK_STATE_NONE);
    this->status_update_ = true;
  }
}

void NukiOpenerComponent::dump_config(){
  LOG_LOCK(TAG, "Nuki Lock", this);    
  LOG_BINARY_SENSOR(TAG, "Is Connected", this->is_connected_);
  LOG_BINARY_SENSOR(TAG, "Is Paired", this->is_paired_);
  LOG_BINARY_SENSOR(TAG, "Battery Critical", this->battery_critical_);
  LOG_SENSOR(TAG, "Battery Level", this->battery_level_);
  ESP_LOGCONFIG(TAG, "Unpair request is %s", this->unpair_? "true":"false");
}

} //namespace nuki_opener
} //namespace esphome
