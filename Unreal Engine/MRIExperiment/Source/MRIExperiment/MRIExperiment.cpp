// MRI Experiment Plugin

#include "MRIExperiment.h"

DEFINE_LOG_CATEGORY(LogMRI);

IMPLEMENT_MODULE(FMRIExperimentModule, MRIExperiment)

void FMRIExperimentModule::StartupModule()
{
	UE_LOG(LogMRI, Log, TEXT("MRIExperiment module loaded"));
}

void FMRIExperimentModule::ShutdownModule()
{
	UE_LOG(LogMRI, Log, TEXT("MRIExperiment module unloaded"));
}
