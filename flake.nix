{
  inputs = {
    self = {
      submodules = true;
    };
    nixpkgs.url = "github:nixos/nixpkgs";
    nixpkgs-wine.url = "github:nixos/nixpkgs/1a79c9a3bc6728831b4ae61c4172f3ee0e6af003";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs =
    { self, flake-utils, nixpkgs, nixpkgs-wine, ... }@inputs:
    import ./winelib.nix inputs
    // flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        originalDependencies = with pkgs; [
          ninja
          cmake
          gcc_multi

          libpng
          libogg
          libvorbis

          self.wineWow.${system}
          vulkan-validation-layers
          pkg-config
        ];
        pkglist =
          with pkgs;
          [
            valgrind
            gdb
            clang-tools
            uncrustify
          ]
          ++ originalDependencies;
      in
      {
        allowUnfree = true;
        devShell = pkgs.mkShell {
          nativeBuildInputs = pkglist;
          buildInputs = pkglist;
          packages = pkglist;

          stdenv = pkgs.stdenvAdapters.useMoldLinker pkgs.gcc15stdenv;

          WINELIB32_TOOLCHAIN = self.mkWinelibToolchain.${system} 32 (self.winelibLibs.${system} pkgs.pkgsi686Linux);
          WINELIB64_TOOLCHAIN = self.mkWinelibToolchain.${system} 64 (self.winelibLibs.${system} pkgs);
        };
      }
    );
}
