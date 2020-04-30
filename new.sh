#!/bin/bash
export PYTHONDONTWRITEBYTECODE=1

THEME=${THEME:-wasm-none}

PREFIX=$(realpath $(dirname "$0"))
AOSP=false


if echo $PYDK|grep -q pydk
then
    echo " * Using HOST python from PyDK build"
    HOST=$(echo -n ${PYDK}/host)
    echo HOST=$HOST

    PYTHON=$(echo -n ${HOST}/bin/python3.?)
    PIP=$(echo -n ${HOST}/bin/pip3.?)
    export LD_LIBRARY_PATH="${HOST}/lib64:${HOST}/lib:$LD_LIBRARY_PATH"
    export PIPU=""

else
    for py in 9 8 7
    do
        if command -v python3.${py}
        then
            export PYTHON=$(command -v python3.${py})
            export PIP=$(command -v pip3.${py})
            break
        fi
    done

    echo " * Using non PyDK-sdk cPython3.x $PYTHON and $PIP"
    export PIPU="--user"
    export PATH=$HOME/.local/bin:$PATH
fi


export PATH=$(dirname $PYTHON):$PATH
echo PATH=$(dirname $PYTHON)
echo PYTHON=$PYTHON
echo PIP=$PIP

if echo "$@"|grep -q dev
then
    echo " skipping pip update"
    shift 1
else
    $PIP install $PIPU --upgrade pip cookiecutter remi

    if $AOSP
    then
        #aosp
        $PIP install --user briefcase
    fi
fi

export COOKIECUTTER_CONFIG=cookiecutter.config
mkdir templates replay

cat > $COOKIECUTTER_CONFIG <<END
cookiecutters_dir: templates
replay_dir: replay
END

# todo add the panda3d mimetypes to no render list

cat > themes/$THEME/cookiecutter.json <<END
{
  "module_name": "cpython",
  "bundle": "org.beerware",
  "app_name": "CPython",
  "usage": "Run CPython on the web",
  "formal_name": "org.beerware.cpython",
  "_copy_without_render": [
    "gradlew",
    "gradle.bat",
    "gradle/wrapper/gradle-wrapper.properties",
    "gradle/wrapper/gradle-wrapper.jar",
    ".gitignore",
    "*.wasm", "*.data",
    "*.png", "*.jpg", "*.bam"
  ]
}
END

echo "

      Seletected THEME [$THEME]

"

if $AOSP
then
    #aosp
    cookiecutter https://github.com/pmp-p/briefcase-android-gradle-template --checkout 3.8p
else
    cookiecutter $(pwd)/themes/$THEME --overwrite-if-exists
fi


