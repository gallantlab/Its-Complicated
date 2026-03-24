// Copyright (c) Gallant Lab. All Rights Reserved.

#include "MRIExperiment.h"
#include "Settings/MRISettings.h"

#include "Misc/ConfigCacheIni.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

// ---------------------------------------------------------------------------
// Section name used in INI files.
// ---------------------------------------------------------------------------

static const TCHAR *SectionName = TEXT("MRI");

// ---------------------------------------------------------------------------
// UExperimentType
// ---------------------------------------------------------------------------

EExperimentType UExperimentType::FromString(const FString &String)
{
	if (String == TEXT("Other"))                    return EExperimentType::Other;

	UE_LOG(LogMRI, Warning, TEXT("Unknown experiment type string \"%s\", defaulting to Navigation."), *String);
	return EExperimentType::Navigation;
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

static void ApplyConfigToSettings(FConfigFile &ConfigFile, UMRISettings &Settings)
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
	ReadBool(ConfigFile, TEXT("SensorsOnlyOnReplay"), Settings.SensorsOnlyOnReplay);
	ReadString(ConfigFile, TEXT("Resolution"), Settings.Resolution);
	ReadInt(ConfigFile, TEXT("RenderAll"), Settings.RenderAll);
	ReadInt(ConfigFile, TEXT("TTLsBeforeExperimentStart"), Settings.TTLsBeforeExperimentStart);
	ReadBool(ConfigFile, TEXT("bSemanticSegmentationEnabled"), Settings.bSemanticSegmentationEnabled);
}

// ---------------------------------------------------------------------------
// UMRISettings - accessors
// ---------------------------------------------------------------------------

void UMRISettings::SetExperimentType(EExperimentType InExperimentType)
{
	ExperimentType = InExperimentType;
}

EExperimentType UMRISettings::GetExperimentType() const
{
	return ExperimentType;
}

// ---------------------------------------------------------------------------
// UMRISettings - resolution helper
// ---------------------------------------------------------------------------

void UMRISettings::SetResolution(int Width, int Height, bool Fullscreen)
{
	Resolution = FString::Printf(TEXT("%dx%d%s"), Width, Height, Fullscreen ? TEXT("f") : TEXT("w"));
}

// ---------------------------------------------------------------------------
// UMRISettings - loading
// ---------------------------------------------------------------------------

void UMRISettings::LoadSettings()
{
	// First, try the default project config directory.
	FString DefaultFilePath = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("MRISettings.ini"));
	if (FPaths::FileExists(DefaultFilePath))
	{
		LoadSettingsFromFile(DefaultFilePath, false);
	}

	// Then allow a command-line override that takes precedence.
	FString CommandLineFilePath;
	if (FParse::Value(FCommandLine::Get(), TEXT("-mri-settings="), CommandLineFilePath))
	{
		if (FPaths::FileExists(CommandLineFilePath))
		{
			LoadSettingsFromFile(CommandLineFilePath);
		}
		else
		{
			UE_LOG(LogMRI, Warning, TEXT("Command-line MRI settings file not found: %s"), *CommandLineFilePath);
		}
	}
}

void UMRISettings::LoadSettingsFromFile(const FString &FilePath, bool bLogOnFailure)
{
	if (!FPaths::FileExists(FilePath))
	{
		if (bLogOnFailure)
		{
			UE_LOG(LogMRI, Error, TEXT("MRI settings file not found: %s"), *FilePath);
		}
		return;
	}

	UE_LOG(LogMRI, Log, TEXT("Loading MRI settings from \"%s\"."), *FilePath);

	FConfigFile ConfigFile;
	ConfigFile.Read(FilePath);

	ApplyConfigToSettings(ConfigFile, *this);
	CurrentFileName = FilePath;
}

void UMRISettings::LoadSettingsFromString(const FString &INIStyleString)
{
	FConfigFile ConfigFile;
	ConfigFile.CombineFromBuffer(INIStyleString);

	ApplyConfigToSettings(ConfigFile, *this);
}

// ---------------------------------------------------------------------------
// UMRISettings - saving
// ---------------------------------------------------------------------------

static FString BoolToString(bool Value)
{
	return Value ? TEXT("true") : TEXT("false");
}

static FString BuildINIContent(const UMRISettings &Settings)
{
	FString Content;

	Content += FString::Printf(TEXT("[%s]\n"), SectionName);
	Content += FString::Printf(TEXT("ExperimentType=%s\n"), *UExperimentType::ToString(Settings.GetExperimentType()));
	Content += FString::Printf(TEXT("Subject=%s\n"), *Settings.Subject);
	Content += FString::Printf(TEXT("AutoTriggerDemoRecording=%s\n"), *BoolToString(Settings.AutoTriggerDemoRecording));
	Content += FString::Printf(TEXT("AutoEyetrackingCalibration=%s\n"), *BoolToString(Settings.AutoEyetrackingCalibration));
	Content += FString::Printf(TEXT("SecondsToDemoStop=%f\n"), Settings.SecondsToDemoStop);
	Content += FString::Printf(TEXT("SensorsOnlyOnReplay=%s\n"), *BoolToString(Settings.SensorsOnlyOnReplay));
	Content += FString::Printf(TEXT("Resolution=%s\n"), *Settings.Resolution);
	Content += FString::Printf(TEXT("RenderAll=%d\n"), Settings.RenderAll);
	Content += FString::Printf(TEXT("TTLsBeforeExperimentStart=%d\n"), Settings.TTLsBeforeExperimentStart);
	Content += FString::Printf(TEXT("bSemanticSegmentationEnabled=%s\n"), *BoolToString(Settings.bSemanticSegmentationEnabled));

	return Content;
}

bool UMRISettings::SaveSettings()
{
	if (CurrentFileName.IsEmpty())
	{
		CurrentFileName = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("MRISettings.ini"));
	}
	return SaveSettingsToFile(CurrentFileName);
}

bool UMRISettings::SaveSettingsToFile(const FString &FilePath)
{
	FString Content = BuildINIContent(*this);

	if (!FFileHelper::SaveStringToFile(Content, *FilePath))
	{
		UE_LOG(LogMRI, Error, TEXT("Failed to save MRI settings to \"%s\"."), *FilePath);
		return false;
	}

	UE_LOG(LogMRI, Log, TEXT("MRI settings saved to \"%s\"."), *FilePath);
	CurrentFileName = FilePath;
	return true;
}

// ---------------------------------------------------------------------------
// UMRISettings - logging
// ---------------------------------------------------------------------------

void UMRISettings::LogSettings() const
{
	UE_LOG(LogMRI, Log, TEXT("== MRI Settings =="));
	UE_LOG(LogMRI, Log, TEXT("ExperimentType            = %s"), *UExperimentType::ToString(ExperimentType));
	UE_LOG(LogMRI, Log, TEXT("Subject                   = %s"), *Subject);
	UE_LOG(LogMRI, Log, TEXT("AutoTriggerDemoRecording  = %s"), AutoTriggerDemoRecording ? TEXT("true") : TEXT("false"));
	UE_LOG(LogMRI, Log, TEXT("AutoEyetrackingCalibration = %s"), AutoEyetrackingCalibration ? TEXT("true") : TEXT("false"));
	UE_LOG(LogMRI, Log, TEXT("SecondsToDemoStop         = %f"), SecondsToDemoStop);
	UE_LOG(LogMRI, Log, TEXT("SensorsOnlyOnReplay       = %s"), SensorsOnlyOnReplay ? TEXT("true") : TEXT("false"));
	UE_LOG(LogMRI, Log, TEXT("Resolution                = %s"), *Resolution);
	UE_LOG(LogMRI, Log, TEXT("RenderAll                 = %d"), RenderAll);
	UE_LOG(LogMRI, Log, TEXT("TTLsBeforeExperimentStart = %d"), TTLsBeforeExperimentStart);
	UE_LOG(LogMRI, Log, TEXT("bSemanticSegmentationEnabled = %s"), bSemanticSegmentationEnabled ? TEXT("true") : TEXT("false"));
}
