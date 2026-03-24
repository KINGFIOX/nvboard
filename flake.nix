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
        nvboardPkg = pkgs.stdenv.mkDerivation {
          pname = "nvboard";
          version = "0.1.0";
          src = self;

          nativeBuildInputs = with pkgs; [ meson ninja pkg-config ];
          buildInputs = with pkgs; [ SDL2 SDL2_image SDL2_ttf ];

          mesonBuildType = "release";

          passthru = {
            inherit (pkgs) SDL2 SDL2_image SDL2_ttf;
          };
        };
      in
      {
        packages = {
          default = nvboardPkg;
          nvboard = nvboardPkg;
        };

        devShells.default = pkgs.mkShell {
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
