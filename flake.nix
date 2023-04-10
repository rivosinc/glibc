# SPDX-FileCopyrightText: Copyright (c) 2022 by Rivos Inc.
# SPDX-FileCopyrightText: Copyright (c) 2003-2022 Eelco Dolstra and the Nixpkgs/NixOS contributors
# Licensed under the MIT License, see LICENSE for details.
# SPDX-License-Identifier: MIT
{
  description = "glibc (rivos)";

  inputs = {
    nixpkgs.url = "github:rivosinc/nixpkgs/rivos/nixos-22.11?allRefs=1";
  };

  outputs = {
    self,
    nixpkgs,
    ...
  }: let
    lib = nixpkgs.lib;
    # Generate a user-friendly version number.
    versionHeader = nixpkgs.lib.fileContents ./version.h;
    # Match the version from "#define VERSION "2.36""
    glibcVersion = builtins.head (builtins.match ".*\n#define VERSION \"([0-9.]+)\"" versionHeader);
    version = "${glibcVersion}-g${self.shortRev or "dirty"}";

    # System types to support.
    supportedSystems = [
      "x86_64-linux"
      "aarch64-linux"
      "riscv64-linux"
    ];

    # Helper function to generate an attrset '{ x86_64-linux = f "x86_64-linux"; ... }'.
    forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

    # Nixpkgs instantiated for supported system types.
    nixpkgsFor = forAllSystems (system:
      import nixpkgs {
        inherit system;
        overlays = [self.overlays.default];
      });
  in rec {
    overlays = rec {
      default = final: prev: let
        inherit (final) callPackage;
      in rec {
        glibcRivos = callPackage ./rivos/nix {
          stdenv = final.gccStdenv;
          src = self;
          inherit version;
        };
        # Being redundant to avoid cycles on boot. TODO: find a better way
        glibcCrossRivos = callPackage ./rivos/nix {
          stdenv = final.gccCrossLibcStdenv;
          src = self;
          inherit version;
        };
        # Only supported on Linux and only on glibc
        glibcLocalesRivos =
          if final.stdenv.hostPlatform.isLinux && final.stdenv.hostPlatform.isGnu
          then
            callPackage ./rivos/nix/locales.nix {
              src = self;
              inherit version;
            }
          else null;
        glibcLocalesUtf8Rivos =
          if final.stdenv.hostPlatform.isLinux && final.stdenv.hostPlatform.isGnu
          then
            callPackage ./rivos/nix/locales.nix {
              allLocales = false;
              src = self;
              inherit version;
            }
          else null;
      };

      cross = final: prev: let
        flakePkgs = default final prev;
      in {
        glibc =
          if final.stdenv.targetPlatform != final.stdenv.buildPlatform
          then flakePkgs.glibcRivos
          else prev.glibc;
        glibcCross =
          if final.stdenv.targetPlatform != final.stdenv.buildPlatform
          then flakePkgs.glibcCrossRivos
          else prev.glibcCross;
        glibcLocales =
          if final.stdenv.targetPlatform != final.stdenv.buildPlatform
          then flakePkgs.glibcLocalesRivos
          else prev.glibcLocales;
        glibcLocalesUtf8 =
          if final.stdenv.targetPlatform != final.stdenv.buildPlatform
          then flakePkgs.glibcLocalesUtf8Rivos
          else prev.glibcLocalesUtf8;
      };

      use-as-default-glibc = final: prev: {
        glibc = final.glibcRivos;
        glibcCross = final.glibcCrossRivos;
        glibcLocales = final.glibcLocalesRivos;
        glibcLocalesUtf8 = final.glibcLocalesUtf8Rivos;
      };
    };

    # Provide some binary packages for selected system types.
    packages = forAllSystems (system: let
      glibc = (nixpkgsFor.${system}).glibcRivos;
      glibcLocales = (nixpkgsFor.${system}).glibcLocalesRivos;
      glibcLocalesUtf8 = (nixpkgsFor.${system}).glibcLocalesUtf8Rivos;
    in {
      inherit glibc glibcLocales glibcLocalesUtf8;
      default = glibc;
    });
    checks = forAllSystems (system: {
      inherit (packages.${system}) glibc;
    });

    formatter = forAllSystems (system: nixpkgsFor.${system}.alejandra);
  };
}
