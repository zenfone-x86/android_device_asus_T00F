"""T00F-specific additions to generated recovery OTAs."""

import common


_GAPPS_CONFIG_ARCHIVE_PATH = "install/gapps-config.txt"
_GAPPS_CONFIG_TEMP_PATH = "/tmp/gapps-config.txt"
_GAPPS_CONFIG_DATA_PATH = "/data/media/0/gapps-config.txt"


def FullOTA_InstallEnd(info):
  """Create the Open GApps exclusion config before a queued GApps flash.

  Open GApps discovers gapps-config.txt through /sdcard, which maps to
  /data/media/0 on this legacy data-media device.  Do not overwrite a config
  the user has already supplied.
  """
  common.ZipWriteStr(info.output_zip, _GAPPS_CONFIG_ARCHIVE_PATH,
                     "# T00F default Open GApps exclusions\nGoogleTTS\n")

  info.script.Print("Creating default Open GApps configuration")
  info.script.Mount("/data")
  info.script.AppendExtra(
      'package_extract_file("%s", "%s");' %
      (_GAPPS_CONFIG_ARCHIVE_PATH, _GAPPS_CONFIG_TEMP_PATH))
  info.script.AppendExtra(
      'run_program("/sbin/sh", "-c", '
      '"mkdir -p /data/media/0; '
      'if [ ! -e %s ]; then '
      'cp %s %s; '
      'chown 1023:1023 %s; '
      'chmod 0660 %s; '
      'fi");' % (
          _GAPPS_CONFIG_DATA_PATH,
          _GAPPS_CONFIG_TEMP_PATH,
          _GAPPS_CONFIG_DATA_PATH,
          _GAPPS_CONFIG_DATA_PATH,
          _GAPPS_CONFIG_DATA_PATH))
  info.script.AppendExtra('delete("%s");' % _GAPPS_CONFIG_TEMP_PATH)
