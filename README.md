<br/>
<div align="center">
  <a href="https://github.com/JsonAsAsset/JsonAsAsset">
    <img src="https://github.com/user-attachments/assets/91b216ba-7bb3-4f48-bf96-69c645451d26" alt="Logo" width="250" height="250">
  </a>

  <h3 align="center">JsonAsAsset</h3>

  <p align="center">
    Powerful Unreal Engine Plugin that imports assets from FModel
    <br />
    <a href="#table-of-contents"><strong>Explore the docs »</strong></a>
  </p>
</div>

<div align="center">
<br/>
    
[![Discord](https://img.shields.io/badge/Join%20Discord-Collector?color=0363ff&logo=discord&logoColor=white&style=for-the-badge)](https://discord.gg/xXEw4jc2UT) [![Buy Me a Coffee](https://img.shields.io/badge/Buy%20Me%20a%20Coffee-Ko--fi?color=ff0de7&logo=ko-fi&logoColor=white&style=for-the-badge)](https://ko-fi.com/t4ctor)

[![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/JsonAsAsset/JsonAsAsset/total?style=for-the-badge&label=DOWNLOADS&color=ff0d0d)](https://github.com/JsonAsAsset/JsonAsAsset/releases)
[![GitHub Repo stars](https://img.shields.io/github/stars/JsonAsAsset/JsonAsAsset?style=for-the-badge&logo=&color=gold)](https://github.com/JsonAsAsset/JsonAsAsset/stargazers)

[![Unreal Engine 5 Supported)](https://img.shields.io/badge/UE5.0+-black?logo=unrealengine&style=for-the-badge&labelColor=grey)](#install)
[![Unreal Engine 4.27.2 Supported)](https://img.shields.io/badge/4.27.2-black?logo=unrealengine&style=for-the-badge&labelColor=grey)](#install)
[![Unreal Engine 4.26.2 Supported)](https://img.shields.io/badge/4.26.2-black?logo=unrealengine&style=for-the-badge&labelColor=grey)](#install)
[![Unreal Engine 4.26.0 Supported)](https://img.shields.io/badge/4.26.0-black?logo=unrealengine&style=for-the-badge&labelColor=grey)](#install)

</div>

### Description:

A user-friendly plugin for [Unreal Engine](https://www.unrealengine.com/en-US) that reads [JSON](https://www.json.org/json-en.html) files extracted from CUEParse [(FModel)](https://fmodel.app), and regenerates assets as they were from the game's files.

✨ [Contributors](#contribute)

### Example Use:

* Importing **materials**, data assets, and data tables — [see more](#asset-types)
* Porting **physics assets** for skeletal meshes
* Porting **sound effects** to Unreal Engine
* Automating asset porting workflows

This project aims to streamline the porting and modding experience, making it easier to bring game assets into Unreal Engine.

-----------------

> [!CAUTION]
> Please note that this plugin is intended solely for **personal and educational use**.
> 
> Do not use it to create or distribute **commercial products** without obtaining the necessary **licenses and permissions**. It is important to respect **intellectual property rights** and only use assets that you are **authorized to use**.
>
> We **do not assume any responsibility** for the way the created content is used.

-----------------

### **Table of Contents**:

> 1. [Asset Types](#asset-types)  
> 2. [Installation](#installation)  
>    2.1 [FModel](#fmodel)  
>    2.2 [Settings](#setup-settings)
> 4. [→ Using JsonAsAsset](#plugin-usage)

**Extras**:
<br>

> - [`☁️ Cloud Server`](#cloud-server)  
> - [`🐛 Common Errors`](#common-errors)
> - [`📗 Licensing`](#licensing)

-----------------

<a name="asset-types"></a>
## 1. Asset Types
If an asset type isn't listed below, **it's not currently supported by the plugin**.

|  | Asset Types |
|--------------------------------|------------------------------------------------------------------------------------------------------------------------|
| **Curve Asset Types** | CurveFloat, CurveTable, CurveVector, CurveLinearColor, CurveLinearColorAtlas |
| **Data Asset Types** | DataAsset, SlateBrushAsset, SlateWidgetStyleAsset, AnimBoneCompressionSettings, AnimCurveCompressionSettings, UserDefinedEnum, UserDefinedStruct |
| **Table Types** | CurveTable, DataTable, StringTable |
| **Material Types** | Material, MaterialFunction, MaterialInstanceConstant, MaterialParameterCollection, SubsurfaceProfile |
| **Sound Types** | Most/all sound classes are supported. SoundWave is downloaded by a [Cloud Server](#cloud-server). |
| **Animation Asset Types** | PoseAsset, Skeleton, SkeletalMeshLODSettings, BlendSpace, BlendSpace1D, AimOffsetBlendSpace, AimOffsetBlendSpace1D |
| **Physics Asset Types** | PhysicsAsset, PhysicalMaterial |
| **Sequencer Asset Types** | CameraAnim |
| **Landscape Asset Types** | LandscapeGrassType, FoliageType_InstancedStaticMesh, FoliageType_Actor |

#### The following asset types add onto a pre-existing asset
|  | Asset Types |
|-----------------------------------|------------------------------------------------------------------------------------------------------------------------------------|
| **Animation Types** | AnimSequence, AnimMontage **[Animation Curves]** |

#### Material Data Prerequisites
Unreal Engine games made below 4.12 (a guess) will have material data, *any games made above that version will most definitely not have any material data*, and therefore the actual data will be stripped and cannot be imported. **Unless you are using a User Generated Content editor**, then it's possible material data will be there.

#### C++ Classes Prerequisites
If your game uses custom C++ classes or structures, **you need to define them**.

See [Unreal Engine Modding Projects](https://github.com/Buckminsterfullerene02/UE-Modding-Tools?tab=readme-ov-file#game-specific-template-projects) for game-specific template projects.

<a name="installation"></a>
## 2. Installation
Follow these steps to install **JsonAsAsset**:

1. **Visit the Releases Page:**  
   Go to the [Releases page](/../../releases) for the plugin.
3. **Download the Appropriate Release:**    
   Download the release that matches your version of Unreal Engine.  
   If a matching release isn’t available, [**compile the plugin yourself**](https://dev.epicgames.com/community/learning/tutorials/qz93/unreal-engine-building-plugins).
5. **Extract the Files:**  
   Extract the downloaded files to your project’s `Plugins` folder. If the folder doesn’t exist, create it in the root directory of your project.
7. **Open Your Project**  
   Launch your Unreal Engine project.
8. **Access the Plugins Window:**  
   Navigate to **Edit → Plugins**. <img align="right" width="387.4" height="75.4" src=https://github.com/user-attachments/assets/c0867324-5129-49e4-9f9c-9804c93cdf73>
10. **Enable JsonAsAsset:**  
   In the Plugins window, search for `JsonAsAsset` and enable it.
11. **Restart the Editor:**  
   Restart the Unreal Engine editor to apply the changes.

--------------------

<a name="fmodel"></a>
#### 2.1 Setup FModel

[<img align="left" width="150" height="150" src="https://github.com/user-attachments/assets/d8e4f9c9-1268-4aee-ab1a-dabee31b3069?raw=true">](https://fmodel.app)

> [!IMPORTANT]
> If you haven't already, **install [FModel](https://fmodel.app) and set it up correctly, then proceed with the setup**.
>
>
> ​There is a specific FModel version for **material data support** found in the discord server.            
> **Material Data Prerequisites still apply.**
>  ​

-------------------

<img align="right" width="315" height="190.4" src=https://github.com/user-attachments/assets/909e2db1-e0d9-4aae-9b8b-7190cf6718b3>

<a name="setup-settings"></a>
#### 2.2 Setup Settings

1. **Open Plugin Settings:**  
   Click on the JsonAsAsset dropdown, then select Open Plugin Settings.
   
3. **Import Settings from FModel:**    
   Locate Load External Configuration and press **FModel Settings**, your settings should be changed.

----------

<a name="plugin-usage"></a>
## 4. Using JsonAsAsset

1. Find a asset in [FModel](https://fmodel.app), and save it by right clicking and pressing `Save Properties`. Locate the file on your computer and copy the location.

2. Press onto the [JsonAsAsset](https://github.com/JsonAsAsset/JsonAsAsset) button on your tool-bar, and a file import should pop-up. <img align="right" width="220" height="98" src=https://github.com/user-attachments/assets/514d7661-f2f6-4343-8c9c-ed875ddb5156>

3. Paste in the file's path, select the file and press open.

4. The asset will import, and bring you to the created asset in the content browser.

To bulk import assets and **what they reference**, you must set up a [`☁️ Cloud Server`](#cloud-server)!

------------

<a name="cloud-server"></a>
## ☁️ Cloud Server

A **Cloud Server** hosts a Web API that lets JsonAsAsset communicate directly with game files. It automatically imports referenced assets for all supported types, except **AnimSequence** and **AnimMontage**. 

**Use an app that includes a built-in cloud server for easy setup:**

<a href="https://github.com/Tectors/j0.dev"><img src="https://github.com/user-attachments/assets/2c450d60-6573-4545-8b20-f6190a87827e" width="126" height="126"/></a>

<a name="common-errors"></a>
## 🐛 Common Errors

<details>
  <summary>Attempted to create a package with name containing double slashes. PackageName: ...</summary>
  
------------
  
Please set your Export Directory to your "Output/Exports" directory

❌: `T:/FModel/Output`

✅: `T:/FModel/Output/Exports`
</details>

<details>
  <summary>Assertion failed: TextureReferenceIndex != INDEX_NONE</summary>

------------
  
This is a known issue in our code that we haven't fully resolved yet. While previous attempts to fix it have been unsuccessful, here's a partial solution to reduce its occurrence:

- Re-launch your Unreal Engine project, go to JsonAsAsset's plugin settings and enable ***"Skip Result Node Connection"***. Also enable ***"Allow Package Saving"***.
</details>

<a name="licensing"></a>
## 📗 Licensing

JsonAsAsset is licensed under the MIT License, read more in the [LICENSE](https://github.com/JsonAsAsset/JsonAsAsset/blob/main/LICENSE) file. The plugin also uses [Detex](https://github.com/hglm/detex) and [NVIDIA Texture Tools](https://docs.nvidia.com/texture-tools/index.html).

<a name="contribute"></a>
## ✨ Contributors

[![Buy Me a Coffee](https://img.shields.io/badge/Buy%20Me%20a%20Coffee-Ko--fi?color=ff0de7&logo=ko-fi&logoColor=white&style=for-the-badge)](https://ko-fi.com/t4ctor)

Thanks go to these wonderful people:

<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tr>
    <td align="center"><a href="https://github.com/Tectors"><img src="https://github.com/Tectors.png" width="100px"/><br/><sub><b>Tector</b></sub></a><br/>Developer</td>
    <td align="center"><a href="https://github.com/GMatrixGames"><img src="https://github.com/GMatrixGames.png" width="100px"/><br/><sub><b>GMatrixGames</b></sub></a><br/>Collaborator</td>
    <td align="center"><a href="https://github.com/zyloxmods"><img src="https://github.com/zyloxmods.png" width="80px"/><br/><sub><b>Zylox</b></sub></a><br/>Maintainer</td>
  </tr>
</table>

- Thanks to the people who contributed to [UEAssetToolkit](https://github.com/Buckminsterfullerene02/UEAssetToolkit-Fixes)! They have helped a lot.
- Logo uses a font by [Brylark](https://ko-fi.com/brylark), support him at his ko-fi!

#### [Would you like to contribute?](https://github.com/JsonAsAsset/JsonAsAsset/blob/main/Source/README.md#key-information-for-contributors-)

