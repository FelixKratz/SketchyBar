const codeTheme = require("prism-react-renderer/themes/palenight")

/** @type {import('@docusaurus/types').DocusaurusConfig} */
module.exports = {
  title: "SketchyBar",
  tagline: "A highly customizable macOS statusbar replacement",
  url: "https://felixkratz.github.io/",
  baseUrl: "/SketchyBar/",
  trailingSlash: false,
  onBrokenLinks: "throw",
  onBrokenMarkdownLinks: "warn",
  favicon: "img/favicon.ico",
  organizationName: "SketchyBar",
  projectName: "felixkratz.github.io",
  themeConfig: {
    colorMode: {
      respectPrefersColorScheme: true,
    },
    navbar: {
      title: "SketchyBar",
      logo: {
        alt: "SketchyBar Logo",
        src: "img/logo.svg",
      },
      items: [
        {
          to: "features",
          label: "Features",
          position: "right",
        },
        {
          to: "setup",
          label: "Setup & Installation",
          position: "right",
        },
        {
          to: "config/bar",
          label: "Configuration",
          position: "right",
        },
        {
          href: "https://github.com/FelixKratz/SketchyBar",
          position: "right",
        },
      ],
    },
    prism: {
      theme: codeTheme,
      darkTheme: codeTheme,
      additionalLanguages: ["c"],
    },
  },
  presets: [
    [
      "@docusaurus/preset-classic",
      {
        docs: {
          routeBasePath: "/",
          sidebarPath: require.resolve("./sidebars.js"),
        },
        theme: {
          customCss: require.resolve("./src/css/style.css"),
        },
      },
    ],
  ],
  themes: [
    [
      require.resolve("@easyops-cn/docusaurus-search-local"),
      /** @type {import("@easyops-cn/docusaurus-search-local").PluginOptions} */
      ({
        hashed: true,
        docsRouteBasePath: '/',	
      }),
    ],
  ],
}
