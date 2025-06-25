# JsonAsAsset - FModel to Unreal Engine Asset Importer

[![Discord](https://img.shields.io/badge/Join%20Discord-Collector?color=0363ff&logo=discord&logoColor=white&style=for-the-badge)](https://discord.gg/xXEw4jc2UT)
[![Buy Me a Coffee](https://img.shields.io/badge/Buy%20Me%20a%20Coffee-Ko--fi?color=29abe0&logo=ko-fi&logoColor=white&style=for-the-badge)](https://ko-fi.com/t4ctor)
[![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/JsonAsAsset/JsonAsAsset/total?style=for-the-badge&label=DOWNLOADS&color=#03ffc8)](https://github.com/JsonAsAsset/JsonAsAsset/releases)
[![GitHub Repo stars](https://img.shields.io/github/stars/JsonAsAsset/JsonAsAsset?style=for-the-badge&logo=&color=gold)](https://github.com/JsonAsAsset/JsonAsAsset/stargazers)

[![Unreal Engine 5 Supported)](https://img.shields.io/badge/UE5.0+-black?logo=unrealengine&style=for-the-badge&labelColor=grey)](#install)
 [![Unreal Engine 4.27.2 Supported)](https://img.shields.io/badge/4.27.2-black?logo=unrealengine&style=for-the-badge&labelColor=grey)](#install)
 [![Unreal Engine 4.26.2 Supported)](https://img.shields.io/badge/4.26.2-black?logo=unrealengine&style=for-the-badge&labelColor=grey)](#install)
 [![Unreal Engine 4.26.0 Supported)](https://img.shields.io/badge/4.26.0-black?logo=unrealengine&style=for-the-badge&labelColor=grey)](#install)

### Description:

A user-friendly plugin for [Unreal Engine](https://www.unrealengine.com/en-US) that reads [JSON](https://www.json.org/json-en.html) files extracted from CUE4Parse [(often FModel)](https://fmodel.app), and regenerates assets as they were from the game's files.

✨ [Contributors](#contribute)

### Example Use:

* Importing **materials**, data assets, and data tables — [see more](#introduction)
* Porting **physics assets** for skeletal meshes
* Porting **sound effects** to Unreal Engine
* Automating asset porting workflows

This project aims to streamline the porting and modding experience, making it easier to bring game assets into Unreal Engine.

-----------------

### **Table of Contents**:

> 1. [Introduction to JsonAsAsset](#introduction)  
>    1.1 [Supported Asset Types](#supported-asset-types)
> 3. [→ Installation](#install)  
>    2.1 [FModel](#setup-fmodel)  
>    2.2 [Settings](#setup-settings)
> 4. [→ Using JsonAsAsset](#plugin-usage)
> 5. [Local Fetch](#local-fetch)  
>    4.1 [Setup Local Fetch Settings](#setup-local-fetch)  
>    4.2 [Launching Local Fetch](#launch-local-fetch)  

**Extras**:
<br>

> - [Common Errors 🐛](#common-errors)
> - [Licensing 📗](#licensing)

-----------------

<a name="introduction"></a>
## 1. Introduction to JsonAsAsset
> [!WARNING]
> Please note that this plugin is intended solely for **personal and educational use**. Do not use it to create or distribute **commercial products** without obtaining the necessary **licenses and permissions**. It is important to respect **intellectual property rights** and only use assets that you are **authorized to use**. We do not assume any responsibility for the way the created content is used.

JsonAsAsset is a user-friendly Unreal Engine plugin for importing assets from packaged games using JSON files. Primarily from CUE4Parse and the use of the application [FModel](https://fmodel.app) to export JSON files.

------------------

<a name="supported-asset-types"></a>
#### 1.1 Supported Asset Types

| Asset Category | Asset Types / Description |
|--------------------------------|------------------------------------------------------------------------------------------------------------------------|
| Curve Asset Types | CurveFloat, CurveTable, CurveVector, CurveLinearColor, CurveLinearColorAtlas |
| Data Asset Types | DataAsset, SlateBrushAsset, SlateWidgetStyleAsset, AnimBoneCompressionSettings, AnimCurveCompressionSettings, UserDefinedEnum, UserDefinedStruct |
| Table Types | CurveTable, DataTable, StringTable |
| Materials | Material, MaterialFunction, MaterialInstanceConstant, MaterialParameterCollection, SubsurfaceProfile |
| Sound Classes | Most/all sound classes are supported. SoundWave is downloaded by [Local Fetch](#local-fetch). |
| Animation Asset Types | PoseAsset, Skeleton, SkeletalMeshLODSettings, BlendSpace, BlendSpace1D, AimOffsetBlendSpace, AimOffsetBlendSpace1D |
| Physics Asset Types | PhysicsAsset, PhysicalMaterial |
| Sequencer Asset Types | CameraAnim |
| Landscape Asset Types | LandscapeGrassType, FoliageType_InstancedStaticMesh, FoliageType_Actor |

##### The following asset types add onto a pre-existing asset
| Asset Category | Asset Types / Description |
|-----------------------------------|------------------------------------------------------------------------------------------------------------------------------------|
| Animations | AnimSequence, AnimMontage [Animation Curves] |

**Material/Niagara Data**:
<br> Unreal Engine games made below 4.12 (a guess) will have material data, but not niagara data, any games made above that version will most definitely not have any material data nor niagara data, and therefore the actual data will be stripped and cannot be imported. **Unless** you are using a User Generated Content editor, then it's possible material data and niagara data will be there.

**NOTE:** If your game uses custom C++ classes or structures, you need to define them before importing with JsonAsAsset. JsonAsAsset does not create C++ files for you. (create them manually from SDKs or automate them) See [Unreal Engine Modding Projects](https://github.com/Buckminsterfullerene02/UE-Modding-Tools?tab=readme-ov-file#game-specific-template-projects) for game-specific template projects.

<a name="install"></a>
## 2. Installation
> [!NOTE]
> JsonAsAsset may not work with every Unreal Engine version. Please check the [Releases page](/../../releases) for compatibility details.

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

<a name="setup-fmodel"></a>
#### 2.1 Setup FModel
If you haven't already, **install FModel and set it up correctly, then proceed with the setup**.

[<img align="left" width="150" height="150" src="https://github.com/user-attachments/assets/d8e4f9c9-1268-4aee-ab1a-dabee31b3069?raw=true">](https://fmodel.app)
The JSON format/file has to be from a program that fits the format of FModel's JSON export files:
<br>

<br> [FModel](https://fmodel.app) *(Software for exploring Unreal Engine games)*
<br><br>

-------------------

<img align="right" width="315" height="190.4" src=https://github.com/user-attachments/assets/909e2db1-e0d9-4aae-9b8b-7190cf6718b3>

<a name="setup-settings"></a>
#### 2.2 Setup Settings

1. **Open Plugin Settings:**  
   Click on the JsonAsAsset dropdown, then select Open Plugin Settings.
   
3. **Import Settings from FModel:**    
   Locate Load External Configuration and press **FModel Settings**, your settings should be changed.
   
   If not, copy your export directory from FModel.

----------

That’s the basic setup! Here's how to use JsonAsAsset.

<a name="plugin-usage"></a>
##### Using JsonAsAsset

1. Find a asset in [FModel](https://fmodel.app), and save it by right clicking and pressing `Save Properties`. Locate the file on your computer and copy the location.

2. Press onto the [JsonAsAsset](https://github.com/JsonAsAsset/JsonAsAsset) button on your tool-bar, and a file import should pop-up. <img align="right" width="220" height="98" src=https://github.com/user-attachments/assets/514d7661-f2f6-4343-8c9c-ed875ddb5156>

3. Paste in the file's path, select the file and press open.

4. The asset will import, and bring you to the created asset in the content browser. (if the asset type is supported)

To bulk import assets and **what they reference**, you must set up [`Local Fetch`](#setup-local-fetch)!

<a name="local-fetch"></a>
## 4. Local Fetch

[Local Fetch](https://github.com/JsonAsAsset/LocalFetch) is a way to automate importing assets that other assets reference, built into JsonAsAsset. **It supports all asset types except AnimSequence/AnimMontages.** Learn how to [set up Local Fetch](#setup-local-fetch).

------------------------

<a name="setup-local-fetch"></a>
#### 4.1 Local Fetch Settings

1. **Open Plugin Settings:**  
   Click on the JsonAsAsset dropdown, then select Open Plugin Settings.

<img align="right" width="452" height="84" src=https://github.com/user-attachments/assets/1747162c-81df-442c-a6ce-d88e563a4ca2>

2. **Enable Local Fetch:**  
   Locate the property **Enable Local Fetch** and turn it on.

3. **Setup Local Fetch's Settings:**  
    Make sure to set these in your settings:
   - **Directory**: specific Paks folder of your game
   - **Unreal Engine**: version that your game is running
   - **Mappings File**: file path to your mappings file
   - **Encryption Keys**: if your game needs AES Keys, set them

###### Fetch Encryption from an API
You can automatically have your AES Keys changed if your game has a API ran by someone else, you can fetch aes keys and mappings from an API in the section **Local Fetch - Encryption**.

------------

<a name="launch-local-fetch"></a>
#### 4.2 Launch Local Fetch

<img align="right" width="240" height="145" src=https://github.com/user-attachments/assets/290b8ea3-6777-443f-bacf-b8df5738ec8f>

Running the API requires ASP.NET 8.0 to be installed; please install this [here](https://dotnet.microsoft.com/en-us/download/dotnet/thank-you/runtime-aspnetcore-8.0.1-windows-x64-installer).

Go ahead and click on the JsonAsAsset logo and hover over the list `"Command-line Application"` and press `"Execute Local Fetch (.EXE)"`.

A window should pop-up, and once the console says `[CORE] Running API`, Local Fetch has been successfully started! 

Make sure to keep this window open until you are done with JsonAsAsset.

</details>

JsonAsAsset will now use Local Fetch to import if a missing asset is referenced.

<a name="common-errors"></a>
## Common Errors 🐛

<details>
  <summary>Attempted to create a package with name containing double slashes. PackageName: ...</summary>
  
------------
  
Please set your Export Directory to your actual "Output/Exports" directory

***WRONG***: T:/FModel/Output

***RIGHT***: T:/FModel/Output/Exports
</details>

<details>
  <summary>Assertion failed: TextureReferenceIndex != INDEX_NONE</summary>

------------
  
This is a known issue in our code that we haven't fully resolved yet. While previous attempts to fix it have been unsuccessful, here’s a partial solution to reduce its occurrence:

- Re-launch your Unreal Engine project, go to JsonAsAsset's plugin settings and enable ***"Skip Result Node Connection"***. Also enable ***"Allow Package Saving"***.
</details>

<details>
  <summary>Local Fetch closes instantly when executing</summary>

------------

There may be a few reasons why the application automatically closes, but mostly the issue stems from something missing:

### 1. Your settings aren't correct, and need modifications
> Make sure all the information about your game is set in the settings.

### 2. ASP.NET 8.0 not installed
> Running the API requires ASP.NET 8.0 to be installed, please install this [here](https://dotnet.microsoft.com/en-us/download/dotnet/thank-you/runtime-aspnetcore-8.0.1-windows-x64-installer).

</details>

<a name="licensing"></a>
## Licensing 📗

JsonAsAsset is licensed under the MIT License, read more in the [LICENSE](https://github.com/JsonAsAsset/JsonAsAsset/blob/main/LICENSE) file. The plugin also uses [Detex](https://github.com/hglm/detex) and [NVIDIA Texture Tools](https://docs.nvidia.com/texture-tools/index.html).

<a name="contribute"></a>
## ✨ Contributors

Thanks go to these wonderful people:

<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tr>
    <td align="center"><a href="https://github.com/Tectors"><img src="https://github.com/Tectors.png" width="100px"/><br/><sub><b>Tector</b></sub></a><br/>Developer</td>
    <td align="center"><a href="https://github.com/GMatrixGames"><img src="https://github.com/GMatrixGames.png" width="100px"/><br/><sub><b>GMatrixGames</b></sub></a><br/>Collaborator</td>
    <td align="center"><a href="https://github.com/zyloxmods"><img src="https://github.com/zyloxmods.png" width="80px"/><br/><sub><b>Zylox</b></sub></a><br/>Maintainer</td>
  </tr>
  <tr>
    <td align="center" colspan="3">
      <a href="https://github.com/NathanFelipeRH"><b>Nathan Felipe</b></a><br/><span>Help with SoundCue Code</span>
    </td>
  </tr>
</table>

- Thanks to the people who contributed to [UEAssetToolkit](https://github.com/Buckminsterfullerene02/UEAssetToolkit-Fixes)! They have helped a lot.
- Logo uses a font by [Brylark](https://ko-fi.com/brylark), support him at his ko-fi!

#### [Would you like to contribute?](https://github.com/JsonAsAsset/JsonAsAsset/blob/main/Source/README.md#key-information-for-contributors-)
