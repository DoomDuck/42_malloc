{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";

  outputs = { self, nixpkgs } : let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages."${system}";
  in {
    devShells.x86_64-linux.default = pkgs.mkShell {
      name = "malloc";
      buildInputs = [ pkgs.gdb ];
      nativeBuildInputs = [ pkgs.fish ];

      shellHook = ''
        exec fish
      '';
    };
  };
}
