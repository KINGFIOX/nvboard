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
          # If there is cmake+ninja in nativeBuildInputs,
          # Nix would get into compilation automatically.
          nativeBuildInputs = with pkgs; [ cmake ninja pkg-config ];
          buildInputs = with pkgs; [ SDL2 SDL2_image SDL2_ttf abseil-cpp ];
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
            abseil-cpp
            cmake
            ninja
            verilator
            python3
            pkg-config
          ];

          shellHook = ''
            export NVBOARD_HOME="$(pwd)"
            echo "NVBOARD_HOME is set to: $NVBOARD_HOME"
            echo "Build nvboard: cmake -B build -G Ninja && ninja -C build"
          '';
        };
      }
    );
}
