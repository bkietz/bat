# following https://www.breakds.org/post/nix-based-c++-workflow/
{
  description = "A flake for building fledermaus tests";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/22.05";

    utils.url = "github:numtide/flake-utils";
    utils.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = { self, nixpkgs, utils }: utils.lib.eachSystem [
    "x86_64-linux"
  ] (
    system:
      let pkgs = import nixpkgs { inherit system; };
      in {
        devShell = pkgs.mkShell rec {
          name = "fledermaus-c++-dev-shell";

          packages = with pkgs; [
            llvmPackages_16.clang
            cmake
            ninja
            doctest
          ];

          shellHook =
            let icon = "f121";
            in ''
              export PS1="$(echo -e '\u${icon}') {\[$(tput sgr0)\]\[\033[38;5;228m\]\w\[$(tput sgr0)\]\[\033[38;5;15m\]} (${name}) \\$ \[$(tput sgr0)\]"
            '';
        };
      }
  );
}
