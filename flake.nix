{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    rust-overlay.url = "github:oxalica/rust-overlay";
  };
  outputs =
    {
      nixpkgs,
      flake-utils,
      rust-overlay,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        overlays = [ (import rust-overlay) ];
        pkgs = import nixpkgs { inherit system overlays; };

        wineWow = pkgs.wineWowPackages.stable;

        mkWinelibToolchain =
          bits: libs:
          pkgs.writeText "winelib${toString bits}.cmake" ''
            set(CMAKE_C_COMPILER   ${wineWow}/bin/winegcc)
            set(CMAKE_CXX_COMPILER ${wineWow}/bin/wineg++)
            set(CMAKE_RC_COMPILER  ${wineWow}/bin/wrc)
            set(CMAKE_C_FLAGS_INIT             "-m${toString bits}")
            set(CMAKE_CXX_FLAGS_INIT           "-m${toString bits}")
            set(CMAKE_EXE_LINKER_FLAGS_INIT    "-m${toString bits}")
            set(CMAKE_SHARED_LINKER_FLAGS_INIT "-m${toString bits}")
            set(CMAKE_PREFIX_PATH "${pkgs.lib.concatMapStringsSep ";" (p: "${pkgs.lib.getDev p};${pkgs.lib.getLib p}") libs}")
            set(ENV{PKG_CONFIG_PATH} "${pkgs.lib.makeSearchPathOutput "dev" "lib/pkgconfig" libs}")
            set(WINELIB ON)
            set(CMAKE_RC_OUTPUT_EXTENSION .res)
            set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")
          '';
        winelibLibs = p: with p; [ libpng zlib libogg libvorbis ];

        originalDependencies = with pkgs; [
          ninja
          cmake
          gcc_multi   # CHANGED from gcc: winegcc -m32 needs 32-bit glibc/libstdc++
          mold

          rust-bin.stable.latest.default
          SDL2
          freetype
          libpng
          libogg
          libvorbis

          wineWow     # CHANGED from wine + wine64

          vulkan-validation-layers

          pkg-config
        ];
        pkglist =
          with pkgs;
          [
            valgrind
            gdb
            clang-tools
          ]
          ++ originalDependencies;
          # Winelib build
          # The output is the bundle itself, meant to be zipped and run on other Linux
          # machines: nothing in it may point back into the Nix store. libpng, zlib and
          # vorbis are the in-tree copies, linked statically, so there are no libraries
          # to ship, and so is the C++ runtime; only glibc and wine come from the machine it runs on.
          out = let mkWinelibPkg = (bits: packages: pkgs.stdenv.mkDerivation {
            pname = "pxtone-collage";
            version = "0";
            src = ./.;

            nativeBuildInputs = with pkgs; [ cmake patchelf wineWow ];

            cmakeFlags = [
              "-DCMAKE_TOOLCHAIN_FILE=${mkWinelibToolchain bits (winelibLibs packages)}"
              "-DUSE_UNICODE=on"
              "-DBUNDLED_PNG=on"
              "-DBUNDLED_VORBIS=on"
              "-DBUNDLE_BUILD_DIR=release"
            ];

            # required
            LDFLAGS = "-static-libstdc++ -static-libgcc";

            # BUNDLE_BUILD_DIR is relative to the source root, above the cmake build dir
            installPhase = ''
              runHook preInstall
              mkdir -p $out
              cp -r ../release/. $out/
              # overwrite the store paths the compiler wrapper put in the rpath (removing the entry leaves the text behind).
              patchelf --set-rpath '$ORIGIN' $out/*.exe.so
              runHook postInstall
            '';

            # keep the launchers' "#!/bin/sh" and the rpath set above.
            dontPatchShebangs = true;
            dontPatchELF = true;
            allowedReferences = [ ];
          }); in {
            default = out.winelib-x86_64;
            winelib-x86_64 = mkWinelibPkg 64 pkgs;
            winelib-x86 = mkWinelibPkg 32 pkgs.pkgsi686Linux;

            # windows-x86_64 = mkPkg 64 pkgs;
            # windows-x86 = mkPkg 32 pkgs.pkgsi686Linux;
          };
      in
      {
        allowUnfree = true;

        packages = out;

        devShell = pkgs.mkShell {
          nativeBuildInputs = pkglist;
          buildInputs = pkglist;
          packages = pkglist;

          stdenv = pkgs.stdenvAdapters.useMoldLinker pkgs.gcc15stdenv;

          WINELIB32_TOOLCHAIN = mkWinelibToolchain 32 (winelibLibs pkgs.pkgsi686Linux);
          WINELIB64_TOOLCHAIN = mkWinelibToolchain 64 (winelibLibs pkgs);
        };
      }
    );
}
