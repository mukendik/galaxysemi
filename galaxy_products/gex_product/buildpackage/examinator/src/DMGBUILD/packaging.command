#!/bin/bash

dir=${0%/*}
if [ -d "$dir" ]; then
  cd "$dir"
fi

#app name, version number, and background image file name
APP_NAME="$1"
DMG_BACKGROUND_IMG="bg.png"

APP_EXE="${APP_NAME}.app/Contents/MacOS/${APP_NAME}"
VERSION="$2"
VOL_NAME="${APP_NAME}_$2"  
DMG_TMP="${VOL_NAME}-temp.dmg"
DMG_FINAL="$3"         
STAGING_DIR="./Install"            


echo "clear out any old DMG data"
rm -rf "${DMG_TMP}" "${DMG_FINAL}"

pushd "${STAGING_DIR}"
popd

#echo "figure out how big our DMG needs to be assumes our contents are at least 1M!"
#SIZE=`du -sh "${STAGING_DIR}" | sed 's/\([0-9\.]*\)M\(.*\)/\1/'` 
#SIZE=`echo "${SIZE} + 1.0" | bc | awk '{print int($1+0.5)}'`
SIZE=2000

#if [ $? -ne 0 ]; then
#   echo "Error: Cannot compute size of staging dir"
#   exit
#fi

echo "create the temp DMG file"
hdiutil create -srcfolder "${STAGING_DIR}" -volname "${VOL_NAME}" -fs HFS+ \
      -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${SIZE}M "${DMG_TMP}"

echo "Created DMG: ${DMG_TMP}"
echo "mount DMG and save the device"
DEVICE=$(hdiutil attach -readwrite -noverify "${DMG_TMP}" | \
         egrep '^/dev/' | sed 1q | awk '{print $1}')

sleep 2

echo "Add link to /Applications"
pushd /Volumes/"${VOL_NAME}"
ln -s /Applications
popd

echo "add a background image to installer"
mkdir /Volumes/"${VOL_NAME}"/.background
cp "${DMG_BACKGROUND_IMG}" /Volumes/"${VOL_NAME}"/.background/

echo "tell the Finder to resize the window, set the background, change the icon size, place the icons in the right position, etc."
echo '
   tell application "Finder"
     tell disk "'${VOL_NAME}'"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {400, 100, 920, 440}
           set viewOptions to the icon view options of container window
           set arrangement of viewOptions to not arranged
           set icon size of viewOptions to 72
           set background picture of viewOptions to file ".background:'${DMG_BACKGROUND_IMG}'"
           set position of item "'${APP_NAME}'.app" of container window to {160, 205}
           set position of item "Applications" of container window to {360, 205}
           close
           open
           update without registering applications
           delay 2
     end tell
   end tell
' | osascript

sync

echo "unmount tmp dmg"
hdiutil detach "${DEVICE}"

echo "Creating compressed image disk ${DMG_FINAL}"
hdiutil convert "${DMG_TMP}" -format UDZO -imagekey zlib-level=9 -o "${DMG_FINAL}"

echo "clean up ${DMG_TMP}"
rm -rf "${DMG_TMP}"

echo "DMG Done successfully."

exit