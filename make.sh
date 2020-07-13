export ANDROID_SDK_ROOT=$PYDK/android-sdk
echo $ANDROID_SDK_ROOT/platform-tools/adb
./gradlew --warning-mode all "$@"
