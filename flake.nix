{
  description = "NVBoard - NJU Virtual Board (SDL-based FPGA board simulation for Verilator)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        stdenv = pkgs.llvmPackages.libcxxStdenv;
        nvboardPkg = stdenv.mkDerivation {
          src = self;
          pname = "nvboard";
          version = "0.1.0";
          nativeBuildInputs = with pkgs; [ meson ninja pkg-config ];
          buildInputs = with pkgs; [ SDL2 SDL2_image SDL2_ttf ];
          mesonBuildType = "release";
        };
      in
      {
        packages = {
          default = nvboardPkg;
          nvboard = nvboardPkg;
        };

        devShells.default = pkgs.mkShell.override { inherit stdenv; } {
          buildInputs = with pkgs; [
            SDL2
            SDL2_image
            SDL2_ttf
            meson
            ninja
            verilator
            python3
            pkg-config
          ];

          shellHook = ''
            export NVBOARD_HOME="$(pwd)"
            echo "NVBOARD_HOME is set to: $NVBOARD_HOME"
            echo "Build nvboard: meson setup build && ninja -C build"
            echo "With upstream example (Makefile): set NVBOARD_HOME and run make -C example run"
          '';
        };
      }
    );
}
