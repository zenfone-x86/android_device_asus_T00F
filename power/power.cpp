/*
 * Copyright (c) 2015 The CyanogenMod Project
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

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <utils/SystemClock.h>
#include <hardware/power.h>
#include <hardware/hardware.h>

#define LOG_TAG "PowerHAL"
#include <utils/Log.h>

#define TOTAL_CPUS 4
#define POWERSAVE_MIN_FREQ 800000
#define POWERSAVE_MAX_FREQ 933000
#define BIAS_PERF_MIN_FREQ 1333000
#define NORMAL_MAX_FREQ 1600000
#define SCREEN_OFF_TARGET_MAX_FREQ 933000

static pthread_mutex_t profile_lock = PTHREAD_MUTEX_INITIALIZER;

enum {
    PROFILE_POWER_SAVE = 0,
    PROFILE_BALANCED,
    PROFILE_HIGH_PERFORMANCE,
    PROFILE_BIAS_POWER,
    PROFILE_BIAS_PERFORMANCE,
    PROFILE_MAX
};

static int current_power_profile = PROFILE_BALANCED;
static bool interactive = true;

static char *cpu_path_min[] = {
    (char *)"/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq",
    (char *)"/sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq",
    (char *)"/sys/devices/system/cpu/cpu2/cpufreq/scaling_min_freq",
    (char *)"/sys/devices/system/cpu/cpu3/cpufreq/scaling_min_freq",
};

static char *cpu_path_max[] = {
    (char *)"/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq",
    (char *)"/sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq",
    (char *)"/sys/devices/system/cpu/cpu2/cpufreq/scaling_max_freq",
    (char *)"/sys/devices/system/cpu/cpu3/cpufreq/scaling_max_freq",
};

#define CPU0_AVAILABLE_FREQS \
    "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies"

struct local_power_module {
    struct power_module base;
};

#define BUF_SIZE 80

static int sysfs_read(const char *path, char buf[BUF_SIZE]) {
    int len;
    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return fd;
    }

    len = read(fd, buf, BUF_SIZE-1);
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error read from %s: %s\n", path, buf);
    } else {
        buf[len] = '\0';
    }

    close(fd);
    return len;
}

static void sysfs_write(const char *path, const char *const s) {
    char buf[BUF_SIZE];
    int len;
    int fd = open(path, O_WRONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);
    }

    close(fd);
}

static void sysfs_write_int(char *path, int value)
{
    char buf[80];
    snprintf(buf, 80, "%d", value);
    return sysfs_write(path, buf);
}

static void set_min_freq(int freq) {
    for (int cpu = 0; cpu < TOTAL_CPUS; cpu++)
	sysfs_write_int(cpu_path_min[cpu], freq);
}

static void set_max_freq(int freq) {
    for (int cpu = 0; cpu < TOTAL_CPUS; cpu++)
        sysfs_write_int(cpu_path_max[cpu], freq);
}

/*
 * The available OPPs are supplied by SFI and differ between the Z2560 and
 * Z2580.  Do not hard-code a frequency that may not exist on one variant.
 */
static bool get_screen_off_limits(int *min_freq, int *max_freq)
{
    char buf[BUF_SIZE];
    char *save_ptr = NULL;
    char *token;
    int lowest_freq = INT_MAX;
    int capped_freq = 0;

    if (sysfs_read(CPU0_AVAILABLE_FREQS, buf) < 0)
        return false;

    for (token = strtok_r(buf, " \n", &save_ptr); token != NULL;
            token = strtok_r(NULL, " \n", &save_ptr)) {
        char *end_ptr = NULL;
        long freq = strtol(token, &end_ptr, 10);

        if (*token == '\0' || *end_ptr != '\0' || freq <= 0 || freq > INT_MAX)
            continue;

        if (freq < lowest_freq)
            lowest_freq = freq;
        if (freq <= SCREEN_OFF_TARGET_MAX_FREQ && freq > capped_freq)
            capped_freq = freq;
    }

    if (lowest_freq == INT_MAX)
        return false;

    *min_freq = lowest_freq;
    *max_freq = capped_freq ? capped_freq : lowest_freq;
    return true;
}

static void get_profile_limits(int profile, int *min_freq, int *max_freq)
{
    *min_freq = POWERSAVE_MIN_FREQ;
    *max_freq = NORMAL_MAX_FREQ;

    switch (profile) {
    case PROFILE_HIGH_PERFORMANCE:
        *min_freq = NORMAL_MAX_FREQ;
        *max_freq = NORMAL_MAX_FREQ;
        break;
    case PROFILE_BIAS_PERFORMANCE:
        *min_freq = BIAS_PERF_MIN_FREQ;
        *max_freq = NORMAL_MAX_FREQ;
        break;
    case PROFILE_BIAS_POWER:
    case PROFILE_POWER_SAVE:
        *max_freq = POWERSAVE_MAX_FREQ;
        break;
    default:
        break;
    }
}

static void apply_power_profile(void)
{
    int min_freq;
    int max_freq;

    get_profile_limits(current_power_profile, &min_freq, &max_freq);

    if (!interactive && !get_screen_off_limits(&min_freq, &max_freq))
        ALOGW("Unable to determine available CPU frequencies; leaving profile limits unchanged");

    if (min_freq > max_freq)
        min_freq = max_freq;

    /* Lower the minimum before capping; raise the cap before restoring it. */
    if (interactive) {
        set_max_freq(max_freq);
        set_min_freq(min_freq);
    } else {
        set_min_freq(min_freq);
        set_max_freq(max_freq);
    }

    ALOGD("%s: profile=%d interactive=%d min=%d max=%d", __func__,
            current_power_profile, interactive, min_freq, max_freq);
}

static void power_set_interactive(struct power_module *, int on)
{
    pthread_mutex_lock(&profile_lock);
    interactive = on != 0;
    apply_power_profile();
    pthread_mutex_unlock(&profile_lock);
}

static void power_init(struct power_module *) {

}

static void set_power_profile(int profile) {
    if (profile < PROFILE_POWER_SAVE || profile >= PROFILE_MAX) {
        ALOGE("%s: invalid profile %d", __func__, profile);
        return;
    }

    current_power_profile = profile;
    apply_power_profile();
}

static void power_hint( __attribute__((unused)) struct power_module *module,
                      power_hint_t hint, void *data)
{
    if (hint == POWER_HINT_SET_PROFILE) {
        /*
         * The HIDL 1.0 compatibility wrapper represents an integer hint value
         * of zero as NULL.  PROFILE_POWER_SAVE is zero, so NULL is valid here.
         */
        int profile = data ? *(int32_t *)data : PROFILE_POWER_SAVE;

        pthread_mutex_lock(&profile_lock);
        set_power_profile(profile);
        pthread_mutex_unlock(&profile_lock);
        return;
    }

    // Skip other hints in powersave mode
    if (current_power_profile == PROFILE_POWER_SAVE)
        return;

    // unimplemented for now
    switch (hint) {
    case POWER_HINT_INTERACTION:
        break;

    case POWER_HINT_VSYNC:
        break;

    default:
        break;
    }
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

void set_feature(struct power_module *module __unused, feature_t feature, int state)
{
    char tmp_str[BUF_SIZE];
#ifdef TAP_TO_WAKE_NODE
    if (feature == POWER_FEATURE_DOUBLE_TAP_TO_WAKE) {
        snprintf(tmp_str, BUF_SIZE, "%d", state);
        sysfs_write(TAP_TO_WAKE_NODE, tmp_str);
    }
#endif
}

static int get_feature(__attribute__((unused)) struct power_module *module,
                       feature_t feature)
{
    if (feature == POWER_FEATURE_SUPPORTED_PROFILES)
        return PROFILE_MAX;

    return -1;
}

struct local_power_module HAL_MODULE_INFO_SYM = {
    .base = {
       .common = {
            .tag = HARDWARE_MODULE_TAG,
            .module_api_version = POWER_MODULE_API_VERSION_0_3,
            .hal_api_version = HARDWARE_HAL_API_VERSION,
            .id = POWER_HARDWARE_MODULE_ID,
            .name = "Mofd_v1 Power HAL",
            .author = "The CyanogenMod Project",
            .methods = &power_module_methods,
            .dso = 0,
            .reserved = {0},
        },
        .init = power_init,
        .setInteractive = power_set_interactive,
        .powerHint = power_hint,
        .setFeature = set_feature,
        .getFeature = get_feature,
    },
};
