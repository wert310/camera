{
  outputs = { self, nixpkgs }: {
    packages.x86_64-linux.default = (import nixpkgs { system = "x86_64-linux"; }).callPackage ./default.nix {};
  };
}
