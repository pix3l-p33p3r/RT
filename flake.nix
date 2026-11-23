{
  description = "RT 42 ray tracer dev shell (Nix OpenCL dual-GPU: AMD RX 6500 + Intel UHD 770)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    # Mesa 26 rusticl needs kernel DRM >=3.59 (6.11+). Pin 24.05 for 5.15 cluster kernels.
    nixpkgs-2405.url = "github:NixOS/nixpkgs/nixos-24.05";
  };

  outputs = { self, nixpkgs, nixpkgs-2405 }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; config.allowUnfree = true; };
      pkgs2405 = import nixpkgs-2405 { inherit system; config.allowUnfree = true; };
      icdDir = pkgs.symlinkJoin {
        name = "rt-opencl-vendors";
        paths = [
          pkgs2405.mesa.opencl
          pkgs.intel-compute-runtime
        ];
      };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = [
          pkgs.gcc
          pkgs.gnumake
          pkgs.pkg-config
          pkgs.libx11
          pkgs.libxext
          pkgs.zlib
          pkgs.libbsd
          pkgs.ocl-icd
          pkgs.opencl-headers
          pkgs.clinfo
          pkgs2405.mesa.opencl
          pkgs.intel-compute-runtime
        ];
        shellHook = ''
          export OCL_ICD_VENDORS="${icdDir}/etc/OpenCL/vendors"
          export OPENCL_VENDOR_PATH="${icdDir}/etc/OpenCL/vendors"
          export RUSTICL_ENABLE="radeonsi,iris"
          export RT_NIX_OPENCL=1
        '';
      };
    };
}
