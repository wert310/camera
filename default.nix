{pkgs ? import <nixpkgs> {},...}:
with pkgs;
stdenv.mkDerivation {
  name = "camera";
  src = ./.;
  buildInputs = [ gcc sdl3 ];
  buildPhase = ''
    mkdir -p $out/bin
    $CC camera.c -o $out/bin/camera -lSDL3
  '';
}
