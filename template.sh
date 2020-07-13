#!/bin/sh
export PYTHONDONTWRITEBYTECODE=1
export PYSET=${PYSET:-false}

PREFIX=$(realpath $(dirname "$0"))

if [ -f $PYDK/aosp/bin/activate ]
then
    echo " * Using HOST python from PyDK build"
    HOST=$(echo -n ${PYDK}/aosp)
    echo HOST=$HOST

    PYTHON=$(echo -n ${HOST}/bin/python3.?)
    PIP=$(echo -n ${HOST}/bin/pip3.?)
    export LD_LIBRARY_PATH="${HOST}/lib64:${HOST}/lib:$LD_LIBRARY_PATH"
    export PIPU=""
else
    if $PYSET
    then
        echo
    else
        echo " * Using non PyDK-sdk cPython3"
        export PYTHON=$(command -v python3)
        export PIP=$(command -v pip3)
        export PIPU="--user"
        export PATH=~/.local/bin:$PATH
    fi
fi

export PATH=$(dirname $PYTHON):$PATH
echo PATH=$(dirname $PYTHON)
echo PYTHON=$PYTHON
echo PIP=$PIP


$PIP install $PIPU --upgrade pip
$PIP install $PIPU cookiecutter

export COOKIECUTTER_CONFIG=cookiecutter.config
mkdir -p templates replay

cat > $COOKIECUTTER_CONFIG <<END
cookiecutters_dir: templates
replay_dir: replay
END


cat > templates/cookiecutter.json <<END
{
  "module_name": "empty",
  "bundle": "org.beerware",
  "app_name": "EmptyApp",
  "formal_name": "org.beerware.empty",
  "_copy_without_render": [
    "gradlew",
    "gradle.bat",
    "gradle/wrapper/gradle-wrapper.properties",
    "gradle/wrapper/gradle-wrapper.jar",
    ".gitignore",
    "*.png"
  ]
}
END

#cookiecutter https://github.com/pmp-p/briefcase-android-gradle-template --checkout 3.8p
cookiecutter ${TEMPLATE:-/data/cross/pydk/briefcase-android-gradle-template}

