<!-- Improved compatibility of back to top link: See: https://github.com/othneildrew/Best-README-Template/pull/73 -->
<a id="readme-top"></a>
<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** I left this header up here to give credit where credit is do for this nice Read.me template.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![project_license][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]

<!-- PROJECT Title -->
<div align="center">
<h3 align="center">cCircularBufferDriver</h3>
  <p align="center">
    A library used by my personal projects for a fixed-size, ring-style buffer -- storing elements of a fixed size in a preallocated block of memory, oldest in / oldest out.
    <br />
    <a href="https://github.com/indyIOT/cCircularBufferDriver/issues/new?labels=bug&template=bug-report---.md">Report Bug</a>
    &middot;
    <a href="https://github.com/indyIOT/cCircularBufferDriver/issues/new?labels=enhancement&template=feature-request---.md">Request Feature</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#top-contributors">Top Contributors</a></li>
    <li><a href="#change-Log">Change Log</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## About The Project
C project that is a cmake library implementing a circular (ring) buffer -- fixed capacity, fixed element size, no dynamic allocation.
<p align="right">(<a href="#readme-top">back to top</a>)</p>



### Built With

* [![CMake][CMake.js]][CMake-url]

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

This project may be cloned on its own or as part of a larger test product. It is a required library for all of my c projects. It relies on the cCommonMacros and cCommonTypes libraries.

### Prerequisites

You need a c standard tool chain with CMake installed. 


### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/indyIOT/cCircularBufferDriver.git
   ```
3. Run Make
   ```sh
   make
   ```
4. Change to the build directory and run CMake:
   ```sh
   cd build
   cmake ..
   cmake --build .
   ```
5. Change git remote url to avoid accidental pushes to base project
   ```sh
   git remote set-url origin indyIOT/cCircularBufferDriver
   git remote -v # confirm the changes
   ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- Usage -->
## Usage

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- Top Contributors -->
## Top Contributors

<a href="https://github.com/indyIOT/cCircularBufferDriver/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=indyIOT/cCircularBufferDriver" alt="contrib.rocks image" />
</a>
<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- Changelog -->
## Change Log
<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- LICENSE -->
## License

Distributed under the project_license. See `LICENSE` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

Project Link: [https://github.com/indyIOT/cCircularBufferDriver](https://github.com/indyIOT/cCircularBufferDriver)

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/indyIOT/cCircularBufferDriver.svg?style=for-the-badge
[contributors-url]: https://github.com/indyIOT/cCircularBufferDriver/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/indyIOT/cCircularBufferDriver.svg?style=for-the-badge
[forks-url]: https://github.com/indyIOT/cCircularBufferDriver/network/members
[stars-shield]: https://img.shields.io/github/stars/indyIOT/cCircularBufferDriver.svg?style=for-the-badge
[stars-url]: https://github.com/indyIOT/cCircularBufferDriver/stargazers
[issues-shield]: https://img.shields.io/github/issues/indyIOT/cCircularBufferDriver.svg?style=for-the-badge
[issues-url]: https://github.com/indyIOT/cCircularBufferDriver/issues
[license-shield]: https://img.shields.io/github/license/indyIOT/cCircularBufferDriver.svg?style=for-the-badge
[license-url]: https://github.com/indyIOT/cAlgoImplementations/blob/main/LICENSE
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://www.linkedin.com/in/anthonygarza2020

<!-- Shields.io badges. You can a comprehensive list with many more badges at: https://github.com/inttter/md-badges -->
[CMake.js]: https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=fff
[CMake-url]: https://cmake.org/
