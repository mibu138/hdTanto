#!/usr/bin/env bash
export PYTHONPATH=$PYTHONPATH:$HOME/dev/USD/build/lib/python
export PXR_PLUGINPATH_NAME=$PWD/hdTanto/resources/plugInfo.json:$PXR_PLUGINPATH_NAME
export PATH=$PATH:$HOME/dev/USD/build/bin
export LD_LIBRARY_PATH=$PWD/tantoren:$HOME/lib:$PWD:$LD_LIBRARY_PATH
