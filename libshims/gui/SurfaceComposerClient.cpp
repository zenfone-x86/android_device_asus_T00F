/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gui/SurfaceComposerClient.h>

namespace android {

/*
 * Android 8 added parent, window type and owner UID arguments to this method.
 * Export the Android 7 ABI used by the stock Clovertrail camera HAL and
 * forward it to the current implementation with the new arguments unset.
 */
sp<SurfaceControl> createSurfaceLegacy(
        SurfaceComposerClient* client, const String8& name, uint32_t width,
        uint32_t height, PixelFormat format, uint32_t flags)
        __asm__("_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8Ejjij");

sp<SurfaceControl> createSurfaceLegacy(
        SurfaceComposerClient* client, const String8& name, uint32_t width,
        uint32_t height, PixelFormat format, uint32_t flags) {
    return client->createSurface(name, width, height, format, flags, nullptr, 0, 0);
}

} // namespace android
