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
      # Clover (mesa.icd) is OpenCL 1.1 and loses to rusticl. Ship rusticl + NEO only.
      icdDir = pkgs.runCommand "rt-opencl-vendors" { } ''
        mkdir -p $out/etc/OpenCL/vendors
        ln -s ${pkgs2405.mesa.opencl}/etc/OpenCL/vendors/rusticl.icd \
          $out/etc/OpenCL/vendors/rusticl.icd
        ln -s ${pkgs.intel-compute-runtime}/etc/OpenCL/vendors/intel-neo.icd \
          $out/etc/OpenCL/vendors/intel-neo.icd
      '';
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
          # Mesa 24.05 loads gcc 13 libstdc++. NEO needs GLIBCXX_3.4.35 (gcc 16).
          # Applied only to ./rt_ocl: host glibc 2.35 cannot load this library.
          export RT_LIBSTDCXX="${pkgs.stdenv.cc.cc.lib}/lib/libstdc++.so.6"
          # radeonsi only. iris prints the incomplete-OpenCL warning; NEO owns the iGPU.
          export RUSTICL_ENABLE="radeonsi"
          export RT_NIX_OPENCL=1
        '';
      };
    };
}
