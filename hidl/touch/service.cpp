/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.touch@1.0-service.sony"

#include <android-base/logging.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <touch/sony/HighTouchPollingRate.h>

using ::vendor::lineage::touch::V1_0::IHighTouchPollingRate;
using ::vendor::lineage::touch::V1_0::implementation::HighTouchPollingRate;

int main() {
    android::sp<IHighTouchPollingRate> htprService = new HighTouchPollingRate();

    android::hardware::configureRpcThreadpool(1, true /*callerWillJoin*/);

    if (htprService->registerAsService() != android::OK) {
        LOG(ERROR) << "Cannot register high touch polling rate HAL service.";
        return 1;
    }

    LOG(INFO) << "Touchscreen HAL service ready.";

    android::hardware::joinRpcThreadpool();

    LOG(ERROR) << "Touchscreen HAL service failed to join thread pool.";
    return 1;
}
