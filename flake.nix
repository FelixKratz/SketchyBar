{
  description = "A minimal status bar for macOS";

  # TODO: Move to official nixpkgs once PR is merged
  #       https://github.com/NixOS/nixpkgs/pull/117780
  inputs.nixpkgs.url = "github:cmacrae/nixpkgs/fix_spacebar";

  outputs = { self, nixpkgs }:
    let
      pkgs = import nixpkgs {
        config = {};
        system = "x86_64-darwin";
      };

      buildInputs = with pkgs.darwin.apple_sdk.frameworks; [
        Carbon
        Cocoa
        ScriptingBridge
        SkyLight
      ];

      shellInputs = buildInputs ++ [ pkgs.asciidoctor ];
    in
      {
        packages.x86_64-darwin.spacebar =
          pkgs.stdenv.mkDerivation rec {
            pname = "spacebar";
            version = "1.3.0";
            src = self;

            inherit buildInputs;

            installPhase = ''
              mkdir -p $out/bin
              mkdir -p $out/share/man/man1/
              cp ./bin/spacebar $out/bin/spacebar
              cp ./doc/spacebar.1 $out/share/man/man1/spacebar.1
            '';

            meta = with pkgs.lib; {
              description = "A minimal status bar for macOS";
              homepage = "https://github.com/cmacrae/spacebar";
              platforms = platforms.darwin;
              maintainers = [ maintainers.cmacrae ];
              license = licenses.mit;
            };
          };

        overlay = final: prev: {
          spacebar = self.packages.x86_64-darwin.spacebar;
        };

        defaultPackage.x86_64-darwin = self.packages.x86_64-darwin.spacebar;

        devShell.x86_64-darwin = pkgs.stdenv.mkDerivation {
          name = "spacebar";
          buildInputs = shellInputs;
        };
      };
}
