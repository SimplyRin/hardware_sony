/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "powerhal-libperfmgr"

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android/binder_ibinder_platform.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <perfmgr/HintManager.h>

#include <thread>

#include "Power.h"
#include "PowerExt.h"
#include "PowerSessionManager.h"
#include "disp-power/DisplayLowPower.h"

using aidl::google::hardware::power::impl::pixel::DisplayLowPower;
using aidl::google::hardware::power::impl::pixel::Power;
using aidl::google::hardware::power::impl::pixel::PowerExt;
using aidl::google::hardware::power::impl::pixel::PowerSessionManager;
using ::android::perfmgr::HintManager;

constexpr std::string_view kPowerHalInitProp("vendor.powerhal.init");

int main() {
    android::base::SetDefaultTag(LOG_TAG);
    // Parse config but do not start the looper
    std::shared_ptr<HintManager> hm = HintManager::GetInstance();
    if (!hm) {
        LOG(FATAL) << "HintManager Init failed";
    }

    std::shared_ptr<DisplayLowPower> dlpw = std::make_shared<DisplayLowPower>();

    // single thread
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    // core service
    std::shared_ptr<Power> pw = ndk::SharedRefBase::make<Power>(dlpw);
    ndk::SpAIBinder pwBinder = pw->asBinder();
    AIBinder_setMinSchedulerPolicy(pwBinder.get(), SCHED_NORMAL, -20);

    // extension service
    std::shared_ptr<PowerExt> pwExt = ndk::SharedRefBase::make<PowerExt>(dlpw);
    auto pwExtBinder = pwExt->asBinder();
    AIBinder_setMinSchedulerPolicy(pwExtBinder.get(), SCHED_NORMAL, -20);

    // attach the extension to the same binder we will be registering
    CHECK(STATUS_OK == AIBinder_setExtension(pwBinder.get(), pwExt->asBinder().get()));

    const std::string instance = std::string() + Power::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(pw->asBinder().get(), instance.c_str());
    CHECK(status == STATUS_OK);
    LOG(INFO) << "Sony Power HAL AIDL Service with Extension is started.";

    std::thread initThread([&]() {
        ::android::base::WaitForProperty(kPowerHalInitProp.data(), "1");
        HintManager::GetInstance()->Start();
        dlpw->Init();
    });
    initThread.detach();

    ABinderProcess_joinThreadPool();

    // should not reach
    LOG(ERROR) << "Sony Power HAL AIDL Service with Extension just died.";
    return EXIT_FAILURE;
}
