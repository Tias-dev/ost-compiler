#!/usr/bin/env bash

if [[ $1 == "main" ]]; then
  ./build/main
fi

if [[ $1 == "tests" ]]; then
  ./build/tests
fi
