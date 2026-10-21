// Copyright (c) Gallant Lab. All Rights Reserved.

#include "ExperimentSettings.h"

#include "Misc/ConfigCacheIni.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

// ---------------------------------------------------------------------------
// Section name used in INI files.
// ---------------------------------------------------------------------------

static const TCHAR *SectionName = TEXT("Experiment");

// ---------------------------------------------------------------------------
// UExperimentType
// ---------------------------------------------------------------------------

EExperimentType UExperimentType::FromString(const FString &String)
{
	if (String == TEXT("Other"))                    return EExperimentType::Other;

	UE_LOG(LogExperiment, Warning, TEXT("Unknown experiment type string \"%s\", defaulting to Other."), *String);
	return EExperimentType::Other;
}

FString UExperimentType::ToString(EExperimentType ExperimentType)
{
	switch (ExperimentType)
	{
		case EExperimentType::Other:                    return TEXT("Other");
		default:                                        return TEXT("Unknown");
	}
}

// ---------------------------------------------------------------------------
// Helpers - reading a single value from an FConfigFile.
// ---------------------------------------------------------------------------

static bool ReadBool(FConfigFile &ConfigFile, const TCHAR *Key, bool &OutValue)
{
	FString StringValue;
	if (ConfigFile.GetString(SectionName, Key, StringValue))
	{
		OutValue = StringValue.ToBool();
		return true;
	}
	return false;
}

static bool ReadInt(FConfigFile &ConfigFile, const TCHAR *Key, int &OutValue)
{
	FString StringValue;
	if (ConfigFile.GetString(SectionName, Key, StringValue))
	{
		OutValue = FCString::Atoi(*StringValue);
		return true;
	}
	return false;
}

static bool ReadFloat(FConfigFile &ConfigFile, const TCHAR *Key, float &OutValue)
{
	FString StringValue;
	if (ConfigFile.GetString(SectionName, Key, StringValue))
	{
		OutValue = FCString::Atof(*StringValue);
		return true;
	}
	return false;
}

static bool ReadString(FConfigFile &ConfigFile, const TCHAR *Key, FString &OutValue)
{
	return ConfigFile.GetString(SectionName, Key, OutValue);
}

// ---------------------------------------------------------------------------
// Helpers - populating settings from an FConfigFile.
// ---------------------------------------------------------------------------

static void ReadCameras(FConfigFile &ConfigFile, UExperimentSettings &Settings)
{
	FString CameraList;
	if (!ReadString(ConfigFile, TEXT("Cameras"), CameraList))
		return;

	Settings.Cameras.Empty();
	TArray<FString> Names;
	CameraList.ParseIntoArray(Names, TEXT(","), true);
	for (FString &Name : Names)
	{
		Name.TrimStartAndEndInline();
		FExperimentCameraDescription Description;
		Description.Name = Name;

		const FString Section = FString::Printf(TEXT("%s/Camera/%s"), SectionName, *Name);
		FString Value;
		if (ConfigFile.GetString(*Section, TEXT("PostProcessing"), Value))
			Description.PostProcessing = PostProcessEffectFromString(Value);
		if (ConfigFile.GetString(*Section, TEXT("ImageSizeX"), Value))
			Description.ImageSizeX = FCString::Atoi(*Value);
		if (ConfigFile.GetString(*Section, TEXT("ImageSizeY"), Value))
			Description.ImageSizeY = FCString::Atoi(*Value);
		if (ConfigFile.GetString(*Section, TEXT("FOV"), Value))
			Description.FOV = FCString::Atof(*Value);

		Settings.Cameras.Add(Description);
	}
}

static void ApplyConfigToSettings(FConfigFile &ConfigFile, UExperimentSettings &Settings)
{
	FString ExperimentTypeString;
	if (ReadString(ConfigFile, TEXT("ExperimentType"), ExperimentTypeString))
	{
		Settings.SetExperimentType(UExperimentType::FromString(ExperimentTypeString));
	}

	ReadString(ConfigFile, TEXT("Subject"), Settings.Subject);
	ReadBool(ConfigFile, TEXT("AutoTriggerDemoRecording"), Settings.AutoTriggerDemoRecording);
	ReadBool(ConfigFile, TEXT("AutoEyetrackingCalibration"), Settings.AutoEyetrackingCalibration);
	ReadFloat(ConfigFile, TEXT("SecondsToDemoStop"), Settings.SecondsToDemoStop);
	ReadString(ConfigFile, TEXT("Resolution"), Settings.Resolution);
	ReadInt(ConfigFile, TEXT("RenderAll"), Settings.RenderAll);
	ReadInt(ConfigFile, TEXT("TTLsBeforeExperimentStart"), Settings.TTLsBeforeExperimentStart);
	ReadBool(ConfigFile, TEXT("ShowFPS"), Settings.ShowFPS);
	ReadString(ConfigFile, TEXT("ReticuleType"), Settings.ReticuleType);
	ReadCameras(ConfigFile, Settings);
}

// ---------------------------------------------------------------------------
// UExperimentSettings - accessors
// ---------------------------------------------------------------------------

void UExperimentSettings::SetExperimentType(EExperimentType InExperimentType)
{
	ExperimentType = InExperimentType;
}

EExperimentType UExperimentSettings::GetExperimentType() const
{
	return ExperimentType;
}

// ---------------------------------------------------------------------------
// UExperimentSettings - resolution helper
// ---------------------------------------------------------------------------

void UExperimentSettings::SetResolution(int Width, int Height, bool Fullscreen)
{
	Resolution = FString::Printf(TEXT("%dx%d%s"), Width, Height, Fullscreen ? TEXT("f") : TEXT("w"));
}

// ---------------------------------------------------------------------------
// UExperimentSettings - loading
// ---------------------------------------------------------------------------

void UExperimentSettings::LoadSettings()
{
	// First, try the default project config directory.
	FString DefaultFilePath = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("ExperimentSettings.ini"));
	if (FPaths::FileExists(DefaultFilePath))
	{
		LoadSettingsFromFile(DefaultFilePath, false);
	}

	// Then allow a command-line override that takes precedence.
	FString CommandLineFilePath;
	if (FParse::Value(FCommandLine::Get(), TEXT("-experiment-settings="), CommandLineFilePath))
	{
		if (FPaths::FileExists(CommandLineFilePath))
		{
			LoadSettingsFromFile(CommandLineFilePath);
		}
		else
		{
			UE_LOG(LogExperiment, Warning, TEXT("Command-line Experiment settings file not found: %s"), *CommandLineFilePath);
		}
	}
}

void UExperimentSettings::LoadSettingsFromFile(const FString &FilePath, bool bLogOnFailure)
{
	if (!FPaths::FileExists(FilePath))
	{
		if (bLogOnFailure)
		{
			UE_LOG(LogExperiment, Error, TEXT("Experiment settings file not found: %s"), *FilePath);
		}
		return;
	}

	UE_LOG(LogExperiment, Log, TEXT("Loading Experiment settings from \"%s\"."), *FilePath);

	FConfigFile ConfigFile;
	ConfigFile.Read(FilePath);

	ApplyConfigToSettings(ConfigFile, *this);
	CurrentFileName = FilePath;
}

void UExperimentSettings::LoadSettingsFromString(const FString &INIStyleString)
{
	FConfigFile ConfigFile;
	ConfigFile.CombineFromBuffer(INIStyleString, TEXT("ExperimentSettings"));

	ApplyConfigToSettings(ConfigFile, *this);
}

// ---------------------------------------------------------------------------
// UExperimentSettings - saving
// ---------------------------------------------------------------------------

static FString BoolToString(bool Value)
{
	return Value ? TEXT("true") : TEXT("false");
}

static FString BuildINIContent(const UExperimentSettings &Settings)
{
	FString Content;

	Content += FString::Printf(TEXT("[%s]\n"), SectionName);
	Content += FString::Printf(TEXT("ExperimentType=%s\n"), *UExperimentType::ToString(Settings.GetExperimentType()));
	Content += FString::Printf(TEXT("Subject=%s\n"), *Settings.Subject);
	Content += FString::Printf(TEXT("AutoTriggerDemoRecording=%s\n"), *BoolToString(Settings.AutoTriggerDemoRecording));
	Content += FString::Printf(TEXT("AutoEyetrackingCalibration=%s\n"), *BoolToString(Settings.AutoEyetrackingCalibration));
	Content += FString::Printf(TEXT("SecondsToDemoStop=%f\n"), Settings.SecondsToDemoStop);
	Content += FString::Printf(TEXT("Resolution=%s\n"), *Settings.Resolution);
	Content += FString::Printf(TEXT("RenderAll=%d\n"), Settings.RenderAll);
	Content += FString::Printf(TEXT("TTLsBeforeExperimentStart=%d\n"), Settings.TTLsBeforeExperimentStart);
	Content += FString::Printf(TEXT("ShowFPS=%s\n"), *BoolToString(Settings.ShowFPS));
	Content += FString::Printf(TEXT("ReticuleType=%s\n"), *Settings.ReticuleType);

	if (Settings.Cameras.Num() > 0)
	{
		TArray<FString> Names;
		for (const FExperimentCameraDescription &Camera : Settings.Cameras)
			Names.Add(Camera.Name);
		Content += FString::Printf(TEXT("Cameras=%s\n"), *FString::Join(Names, TEXT(",")));

		for (const FExperimentCameraDescription &Camera : Settings.Cameras)
		{
			Content += FString::Printf(TEXT("\n[%s/Camera/%s]\n"), SectionName, *Camera.Name);
			Content += FString::Printf(TEXT("PostProcessing=%s\n"), *PostProcessEffectToString(Camera.PostProcessing));
			Content += FString::Printf(TEXT("ImageSizeX=%d\n"), Camera.ImageSizeX);
			Content += FString::Printf(TEXT("ImageSizeY=%d\n"), Camera.ImageSizeY);
			Content += FString::Printf(TEXT("FOV=%f\n"), Camera.FOV);
		}
	}

	return Content;
}

bool UExperimentSettings::SaveSettings()
{
	if (CurrentFileName.IsEmpty())
	{
		CurrentFileName = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("ExperimentSettings.ini"));
	}
	return SaveSettingsToFile(CurrentFileName);
}

bool UExperimentSettings::SaveSettingsToFile(const FString &FilePath)
{
	FString Content = BuildINIContent(*this);

	if (!FFileHelper::SaveStringToFile(Content, *FilePath))
	{
		UE_LOG(LogExperiment, Error, TEXT("Failed to save Experiment settings to \"%s\"."), *FilePath);
		return false;
	}

	UE_LOG(LogExperiment, Log, TEXT("Experiment settings saved to \"%s\"."), *FilePath);
	CurrentFileName = FilePath;
	return true;
}

// ---------------------------------------------------------------------------
// UExperimentSettings - logging
// ---------------------------------------------------------------------------

void UExperimentSettings::LogSettings() const
{
	UE_LOG(LogExperiment, Log, TEXT("== Experiment Settings =="));
	UE_LOG(LogExperiment, Log, TEXT("ExperimentType            = %s"), *UExperimentType::ToString(ExperimentType));
	UE_LOG(LogExperiment, Log, TEXT("Subject                   = %s"), *Subject);
	UE_LOG(LogExperiment, Log, TEXT("AutoTriggerDemoRecording  = %s"), AutoTriggerDemoRecording ? TEXT("true") : TEXT("false"));
	UE_LOG(LogExperiment, Log, TEXT("AutoEyetrackingCalibration = %s"), AutoEyetrackingCalibration ? TEXT("true") : TEXT("false"));
	UE_LOG(LogExperiment, Log, TEXT("SecondsToDemoStop         = %f"), SecondsToDemoStop);
	UE_LOG(LogExperiment, Log, TEXT("Resolution                = %s"), *Resolution);
	UE_LOG(LogExperiment, Log, TEXT("RenderAll                 = %d"), RenderAll);
	UE_LOG(LogExperiment, Log, TEXT("TTLsBeforeExperimentStart = %d"), TTLsBeforeExperimentStart);
	UE_LOG(LogExperiment, Log, TEXT("ShowFPS                   = %s"), ShowFPS ? TEXT("true") : TEXT("false"));
	UE_LOG(LogExperiment, Log, TEXT("ReticuleType              = %s"), *ReticuleType);
}
