#!/bin/sh
export PYDK=${PYDK:-/data/cross/pydk}

APK=${1:-org.beerware.wapy}
PYDK_ABI=${PYDK_ABI:-aosp-38}

export ANDROID_HOME=${ANDROID_HOME:-$PYDK/android-sdk}

# probably would not work from elsewhere
export NDK_HOME=${NDK_HOME:-${ANDROID_HOME}/ndk-bundle}


# will we use a test device for generated apk

export DEVICE=${DEVICE:-true}


ADB=$ANDROID_HOME/platform-tools/adb

if echo "dev$DEVICE" |grep -q "false"
then
    ADB=true
else
    if $ADB devices -l|grep 'product:'
    then
        echo "at least one device found, will use adb"
        echo "#TODO: handle multi devices with -s"
        #ADB="$ADB -s $DEVICE"
    else
        echo "no device '$DEVICE' found won't deploy with adb"
        ADB=true
    fi
fi

export ADB


# check for wasm3 even if not used atm

if [ -f wasm3/LICENSE ]
then
    echo found wasm3
else
    git clone https://github.com/wasm3/wasm3.git
fi

if [ -f "$ADB" ]
then
    echo found android-sdk

    if [ -f ${NDK_HOME}/source.properties ]
    then
        echo Ndk found
        export AOSP=true
    else
        echo no ndk found maybe trigger its install with the "hello jni sample"
    fi
else
    echo please set ANDROID_HOME or put android-sdk there: $PYDK/android-sdk
    export AOSP=false
fi


# select a python+pip combo ( cmake, cookiecutter and f-strings support depend on that )

WD=$(pwd)

if [ -f $PYDK/aosp/bin/activate ]
then
    echo " * Using HOST python from PyDK build"

    . $PYDK/aosp/bin/activate

    HOST=$(echo -n ${PYDK}/aosp)
    echo HOST=$HOST

    PYTHON=$(echo -n ${HOST}/bin/python3.?)
    export PIP="$PYTHON -u -B -m pip"
    export LD_LIBRARY_PATH="${HOST}/lib64:${HOST}/lib:$LD_LIBRARY_PATH"
    export PIPU=""
else
    echo " * Using non PyDK-sdk cPython3"
    export PYTHON=$(command -v python3)
    export PIP="$PYTHON -u -B -m pip"
    export PIPU="--user"
    export PATH=~/.local/bin:$PATH

fi

export PYSET=true



# last chance to get off the train

echo "
    Target : '$APK'
    PYDK : $PYDK
    PYDK_ABI : $PYDK_ABI

ANDROID_HOME=$ANDROID_HOME
NDK_HOME=$NDK_HOME

$ADB

press <enter> to continue
"
read






cd "${WD}"

$PIP install $PIPU --upgrade pip
$PIP install $PIPU future-fstrings[rewrite]

if $AOSP
then
    $PIP install $PIPU scikit-build cmake==3.10.3
fi


APK_FILE=$(find $APK/|grep "\.apk$")


$ADB uninstall $APK



cd /data/cross/pydk-applications && rm -rf ${APK}

# auto answer [enter]
TEMPLATE=${TEMPLATE:-$(pwd)/wapy-android} ./template.sh ${APK} <<END




END


mkdir -p ${APK}/prebuilt/

export MERGE="cp -aRfuxp"

#cp -aRxurfp ${PYDK}/prebuilt/$PYDK_ABI/arm* ${PYDK}/prebuilt/$PYDK_ABI/x86* ${APK}/prebuilt/

for src in $PYDK_ABI ${APK}
do
    $MERGE ${PYDK}/sources.py/common.${src}/* ${APK}/assets/
    $MERGE ${PYDK}/sources.py/patches.${src}/* ${APK}/assets/
done

$MERGE ${PYDK}/projects/pydk-all/* ${APK}/assets/
$MERGE src/${APK}/* ${APK}/assets/


function install_run
{
    APK_FILE=$1
    if [ -f $APK_FILE ]
    then
        # gradle done it
        #$ADB install $APK_FILE

        echo "  * running $APK_FILE"

        aapt=$(find $ANDROID_HOME/build-tools/|grep aapt$|sort|tail -n1)
        pkg=$($aapt dump badging "$APK_FILE"|awk -F" " '/package/ {print $2}'|awk -F"'" '/name=/ {print $2}')
        act=$($aapt dump badging "$APK_FILE"|awk -F" " '/launchable-activity/ {print $2}'|awk -F"'" '/name=/ {print $2}')
        echo "Running $pkg/$act"
        $ADB shell am start -n "$pkg/$act"

        echo "press <enter> to kill app"
        read
        echo "$ADB shell am force-stop $APK"
        $ADB shell am force-stop $APK
    fi
}

if cd ${APK}
then
    if ../make.sh installDebug
    then
        APK_FILE=$(find .|grep "\.apk$")
        install_run $APK_FILE
    else
        echo 'android/sdk/ndk/gradle failure'
    fi
else
    echo 'pydk-applications failure'
fi




