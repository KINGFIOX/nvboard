{
  description = "NVBoard - NJU Virtual Board (SDL-based FPGA board simulation for Verilator)";

  inputs.nixpkgs.url = "nixpkgs";

  outputs = { self, nixpkgs }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
      pkgsFor = system: import nixpkgs { inherit system; };
    in
    {
      packages = forAllSystems (system:
        let
          pkgs = pkgsFor system;
          # Exclude build and .git from source so Nix build is reproducible
          src = pkgs.lib.cleanSourceWith {
            src = pkgs.lib.cleanSource self;
            filter = path: type:
              type != "directory" || (builtins.baseNameOf path != "build" && builtins.baseNameOf path != ".git");
          };
        in
        {
          default = pkgs.stdenv.mkDerivation {
            pname = "nvboard";
            version = "0.1.0";
            inherit src;

            nativeBuildInputs = with pkgs; [ meson ninja pkg-config ];
            buildInputs = with pkgs; [ SDL2 SDL2_image SDL2_ttf ];

            dontUseMesonConfigure = true;
            mesonBuildType = "release";

            configurePhase = ''
              runHook preConfigure
              meson setup build --prefix=$out --buildtype=release
              runHook postConfigure
            '';

            buildPhase = ''
              runHook preBuild
              ninja -C build
              runHook postBuild
            '';

            installPhase = ''
              runHook preInstall
              mkdir -p $out/lib $out/include $out/usr/include
              cp build/libnvboard.a $out/lib/
              cp -r include/* $out/include/
              cp -r usr/include/* $out/usr/include/
              runHook postInstall
            '';

            passthru = {
              inherit (pkgs) SDL2 SDL2_image SDL2_ttf;
            };
          };

          nvboard = self.packages.${system}.default;
        });

      devShells = forAllSystems (system:
        let
          pkgs = pkgsFor system;
        in
        {
          default = pkgs.mkShell {
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
        });
    };
}
