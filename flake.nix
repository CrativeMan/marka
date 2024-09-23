{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    { self
    , nixpkgs
    , flake-utils
    ,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };
      in
      with pkgs; {
        formatter = pkgs.alejandra;
        devShell = mkShell.override { stdenv = clangStdenv; } {
          packages = [
            clang-tools
            llvmPackages.clangUseLLVM
            gcc
            clang
            cmake

          ];
        };

        defaultPackage = stdenv.mkDerivation {
          name = "marka";
          src = ./.;
          buildInputs = [ gcc ];
          buildPhase = ''
            mkdir -p build
            make
            cp marka build/marka
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp build/marka $out/bin/
          '';

        };
      }
    );
}
