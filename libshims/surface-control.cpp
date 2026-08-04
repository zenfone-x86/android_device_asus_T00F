/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <gui/SurfaceControl.h>

extern "C" {
    /* Blob expects setLayer(uint32_t) but 8.1 has setLayer(int32_t).
     * Both are 32-bit and ABI-compatible on x86. */
    void _ZN7android14SurfaceControl8setLayerEj(uint32_t layer) {}
}
