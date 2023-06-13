{ pkgs, ... }:

{
  packages = with pkgs; [
    git
    gcc
    (python3.withPackages (ps: with ps; [breathe sphinx]))
    doxygen
  ];

  languages.cplusplus.enable = true;
}
