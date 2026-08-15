# LineageOS 15.1 Porting Summary for ASUS T00F

## LineageOS product was not registered

- **Issue:** `lineage_T00F` was not exposed as a build product.
- **Root Cause:** `AndroidProducts.mk` referenced the wrong product makefile.
- **Solution:** Register the LineageOS product makefile.
- **Affected Files:** `device/asus/T00F/AndroidProducts.mk`
- **Git Hash:** `device/asus/T00F/dbcb57944e69a6f125b77d0cd488147e54e85d91`

## Legacy SurfaceControl shim was incompatible with Oreo

- **Issue:** The legacy display shim was incompatible with the Oreo SurfaceControl ABI and later proved unnecessary.
- **Root Cause:** The device tree carried a CyanogenMod-era shim for symbols and layer behavior no longer used by the Oreo stack.
- **Solution:** First correct the shim ABI, then remove the obsolete shim from the build.
- **Affected Files:** `device/asus/T00F/libshims/surface-control.cpp`, `device/asus/T00F/libshims/Android.mk`
- **Git Hash:** `device/asus/T00F/05561ff40f6891f3a6a16f4db123f291f07154f3`, `device/asus/T00F/6b0af144883b5eba9650213e7bd9fab1304dbe4c`

## Duplicate Bluetooth SELinux transition

- **Issue:** SELinux policy compilation contained a duplicate Bluetooth domain transition.
- **Root Cause:** The device policy repeated a transition already supplied by the platform policy.
- **Solution:** Remove the duplicate device-specific transition.
- **Affected Files:** `device/asus/T00F/sepolicy/bluetooth.te`
- **Git Hash:** `device/asus/T00F/09b1dd0f0cea2143d84ea8d587d862bf2de3acd4`

## Missing legacy gralloc display-status operation

- **Issue:** Intel display userspace expected a gralloc operation absent from the Oreo headers.
- **Root Cause:** The proprietary Intel graphics stack uses a vendor extension that is not part of the standard Oreo HAL API.
- **Solution:** Restore the legacy display-status operation declaration.
- **Affected Files:** `device/asus/T00F/include/hal/hal_public.h`
- **Git Hash:** `device/asus/T00F/bd91e80c4b23dd2d46d819d588feb9cd9db3daae`

## healthd did not build against the Oreo battery API

- **Issue:** The CloverTrail healthd board implementation was incompatible with LineageOS 15.1.
- **Root Cause:** Oreo changed the healthd board API and required the implementation to link the correct healthd library.
- **Solution:** Adapt the callback signature and module dependencies to the Oreo API.
- **Affected Files:** `device/asus/T00F/libhealthd/Android.mk`, `device/asus/T00F/libhealthd/healthd_board_clovertrail.cpp`
- **Git Hash:** `device/asus/T00F/9b882da55bbf9aa9ccf5b8c7b9c50d1193c261fe`

## CM hardware classes used the old CyanogenMod namespace

- **Issue:** Device hardware classes were not found under LineageOS.
- **Root Cause:** Java sources still used `org.cyanogenmod.hardware` instead of `org.lineageos.hardware`.
- **Solution:** Move the classes and update their package declarations to the LineageOS namespace.
- **Affected Files:** `device/asus/T00F/cmhw/org/lineageos/hardware/HighTouchSensitivity.java`, `device/asus/T00F/cmhw/org/lineageos/hardware/KeyDisabler.java`, `device/asus/T00F/cmhw/org/lineageos/hardware/LongTermOrbits.java`, `device/asus/T00F/cmhw/org/lineageos/hardware/TouchscreenGestures.java`
- **Git Hash:** `device/asus/T00F/169c0fa14900d64ff0c024abc19328e06cd3b85a`

## Intel video HAL headers were missing

- **Issue:** The build failed with missing `linux/vxd_drm.h` and `ufo/gralloc.h` headers.
- **Root Cause:** The legacy Intel video and gralloc sources depend on vendor kernel/UFO interfaces not exported by the Oreo platform tree.
- **Solution:** Add the required compatibility headers to the device tree.
- **Affected Files:** `device/asus/T00F/include/ufo/gralloc.h`, `device/asus/T00F/kernel-headers/linux/vxd_drm.h`
- **Git Hash:** `device/asus/T00F/9d5786db028004094b9da322ba26577606fbbb75`

## Obsolete modem initialization blocked a clean Oreo boot

- **Issue:** The legacy modem services repeatedly failed during boot and generated unnecessary initialization activity.
- **Root Cause:** The CM14 Intel modem initialization path is not compatible with the LineageOS 15.1 radio stack used by this port.
- **Solution:** Disable the obsolete modem initialization components and service startup.
- **Affected Files:** `device/asus/T00F/rootdir/Android.mk`, `device/asus/T00F/rootdir/etc/init.redhookbay.rc`
- **Git Hash:** `device/asus/T00F/f8e49bba1fc3ed7fe4a826615c74b551ed2da084`

## Camera kernel modules were unavailable at boot

- **Issue:** Camera modules could not be loaded early enough from the system partition.
- **Root Cause:** The legacy camera stack needs its kernel modules before the normal system module path is available.
- **Solution:** Package the camera modules in the boot ramdisk and load them from init.
- **Affected Files:** `device/asus/T00F/mkbootimg.mk`, `device/asus/T00F/rootdir/etc/init.redhookbay.rc`
- **Git Hash:** `device/asus/T00F/18e79491e32a6e4e4ee3ca30fab090bc14212768`

## Standard mkbootimg could not produce a bootable Intel image

- **Issue:** LineageOS-generated boot images were not packed in the format expected by the T00F firmware, and later builds could fail while replacing the output image.
- **Root Cause:** T00F uses an Intel OSIP-compatible boot image layout and the wrapper did not safely handle all input/output path combinations.
- **Solution:** Add an Intel boot-image wrapper and harden its temporary-output and replacement handling.
- **Affected Files:** `device/asus/T00F/BoardConfig.mk`, `device/asus/T00F/mkbootimg/mkbootimg-wrap`
- **Git Hash:** `device/asus/T00F/6f9ae082ce65a8eccd1980969da321240eccf305`, `device/asus/T00F/78144115a50fe2aea4cc1184a7b9b216cb84bcf9`

## Legacy proprietary libraries failed to link or load

- **Issue:** Keystore, crypto, camera, and other legacy blobs failed because required old symbols were absent on Oreo.
- **Root Cause:** The blobs were built against older Android/OpenSSL/graphics ABIs.
- **Solution:** Configure device linker shims that provide the required compatibility symbols.
- **Affected Files:** `device/asus/T00F/BoardConfig.mk`
- **Git Hash:** `device/asus/T00F/a04eb2ba57269c917f1859cbb47931edf1786e89`

## Kernel panic routing to recovery was unsafe

- **Issue:** A diagnostic `panic_recovery=1` option routed hard resets to recovery but also invalidated OSIP during every normal boot.
- **Root Cause:** Arming recovery reused the normal recovery-target callback, which writes the reboot reason and invalidates the main OSIP entry immediately.
- **Solution:** Remove the unsafe boot option and stop invoking the OSIP recovery callback during initialization.
- **Affected Files:** `device/asus/T00F/BoardConfig.mk`, `kernel/asus/T00F/drivers/external_drivers/drivers/platform/x86/intel_mid_osip.c`
- **Git Hash:** `device/asus/T00F/5e5ac8e6163746cf3a918c8a3c803589ce51ae29`, `kernel/asus/T00F/d1c71544205253dac2e48cdb78f90386c606c283`, `device/asus/T00F/5361942d8fa303c5a1f1cc0a990e8317a33fa94f`, `kernel/asus/T00F/91a47bb0ada678c78d73e0848db43afaeb4f1a46`

## Boot, system, and vendor identity mismatch warning

- **Issue:** Android reported that the vendor image did not match the system image.
- **Root Cause:** The independently generated system, vendor, and boot properties advertised inconsistent build identities.
- **Solution:** Keep the image identities aligned with the stock/vendor firmware expectations.
- **Affected Files:** `device/asus/T00F/BoardConfig.mk`, `device/asus/T00F/lineage.mk`
- **Git Hash:** `device/asus/T00F/7819f7caeaa0b3f162d5c42a594aa060cf5faf2c`

## Incorrect misc partition path

- **Issue:** Recovery and reboot-control operations could not access the misc partition reliably.
- **Root Cause:** `fstab.redhookbay` pointed misc at the wrong block device.
- **Solution:** Correct the misc partition block-device path.
- **Affected Files:** `device/asus/T00F/rootdir/etc/fstab.redhookbay`
- **Git Hash:** `device/asus/T00F/2bd1db9dd3184d64e51ff76e737a02bbafb65297`

## Recovery image size did not match the real partition

- **Issue:** Recovery image generation used an incorrect partition limit and later exceeded the actual available space.
- **Root Cause:** `BOARD_RECOVERYIMAGE_PARTITION_SIZE` did not reflect the T00F recovery layout, and the recovery ramdisk included the optional `bu` utility.
- **Solution:** Set the real recovery size and omit `bu` from recovery to keep the image within the partition.
- **Affected Files:** `device/asus/T00F/BoardConfig.mk`, `device/asus/T00F/libintel-updater/Android.mk`, `bootable/recovery/Android.mk`
- **Git Hash:** `device/asus/T00F/552fd2e393704af87f15046b2426cc0bfef8aa10`, `device/asus/T00F/b236899b7f2915c0d5dab957ee40d027613d6219`, `bootable/recovery/96e505c270934eaa034595a31a98cf13d8207f90`

## Developer options crashed on hybrid recovery boot

- **Issue:** Opening Settings > Developer options crashed.
- **Root Cause:** Hybrid boot/recovery images did not install the recovery fstab expected by framework storage inspection.
- **Solution:** Copy the recovery fstab into the generated image.
- **Affected Files:** `device/asus/T00F/device.mk`
- **Git Hash:** `device/asus/T00F/fc9f2c9bf32445728d187b8a93b25ec1f8e799d0`

## Invalid `/logs` partition mounts

- **Issue:** Init repeatedly attempted to mount and use a `/logs` partition that does not exist on T00F.
- **Root Cause:** Leftover Intel debug init rules assumed a partition absent from this device layout.
- **Solution:** Remove the invalid mount and related log-copy rules.
- **Affected Files:** `device/asus/T00F/rootdir/etc/init.debug.rc`, `device/asus/T00F/rootdir/etc/init.redhookbay.rc`
- **Git Hash:** `device/asus/T00F/4e95b85bcf2317519b570f3b46bdad4b825a12da`

## Device overlay path was registered twice

- **Issue:** The same overlay directory was applied twice.
- **Root Cause:** `lineage.mk` duplicated a device overlay path already inherited elsewhere.
- **Solution:** Remove the duplicate overlay registration.
- **Affected Files:** `device/asus/T00F/lineage.mk`
- **Git Hash:** `device/asus/T00F/83d3b438c997edbfe3768d1313100e72db67292c`

## Automatic rotation did not work

- **Issue:** Android did not receive orientation sensor events, so automatic rotation failed.
- **Root Cause:** The source-built sensor HAL was not packaged and the sensors HIDL passthrough service was absent from the device manifest.
- **Solution:** Package `sensors.redhookbay` and declare the sensors 1.0 passthrough HAL.
- **Affected Files:** `device/asus/T00F/device.mk`, `device/asus/T00F/manifest.xml`
- **Git Hash:** `device/asus/T00F/9c1ff337858c5d57effc20e481664d6ce1ad8d56`

## OSIP OTA flashing support was broken

- **Issue:** Recovery packages could not safely update Intel boot/recovery OSIP entries.
- **Root Cause:** The updater integration used incorrect device nodes, file contexts, and OSIP update behavior for the T00F partition layout.
- **Solution:** Restore the Intel updater path and align recovery updater logic and SELinux labels with T00F.
- **Affected Files:** `device/asus/T00F/BoardConfig.mk`, `device/asus/T00F/libintel-updater/libosip_updater.cpp`, `device/asus/T00F/recovery/recovery_updater.cpp`, `device/asus/T00F/sepolicy/file_contexts`, `build/make/tools/releasetools/common.py`, `build/make/tools/releasetools/edify_generator.py`
- **Git Hash:** `device/asus/T00F/cec03dd6c42c4f3d0bcd61048448228972b3c922`, `build/make/2ec129cf00d56193785b714c1862385ee753249e`

## OTA device assertion rejected legacy recovery

- **Issue:** A valid T00F update package failed with Error 7 when recovery returned empty product properties.
- **Root Cause:** Releasetools could only generate assertions for non-empty device names, while the legacy recovery exposed neither `ro.product.device` nor `ro.build.product`.
- **Solution:** Support an explicitly empty assertion entry, accepting it only when both recovery properties are empty while still rejecting a definite unsupported device.
- **Affected Files:** `build/make/tools/releasetools/edify_generator.py`
- **Git Hash:** `build/make/f6a5c25a1eaf1a08f24031ad74b7fe5a171e3399`

## Symlinked device tree was not discovered by the build system

- **Issue:** The LineageOS product was not found when `device/asus/T00F` was provided through a symbolic link.
- **Root Cause:** Product discovery used `find` without following symbolic links.
- **Solution:** Use `find -L` while locating `lineage.mk`.
- **Affected Files:** `build/make/core/product_config.mk`
- **Git Hash:** `build/make/1aa7160a261ed89befbb4cc28a1866eb29e881b7`

## Releasetools changes did not invalidate target-files output

- **Issue:** OTA packages could retain stale updater scripts after changing the device releasetools extension.
- **Root Cause:** The target-files package stored the extension path as metadata but did not declare it as a build dependency.
- **Solution:** Add the device releasetools extension as a prerequisite of the target-files package.
- **Affected Files:** `build/make/core/Makefile`
- **Git Hash:** `build/make/244a4a149f1fe450c542c78f508922019df92e27`

## Device-specific SELinux denials

- **Issue:** Oreo services encountered denials for ASUS configuration, Wi-Fi supplicant, properties, proc entries, and system-server operations.
- **Root Cause:** The inherited policy did not describe several legacy T00F nodes and service interactions.
- **Solution:** Add narrowly scoped device types, labels, property contexts, and allow rules.
- **Affected Files:** `device/asus/T00F/sepolicy/asus_config.te`, `device/asus/T00F/sepolicy/file.te`, `device/asus/T00F/sepolicy/genfs_contexts`, `device/asus/T00F/sepolicy/hal_wifi_supplicant_default.te`, `device/asus/T00F/sepolicy/property_contexts`, `device/asus/T00F/sepolicy/system_server.te`
- **Git Hash:** `device/asus/T00F/f0954771aa432e0c55987f95893f5c312dc2f086`

## Boot-time VMA corruption panic

- **Issue:** The kernel could panic while bootanimation assigned anonymous VMA names.
- **Root Cause:** `prctl_set_vma_anon_name()` calculated a clamped end address but passed the original end, allowing an update to cross the current VMA and corrupt the VMA tree.
- **Solution:** Pass the clamped address to the VMA update helper.
- **Affected Files:** `kernel/asus/T00F/kernel/sys.c`
- **Git Hash:** `kernel/asus/T00F/a7b674352c8be647bc5f7212758233bd34aa6e40`

## sdcardfs fallback mount kernel panic

- **Issue:** A failed sdcardfs mount could panic during the following fallback mount.
- **Root Cause:** The mount failure path freed per-mount data twice and corrupted the kmalloc freelist.
- **Solution:** Let `free_vfsmnt()` perform the single required release.
- **Affected Files:** `kernel/asus/T00F/fs/namespace.c`
- **Git Hash:** `kernel/asus/T00F/da6d09f0ee553f361b4344e7bfc0051865721bc5`

## Random reboot during display suspend/resume

- **Issue:** Screen-off or resume could hard-reset the device and send it to recovery.
- **Root Cause:** Aborted suspend attempts caused duplicate S0ix PM QoS operations; the legacy one-shot panel reset and synchronous VSYNC-timeout recovery raced the DSI power sequence.
- **Solution:** Guard the QoS request lifecycle, disable the one-shot panel-reset workaround, and stop power-cycling the panel synchronously from the VSYNC ioctl timeout path.
- **Affected Files:** `kernel/asus/T00F/drivers/external_drivers/intel_media/common/psb_drv.c`, `kernel/asus/T00F/drivers/external_drivers/intel_media/common/psb_powermgmt.c`, `kernel/asus/T00F/drivers/external_drivers/intel_media/display/pnw/drv/mdfld_dsi_dpi.a500cg.c`
- **Git Hash:** `kernel/asus/T00F/f9ca91d43ee979f21f2fe8f2080757274775345c`

## USB defaulted to an unsuitable configuration

- **Issue:** USB did not default to the expected file-transfer mode after boot.
- **Root Cause:** The product configuration selected a legacy/default USB function set instead of MTP.
- **Solution:** Set MTP as the default USB configuration.
- **Affected Files:** `device/asus/T00F/device.mk`
- **Git Hash:** `device/asus/T00F/e20ba68022dd230663ad6787851bb3ddf7f3a195`

## Legacy camera userspace and sensor modules were incomplete

- **Issue:** The camera stack could not initialize correctly on Oreo.
- **Root Cause:** The legacy wrapper needed an Oreo-compatible SurfaceComposer shim and service setup, while required AtomISP sensor modules were not enabled or registered.
- **Solution:** Update the camera wrapper/init integration, add the SurfaceComposer compatibility shim, and enable/register the legacy camera sensor modules in the kernel.
- **Affected Files:** `device/asus/T00F/camera/Android.mk`, `device/asus/T00F/camera/CameraWrapper.cpp`, `device/asus/T00F/libshims/Android.mk`, `device/asus/T00F/libshims/gui/SurfaceComposerClient.cpp`, `device/asus/T00F/rootdir/etc/init.avc.rc`, `device/asus/T00F/rootdir/etc/init.redhookbay.rc`, `kernel/asus/T00F/arch/x86/configs/lineage_T00F_defconfig`, `kernel/asus/T00F/drivers/external_drivers/camera/drivers/media/i2c/Makefile`, `kernel/asus/T00F/drivers/external_drivers/camera/drivers/media/pci/atomisp2/atomisp_driver_css15/atomisp_v4l2.c`, `kernel/asus/T00F/drivers/external_drivers/camera/drivers/media/pci/atomisp2/css2300_build/Makefile`
- **Git Hash:** `device/asus/T00F/f1e4f96ed04b76ea7e830d8452d15a893c331c4d`, `kernel/asus/T00F/e1b888f467c7c008ac73fdfb55ae60da4c569e9f`

## Wrong-architecture libstlport blob

- **Issue:** An ARM `libstlport.so` conflicted with the x86 userspace build.
- **Root Cause:** The proprietary-file list and vendor tree retained the ARM copy instead of the x86 library path.
- **Solution:** Correct the x86 proprietary path and remove the ARM vendor blob.
- **Affected Files:** `device/asus/T00F/proprietary-files.txt`, `vendor/asus/T00F/T00F/proprietary/vendor/lib/libstlport.so`
- **Git Hash:** `device/asus/T00F/19a32205df9faa58ae90355879c63c407e96c88b`, `vendor/asus/T00F/2e6e94021c1be79532de37f6a167466a41791261`

## Legacy Intel audio HAL was incompatible with Oreo tinyalsa

- **Issue:** Audio controls or streams could fail and wedge when used with the proprietary Intel audio HAL.
- **Root Cause:** Oreo's Soong tinyalsa build changed the `pcm_config` layout, stopped using the T00F kernel UAPI headers, and no longer validated mixer values as the CM14 implementation did.
- **Solution:** Restore the legacy `pcm_config` ABI, compile against the generated T00F kernel headers, zero the unsupported silence field, and reject out-of-range mixer values.
- **Affected Files:** `external/tinyalsa/Android.bp`, `external/tinyalsa/include/tinyalsa/asoundlib.h`, `external/tinyalsa/mixer.c`, `external/tinyalsa/pcm.c`
- **Git Hash:** `external/tinyalsa/351a87d0ae86c8f18bc781d6cbe86b5a7daa1454`

## PVR hardware bitmap upload corruption

- **Issue:** Hardware bitmaps displayed corrupted pixels or stale data on the T00F PowerVR GPU.
- **Root Cause:** The legacy PVR driver corrupts EGLImage-backed GL uploads when the gralloc stride is wider than the logical bitmap width.
- **Solution:** On T00F, allocate CPU-writable hardware bitmap buffers, clear row padding, and copy the bitmap row by row; retain the GL upload as a fallback.
- **Affected Files:** `frameworks/base/libs/hwui/pipeline/skia/SkiaOpenGLPipeline.cpp`, `frameworks/base/libs/hwui/renderthread/OpenGLPipeline.cpp`
- **Git Hash:** `frameworks/base/e75a1c524a6a369133db8942a3456f6f8392094d`

## Unsupported hardware video codecs caused playback problems

- **Issue:** Google codec entries and the legacy Intel AVC decoder caused unsupported or unstable playback paths.
- **Root Cause:** The advertised codec capabilities did not match the functional CloverTrail video implementation under Oreo.
- **Solution:** Remove unsupported Google codec declarations and prefer the software AVC decoder for regular playback.
- **Affected Files:** `device/asus/T00F/media/media_codecs.xml`
- **Git Hash:** `device/asus/T00F/3f3cd90c196240c9b156440aed96b144bcd38dc0`, `device/asus/T00F/312918a345ca2c9dea87bbe179f918a723c6dedd`

## Software AVC decoder selected unsupported CPU instructions

- **Issue:** Software H.264 decoding could terminate with an illegal-instruction fault on T00F.
- **Root Cause:** The generic x86 libavc configuration selected SSE4.2 routines, but the Atom Z2580 supports SSSE3 and not SSE4.x.
- **Solution:** Build and select the SSSE3 decoder routines and remove the SSE4.2 source and function selection from the CloverTrail path.
- **Affected Files:** `external/libavc/Android.bp`, `external/libavc/decoder/x86/ih264d_function_selector.c`
- **Git Hash:** `external/libavc/14fc6580a9f34a8655422bde135e817df3c1899c`

## Camera application disappeared from the launcher

- **Issue:** Snap could disappear from the launcher even though the camera stack was present.
- **Root Cause:** `DisableCameraReceiver` could disable the launcher alias during early boot before the legacy camera HAL finished enumerating cameras.
- **Solution:** Put the launcher intent filter directly on `CameraActivity` and remove the fragile alias and disable receiver.
- **Affected Files:** `packages/apps/Snap/AndroidManifest.xml`
- **Git Hash:** `packages/apps/Snap/70d1471d11d692c93fd93025da063c04ab25e803`

## Legacy suspend did not complete the display resume path

- **Issue:** The display could remain powered down or black after waking the device.
- **Root Cause:** The CloverTrail early-suspend implementation requires writing `on` to `/sys/power/state` to invoke late-resume callbacks, but Oreo's libsuspend backend no longer performed that operation.
- **Solution:** Detect the legacy early-suspend interface and write `on` when autosuspend is disabled.
- **Affected Files:** `system/core/libsuspend/autosuspend_wakeup_count.c`
- **Git Hash:** `system/core/0e87280526b3d62c6af4e0f8b928562156e8d00d`

## Obsolete CA certificate store broke modern TLS connections

- **Issue:** Applications could reject otherwise valid modern HTTPS certificate chains.
- **Root Cause:** The inherited CA store was obsolete and lacked current roots and removals.
- **Solution:** Synchronize the trusted CA certificate set with the current AOSP source.
- **Affected Files:** `system/ca-certificates/files/*.0`
- **Git Hash:** `system/ca-certificates/7728de48ecf02265784e51ec8a04471f0b25e29d`

## System partition had insufficient room for Open GApps

- **Issue:** Open GApps installation could exceed the available system partition space.
- **Root Cause:** Dexpreopt artifacts and optional LineageOS/Google TTS packages consumed space needed by GApps.
- **Solution:** Disable dexpreopt, omit optional applications, and configure the GApps installer to exclude GoogleTTS.
- **Affected Files:** `device/asus/T00F/BoardConfig.mk`, `device/asus/T00F/lineage.mk`, `device/asus/T00F/releasetools/releasetools.py`, `build/make/target/product/core.mk`, `build/make/target/product/generic_no_telephony.mk`, `vendor/lineage/config/common.mk`
- **Git Hash:** `device/asus/T00F/fcdc7f0b6b6a7262eef4db0d6adf3cbfd7066d7c`, `device/asus/T00F/598f7f7739721388504d5a65cf55b7f71bd079b2`, `device/asus/T00F/730bfaff5e46c3192e129e94729be35afff40728`, `build/make/26129063b15985a1d9c896b2a219fff9bf4d1a8c`, `vendor/lineage/507f31021742e08a734e94b2db249c58c45b3207`

## KVM was unavailable to Termux/QEMU

- **Issue:** Termux/QEMU could not use hardware virtualization.
- **Root Cause:** KVM support was disabled in the kernel and the device node lacked the required userspace permissions.
- **Solution:** Enable x86 KVM in the kernel and expose `/dev/kvm` through ueventd.
- **Affected Files:** `kernel/asus/T00F/arch/x86/configs/lineage_T00F_defconfig`, `device/asus/T00F/rootdir/etc/ueventd.redhookbay.rc`
- **Git Hash:** `kernel/asus/T00F/8b2158b9cbda5c781476b37256d84299d8069424`, `device/asus/T00F/9b5525eae51d92265332f31bd03eb0f4add1689f`

## Product name did not identify the ASUS device

- **Issue:** The built product name was generic or inconsistent with the device identity.
- **Root Cause:** The product makefile retained the inherited porting name.
- **Solution:** Set the product name to ASUS Zenfone.
- **Affected Files:** `device/asus/T00F/lineage.mk`
- **Git Hash:** `device/asus/T00F/22f07b9e775d379196d82d05b1f35f6618cbbbe3`

## Modern ARM applications can exceed Houdini v2 symbol resolution

- **Issue:** Some ARMv7 applications, including Microsoft Remote Desktop (`com.microsoft.rdc.androidx`), fail while loading native libraries with unresolved symbols such as `__sF`.
- **Root Cause:** T00F uses Intel Houdini 6.1.2 for ARMv7-to-x86 32-bit translation.  It implements Native Bridge v2.  The bundled guest libc can be loaded, but Houdini v2 still fails to resolve some modern application's versioned/data symbols; framework linker namespaces cannot correct this because they require Native Bridge v3.
- **Solution:** There is currently no complete software-side fix.  A compatible ARMv7-to-x86 32-bit Native Bridge v3 Houdini (`houdini8_x`) would be required to provide namespace-aware loading, but Intel did not provide that Oreo x86 variant.  Do not paper over individual unresolved symbols: it can make other translated applications fail at a later dependency.
- **Affected Files:** `system/core/libnativebridge/native_bridge.cc`, `system/core/libnativeloader/native_loader.cpp`, `bionic/libdl/libdl.c`
- **Status:** Unresolved; retain only the verified Native Bridge v2 compatibility and Houdini host-load fixes.

## Software HEVC decoder crashed during codec discovery

- **Issue:** Enabling the Google low-end software codec list made `media.codec` crash during boot, which in turn caused SystemUI/Desktop to restart repeatedly.
- **Root Cause:** The x86 libhevc configuration compiled and selected SSE4.2 routines.  The Atom Z2580 (CloverTrail) supports SSSE3 but not SSE4.x, and MediaCodec probes the advertised HEVC decoder while constructing its codec list.
- **Solution:** Restore `media_codecs_google_video_le.xml` through the device codec configuration, then build libhevc with an SSSE3 baseline, omit SSE4.2 sources, and fall back to SSSE3 if an explicit SSE4.2 or AVX2 architecture is requested.
- **Affected Files:** `device/asus/T00F/media/media_codecs.xml`, `external/libhevc/Android.bp`, `external/libhevc/decoder/x86/ihevcd_function_selector.c`
- **Verification:** Rebuilt `libstagefright_soft_hevcdec.so` has no SSE4.2 selector symbols and selects `ARCH_X86_SSSE3`; after installing that library, the device boots with the Google VP8/VP9/HEVC/H.263 codec declarations enabled.
- **Git Hash:** `device/asus/T00F/0a46f17d529263eb39e866d36ee9e7f1af32f5ee`, `external/libhevc/2ada391f9896f688c439e5982689f12afa5d4ade`
