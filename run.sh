#!/bin/sh

PREFIX=$(realpath $(dirname "$0"))

for py in 9 8 7
do
    if command -v python3.${py}
    then
        export PYTHON=$(command -v python3.${py})
        export PIP=$(command -v pip3.${py})
        break
    fi
done

if echo $@|grep -q dev
then
    echo no pip
    shift 1
else
    $PIP install --user -r $PYDK/sources.py/pythons/requirements.txt
fi

PYTHONPATH=$PYDK/sources.py/pythons $PYTHON -u -B -i -m pythons.js "$@"

