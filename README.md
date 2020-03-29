<!-- Please be careful editing the below HTML, as GitHub is quite finicky with anything that looks like an HTML tag in GitHub Flavored Markdown. -->
<p align="center">
  <b>Status Bar for the Mac.</b>
</p>
<p align="center">
  <a href="https://travis-ci.org/somdoron/spacebar">
    <img src="https://travis-ci.org/somdoron/spacebar.svg?branch=master" alt="CI Status Badge">
  </a>
  <a href="https://github.com/somdoron/spacebar/blob/master/LICENSE.txt">
    <img src="https://img.shields.io/github/license/somdoron/spacebar.svg?color=green" alt="License Badge">
  </a>
  <a href="https://github.com/spacebar/blob/blob/master/CHANGELOG.md">
    <img src="https://img.shields.io/badge/view-changelog-green.svg" alt="Changelog Badge">
  </a>
  <a href="https://github.com/somdoron/spacebar/releases">
    <img src="https://img.shields.io/github/commits-since/somdoron/spacebar/latest.svg?color=green" alt="Version Badge">
  </a>
</p>

## About

spacebar is a status bar for [&nearr;&nbsp;yabai][gh-yabai] tiling window management.

## Installation and Configuration

- TODO: release package 

- Sample configuration files can be found in the [&nearr;&nbsp;examples][spacebar-examples] directory. Refer to the [&nearr;&nbsp;documentation][spacebar-docs].


## Requirements and Caveats

Please read the below requirements carefully.
Make sure you fulfil all of them before filing an issue.

|Requirement|Note|
|-:|:-|
|Operating&nbsp;System|macOS Catalina 10.15.0+ is supported.|
|Accessibility&nbsp;API|spacebar must be given permission to utilize the Accessibility API and will request access upon launch. The application must be restarted after access has been granted.|

Please also take note of the following caveats.

|Caveat|Note|
|-:|:-|
|Code&nbsp;Signing|When building from source (or installing from HEAD), it is recommended to codesign the binary so it retains its accessibility and automation privileges when updated or rebuilt.|
|Mission&nbsp;Control|In the Mission Control preferences pane in System Preferences, the setting "Automatically rearrange Spaces based on most recent use" should be disabled.|

## License and Attribution

spacebar is licensed under the [&nearr;&nbsp;MIT&nbsp;License][spacebar-license], a short and simple permissive license with conditions only requiring preservation of copyright and license notices.
Licensed works, modifications, and larger works may be distributed under different terms and without source code.

Thanks to [@koekeishiya][gh-koekeishiya] for creating yabai.

## Disclaimer

Use at your own discretion.
I take no responsibility if anything should happen to your machine while trying to install, test or otherwise use this software in any form.

<!-- Project internal links -->
[spacebar-license]: LICENSE.txt
[spacebar-examples]: https://github.com/somdoron/spacebar/tree/master/examples
[spacebar-docs]: https://github.com/somdoron/spacebar/blob/master/doc/spacebar.asciidoc

<!-- Links to other GitHub projects/users -->
[gh-koekeishiya]: https://github.com/koekeishiya
[gh-yabai]: https://github.com/koekeishiya/yabai
