{
  projectRootFile = "flake.nix";
  programs.alejandra.enable = true;

  # Other formatting tools
  programs.clang-format.enable = true;
}
