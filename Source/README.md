# Information
This plugin's importing feature uses data based off [CUEParse](https://github.com/FabianFG/CUE4Parse)'s JSON export format.

# Code Style
❌
```c++
bool Cloud::Status::IsOpened()
{
	return IsProcessRunning("j0.dev.exe");
}
```

✅
```c++
bool Cloud::Status::IsOpened() {
	return IsProcessRunning("j0.dev.exe");
}
```

# Cloud ☁️

Cloud Server's API is located at [Tectors/j0.dev](https://github.com/Tectors/j0.dev/tree/main/Source/vj0.Cloud)

# Settings
Link: [`Public/Settings/JsonAsAssetSettings.h`](https://github.com/JsonAsAsset/JsonAsAsset/blob/main/Source/JsonAsAsset/Public/Settings/JsonAsAssetSettings.h)

## Adding Asset Types
> *Asset types without manual code will use **basic** importing, meaning it will only take the properties of the base object and import them.*
- Normal Asset types are found in [`JsonAsAsset/Private/Importers/Constructor/Types.cpp`](https://github.com/JsonAsAsset/JsonAsAsset/blob/main/Source/JsonAsAsset/Private/Importers/Constructor/Types.cpp#9) You don't need to add anything here if you made a custom IImporter with the REGISTER_IMPORTER macro.

##### Custom Logic for Asset Types

Adding **manual** asset type imports is done in the [`JsonAsAsset/Public/Importers/Types`](https://github.com/JsonAsAsset/JsonAsAsset/tree/main/Source/JsonAsAsset/Public/Importers/Types) folder. Use other importers for reference on how to create one.

##### Cloning JsonAsAsset
```
git clone https://github.com/JsonAsAsset/JsonAsAsset --recursive
```

##### Adding JsonAsAsset as a sub-module
```
git submodule add https://github.com/JsonAsAsset/JsonAsAsset Plugins/JsonAsAsset
git submodule update --init --recursive
```
