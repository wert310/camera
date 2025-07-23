with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "webcam";
  src = ./.;
  buildInputs = [ gcc sdl3 ];
  buildPhase = ''
    mkdir -p $out/bin
    $CC webcam.c -o $out/bin/webcam -lSDL3
  '';
}
