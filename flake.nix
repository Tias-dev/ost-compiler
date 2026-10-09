{
  description = "Default flake-parts based setup";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
    flake-parts.url = "github:hercules-ci/flake-parts";
    import-tree.url = "github:vic/import-tree";
    systems.url = "github:nix-systems/default";
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = inputs:
    inputs.flake-parts.lib.mkFlake {inherit inputs;} {
      imports = [inputs.treefmt-nix.flakeModule];
      systems = import inputs.systems;
      perSystem = {
        inputs',
        pkgs,
        self',
        ...
      }: {
        packages = {
          ost-toolkit = pkgs.callPackage ./default.nix {};
          default = self'.packages.ost-toolkit;
        };
        treefmt = import ./treefmt.nix;
      };
    };
}
