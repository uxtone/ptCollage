{nixpkgs, flake-utils, ...}: flake-utils.lib.eachDefaultSystem (
   system:
   let
    pkgs = import nixpkgs { inherit system; };
    mkWinelibToolchain =
      bits:
      libs:
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
    mkWinelibPackage = (bits: packages: pkgs.stdenv.mkDerivation {
      pname = "pxtone-collage";
      version = "0";
      src = ./.;

      nativeBuildInputs = with pkgs; [ cmake patchelf wineWow ];

      cmakeFlags = [
        "-DCMAKE_TOOLCHAIN_FILE=${mkWinelibToolchain bits (winelibDependencies packages)}"
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
    });
    wineWow = pkgs.wineWow64Packages.stable;
    winelibDependencies = p: with p; [ libpng zlib libogg libvorbis ];
   in {
     inherit mkWinelibToolchain mkWinelibPackage wineWow;
     winelibLibs = winelibDependencies;
     packages = let winelib-x86_64 = mkWinelibPackage 64 pkgs; in {
       default = winelib-x86_64;
       inherit winelib-x86_64;
       winelib-x86 = mkWinelibPackage 32 pkgs.pkgsi686Linux;
     };
   }
)
