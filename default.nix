{
  stdenv,
  cmake,
  lib,
}: let
  fs = lib.fileset;
in
  stdenv.mkDerivation {
    pname = "ost-toolkit";
    version = "1.4";

    src = fs.toSource {
      root = ./.;
      fileset = fs.unions [
        ./CMakeLists.txt
        ./include
        ./src
        ./tests
        ./Builder.cpp
        ./Compiler.cpp
        ./Debugger.cpp
        ./Tests.cpp
        ./Tu4Run.cpp
      ];
    };

    nativeBuildInputs = [
      cmake
    ];
  }
